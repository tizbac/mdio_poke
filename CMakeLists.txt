cmake_minimum_required(VERSION 3.0)

project(mdio_poke)

add_executable(mdio_poke main.cpp)

install(TARGETS mdio_poke RUNTIME DESTINATION bin)
