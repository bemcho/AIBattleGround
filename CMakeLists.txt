cmake_minimum_required (VERSION 3.2.3)

project(AIBattleGround)
set (CMAKE_CXX_FLAGS "  -v -m64  -O3 -Weverything  -Wno-c++98-compat -Wno-global-constructors -Wno-exit-time-destructors -Wno-padded -Wno-missing-prototypes -Wno-missing-variable-declarations")
set( CMAKE_PREFIX_PATH /usr/local)
file(GLOB SOURCE_MAIN
        "${CMAKE_SOURCE_DIR}/Source/Base/*.hpp"
        "${CMAKE_SOURCE_DIR}/Source/Base/*.inl"
        "${CMAKE_SOURCE_DIR}/*.hpp"
        "${CMAKE_SOURCE_DIR}/*.cpp"
        "${CMAKE_SOURCE_DIR}/Source/Intro/*.cpp"
        "${CMAKE_SOURCE_DIR}/Source/Intro/*.hpp"
        )
set(CMAKE_COLOR_MAKEFILE TRUE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)
######################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/bin")
# Setup target with resource copying
#find_library(Urho3D_LIBRARY NAMES Urho3D REQUIRED HINT /usr/local/)
#find_path(URHO3D_INCLUDE_DIRS NAMES Urho3D REQUIRED HINT /usr/local/include)
#if(Urho3D_LIBRARY AND URHO3D_INCLUDE_DIRS)
#    include_directories(${URHO3D_INCLUDE_DIRS})
#    include_directories(/usr/local/include/Urho3D/ThirdParty/)
#    include_directories(/usr/local/include/Urho3D/)
#    message("Urho3D includes Found: " ${URHO3D_INCLUDE_DIRS})
#    message("Urho3D lib Found: " ${Urho3D_LIBRARY})
#else()
#    message("Can not find Urho3D lib and includes!")
#endif()
# Include Urho3D Cmake common module
set(URHO3D_HOME /home/bemcho/.urho3d/install/linux-clang)
include (UrhoCommon)
# Define target name
set (TARGET_NAME AIBattleGround)

# Define source files
define_source_files (EXTRA_H_FILES ${COMMON_SAMPLE_H_FILES}
        GLOB_CPP_PATTERNS ${CMAKE_SOURCE_DIR}/AIBattleGroundApp.cpp ${CMAKE_SOURCE_DIR}/Source/*.cpp* ${CMAKE_SOURCE_DIR}/Source/Base/*.cpp* ${CMAKE_SOURCE_DIR}/Source/Intro/*.cpp*
        GLOB_H_PATTERNS ${CMAKE_SOURCE_DIR}/AIBattleGroundApp.hpp ${CMAKE_SOURCE_DIR}/Source/*.hpp ${CMAKE_SOURCE_DIR}/Source/Base/*.hpp  ${CMAKE_SOURCE_DIR}/Source/Intro/*.hpp  RECURSE GROUP)

# Setup target with resource copying
setup_main_executable ()
