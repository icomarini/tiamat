#pragma once

#include "tiamat/Matrix.hpp"
#include "tiamat/vectors.hpp"

namespace tmt {

// template<typename T = Float32>
// class View {
// public:
//     using value_type = T;

//     View(const Vector<3, T>& eye, const Vector<3, T>& center, const Vector<3, T>& up)
//         : f { -normalize(center - eye) }
//         , s { normalize(cross(-f, up)) }
//         , u { cross(s, -f) }
//         , e { -dot(s, eye), -dot(u, eye), dot(-f, eye) } {
//     }

//     // private:
//     Vector<3, T> f;
//     Vector<3, T> s;
//     Vector<3, T> u;
//     Vector<3, T> e;
// };

constexpr Sparsity viewSparsity {
    std::array { Entry::any,  Entry::any,  Entry::any,  Entry::any  },
    std::array { Entry::any,  Entry::any,  Entry::any,  Entry::any  },
    std::array { Entry::any,  Entry::any,  Entry::any,  Entry::any  },
    std::array { Entry::zero, Entry::zero, Entry::zero, Entry::pone },
};

template<std::floating_point Value>
class View : public Matrix<Value, viewSparsity> {
    template<IsVector Vec>
        requires(Vec::size == 3 && std::floating_point<typename Vec::Value>)
    View(const Vec& eye, const Vec& center, const Vec& up) {
        const auto f { -normalize(center - eye) };
        const auto s { normalize(cross(-f, up)) };
        const auto u { cross(s, -f) };
        const auto e { -dot(s, eye), -dot(u, eye), dot(-f, eye) };
    }
};
}  // namespace tmt
