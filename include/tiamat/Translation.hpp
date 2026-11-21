#pragma once

#include "tiamat/Matrix.hpp"

namespace tmt {

template<Size dimension>
    requires(dimension > 0)
consteval auto generateTranslationSparsity() {
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

template<Size dimension>
    requires(dimension > 0)
constexpr auto translationSparsity = generateTranslationSparsity<dimension>();

template<typename Value, Size dimension>
    requires(dimension > 0)
class Translation : public Matrix<Value, translationSparsity<dimension>> {
public:
    using Matrix<Value, translationSparsity<dimension>>::Matrix;
};

template<typename Value, Size dimension>
    requires(dimension > 0)
constexpr Translation<Value, dimension> inverse(const Translation<Value, dimension>& M) {
    Translation<Value, dimension> inv;
    template for (constexpr auto row : std::views::indices(dimension)) {
        get<row, dimension>(inv) = -get<row, dimension>(M);
    }
    return inv;
}


template<IsArithmetic Value, IsSparsity auto sparsity>
    requires(sparsity == translationSparsity<sparsity.rows - 1>)
struct Dispatch<Value, sparsity> {
    using Type = Translation<Value, sparsity.rows - 1>;
};
}  // namespace tmt