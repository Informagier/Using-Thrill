
#include "connect4.hpp"

#include <thrill/api/dia.hpp>
#include <thrill/api/distribute.hpp>
#include <thrill/api/collapse.hpp>
#include <thrill/api/size.hpp>
#include <thrill/api/reduce_by_key.hpp>
#include <thrill/api/rebalance.hpp>

#include <ostream>
#include <random>
#include <vector>
#include <iostream>

using thrill::DIA;

constexpr int width = 4;
constexpr int height = 4;
constexpr int size = width * height;

typedef Connect4<width, height> C4;

void Process(thrill::Context &ctx)
{
    std::vector<C4> init{};
    init.push_back(C4::init());
    auto initDIA = thrill::api::Distribute(ctx, init);
    auto res = initDIA.Collapse();
    for (int i = 0; i < size; i++)
    {
        res = res.FlatMap<C4>(
                     [](const C4 c4, auto emit) -> void {
                         std::vector<C4> results = c4.nextFields();
                         for (int j = 0; j < results.size(); j++)
                             emit(results[j]);
                     })
                 .Collapse();
        auto s = res.Size();
        if (ctx.my_rank() == 1)
            std::cout << "turn " << (i + 1) << "; size " << s << std::endl;
    }
    std::cout << res.Size() << std::endl;
}
int main()
{
    return thrill::Run(
        [&](thrill::Context &ctx) { Process(ctx); });
}
