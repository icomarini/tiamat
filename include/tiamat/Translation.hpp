#pragma once

#include "tiamat/Matrix.hpp"

namespace tmt {

template<Size dimension>
    requires(dimension > 0)
consteval auto translationSparsity() {
    Sparsity<dimension + 1, dimension + 1> sparsity;
    template for (constexpr auto row : std::views::indices(dimension + 1)) {
        template for (constexpr auto column : std::views::indices(dimension + 1)) {
            if constexpr (row == column) {
                sparsity[row][column] = Entry::pone;
            } else if constexpr (column == dimension) {
                sparsity[row][column] = Entry::any;
            } else {
                sparsity[row][column] = Entry::zero;
            }
        }
    }
    return sparsity;
}

template<typename Value, Size dimension>
class Translation : public Matrix<Value, translationSparsity<dimension>()> {
public:
    using Matrix<Value, translationSparsity<dimension>()>::Matrix;
};

// template<IsArithmetic Value>
// struct Dispatch<Value, translationSparsity<0>()> {
//     using Type = Translation<Value, 0>;
// };

template<IsArithmetic Value>
struct Dispatch<Value, translationSparsity<1>()> {
    using Type = Translation<Value, 1>;
};

template<IsArithmetic Value>
struct Dispatch<Value, translationSparsity<2>()> {
    using Type = Translation<Value, 2>;
};

template<IsArithmetic Value>
struct Dispatch<Value, translationSparsity<3>()> {
    using Type = Translation<Value, 3>;
};
}  // namespace tmt