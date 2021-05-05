#ifndef CONNECT_4
#define CONNECT_4

#include <array>
#include <vector>
#include <utility>
#include <bitset>
#include <string>

using namespace std::rel_ops;

typedef unsigned int bitboard;
typedef int stone;
constexpr stone CURRENT_PLAYER = 1;
constexpr stone OTHER_PLAYER = 2;
constexpr stone NONE = 0;

template <int WIDTH_, int HEIGHT_>
class Connect4
{
public:
    static constexpr int HEIGHT = HEIGHT_;
    static constexpr int WIDTH = WIDTH_;
    static constexpr int H1 = HEIGHT + 1;
    static constexpr int H2 = HEIGHT + 2;
    static constexpr int SIZE = WIDTH * HEIGHT;
    static constexpr int SIZE1 = WIDTH * H1;
    static constexpr bitboard ALL1 = (1ULL << SIZE1) - 1ULL;
    static constexpr bitboard COL1 = (1ULL << H1) - 1ULL;
    static constexpr bitboard BOTTOM = ALL1 / COL1;
    static constexpr bitboard TOP = BOTTOM << HEIGHT;
    static std::array<int, WIDTH> HEIGHTS_INIT_ARRAY()
    {
        std::array<int, WIDTH> ret{};
        for (int i = 0; i < WIDTH; i++)
        {
            ret[i] = (i * H1);
        }
        return ret;
    }

    stone getStone(int, int);

    class Minified
    {
    public:
        bitboard curr;
        bitboard other;

    private:
        static std::array<int, WIDTH> getHeights(const Connect4 &partial)
        {
            std::array<int, WIDTH> ret = HEIGHTS_INIT_ARRAY();
            for (int i = 0; i < WIDTH; i++)
            {
                for (int j = HEIGHT - 1; j >= 0; j--)
                {
                    stone s = partial.getStone(i, j);
                    if (s != 0)
                    {
                        ret[i] += j + 1;
                        break;
                    }
                }
            }
            return ret;
        }

    public:
        Connect4 toConnect4() const
        {
            Connect4 ret;
            ret.curr = curr;
            ret.other = other;
            std::bitset<sizeof(bitboard)> currCount(curr);
            std::bitset<sizeof(bitboard)> otherCount(other);
            ret.move = currCount.count() + otherCount.count();
            ret.heights = getHeights(ret);
            return ret;
        };
        bool operator==(const Minified &that) const
        {
            return curr == that.curr && other == that.other;
        }
        bool operator!=(const Minified &that) const
        {
            return !(*this == that);
        }

        bool operator<(const Minified &that) const
        {
            if (curr < that.curr)
                return true;
            else if (curr == that.curr)
                return other < that.other;
            else
                return false;
        }

        size_t hash() const
        {
            return std::hash<size_t>()(std::hash<bitboard>()(curr) << 16) ^ (std::hash<bitboard>()(other) >> 16);
        }
    };

    static bool isLegal(bitboard newBoard) { return (newBoard & TOP) == 0; }

    static bitboard flipBoard(bitboard board)
    {
        bitboard ret = 0;
        for (int i = 0; i < WIDTH; i++)
        {
            int columnIndex = WIDTH - 1 - i;
            bitboard shift = columnIndex * H1;
            bitboard column = ((board & (COL1 << shift))) >> shift;
            ret |= column << (i * H1);
        }
        return ret;
    }

    bitboard curr;
    bitboard other;
    int move;
    std::array<int, WIDTH> heights;

    static Connect4 init()
    {
        Connect4 ret{};
        ret.curr = 0;
        ret.other = 0;
        ret.move = 0;
        ret.heights = HEIGHTS_INIT_ARRAY();
        return ret;
    }

    Minified minified() const
    {
        Minified ret;
        ret.curr = curr;
        ret.other = other;
        return ret;
    }

