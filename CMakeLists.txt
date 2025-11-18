cmake_minimum_required(VERSION 3.20)

project(OstenEngine VERSION 0.1 LANGUAGES CXX)



set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Vulkan REQUIRED)
if (${UNIX})
    find_package(X11 REQUIRED)
endif (${UNIX})
# find_package(imgui)


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

add_subdirectory(external/glm)
# target_include_directories(external/imgui)

# Set where the ImGui files are stored
set(IMGUI_PATH  "external/imgui")
    
# Compile as static library 
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp) 
add_library("imgui" STATIC ${IMGUI_SOURCES})
target_include_directories("imgui" PUBLIC ${IMGUI_PATH})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build) 

add_executable(${PROJECT_NAME} 
    main.cpp
    src/renderer/model_loader/model_loader.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_vulkan.cpp
    src/application.cpp
    src/renderer/instance/vulkan/instance.cpp
    src/renderer/device/vulkan/device.cpp
    src/renderer/swap_chain/vulkan/swap_chain.cpp
    src/renderer/render_data/vulkan/render_data.cpp
    src/renderer/render_pipeline.cpp
    src/renderer/texture/vulkan/texture.cpp
    src/engine/entity_manager/entity_manager.cpp
    src/engine/transform.cpp
    )

# target_include_directories(${PROJECT_NAME} PRIVATE ${OstenEngine_INCLUDE_DIRS})
if (${UNIX})
    target_link_libraries(${PROJECT_NAME} 
        PRIVATE 
        Vulkan::Vulkan 
        glfw 
        glm::glm
        PUBLIC
        imgui
        X11
        )    
endif (${UNIX})

target_link_libraries(${PROJECT_NAME} 
        PRIVATE 
        Vulkan::Vulkan 
        glfw 
        glm::glm
        PUBLIC
        imgui
        )  
