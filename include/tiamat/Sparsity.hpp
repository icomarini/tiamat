#pragma once
#include <cstdint>
#include <array>
#include <ranges>
#include <utility>

namespace tmt {
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
        template for (constexpr auto column : std::views::indices(B.columns)) {
            template for (constexpr auto middle : std::views::indices(A.columns)) {
                C[row][column] = C[row][column] + A[row][middle] * B[middle][column];
            }
        }
    }
    return C;
}

constexpr auto transpose(const IsSparsity auto A) {
    Sparsity<A.columns, A.rows> B {};
    template for (constexpr auto row : std::views::indices(A.rows)) {
        template for (constexpr auto column : std::views::indices(A.columns)) {
            B[column][row] = A[row][column];
        }
    }
    return B;
}

constexpr auto inverse(const IsSparsity auto A) {
    Sparsity<A.rows, A.columns> B {};
    template for (constexpr auto row : std::views::indices(A.rows)) {
        template for (constexpr auto column : std::views::indices(A.columns)) {
            B[row][column] = Entry::any;
        }
    }
    return B;
}

}  // namespace tmt