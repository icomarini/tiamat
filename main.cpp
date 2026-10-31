#include "tiamat/tiamat.hpp"

#include <iostream>
#include <functional>
#include <cassert>

// namespace tiamat {

// constexpr auto translationLayout = encodeLayout({
//     //           Entry::zero  Entry::pone  Entry::mone  Entry::any
//     std::array { Entry::pone, Entry::zero, Entry::zero, Entry::any  }, // Entry::zero
//     std::array { Entry::zero, Entry::pone, Entry::zero, Entry::any  }, // Entry::pone
//     std::array { Entry::zero, Entry::zero, Entry::pone, Entry::any  }, // Entry::mone
//     std::array { Entry::zero, Entry::zero, Entry::zero, Entry::pone }, // Entry::any
// });

// template<typename Value = float>
// class Translation : public Matrix<Value, translationLayout> {
// public:
//     constexpr Translation()                   = default;
//     constexpr Translation(const Translation&) = default;
//     constexpr Translation(Translation&&)      = default;
//     constexpr Translation(const Value x, const Value y, const Value z)
//         : Matrix<Value, translationLayout> { x, y, z } {
//     }
// };

// template<typename Value>
// struct Dispatch<Value, translationLayout> {
//     using Type = Translation<Value>;
// };


// using Identity = Matrix<float, encodeLayout({
//                                    //           Entry::zero  Entry::pone  Entry::mone  Entry::any
//                                    std::array { Entry::pone, Entry::zero, Entry::zero, Entry::zero }, // Entry::zero
//                                    std::array { Entry::zero, Entry::pone, Entry::zero, Entry::zero }, // Entry::pone
//                                    std::array { Entry::zero, Entry::zero, Entry::pone, Entry::zero }, // Entry::mone
//                                    std::array { Entry::zero, Entry::zero, Entry::zero, Entry::pone }, // Entry::any
// })>;
// constexpr Identity identity;


// template<typename Value, Layout layout>
// std::ostream& operator<<(std::ostream& stream, const Matrix<Value, layout>& matrix) {
//     template for (constexpr auto row : std::views::iota(0, 4)) {
//         template for (constexpr auto col : std::views::iota(0, 4)) {
//             stream << " " << get<row, col>(matrix);
//         }
//         stream << std::endl;
//     }
//     return stream;
// }

// template<typename Mat>
// bool testMat() {
//     static_assert(multiply(Mat::layout, Mat::layout) == Mat::layout);
//     static_assert(multiply(Identity::layout, Mat::layout) == Mat::layout);
//     static_assert(multiply(Mat::layout, Identity::layout) == Mat::layout);

//     constexpr Mat cm1;
//     constexpr Mat cm2;
//     static_assert((cm1 * cm1).layout == Mat::layout);
//     // static_assert(typ)

//     static_assert((cm1 * identity).layout == Mat::layout);
//     static_assert((identity * cm1).layout == Mat::layout);

//     static_assert((cm1 * cm2).layout == Mat::layout);
//     static_assert((cm2 * cm1).layout == Mat::layout);

//     return true;
// }


