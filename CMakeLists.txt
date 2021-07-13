cmake_minimum_required(VERSION 3.19)
project(BriskAudio_main)

set(CMAKE_CXX_STANDARD 14)

include_directories(BriskAudio-main/include)
include_directories(BriskAudio-main/src)

add_executable(BriskAudio_main
        BriskAudio-main/include/BriskAudio.hpp
        BriskAudio-main/src/BriskAudioWindows.cpp
        BriskAudio-main/src/BriskAudioWindows.hpp
        BriskAudio-main/src/Test.cpp)
