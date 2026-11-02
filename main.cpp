#include "tiamat/tiamat.hpp"

#include <iostream>
#include <functional>
#include <cassert>
#include <chrono>

#define GLM_FORCE_INTRINSICS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

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

void testVector0() {
    // (n,n) * (n,1) = (n,1)
    using namespace tiamat;
    constexpr auto x = Translation<int, 3> { 1, 2, 3 } * Translation<int, 3> { 1, 2, 3 } * identity<int, 4> *
                       ColumnVector<int, 4> { 0, 0, 0, 1 };
    static_assert(decltype(x)::sparsity.rows == 4);
    static_assert(decltype(x)::sparsity.columns == 1);
    static_assert(IsVector<decltype(x)>);
    static_assert(std::is_same_v<std::decay_t<decltype(x)>, ColumnVector<int, 4>>);
    static_assert(valid<decltype(x)::sparsity, 0>());
    static_assert(get<0>(x) == 2);
    static_assert(get<1>(x) == 4);
    static_assert(get<2>(x) == 6);
    static_assert(get<3>(x) == 1);
    // static_assert(get<0>(x) == 1);
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
    { const Rotation<float, 3> rotation; }
}


std::ostream& operator<<(std::ostream& stream, const glm::mat4& matrix) {
    template for (constexpr auto row : std::views::indices(4)) {
        template for (constexpr auto column : std::views::indices(4)) {
            stream << " " << matrix[column][row];
        }
        stream << std::endl;
    }
    return stream;
}

namespace tiamat {
template<IsMatrix Mat>
std::ostream& operator<<(std::ostream& stream, const Mat& matrix) {
    // constexpr auto sparsity = Mat::sparsity;
    template for (constexpr auto row : std::views::indices(Mat::sparsity.rows)) {
        template for (constexpr auto column : std::views::indices(Mat::sparsity.columns)) {
            stream << " " << get<row, column>(matrix);
        }
        stream << std::endl;
    }
    return stream;
}
}  // namespace tiamat

// A typical struct for a 3D vertex
struct glmVertex {
    glm::vec3 position;
    glm::vec3 normal;
};

void transformVertices(const std::vector<glmVertex>& sourceVertices, std::vector<glmVertex>& outVertices,
                       const glm::mat4& modelMatrix, const glm::mat3& normalMatrix) {
    size_t vertexCount = sourceVertices.size();

    // An intensive loop processing thousands of elements sequentially
    for (size_t i = 0; i < vertexCount; ++i) {
        // 1. Transform positions (requires expanding vec3 to vec4 for matrix math)
        glm::vec4 localPos      = glm::vec4(sourceVertices[i].position, 1.0f);
        glm::vec4 worldPos      = modelMatrix * localPos;
        outVertices[i].position = glm::vec3(worldPos);

        // 2. Transform normals and re-normalize to ensure standard length
        outVertices[i].normal = glm::normalize(normalMatrix * sourceVertices[i].normal);
    }
}

