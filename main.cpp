#include <ostream>
#include <random>
#include <vector>
#include <iostream>

#include <thrill/api/dia.hpp>
#include <thrill/api/union.hpp>
#include <thrill/api/equal_to_dia.hpp>
#include <thrill/api/concat_to_dia.hpp>
#include <thrill/api/collapse.hpp>
#include <thrill/api/size.hpp>
#include <thrill/api/reduce_by_key.hpp>
#include <thrill/api/group_by_key.hpp>
#include <thrill/api/rebalance.hpp>
#include <thrill/api/inner_join.hpp>
#include <thrill/api/print.hpp>

#include <thrill/common/stats_timer.hpp>

#include "connect4.hpp"
#include "distinct.hpp"

using thrill::DIA;

constexpr int width = 6;
constexpr int height = 4;
constexpr int size = width * height;

typedef char GameResult;
constexpr GameResult UNDEFINED = 0;
constexpr GameResult RED = 1;
constexpr GameResult YELLOW = 2;
constexpr GameResult TIE = 3;

typedef Connect4<width, height> C4;
typedef C4::Minified C4M;
typedef std::pair<C4M, GameResult> Game;
typedef std::pair<std::string, double> TimeInfo;

template <typename ValueType, typename Stack>
auto Force(DIA<ValueType, Stack> &input)
{
    auto id = input.Map([&](ValueType x) { return x; });
    id.Size();
    return id;
}

std::string displayGameResult(GameResult &res)
{
    if (res == RED)
    {
        return "Red";
    }
    else if (res == YELLOW)
    {
        return "Yellow";
    }
    else if (res == TIE)
    {
        return "Tie";
    }
    else
    {
        return "WHAT!?";
    }
}

GameResult getPlayer(int turn)
{
    return (turn % 2 == 0) ? RED : YELLOW;
}

template <typename Stack>
std::vector<DIA<Game, Stack>> propagateResults(const std::vector<DIA<Game, Stack>> &possibilityTree)
{
    std::vector<DIA<Game, Stack>> ret{possibilityTree};
    for (size_t i = ret.size() - 1; i > 0; i--)
    {
        auto backwardsMapping = ret[i - 1].template FlatMap<std::pair<C4M, Game>>([](const Game &game, auto emit) {
            if (game.second == UNDEFINED)
            {
                C4 c4 = game.first.toConnect4();
                std::vector<C4> nextGames = c4.nextFields();
                for (C4 nextGame : nextGames)
                {
                    emit(std::pair<C4M, Game>{nextGame.minified(), game});
                }
            }
        });
        auto establishedResults = ret[i - 1].Filter([](const Game &game) {
            return game.second != UNDEFINED;
        });
        auto joined = thrill::api::InnerJoin(
            thrill::api::LocationDetectionTag,
            ret[i],
            backwardsMapping,
            [](const Game &game) -> C4M {
                return game.first;
            },
            [](const std::pair<C4M, Game> &value) -> C4M {
                return value.first;
            },
            [&](const Game &game, const std::pair<C4M, Game> &prevGame) -> std::pair<C4M, std::pair<GameResult, GameResult>> {
                const C4M parentField = prevGame.second.first;
                return std::pair<C4M, std::pair<GameResult, GameResult>>{parentField, std::pair<GameResult, GameResult>{getPlayer(parentField.toConnect4().move), game.second}};
            },
            [](const C4M &mini) -> size_t { return mini.hash(); });
        // auto reduced = joined.ReduceByKey(
        //                          [](const std::pair<C4M, std::pair<GameResult, GameResult>> &game) { return game.first; },
        //                          [](const std::pair<C4M, std::pair<GameResult, GameResult>> &a, const std::pair<C4M, std::pair<GameResult, GameResult>> &b) -> std::pair<C4M, std::pair<GameResult, GameResult>> {
        //                              C4M game = a.first;
        //                              const GameResult me = a.second.first;
        //                              const GameResult aRes = a.second.second;
        //                              const GameResult bRes = b.second.second;
        //                              GameResult res = aRes;
        //                              if (aRes == me || bRes == me)
        //                                  res = me;
        //                              else if (aRes == UNDEFINED || bRes == UNDEFINED)
        //                                  res = UNDEFINED;
        //                              else if (aRes == TIE || bRes == TIE)
        //                                  res = TIE;
        //                              return std::pair<C4M, std::pair<GameResult, GameResult>>{game, std::pair<GameResult, GameResult>{me, res}};
        //                          },
        //                          thrill::api::DefaultReduceConfig(),
        //                          [](const C4M &mini) -> size_t { return mini.hash(); })
        //                    .Map([](const std::pair<C4M, std::pair<GameResult, GameResult>> &intermediateResult) -> Game {
        //                        return Game{intermediateResult.first, intermediateResult.second.second};
        //                    });
        auto reduced = joined.template GroupByKey<Game>(
            thrill::api::NoLocationDetectionTag, // Don't need Location detection, because the Join already partitioned the DIA
            [](const std::pair<C4M, std::pair<GameResult, GameResult>> &game) -> C4M { return game.first; },
            [](auto &iter, const C4M key) -> Game {
                const auto init = iter.Next();
                const GameResult me = init.second.first;
                GameResult res = init.second.second;
                std::vector<GameResult> results;
                while (iter.HasNext())
                {
                    results.push_back(iter.Next().second.second);
                }
                if (res != me)
                {
                    for (GameResult gRes : results)
                    {
                        if (gRes == me)
                        {
                            res = me;
                            break;
                        }
                        else if (gRes == UNDEFINED || res == UNDEFINED)
                            res = UNDEFINED;
                        else if (gRes == TIE || res == TIE)
                            res = TIE;
                    }
                }
                return Game(key, res);
            },
            [](const C4M &mini) -> size_t { return mini.hash(); });
        ret[i - 1] = thrill::api::Union(reduced, establishedResults).Rebalance().Collapse();
    }
    return ret;
}

