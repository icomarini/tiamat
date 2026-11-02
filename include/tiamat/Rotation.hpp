#pragma once

#include "tiamat/Matrix.hpp"
#include "tiamat/Quaternion.hpp"

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

template<std::floating_point Value, Size dimension>
class Rotation : public Matrix<Value, rotationSparsity<dimension>()> {
public:
    using Matrix<Value, rotationSparsity<dimension>()>::Matrix;

    Rotation(const Quaternion<Value>& quaternion)
        requires(dimension == 3)
    {
        const auto      xx { quaternion.x * quaternion.x };
        const auto      yy { quaternion.y * quaternion.y };
        const auto      zz { quaternion.z * quaternion.z };
        const auto      xz { quaternion.x * quaternion.z };
        const auto      xy { quaternion.x * quaternion.y };
        const auto      yz { quaternion.y * quaternion.z };
        const auto      wx { quaternion.w * quaternion.x };
        const auto      wy { quaternion.w * quaternion.y };
        const auto      wz { quaternion.w * quaternion.z };
        constexpr Value zero { 0 };
        constexpr Value half { 0.5 };
        constexpr Value two { 2 };
        get<0, 0>(*this) = two * (half - yy - zz);
        get<0, 1>(*this) = two * (zero + xy - wz);
        get<0, 2>(*this) = two * (zero + xz + wy);

        get<1, 0>(*this) = two * (zero + xy + wz);
        get<1, 1>(*this) = two * (half - xx - zz);
        get<1, 2>(*this) = two * (zero + yz - wx);

        get<2, 0>(*this) = two * (zero + xz - wy);
        get<2, 1>(*this) = two * (zero + yz + wx);
        get<2, 2>(*this) = two * (half - xx - yy);
        // return two * Matrix<3, 3, T> {
        //     half - yy - zz, zero + xy - wz, zero + xz + wy,  //
        //     zero + xy + wz, half - xx - zz, zero + yz - wx,  //
        //     zero + xz - wy, zero + yz + wx, half - xx - yy,  //
        // };
    }
};

// template<IsArithmetic Value>
// struct Dispatch<Value, rotationSparsity<1>()> {
//     using Type = Rotation<Value, 1>;
// };

template<IsArithmetic Value>
struct Dispatch<Value, rotationSparsity<2>()> {
    using Type = Rotation<Value, 2>;
};

template<IsArithmetic Value>
struct Dispatch<Value, rotationSparsity<3>()> {
    using Type = Rotation<Value, 3>;
};
}  // namespace tiamat