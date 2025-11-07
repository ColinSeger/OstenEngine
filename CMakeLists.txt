cmake_minimum_required(VERSION 3.20)

project(OstenEngine VERSION 0.1 LANGUAGES CXX)



set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Vulkan REQUIRED)
# find_package(imgui)


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw-3.4)

add_subdirectory(external/glm)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build) 

add_executable(${PROJECT_NAME} 
    main.cpp 
    src/application.cpp
    src/renderer/instance/vulkan/instance.cpp
    src/renderer/device/vulkan/device.cpp
    src/renderer/swap_chain/vulkan/swap_chain.cpp
    src/renderer/render_data/vulkan/render_data.cpp
    src/renderer/render_pipeline.cpp
    )

# target_include_directories(${PROJECT_NAME} PRIVATE ${OstenEngine_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    Vulkan::Vulkan 
    glfw 
    glm::glm
    # imgui::imgui
    )
