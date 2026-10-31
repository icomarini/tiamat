#pragma once
#include <cstdint>
#include <array>
#include <ranges>
#include <utility>

namespace tiamat {
enum class Entry {
    zero = 0,
    pone,
    mone,
    any,
};

constexpr Entry operator+(const Entry a, const Entry b) {
    static constexpr std::array<std::array<Entry, 4>, 4> table {
        //           Entry::zero  Entry::pone  Entry::mone  Entry::any
        std::array { Entry::zero, Entry::pone, Entry::mone, Entry::any }, // Entry::zero
        std::array { Entry::pone, Entry::any,  Entry::zero, Entry::any }, // Entry::pone
        std::array { Entry::mone, Entry::zero, Entry::any,  Entry::any }, // Entry::mone
        std::array { Entry::any,  Entry::any,  Entry::any,  Entry::any }, // Entry::any
    };
    return table[static_cast<uint8_t>(a)][static_cast<uint8_t>(b)];
}

constexpr Entry operator*(const Entry a, const Entry b) {
    static constexpr std::array<std::array<Entry, 4>, 4> table {
        //           Entry::zero  Entry::pone  Entry::mone  Entry::any
        std::array { Entry::zero, Entry::zero, Entry::zero, Entry::zero }, // Entry::zero
        std::array { Entry::zero, Entry::pone, Entry::mone, Entry::any  }, // Entry::pone
        std::array { Entry::zero, Entry::mone, Entry::pone, Entry::any  }, // Entry::mone
        std::array { Entry::zero, Entry::any,  Entry::any,  Entry::any  }, // Entry::any
    };
    return table[static_cast<uint8_t>(a)][static_cast<uint8_t>(b)];
}

// std::string toString(const Entry e) {
//     switch (e) {
//     case Entry::zero:
//         return "zero";
//     case Entry::pone:
//         return "+one";
//     case Entry::mone:
//         return "-one";
//     case Entry::any:
//         return "any!";
//     default:
//         std::unreachable();
//     }
// }

// using Layout = uint32_t;

// static constexpr Entry entry(const Layout layout, const int position)
// // requires(0 <= position && position < 16)
// {
//     const auto position1 = (layout >> (2 * position)) & 1;
//     const auto position2 = (layout >> (2 * position + 1)) & 1;
//     return position1 ? (position2 ? Entry::any : Entry::mone) : (position2 ? Entry::pone : Entry::zero);
// }

// template<uint8_t row, uint8_t col>
//     requires(row < 4 && col < 4)
// constexpr Entry getEntry(const Layout layout) {
//     return entry(layout, row * 4 + col);
// }


// template<uint8_t row, uint8_t col>
// constexpr Layout setEntry(Layout layout, const Entry entry) {
//     auto       setBit   = [&](int p) { layout |= (1U << p); };
//     auto       clearBit = [&](int p) { layout &= ~(1U << p); };
//     const auto position = row * 4 + col;
//     const auto left     = 2 * position;
//     const auto right    = 2 * position + 1;
//     switch (entry) {
//     case Entry::zero:
//         clearBit(left);
//         clearBit(right);
//         return layout;
//     case Entry::pone:
//         clearBit(left);
//         setBit(right);
//         return layout;
//     case Entry::mone:
//         setBit(left);
//         clearBit(right);
//         return layout;
//     case Entry::any:
//         setBit(left);
//         setBit(right);
//         return layout;
//     default:
//         throw("bad");
//     }
//     // return set(layout, row * 4 + col, entry);
// }

// consteval std::size_t count(const Layout layout, const Entry entry) {
//     std::size_t count {};
//     template for (constexpr auto row : std::views::iota(0, 4)) {
//         template for (constexpr auto col : std::views::iota(0, 4)) {
//             if (getEntry<row, col>(layout) == entry) {
//                 ++count;
//             }
//         }
//     }
//     return count;
// }

// template<uint8_t row, uint8_t col, Layout layout>
// consteval uint8_t index() {
//     int idx { -1 };

//     template for (constexpr auto n : std::views::iota(0, row * 4 + col + 1)) {
//         const auto e = entry(layout, n);
//         if (e == Entry::any) {
//             ++idx;
//         }
//     }
//     if (idx > -1) {
//         return idx;
//     }
//     std::unreachable();
// }


// constexpr Layout multiply(const Layout a, const Layout b) {
//     Layout c {};
//     template for (constexpr auto row : std::views::iota(0, 4)) {
//         template for (constexpr auto col : std::views::iota(0, 4)) {
//             template for (constexpr auto mid : std::views::iota(0, 4)) {
//                 const auto entryA = getEntry<row, mid>(a);
//                 const auto entryB = getEntry<mid, col>(b);
//                 const auto entryC = getEntry<row, col>(c);
//                 c                 = setEntry<row, col>(c, entryC + entryA * entryB);
//             }
//         }
//     }
//     return c;
// }

// constexpr Layout encodeLayout(const std::array<std::array<Entry, 4>, 4>& table) {
//     Layout layout {};
//     template for (constexpr auto row : std::views::iota(0, 4)) {
//         template for (constexpr auto col : std::views::iota(0, 4)) {
//             layout = setEntry<row, col>(layout, table[row][col]);
//         }
//     }
//     return layout;
// }

// ===================================================================================================

using Index = std::size_t;
using Size  = std::size_t;

// Sparsity definition
template<Size r, Size c>
    requires(r > 0 && c > 0)
struct Sparsity : std::array<std::array<Entry, c>, r> {
    static constexpr auto rows    = r;
    static constexpr auto columns = c;
};

// Deduction guide
template<Size columns, std::same_as<std::array<Entry, columns>>... Tail>
Sparsity(std::array<Entry, columns>, Tail...) -> Sparsity<1 + sizeof...(Tail), columns>;

// Non Sparsity trait
template<typename T>
struct IsSparsityTrait : std::false_type { };

// Sparsity trait
template<Size rows, Size columns>
struct IsSparsityTrait<Sparsity<rows, columns>> : std::true_type { };

// Sparsity concept
template<typename T>
concept IsSparsity = IsSparsityTrait<std::remove_cvref_t<T>>::value;


template<IsSparsity auto sparsity, const Index row, const Index column>
consteval bool valid() {
    return row < sparsity.rows && column < sparsity.columns;
}

template<IsSparsity auto sparsity, Entry entry>
consteval Size count() {
    Size count {};
    template for (constexpr auto row : std::views::indices(sparsity.rows)) {
        template for (constexpr auto col : std::views::indices(sparsity.columns)) {
            if (sparsity[row][col] == entry) {
                ++count;
            }
        }
    }
    return count;
}

template<IsSparsity auto sparsity, Index row, Index column>
    requires(valid<sparsity, row, column>() && count<sparsity, Entry::any>() > 0 && sparsity[row][column] == Entry::any)
consteval Index index() {
    Index index {};
    template for (constexpr auto i : std::views::indices(sparsity.rows)) {
        template for (constexpr auto j : std::views::indices(sparsity.columns)) {
            if constexpr (i == row && j == column) {
                return index;
            } else if (sparsity[i][j] == Entry::any) {
                ++index;
            }
        }
    }
}

constexpr auto operator*(const IsSparsity auto A, const IsSparsity auto B)
    requires(A.columns == B.rows)
{
    Sparsity<A.rows, B.columns> C {};
    template for (constexpr auto row : std::views::indices(A.rows)) {
        template for (constexpr auto col : std::views::indices(B.columns)) {
            template for (constexpr auto mid : std::views::indices(A.columns)) {
                C[row][col] = C[row][col] + A[row][mid] * B[mid][col];
            }
        }
    }
    return C;
}


// template<uint8_t row, uint8_t col, typename Value, Layout layout>
//     requires(getEntry<row, col>(layout) == Entry::any)
// constexpr auto& get(Matrix<Value, layout>& m) {
//     return m.data[index<row, col, layout>()];
// }

}  // namespace tiamat