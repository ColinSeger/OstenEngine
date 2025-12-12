cmake_minimum_required(VERSION 3.20)

# set(CMAKE_C_COMPILER gcc)
# set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


project(OstenEngine VERSION 0.1 LANGUAGES CXX)


find_package(Vulkan REQUIRED)
if (${UNIX})
    find_package(X11 REQUIRED)
endif (${UNIX})


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

# Set where the ImGui files are stored
set(IMGUI_PATH  "external/imgui")

# Compile as static library
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)
add_library("imgui" STATIC ${IMGUI_SOURCES})
target_include_directories("imgui" PUBLIC ${IMGUI_PATH})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

add_executable(${PROJECT_NAME}
    main.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_vulkan.cpp
    )

if (${UNIX})
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Vulkan::Vulkan
        glfw
        PUBLIC
        imgui
        X11
        )
endif (${UNIX})

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Vulkan::Vulkan
        glfw
        PUBLIC
        imgui
        )