void testSparsity0() {
    using namespace tiamat;

    constexpr Sparsity sparsity {
        std::array { Entry::zero, Entry::pone, Entry::any },
         std::array { Entry::zero, Entry::mone, Entry::any }
    };
    static_assert(sparsity.rows == 2);
    static_assert(sparsity.columns == 3);

    static_assert(valid<sparsity, 0, 0>());
    static_assert(valid<sparsity, 0, 1>());
    static_assert(valid<sparsity, 0, 2>());
    static_assert(valid<sparsity, 1, 0>());
    static_assert(valid<sparsity, 1, 1>());
    static_assert(valid<sparsity, 1, 2>());
    // static_assert(!valid<sparsity, -1, 0>());
    // static_assert(!valid<sparsity, 0, -1>());
    static_assert(!valid<sparsity, 3, 2>());
    static_assert(!valid<sparsity, 1, 4>());

    static_assert(sparsity[0][0] == Entry::zero);
    static_assert(sparsity[0][1] == Entry::pone);
    static_assert(sparsity[0][2] == Entry::any);
    static_assert(sparsity[1][0] == Entry::zero);
    static_assert(sparsity[1][1] == Entry::mone);
    static_assert(sparsity[1][2] == Entry::any);

    static_assert(count<sparsity, Entry::zero>() == 2);
    static_assert(count<sparsity, Entry::pone>() == 1);
    static_assert(count<sparsity, Entry::mone>() == 1);
    static_assert(count<sparsity, Entry::any>() == 2);

    // static_assert(index<sparsity, 0, 0>() == 0);
    // static_assert(index<sparsity, 0, 1>() == 0);
    static_assert(index<sparsity, 0, 2>() == 0);
    // static_assert(index<sparsity, 1, 0>() == 1);
    // static_assert(index<sparsity, 1, 1>() == 1);
    static_assert(index<sparsity, 1, 2>() == 1);
    // static_assert(index<sparsity, 0, 0>() == 0);
}

void testSparsity1() {
    using namespace tiamat;
    {
        constexpr Sparsity sparsity {
            std::array { Entry::pone, Entry::zero },
             std::array { Entry::zero, Entry::pone }
        };
        constexpr auto result = sparsity * sparsity;
        static_assert(result.rows == 2);
        static_assert(result.columns == 2);
        static_assert(result[0][0] == Entry::pone);
        static_assert(result[0][1] == Entry::zero);
        static_assert(result[1][0] == Entry::zero);
        static_assert(result[1][1] == Entry::pone);
    }

    {
        constexpr Sparsity sparsity {
            std::array { Entry::mone, Entry::zero },
             std::array { Entry::zero, Entry::pone }
        };
        constexpr auto result = sparsity * sparsity;
        static_assert(result.rows == 2);
        static_assert(result.columns == 2);
        static_assert(result[0][0] == Entry::pone);
        static_assert(result[0][1] == Entry::zero);
        static_assert(result[1][0] == Entry::zero);
        static_assert(result[1][1] == Entry::pone);
    }

    {
        constexpr Sparsity sparsity {
            std::array { Entry::zero, Entry::pone },
             std::array { Entry::pone, Entry::zero }
        };
        constexpr auto result = sparsity * sparsity;
        static_assert(result.rows == 2);
        static_assert(result.columns == 2);
        static_assert(result[0][0] == Entry::pone);
        static_assert(result[0][1] == Entry::zero);
        static_assert(result[1][0] == Entry::zero);
        static_assert(result[1][1] == Entry::pone);
    }

    {
        constexpr Sparsity sparsity1 {
            std::array { Entry::any, Entry::any },
        };
        constexpr Sparsity sparsity2 {
            std::array { Entry::any },
            std::array { Entry::any },
        };
        constexpr auto result = sparsity1 * sparsity2;
        static_assert(result.rows == 1);
        static_assert(result.columns == 1);
        static_assert(result[0][0] == Entry::any);
        static_assert(index<result, 0, 0>() == 0);
    }
}

void testMatrix0() {
    using namespace tiamat;
    {
        constexpr Sparsity sparsity {
            std::array { Entry::pone, Entry::zero, Entry::any  },
            std::array { Entry::zero, Entry::pone, Entry::any  },
            std::array { Entry::zero, Entry::zero, Entry::pone },
        };
        Matrix<int, sparsity> matrix;
        static_assert(IsMatrix<decltype(matrix)>);
        static_assert(get<0, 0>(matrix) == 1);
        static_assert(get<0, 1>(matrix) == 0);
        const auto m02 = get<0, 2>(matrix);
        assert(m02 == 0);
        assert((get<0, 2>(matrix) == 0));
        get<0, 2>(matrix) = 666;
        assert((get<0, 2>(matrix) == 666));
        get<1, 2>(matrix) = 667;
        assert((get<1, 2>(matrix) == 667));

        static_assert(get<1, 0>(matrix) == 0);
        static_assert(get<1, 1>(matrix) == 1);
        // static_assert(get<0, 2>(matrix) == 1);
        static_assert(get<2, 0>(matrix) == 0);
        static_assert(get<2, 1>(matrix) == 0);
        static_assert(get<2, 2>(matrix) == 1);

        // constexpr auto m00 = get<0, 0>(matrix);
        auto result = matrix * matrix;
        // static_assert(std::is_same_v<decltype(result), decltype(matrix)>); // it's a translation!!!
        assert((get<0, 2>(result) == 2 * 666));
        assert((get<1, 2>(result) == 2 * 667));
    }
}

