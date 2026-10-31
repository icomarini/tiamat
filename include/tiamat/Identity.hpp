#pragma once
#include "tiamat/Matrix.hpp"

namespace tiamat {

template<Size size>
consteval auto identitySparsity() {
    Sparsity<size, size> sparsity;
    template for (constexpr auto row : std::views::indices(size)) {
        template for (constexpr auto column : std::views::indices(size)) {
            sparsity[row][column] = row == column ? Entry::pone : Entry::zero;
        }
    }
    return sparsity;
}

template<typename Value, Size size>
class Identity : public Matrix<Value, identitySparsity<size>()> { };

template<IsArithmetic Value>
struct Dispatch<Value, identitySparsity<1>()> {
    using Type = Identity<Value, 1>;
};

template<IsArithmetic Value>
struct Dispatch<Value, identitySparsity<2>()> {
    using Type = Identity<Value, 2>;
};

template<IsArithmetic Value>
struct Dispatch<Value, identitySparsity<3>()> {
    using Type = Identity<Value, 3>;
};
}  // namespace tiamat