#pragma once

#include "tiamat/Matrix.hpp"

#include <cmath>

namespace tiamat {

template<Size size>
consteval auto columnVectorSparsity() {
    Sparsity<size, 1> sparsity;
    template for (constexpr auto row : std::views::indices(size)) {
        sparsity[row][0] = Entry::any;
    }
    return sparsity;
}

template<typename Value, Size s>
class ColumnVector : public Matrix<Value, columnVectorSparsity<s>()> {
public:
    static constexpr auto size = s;
    using Matrix<Value, columnVectorSparsity<size>()>::Matrix;
};

// template<IsArithmetic Value>
// struct Dispatch<Value, translationSparsity<0>()> {
//     using Type = Translation<Value, 0>;
// };

template<IsArithmetic Value>
struct Dispatch<Value, columnVectorSparsity<1>()> {
    using Type = ColumnVector<Value, 1>;
};

template<IsArithmetic Value>
struct Dispatch<Value, columnVectorSparsity<2>()> {
    using Type = ColumnVector<Value, 2>;
};

template<IsArithmetic Value>
struct Dispatch<Value, columnVectorSparsity<3>()> {
    using Type = ColumnVector<Value, 3>;
};

template<IsArithmetic Value>
struct Dispatch<Value, columnVectorSparsity<4>()> {
    using Type = ColumnVector<Value, 4>;
};


template<Size size>
consteval auto rowVectorSparsity() {
    Sparsity<1, size> sparsity;
    template for (constexpr auto column : std::views::indices(size)) {
        sparsity[0][column] = Entry::any;
    }
    return sparsity;
}

template<typename Value, Size s>
class RowVector : public Matrix<Value, rowVectorSparsity<s>()> {
public:
    static constexpr auto size = s;
    using Matrix<Value, rowVectorSparsity<size>()>::Matrix;
};

// template<IsArithmetic Value>
// struct Dispatch<Value, translationSparsity<0>()> {
//     using Type = Translation<Value, 0>;
// };

// template<IsArithmetic Value>
// struct Dispatch<Value, rowVectorSparsity<1>()> {
//     using Type = RowVector<Value, 1>;
// };

template<IsArithmetic Value>
struct Dispatch<Value, rowVectorSparsity<2>()> {
    using Type = RowVector<Value, 2>;
};

template<IsArithmetic Value>
struct Dispatch<Value, rowVectorSparsity<3>()> {
    using Type = RowVector<Value, 3>;
};

template<IsArithmetic Value>
struct Dispatch<Value, rowVectorSparsity<4>()> {
    using Type = RowVector<Value, 4>;
};

template<typename T>
concept IsVector = IsMatrix<T> && (T::sparsity.rows == 1 || T::sparsity.columns == 1);

template<IsSparsity auto sparsity, const Index index>
    requires(sparsity.rows == 1 || sparsity.columns == 1)
consteval bool valid() {
    return sparsity.rows == 1 ? index < sparsity.columns : index < sparsity.rows;
}

template<Index idx, IsVector Vec>
    requires(valid<Vec::sparsity, idx>())
constexpr auto& get(const Vec& V) {
    constexpr auto row    = Vec::sparsity.rows == 1 ? 0 : idx;
    constexpr auto column = Vec::sparsity.columns == 1 ? 0 : idx;
    return get<row, column>(V);
}

template<Index idx, IsVector Vec>
    requires(valid<Vec::sparsity, idx>() /*&& Vec::sparsity[row][column] == Entry::any*/)
constexpr auto& get(Vec& v) {
    constexpr auto row    = Vec::sparsity.rows == 1 ? 0 : idx;
    constexpr auto column = Vec::sparsity.columns == 1 ? 0 : idx;
    return get<row, column>(v);
}

template<IsVector Vec>
constexpr auto normalize(const Vec& vec) {
    typename Vec::Value norm {};
    template for (constexpr auto index : std::views::indices(vec.size)) {
        norm += get<index>(vec) * get<index>(vec);
    }
    norm = std::sqrt(norm);
    Vec result;
    template for (constexpr auto index : std::views::indices(vec.size)) {
        get<index>(result) = get<index>(vec) / norm;
    }
    return result;
}

}  // namespace tiamat