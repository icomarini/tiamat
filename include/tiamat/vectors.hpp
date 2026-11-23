#pragma once

#include "tiamat/Matrix.hpp"

#include <cmath>

namespace tmt {

template<Size size>
    requires(size > 0)
consteval auto generateColumnVectorSparsity() {
    Sparsity<size, 1> sparsity;
    template for (constexpr auto row : std::views::indices(size)) {
        sparsity[row][0] = Entry::any;
    }
    return sparsity;
}

template<Size size>
    requires(size > 0)
constexpr auto columnVectorSparsity = generateColumnVectorSparsity<size>();

template<IsArithmetic Value, Size s>
    requires(s > 0)
class ColumnVector : public Matrix<Value, columnVectorSparsity<s>> {
public:
    static constexpr auto size = s;
    using Matrix<Value, columnVectorSparsity<size>>::Matrix;
};

template<IsArithmetic Value, IsSparsity auto sparsity>
    requires(sparsity == columnVectorSparsity<sparsity.rows>)
struct Dispatch<Value, sparsity> {
    using Type = ColumnVector<Value, sparsity.rows>;
};

template<Size size>
consteval auto generateRowVectorSparsity() {
    Sparsity<1, size> sparsity;
    template for (constexpr auto column : std::views::indices(size)) {
        sparsity[0][column] = Entry::any;
    }
    return sparsity;
}

template<Size size>
    requires(size > 0)
constexpr auto rowVectorSparsity = generateRowVectorSparsity<size>();

template<typename Value, Size s>
class RowVector : public Matrix<Value, rowVectorSparsity<s>> {
public:
    static constexpr auto size = s;
    using Matrix<Value, rowVectorSparsity<size>>::Matrix;
};

template<IsArithmetic Value, IsSparsity auto sparsity>
    requires(sparsity == rowVectorSparsity<sparsity.columns>)
struct Dispatch<Value, sparsity> {
    using Type = RowVector<Value, sparsity.columns>;
};

template<typename T>
concept IsVector = IsMatrix<T> && (T::rows == 1 || T::columns == 1);

template<IsSparsity auto sparsity, const Index index>
    requires(sparsity.rows == 1 || sparsity.columns == 1)
consteval bool valid() {
    return sparsity.rows == 1 ? index < sparsity.columns : index < sparsity.rows;
}

template<Index idx, IsVector Vec>
    requires(valid<Vec::sparsity, idx>())
constexpr auto& get(const Vec& V) {
    constexpr auto row    = Vec::rows == 1 ? 0 : idx;
    constexpr auto column = Vec::columns == 1 ? 0 : idx;
    return get<row, column>(V);
}

template<Index idx, IsVector Vec>
    requires(valid<Vec::sparsity, idx>() /*&& Vec::sparsity[row][column] == Entry::any*/)
constexpr auto& get(Vec& v) {
    constexpr auto row    = Vec::rows == 1 ? 0 : idx;
    constexpr auto column = Vec::columns == 1 ? 0 : idx;
    return get<row, column>(v);
}

template<IsVector Vec>
constexpr auto norm(const Vec& vec) {
    typename Vec::Value norm { 0 };
    template for (constexpr auto index : std::views::indices(vec.size)) {
        norm += get<index>(vec) * get<index>(vec);
    }
    return std::sqrt(norm);
}

template<IsVector Vec>
    requires(std::floating_point<typename Vec::Value>)
constexpr auto normalize(const Vec& vec) {
    const auto n = norm(vec);
    if (n > typename Vec::Value { 0 }) {
        Vec result;
        template for (constexpr auto index : std::views::indices(vec.size)) {
            get<index>(result) = get<index>(vec) / n;
        }
        return result;
    };
    return Vec {};
}

}  // namespace tmt