#pragma once
#include "tiamat/Sparsity.hpp"
namespace tiamat {

// template<typename V, Layout l>
//     requires(l != 0)
// class Matrix {
// public:
//     using Value                    = V;
//     static constexpr Layout layout = l;
//     constexpr Matrix()
//         : data {} { };

//     template<typename... Vs>
//     constexpr Matrix(Vs... vs)
//         : data { vs... } {
//     }

//     template<uint8_t row, uint8_t col, typename VV, Layout ll>
//     friend constexpr auto& get(const Matrix<VV, ll>& m);

//     template<uint8_t row, uint8_t col, typename VV, Layout ll>
//         requires(getEntry<row, col>(ll) == Entry::any)
//     friend constexpr auto& get(Matrix<VV, ll>& m);

// private:
//     std::array<Value, count(layout, Entry::any)> data;
// };

// template<typename Value, Layout layout>
// struct Dispatch {
//     using Type = Matrix<Value, layout>;
// };


// template<typename T>
// static constexpr T zero = T { 0 };

// template<typename T>
// static constexpr T one = T { 1 };


// template<uint8_t row, uint8_t col, typename V, Layout l>
// constexpr auto& get(const Matrix<V, l>& m) {
//     constexpr auto e = getEntry<row, col>(l);
//     if constexpr (e == Entry::zero) {
//         return zero<V>;
//     } else if constexpr (e == Entry::pone) {
//         return one<V>;
//     } else if constexpr (e == Entry::mone) {
//         return -one<V>;
//     } else {
//         return m.data[index<row, col, l>()];
//     }
// }

// template<uint8_t row, uint8_t col, typename Value, Layout layout>
//     requires(getEntry<row, col>(layout) == Entry::any)
// constexpr auto& get(Matrix<Value, layout>& m) {
//     return m.data[index<row, col, layout>()];
// }

// template<typename ValueA, Layout layoutA, typename ValueB, Layout layoutB>
// constexpr auto operator*(const Matrix<ValueA, layoutA>& A, const Matrix<ValueB, layoutB>& B) {
//     constexpr auto layoutC = multiply(layoutA, layoutB);

//     using CommonValue = std::common_type_t<ValueA, ValueB>;
//     using MatrixC     = typename Dispatch<CommonValue, layoutC>::Type;
//     MatrixC C;
//     template for (constexpr auto row : std::views::iota(0, 4)) {
//         template for (constexpr auto col : std::views::iota(0, 4)) {
//             template for (constexpr auto mid : std::views::iota(0, 4)) {
//                 if constexpr (getEntry<row, col>(layoutC) == Entry::any) {
//                     constexpr auto entryA = getEntry<row, mid>(layoutA);
//                     constexpr auto entryB = getEntry<mid, col>(layoutB);
//                     auto&          valueC = get<row, col>(C);
//                     if constexpr (entryA == Entry::any && entryB != Entry::any) {
//                         const auto&    valueA = get<row, mid>(A);
//                         constexpr auto valueB = get<mid, col>(B);
//                         valueC += valueA * valueB;
//                     } else if constexpr (entryA != Entry::any && entryB == Entry::any) {
//                         constexpr auto valueA = get<row, mid>(A);
//                         const auto&    valueB = get<mid, col>(B);
//                         valueC += valueA * valueB;
//                     } else if constexpr (entryA == Entry::any && entryB == Entry::any) {
//                         const auto& valueA = get<row, mid>(A);
//                         const auto& valueB = get<mid, col>(B);
//                         valueC += valueA * valueB;
//                     }
//                 }
//             }
//         }
//     }
//     return C;
// }

// namespace experimental {
// =========================================
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