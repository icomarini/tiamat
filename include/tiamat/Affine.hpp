#pragma once

#include "tiamat/Matrix.hpp"

namespace tmt {

template<Size dimension>
    requires(dimension > 0)
consteval auto generateAffineSparsity() {
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

template<Size dimension>
    requires(dimension > 0)
constexpr auto affineSparsity = generateAffineSparsity<dimension>();

template<typename Value, Size dimension>
class Affine : public Matrix<Value, affineSparsity<dimension>> {
public:
    using Matrix<Value, affineSparsity<dimension>>::Matrix;
    using Matrix<Value, affineSparsity<dimension>>::operator=;

    constexpr Affine() {
        template for (constexpr auto row : std::views::indices(dimension)) {
            template for (constexpr auto column : std::views::indices(dimension)) {
                if constexpr (row == column) {
                    get<row, column>(*this) = one<Value>;
                } else {
                    get<row, column>(*this) = zero<Value>;
                }
            }
        }
    }
};

template<IsArithmetic Value, IsSparsity auto sparsity>
    requires(sparsity == affineSparsity<sparsity.rows - 1>)
struct Dispatch<Value, sparsity> {
    using Type = Affine<Value, sparsity.rows - 1>;
};
}  // namespace tmt