namespace tiamat {
template<IsMatrix Mat>
    requires(Mat::sparsity.rows == 4 && Mat::sparsity.columns == 4)
bool equal(const glm::mat4 glm, const Mat& mat, const double tolerance = 1e-10) {
    template for (constexpr auto row : std::views::indices(Mat::sparsity.rows)) {
        template for (constexpr auto column : std::views::indices(Mat::sparsity.columns)) {
            if (std::abs(get<row, column>(mat) - glm[column][row]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

bool equal(const glm::quat glm, const Quaternion<float>& quat, const double tolerance = 1e-10) {
    return std::abs(quat.x - glm.x) < tolerance &&  //
           std::abs(quat.y - glm.y) < tolerance &&  //
           std::abs(quat.z - glm.z) < tolerance &&  //
           std::abs(quat.w - glm.w) < tolerance;
}
}  // namespace tiamat

auto glmMatrices() {
    glm::vec3 translationVector(5.0f, -2.5f, 10.0f);
    glm::mat4 T = glm::translate(glm::mat4(1.0f), translationVector);

    glm::vec3 scaleVector(2.0f, 0.5f, 1.5f);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), scaleVector);


    float     rotationAngle = glm::radians(45.0f);
    glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
    glm::quat rotationQuat = glm::angleAxis(rotationAngle, rotationAxis);
    glm::mat4 R            = glm::toMat4(rotationQuat);

    glm::mat4 modelMatrix  = T * R * S;
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    return std::make_pair(modelMatrix, normalMatrix);
}

void glmWork(const int size) {
    std::vector<glmVertex> sourceVertices(size);
    for (auto& vertex : sourceVertices) {
        vertex.position[0] = 1.1f;
        vertex.position[0] = 1.2f;
        vertex.position[0] = 1.3f;
        vertex.normal[0]   = 1.4f;
        vertex.normal[0]   = 1.5f;
        vertex.normal[0]   = 1.6f;
    }
    std::vector<glmVertex> outVertices(size);
    const auto [modelMatrix, normalMatrix] = glmMatrices();
    transformVertices(sourceVertices, outVertices, modelMatrix, normalMatrix);
}

void testGlm() {
    glm::mat4 glmIdentity = glm::mat4(1.0f);
    assert(equal(glmIdentity, tiamat::identity<float, 4>));

    const float                   trX            = 10.0f;
    const float                   trY            = 0.0f;
    const float                   trZ            = 5.0f;
    glm::mat4                     glmTranslation = glm::translate(glmIdentity, glm::vec3(trX, trY, trZ));
    tiamat::Translation<float, 3> tmtTranslation { trX, trY, trZ };
    assert(equal(glmTranslation, tmtTranslation));

    const float             scX      = 2.0f;
    const float             scY      = 0.5f;
    const float             scZ      = 1.5f;
    glm::mat4               glmScale = glm::scale(glm::mat4(1.0f), glm::vec3(scX, scY, scZ));
    tiamat::Scale<float, 3> tmtScale { scX, scY, scZ };
    assert(equal(glmScale, tmtScale));
    assert(equal(glmTranslation * glmScale, tmtTranslation * tmtScale));
    assert(equal(glmScale * glmTranslation, tmtScale * tmtTranslation));

    const float rtA = 40.0f;
    const float rtX = 1.0f;
    const float rtY = 1.0f;
    const float rtZ = 1.0f;
    // assert(equal(glm::normalize(glm::vec3(rtX, rtY, rtZ)),
    //              tiamat::normalize(tiamat::RowVector<float, 3> { rtX, rtY, rtZ })));
    glm::quat                 glmQuaternion = glm::angleAxis(glm::radians(rtA), glm::vec3(rtX, rtY, rtZ));
    glm::mat4                 glmRotation   = glm::toMat4(glmQuaternion);
    tiamat::Quaternion<float> tmtQuaternion {
        tiamat::radians(rtA), tiamat::RowVector<float, 3> { rtX, rtY, rtZ }
    };
    tiamat::Rotation<float, 3> tmtRotation { tmtQuaternion };
    // assert(equal(glmQuaternion, tmtQuaternion));
    // assert(equal(glmRotation, tmtRotation));

    std::cout << "glm rotation:" << std::endl;
    std::cout << glmRotation;
    std::cout << "tmt rotation:" << std::endl;
    std::cout << tmtRotation;


    const int iterations = 1000000;
    {
        std::vector<glm::mat4> v(iterations);
        auto                   start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; i++) {
            v[i] = glmTranslation * glmTranslation;
        }
        auto finish   = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
        std::cout << "glm took " << duration << " milliseconds" << std::endl;
    }

    // {
    //     auto start = std::chrono::high_resolution_clock::now();
    //     glmWork(100000);
    //     auto finish   = std::chrono::high_resolution_clock::now();
    //     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    //     std::cout << "glm took " << duration << " milliseconds" << std::endl;
    // }

    {
        std::vector<tiamat::Translation<float, 3>> v(iterations);
        auto                                       start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; i++) {
            v[i] = tmtTranslation * tmtTranslation;
        }
        auto finish   = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
        std::cout << "tmt took " << duration << " milliseconds" << std::endl;
    }
}

void test() {
    testSparsity0();
    testSparsity1();
    testMatrix0();
    testTranslation0();
    testRotation0();
    testGlm();
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
