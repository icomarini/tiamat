#pragma once

#include "tiamat/Matrix.hpp"

namespace tmt {

// template<typename T>
// class Perspective {
// public:
//     using value_type = T;

//     constexpr Perspective(const T& fovy, const T& aspect, const T& zNear, const T& zFar)
//         : a11 { -1 / std::tan(fovy / 2) }
//         , a00 { -a11 / aspect }
//         , a22 { zFar / (zNear - zFar) }
//         , a23 { -(zFar * zNear) / (zFar - zNear) } {
//     }

//     T a11;
//     T a00;
//     T a22;
//     T a23;
// };

constexpr Sparsity perspectiveSparsity {
    std::array { Entry::any,  Entry::zero, Entry::zero, Entry::zero },
    std::array { Entry::zero, Entry::any,  Entry::zero, Entry::zero },
    std::array { Entry::zero, Entry::zero, Entry::any,  Entry::any  },
    std::array { Entry::zero, Entry::zero, Entry::mone, Entry::zero },
};

template<IsArithmetic Value>
class Perspective : public Matrix<Value, perspectiveSparsity> {
public:
    // using Matrix<Value, perspectiveSparsity>::Matrix;

    Perspective(const Value& fovy, const Value& aspect, const Value& zNear, const Value& zFar) {
        constexpr Value one { 1 };
        constexpr Value half = one / Value { 2 };
        get<1, 1>(*this)     = -one / std::tan(half * fovy);
        get<0, 0>(*this)     = -get<1, 1>(*this) / aspect;
        get<2, 2>(*this)     = zFar / (zNear - zFar);
        get<2, 3>(*this)     = -(zFar * zNear) / (zFar - zNear);
    }
};

}  // namespace tmt