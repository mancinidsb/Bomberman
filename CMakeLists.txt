cmake_minimum_required(VERSION 3.10)
project(Bomberman3D)

# Define C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES main.cpp)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Platform-specific configurations
if(WIN32)
    # Windows
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)
    target_link_libraries(${PROJECT_NAME} OpenGL::GL GLUT::GLUT)
    
elseif(APPLE)
    # macOS
    find_package(OpenGL REQUIRED)
    find_library(GLUT_LIBRARY GLUT)
    find_path(GLUT_INCLUDE_DIR GLUT/glut.h)
    
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLUT_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} 
        "-framework OpenGL" 
        "-framework GLUT"
        ${GLUT_LIBRARY}
    )
    
else()
    # Linux
    find_package(OpenGL REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLUT REQUIRED glut)
    
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLUT_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} 
        OpenGL::GL 
        ${GLUT_LIBRARIES}
    )
    target_link_directories(${PROJECT_NAME} PRIVATE ${GLUT_LIBRARY_DIRS})
endif()

# Copy assets to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

# Set compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE 
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
)

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
install(DIRECTORY assets DESTINATION share/${PROJECT_NAME}) 