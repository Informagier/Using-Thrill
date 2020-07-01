#ifndef CONNECT_4
#define CONNECT_4

#include <array>
#include <vector>

typedef unsigned long long bitboard;

template <int WIDTH, int HEIGHT>
class Connect4
{
public:
    static constexpr int H1 = HEIGHT + 1;
    static constexpr int H2 = HEIGHT + 2;
    static constexpr int SIZE = WIDTH * HEIGHT;
    static constexpr int SIZE1 = WIDTH * H1;
    static constexpr bitboard ALL1 = (1ULL << SIZE1) - 1;
    static constexpr bitboard COL1 = (1ULL << H1) - 1;
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

    static bool isLegal(bitboard newBoard) { return (newBoard & TOP) == 0; }

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
        for(int i = 0; i < WIDTH; i++){
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

    bool operator==(const Connect4 &that) const
    {
        return curr == that.curr && other == that.other;
    }
};

namespace std
{
    template<int WIDTH, int HEIGHT>
    struct hash<Connect4<WIDTH, HEIGHT>>
    {
        size_t operator()(const Connect4<WIDTH, HEIGHT> &c4) const
        {
            // Compute individual hash values for two data members and combine them using XOR and bit shifting
            return hash<bitboard>()(c4.curr) ^ hash<bitboard>()(c4.other);
        }
    };
} // namespace std

#endif