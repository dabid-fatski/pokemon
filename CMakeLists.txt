cmake_minimum_required(VERSION 3.16)

project(pokedex LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_executable(pokedex
    main.cpp
    updatePokedex.cpp
    getPokedexInfo.cpp
    genMap.cpp
    window.cpp
)

target_link_libraries(pokedex PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Qt6::Widgets
)