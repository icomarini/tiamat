#pragma once
#include "tiamat/Sparsity.hpp"
namespace tiamat {
template<typename T>
static constexpr T zero = T { 0 };

template<typename T>
static constexpr T one = T { 1 };

template<typename T>
concept IsArithmetic = std::integral<T> || std::floating_point<T>;

// Non Matrix trait
template<typename T>
struct IsMatrixTrait : std::false_type { };

// Forward declaration
template<IsArithmetic V, IsSparsity auto s>
class Matrix;

// Matrix trait
template<IsArithmetic Value, IsSparsity auto sparsity>
struct IsMatrixTrait<Matrix<Value, sparsity>> : std::true_type { };

// Good ol' StackOverflow copypaste
template<template<IsArithmetic, IsSparsity auto> class T, class U>
struct isDerivedFrom {
private:
    template<IsArithmetic V, IsSparsity auto s>
    static decltype(static_cast<const T<V, s>&>(std::declval<U>()), std::true_type {}) test(const T<V, s>&);

    static std::false_type test(...);

public:
    static constexpr bool value = decltype(isDerivedFrom::test(std::declval<U>()))::value;
};

// Matrix concept
template<typename T>
concept IsMatrix = IsMatrixTrait<std::remove_cvref_t<T>>::value || isDerivedFrom<Matrix, T>::value;


template<IsArithmetic V, IsSparsity auto s>
class Matrix {
public:
    using Value                    = V;
    static constexpr auto sparsity = s;

    constexpr Matrix()
        : data {} {
    }

    template<typename... Values>
        requires((std::same_as<Value, Values> && ...) && sizeof...(Values) == count<sparsity, Entry::any>())
    constexpr Matrix(Values... values)
        : data { values... } {
    }

private:
    std::array<Value, count<sparsity, Entry::any>()> data;

    template<Index row, Index column, IsMatrix Mat>
        requires(valid<Mat::sparsity, row, column>())
    friend constexpr auto& get(const Mat& M);

    template<Index row, Index column, IsMatrix Mat>
        requires(valid<Mat::sparsity, row, column>() && Mat::sparsity[row][column] == Entry::any)
    friend constexpr auto& get(Mat& M);
};


template<Index row, Index column, IsMatrix Mat>
    requires(valid<Mat::sparsity, row, column>())
constexpr auto& get(const Mat& M) {
    using Value          = typename Mat::Value;
    constexpr auto entry = Mat::sparsity[row][column];
    if constexpr (entry == Entry::zero) {
        return zero<Value>;
    } else if constexpr (entry == Entry::pone) {
        return one<Value>;
    } else if constexpr (entry == Entry::mone) {
        return -one<Value>;
    } else {
        return M.data[index<Mat::sparsity, row, column>()];
    }
}

template<Index row, Index column, IsMatrix Mat>
    requires(valid<Mat::sparsity, row, column>() && Mat::sparsity[row][column] == Entry::any)
constexpr auto& get(Mat& M) {
    return M.data[index<Mat::sparsity, row, column>()];
}

template<IsArithmetic Value, IsSparsity auto sparsity>
struct Dispatch {
    using Type = Matrix<Value, sparsity>;
};

template<IsMatrix MatrixA, IsMatrix MatrixB>
    requires(MatrixA::sparsity.columns == MatrixB::sparsity.rows)
constexpr auto operator*(const MatrixA& A, const MatrixB& B) {
    using CommonValue = std::common_type_t<typename MatrixA::Value, typename MatrixB::Value>;
    using MatrixC     = typename Dispatch<CommonValue, MatrixA::sparsity * MatrixB::sparsity>::Type;
    MatrixC C;
    template for (constexpr auto row : std::views::indices(MatrixA::sparsity.rows)) {
        template for (constexpr auto column : std::views::indices(MatrixB::sparsity.columns)) {
            template for (constexpr auto middle : std::views::indices(MatrixA::sparsity.columns)) {
                if constexpr (MatrixC::sparsity[row][column] == Entry::any) {
                    constexpr auto entryA = MatrixA::sparsity[row][middle];
                    constexpr auto entryB = MatrixB::sparsity[middle][column];
                    auto&          valueC = get<row, column>(C);
                    if constexpr (entryA == Entry::any && entryB != Entry::any) {
                        const auto&    valueA = get<row, middle>(A);
                        constexpr auto valueB = get<middle, column>(B);
                        valueC += valueA * valueB;
                    } else if constexpr (entryA != Entry::any && entryB == Entry::any) {
                        constexpr auto valueA = get<row, middle>(A);
                        const auto&    valueB = get<middle, column>(B);
                        valueC += valueA * valueB;
                    } else if constexpr (entryA == Entry::any && entryB == Entry::any) {
                        const auto& valueA = get<row, middle>(A);
                        const auto& valueB = get<middle, column>(B);
                        valueC += valueA * valueB;
                    }
                }
            }
        }
    }
    return C;
}
// }  // namespace experimental

}  // namespace tiamat