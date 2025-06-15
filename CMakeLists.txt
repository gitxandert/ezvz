cmake_minimum_required(VERSION 3.15)
project(AudioVisualizerRawStack)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Vendored GLM ───
add_subdirectory(external/glm)   # provides the target "glm"

# ─── Source files ───
set(SOURCES
    src/Animation.cpp
    src/AnimationInfo.cpp
    src/AnimationPath.cpp
    src/Canvas.cpp
    src/FileDialogHelper.cpp
    src/Ellipse.cpp
    src/GlobalTransport.cpp
    src/GraphicObject.cpp
    src/Line.cpp
    src/MappingsWindow.cpp
    src/Star.cpp
    src/Timeline.cpp
    src/TimelineTrack.cpp
    src/TrackFeatures.cpp
    src/Triangle.cpp
    src/main.cpp
    src/Rectangle.cpp
    src/ScenesPanel.cpp
    src/Shader.cpp
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

file(GLOB SHADERS "${CMAKE_SOURCE_DIR}/shaders/*.glsl")
foreach(_shader IN LISTS SHADERS)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_shader}"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/
  )
endforeach()

# ─── Include paths ───
target_include_directories(${PROJECT_NAME} PRIVATE
    include
    include/imgui
    include/imgui/backends
    include/glfw/include
    include/glad/include
    include/dirent
)

# ─── ImGui & Glad sources ───
target_sources(${PROJECT_NAME} PRIVATE
    include/imgui/imgui.cpp
    include/imgui/imgui_demo.cpp
    include/imgui/imgui_draw.cpp
    include/imgui/imgui_tables.cpp
    include/imgui/imgui_widgets.cpp
    include/imgui/backends/imgui_impl_glfw.cpp
    include/imgui/backends/imgui_impl_opengl3.cpp
    include/glad/src/glad.c
    include/ImGuiFileDialog.cpp
)

# ─── Libraries ───
link_directories(${PROJECT_SOURCE_DIR}/lib)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glm                        # from add_subdirectory(external/glm)
    ${PROJECT_SOURCE_DIR}/lib/glfw3.lib
    opengl32
)

if (MSVC)
  # Print every #include as the compiler sees it
  target_compile_options(AudioVisualizerRawStack PRIVATE /showIncludes)
endif()