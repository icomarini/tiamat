#include "tiamat/tiamat.hpp"

#include <iostream>
#include <functional>
#include <cassert>

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

    {
        constexpr Translation<int, 3> translation { 10, 11, 12 };
        constexpr Translation<int, 3> result = translation * translation * translation * identity<int, 4>;
        static_assert(get<0, 3>(result) == 30);
        static_assert(get<1, 3>(result) == 33);
        static_assert(get<2, 3>(result) == 36);

        static_assert(decltype(translation)::sparsity.rows == 4);
        static_assert(decltype(translation)::sparsity.columns == 4);

        static_assert(sizeof(translation) == 3 * sizeof(int));
        static_assert(IsMatrix<decltype(translation)>);
        // auto result = translation * translation;
        static_assert(IsMatrix<decltype(result)>);
        static_assert(std::is_same_v<decltype(result), decltype(translation)>);
    }
}

void testRotation0() {
    using namespace tiamat;
    { const Rotation<int, 3> rotation; }
}

void test() {
    testSparsity0();
    testSparsity1();
    testMatrix0();
    testTranslation0();
    testRotation0();
}


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