template <typename Stack>
auto analyzeLayer(const DIA<Game, Stack> &inputLayer)
{
    return inputLayer.Map([](const Game &g) -> Game {
        if (g.second == UNDEFINED)
        {
            C4M field = g.first;
            C4 c4 = field.toConnect4();
            if (c4.hasWon())
            {
                return Game{field, getPlayer(c4.move + 1)};
            }
            else if (c4.move == size)
            {
                return Game{field, TIE};
            }
        }
        return g;
    });
}

template <typename Stack>
auto nextLayer(const DIA<C4, Stack> &inputLayer)
{
    auto nextProtoLayer = inputLayer.template FlatMap<C4M>(
        [](const C4 &c4, auto emit) {
            std::vector<C4> results = c4.nextFields();
            for (size_t j = 0; j < results.size(); j++)
                emit(results[j].aligned().minified());
        });
    return Distinct(
        nextProtoLayer,
        [](const C4M &c4) { return c4; },
        [](const C4M &c4) { return c4.hash(); });
}

template <typename Stack>
auto nextLayer(const DIA<C4M, Stack> &inputLayer)
{
    return nextLayer(inputLayer.Map([](const C4M &mini) -> C4 { return mini.toConnect4(); }));
}

template <typename Stack>
auto nextLayer(const DIA<Game, Stack> &inputLayer)
{
    return nextLayer(inputLayer.Map([](const Game &game) -> C4M { return game.first; })).Map([](const C4M &mini) -> Game { return Game{mini, UNDEFINED}; });
}

void Process(thrill::Context &ctx, bool partial_results)
{
    std::vector<Game> init{};
    init.push_back(Game{C4::init().minified(), UNDEFINED});
    auto initDIA = thrill::api::EqualToDIA(ctx, init).Collapse();
    std::vector<decltype(initDIA)> possibilityTree;
    possibilityTree.push_back(initDIA);

    thrill::common::StatsTimer time_forwardpass(true);
    thrill::common::StatsTimer time_total(true);

    std::vector<double> fpLayerTimes;
    std::vector<double> bpLayerTimes;

    for (size_t i = 0; i < size; i++)
    {
        thrill::common::StatsTimer layerTime(true);
        auto newLayer = nextLayer(possibilityTree.back()).Rebalance().Execute();
        auto analyzedLayer = analyzeLayer(newLayer);
        possibilityTree.push_back(analyzedLayer.Collapse());
        fpLayerTimes.push_back(layerTime.SecondsDouble());
        // auto s = analyzedLayer.Size();
        // if (ctx.my_rank() == 1)
        //     std::cout << "turn " << (i + 1) << "; size " << s << std::endl;
    }
    if (partial_results)
    {
        for (size_t i = 0; i < size; i++)
        {
            possibilityTree[i] = Force(possibilityTree[i]).Collapse();
        }
    }
    time_forwardpass.Stop();
    thrill::common::StatsTimer time_backpass(true);

    // auto tmp = possibilityTree;
    // tmp.clear();
    // for(size_t i = 0; i < possibilityTree.size(); i++) {
    //     tmp.push_back(possibilityTree[i]);
    // }
    auto analyzed_possibility_tree = propagateResults(possibilityTree);
    auto game_output = analyzed_possibility_tree[0]
                           .Map([](const Game &game) -> std::string { Game g = game; return std::string() + g.first.toConnect4().toString() + "\n" + displayGameResult(g.second); });
    for(int i = 0; i < analyzed_possibility_tree.size(); i++) {bpLayerTimes.push_back(-1);}
    for (int i = analyzed_possibility_tree.size() - 1; i >= 0; i--)
    {
        thrill::common::StatsTimer layerTime(true);
        analyzed_possibility_tree[i].Execute();
        bpLayerTimes[i] = layerTime.SecondsDouble();
    }

    game_output.Print("Result");

    time_backpass.Stop();
    time_total.Stop();

    std::vector<TimeInfo> local_time_data{
        TimeInfo("fp", time_forwardpass.SecondsDouble()),
        TimeInfo("bp", time_backpass.SecondsDouble()),
        TimeInfo("total", time_total.SecondsDouble())};

    for (size_t i = 0; i < fpLayerTimes.size(); i++)
    {
        local_time_data.push_back(TimeInfo(std::string() + "fp layer " + (i < 10 ? "0" : "") + std::to_string(i), fpLayerTimes[i]));
        local_time_data.push_back(TimeInfo(std::string() + "bp layer " + (i < 10 ? "0" : "") + std::to_string(i), bpLayerTimes[i]));
    }

    thrill::api::ConcatToDIA(ctx, local_time_data)
        .GroupByKey<std::string>(
            [](const TimeInfo &info) -> std::string { return info.first; },
            [](auto &iter, std::string key) -> std::string {
                double sum = 0;
                double count = 0;
                double max = 0;
                while (iter.HasNext())
                {
                    double time = iter.Next().second;
                    sum += time;
                    if (time > max)
                    {
                        max = time;
                    }
                    count++;
                }
                double average = sum / count;
                return "Step [" + key + "] - Average Time: " + std::to_string(average) + "s - Maximum Time: " + std::to_string(max) + "s";
            })
        .Sort()
        .Print("Times");
}
int main(int argc, char **)
{
    return thrill::Run(
        [&](thrill::Context &ctx) { Process(ctx, argc > 1); });
}
