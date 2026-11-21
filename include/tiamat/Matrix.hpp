#pragma once
#include "tiamat/Sparsity.hpp"

namespace tmt {
template<typename T>
static constexpr T zero = T { 0 };

template<typename T>
static constexpr T one = T { 1 };

template<typename T>
static constexpr T minusOne = -T { 1 };

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
    static constexpr auto rows     = sparsity.rows;
    static constexpr auto columns  = sparsity.columns;

    explicit constexpr Matrix()
        : data {} {
    }

    template<typename... Values>
        requires((std::same_as<Value, Values> && ...) && sizeof...(Values) == count<sparsity, Entry::any>())
    explicit constexpr Matrix(Values... values)
        : data { values... } {
    }

    template<IsMatrix Mat>
        requires(copyable<sparsity, Mat::sparsity>())
    explicit constexpr Matrix(const Mat& M) {
        template for (constexpr auto row : std::views::indices(rows)) {
            template for (constexpr auto column : std::views::indices(columns)) {
                if constexpr (sparsity[row][column] == Entry::any) {
                    get<row, column>(*this) = get<row, column>(M);
                }
            }
        }
    }

    template<IsMatrix Mat>
        requires(copyable<sparsity, Mat::sparsity>())
    auto& operator=(const Mat& M) {
        template for (constexpr auto row : std::views::indices(rows)) {
            template for (constexpr auto column : std::views::indices(columns)) {
                if constexpr (sparsity[row][column] == Entry::any) {
                    get<row, column>(*this) = get<row, column>(M);
                }
            }
        }
        return *this;
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
        return minusOne<Value>;
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
    requires(MatrixA::columns == MatrixB::rows)
constexpr auto operator*(const MatrixA& A, const MatrixB& B) {
    using CommonValue = std::common_type_t<typename MatrixA::Value, typename MatrixB::Value>;
    using MatrixC     = typename Dispatch<CommonValue, MatrixA::sparsity * MatrixB::sparsity>::Type;
    MatrixC C;
    template for (constexpr auto row : std::views::indices(MatrixA::rows)) {
        template for (constexpr auto column : std::views::indices(MatrixB::columns)) {
            auto& valueC = get<row, column>(C);
            if constexpr (MatrixC::sparsity[row][column] == Entry::any) {
                valueC = zero<CommonValue>;
                template for (constexpr auto middle : std::views::indices(MatrixA::columns)) {
                    constexpr auto entryA = MatrixA::sparsity[row][middle];
                    constexpr auto entryB = MatrixB::sparsity[middle][column];
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
                    } else if constexpr (entryA != Entry::zero && entryB != Entry::zero) {
                        constexpr auto valueA = get<row, middle>(A);
                        constexpr auto valueB = get<middle, column>(B);
                        valueC += valueA * valueB;
                    }
                }
            }
        }
    }
    return C;
}

template<IsMatrix Mat, IsArithmetic Scalar>
constexpr auto operator*(const Mat& A, const Scalar& a) {
    using CommonValue  = std::common_type_t<typename Mat::Value, Scalar>;
    using MatrixResult = typename Dispatch<CommonValue, Mat::sparsity * Entry::any>::Type;
    MatrixResult result;
    template for (constexpr auto row : std::views::indices(Mat::rows)) {
        template for (constexpr auto column : std::views::indices(Mat::columns)) {
            if constexpr (Mat::sparsity[row][column] == Entry::any) {
                get<row, column>(result) = get<row, column>(A) * a;
            } else if constexpr (Mat::sparsity[row][column] == Entry::pone) {
                get<row, column>(result) = a;
            } else if constexpr (Mat::sparsity[row][column] == Entry::mone) {
                get<row, column>(result) = -a;
            }
        }
    }
    return result;
}

template<IsMatrix Mat, std::floating_point Scalar>
constexpr auto operator/(const Mat& A, const Scalar& a) {
    return A * (one<Scalar> / a);
}

template<IsMatrix Mat>
constexpr auto transpose(const Mat& A) {
    using Transposed = typename Dispatch<typename Mat::Value, transpose(Mat::sparsity)>::Type;
    Transposed trA {};
    template for (constexpr auto row : std::views::indices(Mat::rows)) {
        template for (constexpr auto column : std::views::indices(Mat::columns)) {
            if constexpr (Mat::sparsity[row][column] == Entry::any) {
                get<column, row>(trA) = get<row, column>(A);
            }
        }
    }
    return trA;
}

template<IsMatrix Mat>
    requires(Mat::rows == 4 && Mat::columns == 4)
constexpr Matrix<typename Mat::Value, inverse(Mat::sparsity)> inverse(const Mat& m) {
    Matrix<typename Mat::Value, inverse(Mat::sparsity)> inv {};
    get<0, 0>(inv) = get<1, 1>(m) * get<2, 2>(m) * get<3, 3>(m) - get<1, 1>(m) * get<2, 3>(m) * get<3, 2>(m) -
                     get<2, 1>(m) * get<1, 2>(m) * get<3, 3>(m) + get<2, 1>(m) * get<1, 3>(m) * get<3, 2>(m) +
                     get<3, 1>(m) * get<1, 2>(m) * get<2, 3>(m) - get<3, 1>(m) * get<1, 3>(m) * get<2, 2>(m);

    get<1, 0>(inv) = -get<1, 0>(m) * get<2, 2>(m) * get<3, 3>(m) + get<1, 0>(m) * get<2, 3>(m) * get<3, 2>(m) +
                     get<2, 0>(m) * get<1, 2>(m) * get<3, 3>(m) - get<2, 0>(m) * get<1, 3>(m) * get<3, 2>(m) -
                     get<3, 0>(m) * get<1, 2>(m) * get<2, 3>(m) + get<3, 0>(m) * get<1, 3>(m) * get<2, 2>(m);

    get<2, 0>(inv) = get<1, 0>(m) * get<2, 1>(m) * get<3, 3>(m) - get<1, 0>(m) * get<2, 3>(m) * get<3, 1>(m) -
                     get<2, 0>(m) * get<1, 1>(m) * get<3, 3>(m) + get<2, 0>(m) * get<1, 3>(m) * get<3, 1>(m) +
                     get<3, 0>(m) * get<1, 1>(m) * get<2, 3>(m) - get<3, 0>(m) * get<1, 3>(m) * get<2, 1>(m);

    get<3, 0>(inv) = -get<1, 0>(m) * get<2, 1>(m) * get<3, 2>(m) + get<1, 0>(m) * get<2, 2>(m) * get<3, 1>(m) +
                     get<2, 0>(m) * get<1, 1>(m) * get<3, 2>(m) - get<2, 0>(m) * get<1, 2>(m) * get<3, 1>(m) -
                     get<3, 0>(m) * get<1, 1>(m) * get<2, 2>(m) + get<3, 0>(m) * get<1, 2>(m) * get<2, 1>(m);

    get<0, 1>(inv) = -get<0, 1>(m) * get<2, 2>(m) * get<3, 3>(m) + get<0, 1>(m) * get<2, 3>(m) * get<3, 2>(m) +
                     get<2, 1>(m) * get<0, 2>(m) * get<3, 3>(m) - get<2, 1>(m) * get<0, 3>(m) * get<3, 2>(m) -
                     get<3, 1>(m) * get<0, 2>(m) * get<2, 3>(m) + get<3, 1>(m) * get<0, 3>(m) * get<2, 2>(m);

    get<1, 1>(inv) = get<0, 0>(m) * get<2, 2>(m) * get<3, 3>(m) - get<0, 0>(m) * get<2, 3>(m) * get<3, 2>(m) -
                     get<2, 0>(m) * get<0, 2>(m) * get<3, 3>(m) + get<2, 0>(m) * get<0, 3>(m) * get<3, 2>(m) +
                     get<3, 0>(m) * get<0, 2>(m) * get<2, 3>(m) - get<3, 0>(m) * get<0, 3>(m) * get<2, 2>(m);

    get<2, 1>(inv) = -get<0, 0>(m) * get<2, 1>(m) * get<3, 3>(m) + get<0, 0>(m) * get<2, 3>(m) * get<3, 1>(m) +
                     get<2, 0>(m) * get<0, 1>(m) * get<3, 3>(m) - get<2, 0>(m) * get<0, 3>(m) * get<3, 1>(m) -
                     get<3, 0>(m) * get<0, 1>(m) * get<2, 3>(m) + get<3, 0>(m) * get<0, 3>(m) * get<2, 1>(m);

    get<3, 1>(inv) = get<0, 0>(m) * get<2, 1>(m) * get<3, 2>(m) - get<0, 0>(m) * get<2, 2>(m) * get<3, 1>(m) -
                     get<2, 0>(m) * get<0, 1>(m) * get<3, 2>(m) + get<2, 0>(m) * get<0, 2>(m) * get<3, 1>(m) +
                     get<3, 0>(m) * get<0, 1>(m) * get<2, 2>(m) - get<3, 0>(m) * get<0, 2>(m) * get<2, 1>(m);

    get<0, 2>(inv) = get<0, 1>(m) * get<1, 2>(m) * get<3, 3>(m) - get<0, 1>(m) * get<1, 3>(m) * get<3, 2>(m) -
                     get<1, 1>(m) * get<0, 2>(m) * get<3, 3>(m) + get<1, 1>(m) * get<0, 3>(m) * get<3, 2>(m) +
                     get<3, 1>(m) * get<0, 2>(m) * get<1, 3>(m) - get<3, 1>(m) * get<0, 3>(m) * get<1, 2>(m);

    get<1, 2>(inv) = -get<0, 0>(m) * get<1, 2>(m) * get<3, 3>(m) + get<0, 0>(m) * get<1, 3>(m) * get<3, 2>(m) +
                     get<1, 0>(m) * get<0, 2>(m) * get<3, 3>(m) - get<1, 0>(m) * get<0, 3>(m) * get<3, 2>(m) -
                     get<3, 0>(m) * get<0, 2>(m) * get<1, 3>(m) + get<3, 0>(m) * get<0, 3>(m) * get<1, 2>(m);

    get<2, 2>(inv) = get<0, 0>(m) * get<1, 1>(m) * get<3, 3>(m) - get<0, 0>(m) * get<1, 3>(m) * get<3, 1>(m) -
                     get<1, 0>(m) * get<0, 1>(m) * get<3, 3>(m) + get<1, 0>(m) * get<0, 3>(m) * get<3, 1>(m) +
                     get<3, 0>(m) * get<0, 1>(m) * get<1, 3>(m) - get<3, 0>(m) * get<0, 3>(m) * get<1, 1>(m);

    get<3, 2>(inv) = -get<0, 0>(m) * get<1, 1>(m) * get<3, 2>(m) + get<0, 0>(m) * get<1, 2>(m) * get<3, 1>(m) +
                     get<1, 0>(m) * get<0, 1>(m) * get<3, 2>(m) - get<1, 0>(m) * get<0, 2>(m) * get<3, 1>(m) -
                     get<3, 0>(m) * get<0, 1>(m) * get<1, 2>(m) + get<3, 0>(m) * get<0, 2>(m) * get<1, 1>(m);

    get<0, 3>(inv) = -get<0, 1>(m) * get<1, 2>(m) * get<2, 3>(m) + get<0, 1>(m) * get<1, 3>(m) * get<2, 2>(m) +
                     get<1, 1>(m) * get<0, 2>(m) * get<2, 3>(m) - get<1, 1>(m) * get<0, 3>(m) * get<2, 2>(m) -
                     get<2, 1>(m) * get<0, 2>(m) * get<1, 3>(m) + get<2, 1>(m) * get<0, 3>(m) * get<1, 2>(m);

    get<1, 3>(inv) = get<0, 0>(m) * get<1, 2>(m) * get<2, 3>(m) - get<0, 0>(m) * get<1, 3>(m) * get<2, 2>(m) -
                     get<1, 0>(m) * get<0, 2>(m) * get<2, 3>(m) + get<1, 0>(m) * get<0, 3>(m) * get<2, 2>(m) +
                     get<2, 0>(m) * get<0, 2>(m) * get<1, 3>(m) - get<2, 0>(m) * get<0, 3>(m) * get<1, 2>(m);

    get<2, 3>(inv) = -get<0, 0>(m) * get<1, 1>(m) * get<2, 3>(m) + get<0, 0>(m) * get<1, 3>(m) * get<2, 1>(m) +
                     get<1, 0>(m) * get<0, 1>(m) * get<2, 3>(m) - get<1, 0>(m) * get<0, 3>(m) * get<2, 1>(m) -
                     get<2, 0>(m) * get<0, 1>(m) * get<1, 3>(m) + get<2, 0>(m) * get<0, 3>(m) * get<1, 1>(m);

    get<3, 3>(inv) = get<0, 0>(m) * get<1, 1>(m) * get<2, 2>(m) - get<0, 0>(m) * get<1, 2>(m) * get<2, 1>(m) -
                     get<1, 0>(m) * get<0, 1>(m) * get<2, 2>(m) + get<1, 0>(m) * get<0, 2>(m) * get<2, 1>(m) +
                     get<2, 0>(m) * get<0, 1>(m) * get<1, 2>(m) - get<2, 0>(m) * get<0, 2>(m) * get<1, 1>(m);

    const auto det = get<0, 0>(m) * get<0, 0>(inv) + get<0, 1>(m) * get<1, 0>(inv) + get<0, 2>(m) * get<2, 0>(inv) +
                     get<0, 3>(m) * get<3, 0>(inv);

    return det != zero<typename Mat::Value> ? inv / det : throw std::runtime_error("Singular matrix");
}

}  // namespace tmt