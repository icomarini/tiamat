cmake_minimum_required(VERSION 3.27)
project(tiamat VERSION 0.01)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# tiamat =======================================================================

add_executable (tiamat.bin main.cpp)
set_property(TARGET tiamat.bin PROPERTY CXX_STANDARD 26)

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
