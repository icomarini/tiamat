#pragma once
#include "tiamat/Matrix.hpp"

namespace tmt {

template<Size size>
    requires(size > 0)
consteval auto generateIdentitySparsity() {
    Sparsity<size, size> sparsity;
    template for (constexpr auto row : std::views::indices(size)) {
        template for (constexpr auto column : std::views::indices(size)) {
            sparsity[row][column] = row == column ? Entry::pone : Entry::zero;
        }
    }
    return sparsity;
}

template<Size size>
    requires(size > 0)
constexpr auto identitySparsity = generateIdentitySparsity<size>();

template<typename Value, Size size>
class Identity : public Matrix<Value, identitySparsity<size>> {
public:
    using Matrix<Value, identitySparsity<size>>::Matrix;
};

template<IsArithmetic Value, IsSparsity auto sparsity>
    requires(sparsity == identitySparsity<sparsity.rows>)
struct Dispatch<Value, sparsity> {
    using Type = Identity<Value, sparsity.rows>;
};

template<IsArithmetic Value, Size size>
constexpr Identity<Value, size> identity;
}  // namespace tmt