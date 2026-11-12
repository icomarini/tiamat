#pragma once

#include "tiamat/Matrix.hpp"

namespace tmt {

template<Size dimension>
    requires(dimension > 0)
consteval auto scaleSparsity() {
    Sparsity<dimension + 1, dimension + 1> sparsity;
    template for (constexpr auto row : std::views::indices(dimension + 1)) {
        template for (constexpr auto column : std::views::indices(dimension + 1)) {
            if constexpr (row == column) {
                sparsity[row][column] = row != dimension ? Entry::any : Entry::pone;
            } else {
                sparsity[row][column] = Entry::zero;
            }
        }
    }
    return sparsity;
}

template<typename Value, Size dimension>
class Scale : public Matrix<Value, scaleSparsity<dimension>()> {
public:
    using Matrix<Value, scaleSparsity<dimension>()>::Matrix;
    constexpr Scale() {
        template for (constexpr auto i : std::views::indices(dimension)) {
            get<i, i>(*this) = one<Value>;
        }
    }
};

// template<IsArithmetic Value>
// struct Dispatch<Value, translationSparsity<0>()> {
//     using Type = Translation<Value, 0>;
// };

template<IsArithmetic Value>
struct Dispatch<Value, scaleSparsity<1>()> {
    using Type = Scale<Value, 1>;
};

template<IsArithmetic Value>
struct Dispatch<Value, scaleSparsity<2>()> {
    using Type = Scale<Value, 2>;
};

template<IsArithmetic Value>
struct Dispatch<Value, scaleSparsity<3>()> {
    using Type = Scale<Value, 3>;
};
}  // namespace tmt