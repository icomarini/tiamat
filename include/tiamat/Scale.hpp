#pragma once

#include "tiamat/Matrix.hpp"

namespace tmt {

template<Size dimension>
    requires(dimension > 0)
consteval auto generateScaleSparsity() {
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

template<Size dimension>
    requires(dimension > 0)
constexpr auto scaleSparsity = generateScaleSparsity<dimension>();

template<typename Value, Size dimension>
    requires(dimension > 0)
class Scale : public Matrix<Value, scaleSparsity<dimension>> {
public:
    using Matrix<Value, scaleSparsity<dimension>>::Matrix;
    constexpr Scale() {
        template for (constexpr auto i : std::views::indices(dimension)) {
            get<i, i>(*this) = one<Value>;
        }
    }
};

template<std::floating_point Value, Size dimension>
    requires(dimension > 0)
constexpr Scale<Value, dimension> inverse(const Scale<Value, dimension>& M) {
    Scale<Value, dimension> inv;
    template for (constexpr auto row : std::views::indices(dimension)) {
        get<row, row>(inv) = one<Value> / get<row, row>(M);
    }
    return inv;
}

template<IsArithmetic Value, IsSparsity auto sparsity>
    requires(sparsity == scaleSparsity<sparsity.rows - 1>)
struct Dispatch<Value, sparsity> {
    using Type = Scale<Value, sparsity.rows - 1>;
};

}  // namespace tmt