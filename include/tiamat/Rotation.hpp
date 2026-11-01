#pragma once

#include "tiamat/Matrix.hpp"

namespace tiamat {

template<Size dimension>
    requires(dimension > 0)
consteval auto rotationSparsity() {
    Sparsity<dimension + 1, dimension + 1> sparsity;
    template for (constexpr auto row : std::views::indices(dimension + 1)) {
        template for (constexpr auto column : std::views::indices(dimension + 1)) {
            if constexpr (row == dimension && column == dimension) {
                sparsity[row][column] = Entry::pone;
            } else if constexpr (row == dimension || column == dimension) {
                sparsity[row][column] = Entry::zero;
            } else {
                sparsity[row][column] = Entry::any;
            }
        }
    }
    return sparsity;
}

template<typename Value, Size dimension>
class Rotation : public Matrix<Value, rotationSparsity<dimension>()> {
public:
    using Matrix<Value, rotationSparsity<dimension>()>::Matrix;
};

template<IsArithmetic Value>
struct Dispatch<Value, rotationSparsity<1>()> {
    using Type = Rotation<Value, 1>;
};

template<IsArithmetic Value>
struct Dispatch<Value, rotationSparsity<2>()> {
    using Type = Rotation<Value, 2>;
};

template<IsArithmetic Value>
struct Dispatch<Value, rotationSparsity<3>()> {
    using Type = Rotation<Value, 3>;
};
}  // namespace tiamat