cmake_minimum_required(VERSION 3.10)

set(ENGINE_PROJECT_NAME Engine)
project(${ENGINE_PROJECT_NAME} VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

set(ENGINE_PUBLIC_INCLUDES
    core/public/engine.hpp
    core/public/engine_logs.hpp
)

set(ENGINE_PRIVATE_INCLUDES
    core/private/engine.cpp
)

set(IMGUI_INCLUDES
    core/imgui/imgui_impl_glfw.h
    core/imgui/imgui_impl_vulkan.h
    core/imgui/imgui_internal.h
    core/imgui/imstb_textedit.h
    core/imgui/imstb_rectpack.h
    core/imgui/imstb_truetype.h
    core/imgui/imgui.h
    core/imgui/imconfig.h
    core/imgui/imgui_draw.cpp
    core/imgui/imgui_impl_glfw.cpp
    core/imgui/imgui_impl_vulkan.cpp
    core/imgui/imgui_tables.cpp
    core/imgui/imgui_widgets.cpp
    core/imgui/imgui.cpp
)

set(IMGUI core/imgui)

# Find Vulkan libraries
find_package(Vulkan REQUIRED)

add_executable(Engine 
    ${ENGINE_PRIVATE_INCLUDES}
    ${ENGINE_PUBLIC_INCLUDES}
    ${IMGUI_INCLUDES}
)

target_include_directories(${ENGINE_PROJECT_NAME} PUBLIC includes)

# Find and link glfw libraries
add_subdirectory(external/glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw)
target_link_libraries(${ENGINE_PROJECT_NAME} PRIVATE glfw)

add_subdirectory(external/glm ${CMAKE_CURRENT_BINARY_DIR}/glm)
target_link_libraries(${ENGINE_PROJECT_NAME} PRIVATE glm)

# Find and link spdlog libraries
add_subdirectory(external/spdlog ${CMAKE_CURRENT_BINARY_DIR}/spdlog)
target_link_libraries(${ENGINE_PROJECT_NAME} PRIVATE spdlog)

# Link Vulkan libraries
target_link_libraries(${ENGINE_PROJECT_NAME} PRIVATE Vulkan::Vulkan)