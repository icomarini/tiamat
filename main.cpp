#include <cstdint>
#include <bitset>
#include <iostream>
#include <functional>
#include <utility>
#include <ranges>

// namespace core {
// using Size = long unsigned int;

// template<Size begin, Size end, typename Operation>
// constexpr void forEach(const Operation& operation) {
//     if constexpr (begin == end) {
//         return;
//     } else {
//         operation.template operator()<begin>();
//         if constexpr (begin < end - 1) {
//             forEach<begin + 1, end>(operation);
//         }
//     }
// }

// template<Size begin, Size end, typename Array, typename Operation>
// constexpr void forEach(Array& array, const Operation& operation) {
//     if constexpr (begin == end) {
//         return;
//     } else {
//         operation.template operator()<begin>(array[begin]);
//         if constexpr (begin < end - 1) {
//             forEach<begin + 1, end>(array, operation);
//         }
//     }
// }

// template<Size begin0, Size end0, Size begin1, Size end1, typename Operation>
// constexpr void forEach(const Operation& operation) {
//     static_assert(begin0 <= end0);
//     static_assert(begin1 <= end1);
//     forEach<begin0, end0>([&]<int index0>() {
//         forEach<begin1, end1>([&]<int index1>() { operation.template operator()<index0, index1>(); });
//     });
// }

// template<Size begin0, Size end0, Size begin1, Size end1, Size begin2, Size end2, typename Operation>
// constexpr void forEach(const Operation& operation) {
//     static_assert(begin0 <= end0);
//     static_assert(begin1 <= end1);
//     forEach<begin0, end0>([&]<int index0>() {
//         forEach<begin1, end1>([&]<int index1>() {
//             forEach<begin2, end2>([&]<int index2>() { operation.template operator()<index0, index1, index2>(); });
//         });
//     });
// }
// }  // namespace core

namespace tiamat  // TIAMAT: Template Interfaces for Affine MATrices
{
enum class Entry : uint8_t {
    zero = 0,
    pone,
    mone,
    any,
};

std::string toString(const Entry e) {
    switch (e) {
    case Entry::zero:
        return "zero";
    case Entry::pone:
        return "+one";
    case Entry::mone:
        return "-one";
    case Entry::any:
        return "any!";
    default:
        std::unreachable();
    }
}

using Layout = uint32_t;

static constexpr Entry entry(const Layout layout, const int position)
// requires(0 <= position && position < 16)
{
    const auto position1 = (layout >> (2 * position)) & 1;
    const auto position2 = (layout >> (2 * position + 1)) & 1;
    return position1 ? (position2 ? Entry::any : Entry::mone) : (position2 ? Entry::pone : Entry::zero);
}

template<uint8_t row, uint8_t col>
    requires(row < 4 && col < 4)
constexpr Entry getEntry(const Layout layout) {
    return entry(layout, row * 4 + col);
}


template<uint8_t row, uint8_t col>
constexpr Layout setEntry(Layout layout, const Entry entry) {
    auto       setBit   = [&](int p) { layout |= (1U << p); };
    auto       clearBit = [&](int p) { layout &= ~(1U << p); };
    const auto position = row * 4 + col;
    const auto left     = 2 * position;
    const auto right    = 2 * position + 1;
    switch (entry) {
    case Entry::zero:
        clearBit(left);
        clearBit(right);
        return layout;
    case Entry::pone:
        clearBit(left);
        setBit(right);
        return layout;
    case Entry::mone:
        setBit(left);
        clearBit(right);
        return layout;
    case Entry::any:
        setBit(left);
        setBit(right);
        return layout;
    default:
        throw("bad");
    }
    // return set(layout, row * 4 + col, entry);
}

static constexpr std::size_t count(const Layout layout, const Entry entry) {
    std::size_t count {};
    template for (constexpr auto row : std::views::iota(0, 4)) {
        template for (constexpr auto col : std::views::iota(0, 4)) {
            if (getEntry<row, col>(layout) == entry) {
                ++count;
            }
        }
    }
    // core::forEach<0, 4, 0, 4>([&]<int row, int col>() {
    //     if (getEntry<row, col>(layout) == entry) {
    //         ++count;
    //     }
    // });
    return count;
}

constexpr Entry operator+(const Entry a, const Entry b) {
    static constexpr std::array<std::array<Entry, 4>, 4> table {
        //           Entry::zero  Entry::pone  Entry::mone  Entry::any
        std::array { Entry::zero, Entry::pone, Entry::mone, Entry::any }, // Entry::zero
        std::array { Entry::pone, Entry::any,  Entry::zero, Entry::any }, // Entry::pone
        std::array { Entry::mone, Entry::zero, Entry::any,  Entry::any }, // Entry::mone
        std::array { Entry::any,  Entry::any,  Entry::any,  Entry::any }, // Entry::any
    };
    return table[static_cast<uint8_t>(a)][static_cast<uint8_t>(b)];
}

constexpr Entry operator*(const Entry a, const Entry b) {
    static constexpr std::array<std::array<Entry, 4>, 4> table {
        //           Entry::zero  Entry::pone  Entry::mone  Entry::any
        std::array { Entry::zero, Entry::zero, Entry::zero, Entry::zero }, // Entry::zero
        std::array { Entry::zero, Entry::pone, Entry::mone, Entry::any  }, // Entry::pone
        std::array { Entry::zero, Entry::mone, Entry::pone, Entry::any  }, // Entry::mone
        std::array { Entry::zero, Entry::any,  Entry::any,  Entry::any  }, // Entry::any
    };
    return table[static_cast<uint8_t>(a)][static_cast<uint8_t>(b)];
}

constexpr Layout multiply(const Layout a, const Layout b) {
    Layout c {};
    template for (constexpr auto row : std::views::iota(0, 4)) {
        template for (constexpr auto col : std::views::iota(0, 4)) {
            template for (constexpr auto mid : std::views::iota(0, 4)) {
                const auto entryA = getEntry<row, mid>(a);
                const auto entryB = getEntry<mid, col>(b);
                const auto entryC = getEntry<row, col>(c);
                c                 = setEntry<row, col>(c, entryC + entryA * entryB);
            }
        }
    }
    // core::forEach<0, 4, 0, 4, 0, 4>([&]<int row, int col, int mid>() {
    //     const auto entryA = getEntry<row, mid>(a);
    //     const auto entryB = getEntry<mid, col>(b);
    //     const auto entryC = getEntry<row, col>(c);
    //     c                 = setEntry<row, col>(c, entryC + entryA * entryB);
    // });
    return c;
}


template<typename V, Layout l>
    requires(l != 0)
class Matrix {
public:
    using Value                    = V;
    static constexpr Layout layout = l;
    constexpr Matrix()
        : data {} { };

