cmake_minimum_required(VERSION 3.24)
project(opengl LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 QUIET)

# Use the installed GLFW when available; otherwise build it without a system install.
if(NOT TARGET glfw)
    include(FetchContent)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glfw
        URL https://codeload.github.com/glfw/glfw/tar.gz/refs/tags/3.4
        URL_HASH SHA256=c038d34200234d071fae9345bc455e4a8f2f544ab60150765d7704e08f3dac01
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(glfw)
endif()

add_executable(opengl
    src/main.cpp
    src_utils/camera.cpp
    src_utils/camera.h
    src_utils/utils.cpp
    src_utils/utils.h
    src_utils/stb_image.cpp
    src_utils/stb_image.h
)
target_compile_features(opengl PRIVATE cxx_std_17)
set_target_properties(opengl PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF
)
target_include_directories(opengl PRIVATE src src_utils include/glm)
target_link_libraries(opengl PRIVATE GLEW::GLEW glfw OpenGL::GL)

# Keep relative data/ paths usable from Qt Creator's build-directory run target.
add_custom_target(opengl_data ALL
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:opengl>/data"
    COMMENT "Copying OpenGL assets"
)
add_dependencies(opengl opengl_data)

include(CTest)
if(BUILD_TESTING)
    add_executable(camera_tests tests/camera_test.cpp src_utils/camera.cpp)
    target_compile_features(camera_tests PRIVATE cxx_std_17)
    target_include_directories(camera_tests PRIVATE src_utils)
    add_test(NAME camera_math COMMAND camera_tests)
endif()