    std::string toString() const
    {
        std::string ret;
        std::string currString = (move % 2 == 0) ? " @" : " 0";
        std::string otherString = (move % 2 == 0) ? " 0" : " @";
        for (int i = 1; i <= WIDTH; i++)
        {
            ret += " " + std::to_string(i);
        }
        ret += "\n";
        for (int h = HEIGHT - 1; h >= 0; h--)
        {
            for (int w = 0; w < WIDTH; w++)
            {
                stone s = getStone(w, h);
                if (s == CURRENT_PLAYER)
                {
                    ret += currString;
                }
                else if (s == OTHER_PLAYER)
                {
                    ret += otherString;
                }
                else
                {
                    ret += " .";
                }
            }
            ret += "\n";
        }
        if (hasWon())
        {
            ret += otherString + " has won\n";
        }
        return ret;
    }

    std::array<int, WIDTH> flippedHeights() const
    {
        std::array<int, WIDTH> ret;
        for (int i = 0; i < WIDTH; i++)
        {
            int newIndex = WIDTH - i - 1;
            bitboard oldShift = i * H1;
            bitboard newShift = newIndex * H1;
            ret[newIndex] = (heights[i] - oldShift) + newShift;
        }
        return ret;
    }

    Connect4 flipped() const
    {
        Connect4 ret;
        ret.curr = flipBoard(curr);
        ret.other = flipBoard(other);
        ret.heights = flippedHeights();
        ret.move = move;
        return ret;
    }

    Connect4 aligned() const
    {
        Connect4 other = flipped();
        if (other < *this)
        {
            return other;
        }
        else
        {
            return *this;
        }
    }

    bool isPlayable(const int col) const { return isLegal(curr | 1L << heights[col]); }

    bool hasWon(const bitboard newBoard) const
    {
        bitboard tester = newBoard & (newBoard >> HEIGHT);
        if ((tester & (tester >> (2 * HEIGHT))) != 0)
            return true;
        tester = newBoard & (newBoard >> H1);
        if ((tester & (tester >> (2 * H1))) != 0)
            return true;
        tester = newBoard & (newBoard >> H1);
        if ((tester & (tester >> (2 * H2))) != 0)
            return true;
        tester = newBoard & (newBoard >> 1);
        return (tester & (tester >> 2)) != 0;
    }

    bool isLegalHasWon(const bitboard newBoard) const
    {
        return isLegal(newBoard) && hasWon(newBoard);
    }

    bool hasWon() const { return hasWon(other); }
    bool canMove(const int col) const { return isPlayable(col) && (!hasWon()); }

    Connect4 makeMove(const int col) const
    {
        int h = heights[col];
        std::array<int, WIDTH> nextHeights{};
        for (int i = 0; i < WIDTH; i++)
        {
            nextHeights[i] = heights[i];
        }
        nextHeights[col] += 1;
        bitboard nextOther = (curr ^ (1ULL << h));
        Connect4 nextField{};
        nextField.curr = other;
        nextField.other = nextOther;
        nextField.move = move + 1;
        nextField.heights = nextHeights;
        return nextField;
    }

    std::vector<Connect4> nextFields() const
    {
        std::vector<Connect4> ret{};
        for (int i = 0; i < WIDTH; i++)
        {
            if (canMove(i))
            {
                ret.push_back(makeMove(i));
            }
        }
        return ret;
    }

    stone getStone(int width, int height) const
    {
        bitboard mask = 1ull << (H1 * width + height);
        if ((curr & mask) != 0)
        {
            return CURRENT_PLAYER;
        }
        else if ((other & mask) != 0)
        {
            return OTHER_PLAYER;
        }
        else
        {
            return NONE;
        }
    }

    bool operator==(const Connect4 &that) const
    {
        return curr == that.curr && other == that.other;
    }

    bool operator<(const Connect4 &that) const
    {
        if (curr < that.curr)
            return true;
        else if (curr == that.curr)
            return other < that.other;
        else
            return false;
    }
};

namespace std
{
    template <int WIDTH, int HEIGHT>
    struct hash<Connect4<WIDTH, HEIGHT>>
    {
        size_t operator()(const Connect4<WIDTH, HEIGHT> &c4) const
        {
            // Compute individual hash values for two data members and combine them using XOR and bit shifting
            return std::hash<size_t>()(hash<bitboard>()(c4.curr) << 16) ^ (hash<bitboard>()(c4.other) >> 16);
        }
    };
} // namespace std

#endif