    template<typename... Vs>
    constexpr Matrix(Vs... vs)
        : data { vs... } {
    }

    template<uint8_t row, uint8_t col, typename VV, Layout ll>
    friend constexpr auto& get(const Matrix<VV, ll>& m);

    template<uint8_t row, uint8_t col, typename VV, Layout ll>
        requires(getEntry<row, col>(ll) == Entry::any)
    friend constexpr auto& get(Matrix<VV, ll>& m);

private:
    std::array<Value, count(layout, Entry::any)> data;
};

template<typename Value, Layout layout>
struct Dispatch {
    using Type = Matrix<Value, layout>;
};

template<uint8_t row, uint8_t col, Layout layout>
consteval uint8_t index() {
    int idx { -1 };

    template for (constexpr auto n : std::views::iota(0, row * 4 + col + 1)) {
        const auto e = entry(layout, n);
        if (e == Entry::any) {
            ++idx;
        }
    }
    if (idx > -1) {
        return idx;
    }
    std::unreachable();
    // return idx > -1 ? idx : throw("bad");
}

template<typename T>
static constexpr T zero = T { 0 };

template<typename T>
static constexpr T one = T { 1 };


template<uint8_t row, uint8_t col, typename V, Layout l>
constexpr auto& get(const Matrix<V, l>& m) {
    constexpr auto e = getEntry<row, col>(l);
    if constexpr (e == Entry::zero) {
        return zero<V>;
    } else if constexpr (e == Entry::pone) {
        return one<V>;
    } else if constexpr (e == Entry::mone) {
        return -one<V>;
    } else {
        return m.data[index<row, col, l>()];
    }
}

template<uint8_t row, uint8_t col, typename Value, Layout layout>
    requires(getEntry<row, col>(layout) == Entry::any)
constexpr auto& get(Matrix<Value, layout>& m) {
    return m.data[index<row, col, layout>()];
}

constexpr Layout encodeLayout(const std::array<std::array<Entry, 4>, 4>& table) {
    Layout layout {};
    template for (constexpr auto row : std::views::iota(0, 4)) {
        template for (constexpr auto col : std::views::iota(0, 4)) {
            layout = setEntry<row, col>(layout, table[row][col]);
        }
    }
    // core::forEach<0, 4, 0, 4>([&]<int i, int j>() { layout = setEntry<i, j>(layout, table[i][j]); });
    return layout;
}

constexpr auto translationLayout = encodeLayout({
    //           Entry::zero  Entry::pone  Entry::mone  Entry::any
    std::array { Entry::pone, Entry::zero, Entry::zero, Entry::any  }, // Entry::zero
    std::array { Entry::zero, Entry::pone, Entry::zero, Entry::any  }, // Entry::pone
    std::array { Entry::zero, Entry::zero, Entry::pone, Entry::any  }, // Entry::mone
    std::array { Entry::zero, Entry::zero, Entry::zero, Entry::pone }, // Entry::any
});

template<typename Value = float>
class Translation : public Matrix<Value, translationLayout> {
public:
    constexpr Translation()                   = default;
    constexpr Translation(const Translation&) = default;
    constexpr Translation(Translation&&)      = default;
    constexpr Translation(const Value x, const Value y, const Value z)
        : Matrix<Value, translationLayout> { x, y, z } {
    }
};

template<typename Value>
struct Dispatch<Value, translationLayout> {
    using Type = Translation<Value>;
};

// template<typename Value, Layout layout>
// constexpr auto dispatch() {
//     if constexpr (layout == translationLayout) {
//         return Translation<Value> {};
//     } else {
//         return Matrix<Value, layout> {};
//     }
// }

template<typename ValueA, Layout layoutA, typename ValueB, Layout layoutB>
constexpr auto operator*(const Matrix<ValueA, layoutA>& A, const Matrix<ValueB, layoutB>& B) {
    constexpr auto layoutC = multiply(layoutA, layoutB);

    using CommonValue = std::common_type_t<ValueA, ValueB>;
    using MatrixC     = typename Dispatch<CommonValue, layoutC>::Type;
    MatrixC C;
    template for (constexpr auto row : std::views::iota(0, 4)) {
        template for (constexpr auto col : std::views::iota(0, 4)) {
            template for (constexpr auto mid : std::views::iota(0, 4)) {
                if constexpr (getEntry<row, col>(layoutC) == Entry::any) {
                    constexpr auto entryA = getEntry<row, mid>(layoutA);
                    constexpr auto entryB = getEntry<mid, col>(layoutB);
                    auto&          valueC = get<row, col>(C);
                    if constexpr (entryA == Entry::any && entryB != Entry::any) {
                        const auto&    valueA = get<row, mid>(A);
                        constexpr auto valueB = get<mid, col>(B);
                        valueC += valueA * valueB;
                    } else if constexpr (entryA != Entry::any && entryB == Entry::any) {
                        constexpr auto valueA = get<row, mid>(A);
                        const auto&    valueB = get<mid, col>(B);
                        valueC += valueA * valueB;
                    } else if constexpr (entryA == Entry::any && entryB == Entry::any) {
                        const auto& valueA = get<row, mid>(A);
                        const auto& valueB = get<mid, col>(B);
                        valueC += valueA * valueB;
                    }
                }
            }
        }
    }
    // core::forEach<0, 4, 0, 4, 0, 4>([&]<uint8_t row, uint8_t col, uint8_t mid>() {
    //     if constexpr (getEntry<row, col>(layoutC) == Entry::any) {
    //         constexpr auto entryA = getEntry<row, mid>(layoutA);
    //         constexpr auto entryB = getEntry<mid, col>(layoutB);
    //         auto&          valueC = get<row, col>(C);
    //         if constexpr (entryA == Entry::any && entryB != Entry::any) {
    //             const auto&    valueA = get<row, mid>(A);
    //             constexpr auto valueB = get<mid, col>(B);
    //             valueC += valueA * valueB;
    //         } else if constexpr (entryA != Entry::any && entryB == Entry::any) {
    //             constexpr auto valueA = get<row, mid>(A);
    //             const auto&    valueB = get<mid, col>(B);
    //             valueC += valueA * valueB;
    //         } else if constexpr (entryA == Entry::any && entryB == Entry::any) {
    //             const auto& valueA = get<row, mid>(A);
    //             const auto& valueB = get<mid, col>(B);
    //             valueC += valueA * valueB;
    //         }
    //     }
    // });
    return C;
}


using Identity = Matrix<float, encodeLayout({
                                   //           Entry::zero  Entry::pone  Entry::mone  Entry::any
                                   std::array { Entry::pone, Entry::zero, Entry::zero, Entry::zero }, // Entry::zero
                                   std::array { Entry::zero, Entry::pone, Entry::zero, Entry::zero }, // Entry::pone
                                   std::array { Entry::zero, Entry::zero, Entry::pone, Entry::zero }, // Entry::mone
                                   std::array { Entry::zero, Entry::zero, Entry::zero, Entry::pone }, // Entry::any
})>;
constexpr Identity identity;

// using Translation = Matrix<float, layout({
//                                       //           Entry::zero  Entry::pone  Entry::mone  Entry::any
//                                       std::array { Entry::pone, Entry::zero, Entry::zero, Entry::any  }, //
//                                       Entry::zero std::array { Entry::zero, Entry::pone, Entry::zero, Entry::any  },
//                                       // Entry::pone std::array { Entry::zero, Entry::zero, Entry::pone, Entry::any
//                                       }, // Entry::mone std::array { Entry::zero, Entry::zero, Entry::zero,
//                                       Entry::pone }, // Entry::any
// })>;


template<typename Value, Layout layout>
std::ostream& operator<<(std::ostream& stream, const Matrix<Value, layout>& matrix) {
    template for (constexpr auto row : std::views::iota(0, 4)) {
        template for (constexpr auto col : std::views::iota(0, 4)) {
            stream << " " << get<row, col>(matrix);
        }
        stream << std::endl;
    }
    return stream;
}

template<typename Mat>
bool testMat() {
    static_assert(multiply(Mat::layout, Mat::layout) == Mat::layout);
    static_assert(multiply(Identity::layout, Mat::layout) == Mat::layout);
    static_assert(multiply(Mat::layout, Identity::layout) == Mat::layout);

    constexpr Mat cm1;
    constexpr Mat cm2;
    static_assert((cm1 * cm1).layout == Mat::layout);
    // static_assert(typ)

    static_assert((cm1 * identity).layout == Mat::layout);
    static_assert((identity * cm1).layout == Mat::layout);

    static_assert((cm1 * cm2).layout == Mat::layout);
    static_assert((cm2 * cm1).layout == Mat::layout);

    return true;
}

void test() {
    using namespace tiamat;

    // Matrix
    // static_assert(get<0, 0>(identity) == 1);
    static_assert(get<1, 0>(identity) == 0);
    static_assert(get<0, 1>(identity) == 0);
    // static_assert(get<1, 1>(identity) == 1);
    // static_assert(sizeof(identity) == 1);
    std::cout << get<0, 0>(identity) << std::endl;

    std::cout << identity;


    testMat<Translation<>>();
    Translation<> translation;
    static_assert(sizeof(Translation<>) == 3 * sizeof(float));
    static_assert(multiply(Translation<>::layout, Translation<>::layout) == Translation<>::layout);
    static_assert(multiply(Identity::layout, Translation<>::layout) == Translation<>::layout);
    static_assert(multiply(Translation<>::layout, Identity::layout) == Translation<>::layout);
    constexpr Translation<> cTr;
    static_assert((cTr * cTr).layout == Translation<>::layout);
    static_assert((identity * cTr).layout == Translation<>::layout);
    static_assert((cTr * identity).layout == Translation<>::layout);
    get<0, 3>(translation) = 1.4;
    constexpr auto t00     = get<0, 0>(translation);
    static_assert(t00 == 1);
    static_assert(get<0, 0>(translation) == 1);
    static_assert(get<0, 1>(translation) == 0);
    {
        // constexpr Translation t1 = std::invoke([] {
        //     Translation t;
        //     get<0, 3>(t) = 1;
        //     get<1, 3>(t) = 2;
        //     get<2, 3>(t) = 3;
        //     return t;
        // });
        constexpr Translation t1 { 1, 2, 3 };
        static_assert(get<0, 3>(t1 * identity) == 1);
        static_assert(get<1, 3>(t1) == 2);
        static_assert(get<2, 3>(t1) == 3);
        std::cout << "\n" << t1;

        constexpr Translation t2 { 2, 3, 4 };
        std::cout << "\n" << t2;
        constexpr auto t3 = t1 * t2;
        std::cout << "\n" << t3;
        static_assert(get<0, 3>(t3) == 3);
        static_assert(get<1, 3>(t3) == 5);
        static_assert(get<2, 3>(t3) == 7);
        // const auto g = glm::transpose();
    }

    Translation translation2;
    get<0, 3>(translation) = 2;

    static_assert(get<0, 0>(translation) == 1);
    static_assert(get<1, 0>(translation) == 0);
    static_assert(get<0, 1>(translation) == 0);
    static_assert(get<1, 1>(translation) == 1);
    // static_assert(get<0, 3>(translation) == 0);
    // get<0,3>
    // static_assert(sizeof(identity) == 1);

    using Rotation = Matrix<float, encodeLayout({
                                       //           Entry::zero  Entry::pone  Entry::mone  Entry::any
                                       std::array { Entry::any,  Entry::any,  Entry::any,  Entry::zero }, // Entry::zero
                                       std::array { Entry::any,  Entry::any,  Entry::any,  Entry::zero }, // Entry::pone
                                       std::array { Entry::any,  Entry::any,  Entry::any,  Entry::zero }, // Entry::mone
                                       std::array { Entry::zero, Entry::zero, Entry::zero, Entry::pone }, // Entry::any
    })>;
    testMat<Rotation>();
    Rotation rotation;
    static_assert(sizeof(Rotation) == 9 * sizeof(float));
    static_assert(multiply(Rotation::layout, Rotation::layout) == Rotation::layout);
    static_assert(multiply(Identity::layout, Rotation::layout) == Rotation::layout);
    static_assert(multiply(Rotation::layout, Identity::layout) == Rotation::layout);
    constexpr Rotation cRot;
    static_assert((cRot * cRot).layout == Rotation::layout);
    static_assert((identity * cRot).layout == Rotation::layout);
    static_assert((cRot * identity).layout == Rotation::layout);

    using Scale = Matrix<float, encodeLayout({
                                    //           Entry::zero  Entry::pone  Entry::mone  Entry::any
                                    std::array { Entry::any,  Entry::zero, Entry::zero, Entry::zero }, // Entry::zero
                                    std::array { Entry::zero, Entry::any,  Entry::zero, Entry::zero }, // Entry::pone
                                    std::array { Entry::zero, Entry::zero, Entry::any,  Entry::zero }, // Entry::mone
                                    std::array { Entry::zero, Entry::zero, Entry::zero, Entry::pone }, // Entry::any
    })>;
    Scale scale;
    testMat<Scale>();
    {
        constexpr auto s1 = std::invoke([] {
            Scale s;
            get<0, 0>(s) = 1;
            get<1, 1>(s) = 2;
            get<2, 2>(s) = 3;
            return s;
        });
        static_assert(get<0, 0>(s1 * identity) == 1);
        static_assert(get<1, 1>(s1) == 2);
        static_assert(get<2, 2>(s1) == 3);

        constexpr auto s2 = std::invoke([] {
            Scale s;
            get<0, 0>(s) = 2;
            get<1, 1>(s) = 3;
            get<2, 2>(s) = 4;
            return s;
        });
        constexpr auto s3 = s1 * s2;
        static_assert(get<0, 0>(s3) == 2);
        static_assert(get<1, 1>(s3) == 6);
        static_assert(get<2, 2>(s3) == 12);
    }
    static_assert(sizeof(Scale) == 3 * sizeof(float));
    static_assert(multiply(Scale::layout, Scale::layout) == Scale::layout);
    static_assert(multiply(Identity::layout, Scale::layout) == Scale::layout);
    static_assert(multiply(Scale::layout, Identity::layout) == Scale::layout);


    using Transformation =
        Matrix<float, encodeLayout({
                          //           Entry::zero  Entry::pone  Entry::mone  Entry::any
                          std::array { Entry::any,  Entry::any,  Entry::any,  Entry::any  }, // Entry::zero
                          std::array { Entry::any,  Entry::any,  Entry::any,  Entry::any  }, // Entry::pone
                          std::array { Entry::any,  Entry::any,  Entry::any,  Entry::any  }, // Entry::mone
                          std::array { Entry::zero, Entry::zero, Entry::zero, Entry::pone }, // Entry::any
    })>;
    testMat<Transformation>();
    Transformation transformation;
    static_assert(sizeof(Transformation) == 12 * sizeof(float));
    static_assert(multiply(Transformation::layout, Transformation::layout) == Transformation::layout);
    static_assert(multiply(Identity::layout, Transformation::layout) == Transformation::layout);
    static_assert(multiply(Transformation::layout, Identity::layout) == Transformation::layout);
    static_assert(multiply(multiply(Translation<>::layout, Rotation::layout), Scale::layout) == Transformation::layout);
    static_assert(multiply(multiply(Translation<>::layout, Rotation::layout), Identity::layout) ==
                  Transformation::layout);
    static_assert(multiply(Translation<>::layout, multiply(Scale::layout, Rotation::layout)) == Transformation::layout);

    static_assert(multiply(Translation<>::layout, multiply(Rotation::layout, Scale::layout)) == Transformation::layout);
    static_assert(multiply(Translation<>::layout, multiply(Rotation::layout, Identity::layout)) ==
                  Transformation::layout);
    // std::exit(0);
};

}  // namespace tiamat

int main() {
    try {
        tiamat::test();
    } catch (const std::exception& e) {
        std::cerr << "\033[1;31m[ERROR]\033[0m" << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "\033[1;31m[ERROR]\033[0m" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
