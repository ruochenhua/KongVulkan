cmake_minimum_required(VERSION 3.5)

project(3rdparty)

set(CMAKE_CXX_EXTENSIONS 17)
set(3RD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# some thirdparty library
set(GLFW_DIR ${3RD_PARTY_DIR}/glfw)
set(GLM_DIR ${3RD_PARTY_DIR}/glm)
set(YAML_DIR ${3RD_PARTY_DIR}/yaml)
set(ASSIMP_DIR ${3RD_PARTY_DIR}/assimp)

# build glfw
add_subdirectory(${GLFW_DIR})

# build glm
add_subdirectory(${GLM_DIR})

# build assimp
set(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(${ASSIMP_DIR})

# build yaml
set(YAML_CPP_BUILD_TOOLS OFF)
set(YAML_CPP_BUILD_TESTS OFF)
add_subdirectory(${YAML_DIR})

