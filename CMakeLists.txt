cmake_minimum_required(VERSION 3.27)
project(tiamat VERSION 0.01)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# glm ========================================================================
find_package(glm CONFIG REQUIRED)

# 2. Force GLM to be treated as a SYSTEM library
# This overrides the imported target properties for your compilation unit
set_target_properties(glm::glm PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES 
    $<TARGET_PROPERTY:glm::glm,INTERFACE_INCLUDE_DIRECTORIES>
)

# tiamat =======================================================================
add_executable (tiamat.bin main.cpp)
set_property(TARGET tiamat.bin PROPERTY CXX_STANDARD 26)
target_link_libraries(tiamat.bin PRIVATE glm::glm)

target_include_directories(tiamat.bin PUBLIC ${PROJECT_SOURCE_DIR}/include)
# target_link_libraries(tiamat.bin LINK_PUBLIC
#     # vulkan
#     # glfw
#     # stb
#     # tinyobjloader
#     # imgui
#     # ktx
#     # fastgltf
#     # simdjson
#     # miniaudio 
#     # clay
#     )
