cmake_minimum_required(VERSION 3.20)

project(KongVulkan)

set(CMAKE_CXX_EXTENSIONS 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR src)
set(3RD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)

add_subdirectory(${3RD_PARTY_DIR})
find_package(Vulkan REQUIRED FATAL_ERROR)

# imgui编译
# set(IMGUI_DIR ${3RD_PARTY_DIR}/imgui)
# file(GLOB IMGUI_SRC ${IMGUI_DIR}/*.cpp)
# file(GLOB IMGUI_BACKEND_SRC ${IMGUI_DIR}/backends/*.cpp)

file(GLOB_RECURSE MAIN_SRC ${SRC_DIR}/*.cpp)
file(GLOB_RECURSE MAIN_HEAD ${SRC_DIR}/*.h)



# add_executable(KongVulkan ${MAIN_HEAD} ${MAIN_SRC} ${IMGUI_SRC} ${IMGUI_BACKEND_SRC})
add_executable(KongVulkan ${MAIN_HEAD} ${MAIN_SRC})
# include_directories(KongEngine ${3RD_PARTY_DIR})
include_directories(KongVulkan src)
include_directories(KongVulkan ${3RD_PARTY_DIR})

# target_include_directories(KongVulkan PRIVATE ${IMGUI_DIR})
# target_include_directories(KongVulkan PRIVATE ${IMGUI_DIR}/backends)

# link library
target_include_directories(KongVulkan PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(KongVulkan Vulkan::Vulkan)
target_link_libraries(KongVulkan glfw)
target_link_libraries(KongVulkan glm_static)
target_link_libraries(KongVulkan assimp)
target_link_libraries(KongVulkan yaml-cpp)


# include(FetchContent)
#FetchContent_Declare(
#    fetch_vk_bootstrap
#    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
#    GIT_TAG        main
#)
#FetchContent_MakeAvailable(fetch_vk_bootstrap)
#target_link_libraries(KongVulkan vk-bootstrap::vk-bootstrap)