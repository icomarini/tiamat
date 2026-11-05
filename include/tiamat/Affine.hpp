#pragma once

#include "tiamat/Matrix.hpp"

namespace tiamat {

template<Size dimension>
    requires(dimension > 0)
consteval auto affineSparsity() {
    Sparsity<dimension + 1, dimension + 1> sparsity;
    template for (constexpr auto row : std::views::indices(dimension + 1)) {
        template for (constexpr auto column : std::views::indices(dimension + 1)) {
            if constexpr (row == dimension) {
                sparsity[row][column] = column == dimension ? Entry::pone : Entry::zero;
            } else {
                sparsity[row][column] = Entry::any;
            }
        }
    }
    return sparsity;
}

template<typename Value, Size dimension>
class Affine : public Matrix<Value, affineSparsity<dimension>()> {
public:
    using Matrix<Value, affineSparsity<dimension>()>::Matrix;
};

// template<IsArithmetic Value>
// struct Dispatch<Value, translationSparsity<0>()> {
//     using Type = Translation<Value, 0>;
// };

template<IsArithmetic Value>
struct Dispatch<Value, affineSparsity<1>()> {
    using Type = Affine<Value, 1>;
};

template<IsArithmetic Value>
struct Dispatch<Value, affineSparsity<2>()> {
    using Type = Affine<Value, 2>;
};

template<IsArithmetic Value>
struct Dispatch<Value, affineSparsity<3>()> {
    using Type = Affine<Value, 3>;
};
}  // namespace tiamat