void testIdentity0() {
    using namespace tiamat;
    {
        Identity<int, 1> identity;
        static_assert(decltype(identity)::sparsity.rows == 1);
        static_assert(decltype(identity)::sparsity.columns == 1);

        static_assert(identity.sparsity[0][0] == Entry::pone);
        // static_assert(translation.sparsity[0][1] == Entry::zero);
        // static_assert(translation.sparsity[1][0] == Entry::zero);
        // static_assert(translation.sparsity[1][1] == Entry::pone);

        // static_assert(sizeof(identity) == 1 * sizeof(int));
        static_assert(IsMatrix<decltype(identity)>);
        // auto result = identity * identity;
        // static_assert(IsMatrix<decltype(result)>);
        // static_assert(std::is_same_v<decltype(result), decltype(identity)>);
    }
    // {
    //     Translation<int, 2> translation;
    //     static_assert(decltype(translation)::sparsity.rows == 3);
    //     static_assert(decltype(translation)::sparsity.columns == 3);
    //     static_assert(sizeof(translation) == 2 * sizeof(int));
    //     static_assert(IsMatrix<decltype(translation)>);
    //     auto result = translation * translation;
    //     static_assert(IsMatrix<decltype(result)>);
    //     static_assert(std::is_same_v<decltype(result), decltype(translation)>);
    // }
}


void testTranslation0() {
    using namespace tiamat;
    {
        Translation<int, 1> translation;
        static_assert(decltype(translation)::sparsity.rows == 2);
        static_assert(decltype(translation)::sparsity.columns == 2);

        static_assert(translation.sparsity[0][0] == Entry::pone);
        static_assert(translation.sparsity[0][1] == Entry::any);
        static_assert(translation.sparsity[1][0] == Entry::zero);
        static_assert(translation.sparsity[1][1] == Entry::pone);

        static_assert(sizeof(translation) == 1 * sizeof(int));
        static_assert(IsMatrix<decltype(translation)>);
        auto result = translation * translation;
        static_assert(IsMatrix<decltype(result)>);
        static_assert(std::is_same_v<decltype(result), decltype(translation)>);
    }
    {
        Translation<int, 2> translation;
        static_assert(decltype(translation)::sparsity.rows == 3);
        static_assert(decltype(translation)::sparsity.columns == 3);
        static_assert(sizeof(translation) == 2 * sizeof(int));
        static_assert(IsMatrix<decltype(translation)>);
        auto result = translation * translation;
        static_assert(IsMatrix<decltype(result)>);
        static_assert(std::is_same_v<decltype(result), decltype(translation)>);
    }
}

