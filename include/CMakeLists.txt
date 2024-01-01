cmake_minimum_required(VERSION 3.21)

set(sources 
    "${CMAKE_CURRENT_SOURCE_DIR}/fox/reflexpr.hpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

add_library(
    reflexpr
    INTERFACE
    ${sources}
)

if(${IPO_SUPPORTED})
    set_target_properties(reflexpr PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_include_directories(
    reflexpr
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)