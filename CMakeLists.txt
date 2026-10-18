cmake_minimum_required(VERSION 3.25)
include(FetchContent)

FetchContent_Declare(
    Geode
    GIT_REPOSITORY https://github.com/geode-sdk/geode.git
    GIT_TAG main
)
FetchContent_MakeAvailable(Geode)

project(DynamicInputBuffer VERSION 1.0.0)

add_geode_mod(
    DynamicInputBuffer
    src/main.cpp
)