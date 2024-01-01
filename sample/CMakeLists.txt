cmake_minimum_required(VERSION 3.21)

set(sources 
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

add_executable(
    reflexpr-demo
    ${sources}
)

if(${IPO_SUPPORTED})
    set_target_properties(reflexpr-demo PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(MSVC)
    target_compile_options(
        reflexpr-demo
        PRIVATE /WX # all warnings as errors 
        PRIVATE /MP # multi-processor compilation
    )
endif()

target_include_directories(
    reflexpr-demo
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    reflexpr-demo
    reflexpr
)