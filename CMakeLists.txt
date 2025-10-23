cmake_minimum_required(VERSION 3.10.3)

project(opengl-model-viewer VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)

# add GLAD as a static library
add_library(glad STATIC 
    external/glad/src/glad.c
)

target_include_directories(glad PUBLIC 
    external/glad/include
)

add_executable(${PROJECT_NAME}
    source/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    glm::glm
    glfw
    glad
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

# platform-specific linking
# linux
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        GL
        X11
        pthread
        dl
    )
endif()

# macOS
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()

# windows
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        opengl32
    )
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        /W4
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall
        -Wextra
    )
endif()