#pragma once

#include "tiamat/vectors.hpp"

#include <concepts>
#include <numbers>

namespace tiamat {

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

    constexpr Quaternion(const std::floating_point auto radians, const IsVector auto& axis) {
        const auto n    = normalize(axis);
        const auto a    = radians * 0.5;
        const auto sina = std::sin(a);
        x               = get<0>(n) * sina;
        y               = get<1>(n) * sina;
        z               = get<2>(n) * sina;
        w               = std::cos(a);
    }

    Value x;
    Value y;
    Value z;
    Value w;
};
}  // namespace tiamat