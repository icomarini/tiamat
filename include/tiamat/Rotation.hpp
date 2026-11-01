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


    // Rotation(const Quaternion<Value>& quaternion)
    //     requires(dimension == 3)
    // {
    //     const auto  xx { get<0>(rotation) * get<0>(rotation) };
    //     const auto  yy { get<1>(rotation) * get<1>(rotation) };
    //     const auto  zz { get<2>(rotation) * get<2>(rotation) };
    //     const auto  xz { get<0>(rotation) * get<2>(rotation) };
    //     const auto  xy { get<0>(rotation) * get<1>(rotation) };
    //     const auto  yz { get<1>(rotation) * get<2>(rotation) };
    //     const auto  wx { get<3>(rotation) * get<0>(rotation) };
    //     const auto  wy { get<3>(rotation) * get<1>(rotation) };
    //     const auto  wz { get<3>(rotation) * get<2>(rotation) };
    //     constexpr T zero { 0 };
    //     constexpr T half { 0.5 };
    //     constexpr T two { 2 };
    //     return two * Matrix<3, 3, T> {
    //         half - yy - zz, zero + xy - wz, zero + xz + wy,  //
    //         zero + xy + wz, half - xx - zz, zero + yz - wx,  //
    //         zero + xz - wy, zero + yz + wx, half - xx - yy,  //
    //     };
    // }
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