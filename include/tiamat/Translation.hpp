#pragma once

#include "tiamat/Matrix.hpp"

namespace tiamat {

template<Size dimension>
consteval auto translationSparsity() {
    Sparsity<dimension + 1, dimension + 1> sparsity;
    template for (constexpr auto row : std::views::indices(dimension)) {
        template for (constexpr auto column : std::views::indices(dimension)) {
            if constexpr (row == column) {
                sparsity[row][column] = Entry::pone;
            } else if (column == dimension) {
                sparsity[row][column] = Entry::any;
            } else {
                sparsity[row][column] = Entry::zero;
            }
        }
    }
    return sparsity;
}

template<typename Value, Size dimension>
class Translation : public Matrix<Value, translationSparsity<dimension>()> { };

template<IsArithmetic Value>
struct Dispatch<Value, translationSparsity<2>()> {
    using Type = Translation<Value, 2>;
};
}  // namespace tiamat