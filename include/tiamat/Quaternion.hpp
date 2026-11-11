#pragma once

#include "tiamat/vectors.hpp"

#include <concepts>
#include <numbers>

namespace tmt {

template<std::floating_point Value>
constexpr auto radians(const Value degree) {
    constexpr auto scale { std::numbers::pi_v<Value> / Value { 180 } };
    return degree * scale;
}

template<std::floating_point Value>
class Quaternion {
public:
    constexpr Quaternion(const Value x, const Value y, const Value z, const Value w)
        : x { x }
        , y { y }
        , z { z }
        , w { w } {
    }

    template<IsVector Vec>
        requires(Vec::size == 3)
    constexpr Quaternion(const std::floating_point auto radians, const Vec& axis) {
        // const auto n    = normalize(axis);
        const auto a    = radians * 0.5;
        const auto sina = std::sin(a);
        x               = get<0>(axis) * sina;
        y               = get<1>(axis) * sina;
        z               = get<2>(axis) * sina;
        w               = std::cos(a);
    }

    Value x;
    Value y;
    Value z;
    Value w;
};
}  // namespace tmt