void test() {
    testSparsity0();
    testSparsity1();
    testMatrix0();
    testTranslation0();
    // tiamat::Mat<float,
    //             Sparsity {
    //                 std::array { Entry::pone, Entry::zero, Entry::zero, Entry::any  },
    //                 std::array { Entry::zero, Entry::pone, Entry::zero, Entry::any  },
    //                 std::array { Entry::zero, Entry::zero, Entry::pone, Entry::any  },
    //                 std::array { Entry::zero, Entry::zero, Entry::zero, Entry::pone },
    // }>
    //     test;
    // static_assert(sizeof(decltype(test)) == 3 * sizeof(float));
    // static_assert(decltype(test)::sparsity.valid(0, 3) == 0);


    // Matrix
    // static_assert(get<0, 0>(identity) == 1);
    // static_assert(get<1, 0>(identity) == 0);
    // static_assert(get<0, 1>(identity) == 0);
    // static_assert(get<1, 1>(identity) == 1);
    // static_assert(sizeof(identity) == 1);
    // std::cout << get<0, 0>(identity) << std::endl;

    // std::cout << identity;


    // testMat<Translation<>>();
    // Translation<> translation;
    // static_assert(sizeof(Translation<>) == 3 * sizeof(float));
    // static_assert(multiply(Translation<>::layout, Translation<>::layout) == Translation<>::layout);
    // static_assert(multiply(Identity::layout, Translation<>::layout) == Translation<>::layout);
    // static_assert(multiply(Translation<>::layout, Identity::layout) == Translation<>::layout);
    // constexpr Translation<> cTr;
    // static_assert((cTr * cTr).layout == Translation<>::layout);
    // static_assert((identity * cTr).layout == Translation<>::layout);
    // static_assert((cTr * identity).layout == Translation<>::layout);
    // get<0, 3>(translation) = 1.4;
    // constexpr auto t00     = get<0, 0>(translation);
    // static_assert(t00 == 1);
    // static_assert(get<0, 0>(translation) == 1);
    // static_assert(get<0, 1>(translation) == 0);
    // {
    //     // constexpr Translation t1 = std::invoke([] {
    //     //     Translation t;
    //     //     get<0, 3>(t) = 1;
    //     //     get<1, 3>(t) = 2;
    //     //     get<2, 3>(t) = 3;
    //     //     return t;
    //     // });
    //     constexpr Translation t1 { 1, 2, 3 };
    //     static_assert(get<0, 3>(t1 * identity) == 1);
    //     static_assert(get<1, 3>(t1) == 2);
    //     static_assert(get<2, 3>(t1) == 3);
    //     std::cout << "\n" << t1;

    //     constexpr Translation t2 { 2, 3, 4 };
    //     std::cout << "\n" << t2;
    //     constexpr auto t3 = t1 * t2;
    //     static_assert(std::is_same_v<decltype(t3), decltype(t2)>);
    //     static_assert(std::is_same_v<decltype(t3), decltype(t1)>);
    //     std::cout << "\n" << t3;
    //     static_assert(get<0, 3>(t3) == 3);
    //     static_assert(get<1, 3>(t3) == 5);
    //     static_assert(get<2, 3>(t3) == 7);
    //     // const auto g = glm::transpose();
    // }

    // Translation translation2;
    // get<0, 3>(translation) = 2;

    // static_assert(get<0, 0>(translation) == 1);
    // static_assert(get<1, 0>(translation) == 0);
    // static_assert(get<0, 1>(translation) == 0);
    // static_assert(get<1, 1>(translation) == 1);
    // // static_assert(get<0, 3>(translation) == 0);
    // // get<0,3>
    // // static_assert(sizeof(identity) == 1);

    // using Rotation = Matrix<float, encodeLayout({
    //                                    //           Entry::zero  Entry::pone  Entry::mone  Entry::any
    //                                    std::array { Entry::any,  Entry::any,  Entry::any,  Entry::zero }, //
    //                                    Entry::zero std::array { Entry::any,  Entry::any,  Entry::any,  Entry::zero },
    //                                    // Entry::pone std::array { Entry::any,  Entry::any,  Entry::any,  Entry::zero
    //                                    }, // Entry::mone std::array { Entry::zero, Entry::zero, Entry::zero,
    //                                    Entry::pone }, // Entry::any
    // })>;
    // testMat<Rotation>();
    // Rotation rotation;
    // static_assert(sizeof(Rotation) == 9 * sizeof(float));
    // static_assert(multiply(Rotation::layout, Rotation::layout) == Rotation::layout);
    // static_assert(multiply(Identity::layout, Rotation::layout) == Rotation::layout);
    // static_assert(multiply(Rotation::layout, Identity::layout) == Rotation::layout);
    // constexpr Rotation cRot;
    // static_assert((cRot * cRot).layout == Rotation::layout);
    // static_assert((identity * cRot).layout == Rotation::layout);
    // static_assert((cRot * identity).layout == Rotation::layout);

    // using Scale = Matrix<float, encodeLayout({
    //                                 //           Entry::zero  Entry::pone  Entry::mone  Entry::any
    //                                 std::array { Entry::any,  Entry::zero, Entry::zero, Entry::zero }, // Entry::zero
    //                                 std::array { Entry::zero, Entry::any,  Entry::zero, Entry::zero }, // Entry::pone
    //                                 std::array { Entry::zero, Entry::zero, Entry::any,  Entry::zero }, // Entry::mone
    //                                 std::array { Entry::zero, Entry::zero, Entry::zero, Entry::pone }, // Entry::any
    // })>;
    // Scale scale;
    // testMat<Scale>();
    // {
    //     constexpr auto s1 = std::invoke([] {
    //         Scale s;
    //         get<0, 0>(s) = 1;
    //         get<1, 1>(s) = 2;
    //         get<2, 2>(s) = 3;
    //         return s;
    //     });
    //     static_assert(get<0, 0>(s1 * identity) == 1);
    //     static_assert(get<1, 1>(s1) == 2);
    //     static_assert(get<2, 2>(s1) == 3);

    //     constexpr auto s2 = std::invoke([] {
    //         Scale s;
    //         get<0, 0>(s) = 2;
    //         get<1, 1>(s) = 3;
    //         get<2, 2>(s) = 4;
    //         return s;
    //     });
    //     constexpr auto s3 = s1 * s2;
    //     static_assert(get<0, 0>(s3) == 2);
    //     static_assert(get<1, 1>(s3) == 6);
    //     static_assert(get<2, 2>(s3) == 12);
    // }
    // static_assert(sizeof(Scale) == 3 * sizeof(float));
    // static_assert(multiply(Scale::layout, Scale::layout) == Scale::layout);
    // static_assert(multiply(Identity::layout, Scale::layout) == Scale::layout);
    // static_assert(multiply(Scale::layout, Identity::layout) == Scale::layout);


    // using Transformation =
    //     Matrix<float, encodeLayout({
    //                       //           Entry::zero  Entry::pone  Entry::mone  Entry::any
    //                       std::array { Entry::any,  Entry::any,  Entry::any,  Entry::any  }, // Entry::zero
    //                       std::array { Entry::any,  Entry::any,  Entry::any,  Entry::any  }, // Entry::pone
    //                       std::array { Entry::any,  Entry::any,  Entry::any,  Entry::any  }, // Entry::mone
    //                       std::array { Entry::zero, Entry::zero, Entry::zero, Entry::pone }, // Entry::any
    // })>;
    // testMat<Transformation>();
    // Transformation transformation;
    // static_assert(sizeof(Transformation) == 12 * sizeof(float));
    // static_assert(multiply(Transformation::layout, Transformation::layout) == Transformation::layout);
    // static_assert(multiply(Identity::layout, Transformation::layout) == Transformation::layout);
    // static_assert(multiply(Transformation::layout, Identity::layout) == Transformation::layout);
    // static_assert(multiply(multiply(Translation<>::layout, Rotation::layout), Scale::layout) ==
    // Transformation::layout); static_assert(multiply(multiply(Translation<>::layout, Rotation::layout),
    // Identity::layout) ==
    //               Transformation::layout);
    // static_assert(multiply(Translation<>::layout, multiply(Scale::layout, Rotation::layout)) ==
    // Transformation::layout);

    // static_assert(multiply(Translation<>::layout, multiply(Rotation::layout, Scale::layout)) ==
    // Transformation::layout); static_assert(multiply(Translation<>::layout, multiply(Rotation::layout,
    // Identity::layout)) ==
    //               Transformation::layout);
    // std::exit(0);
};


int main() {
    try {
        std::cout << "Tiamat test begin" << std::endl;
        test();
        std::cout << "Tiamat test end" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\033[1;31m[ERROR]\033[0m" << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "\033[1;31m[ERROR]\033[0m" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
