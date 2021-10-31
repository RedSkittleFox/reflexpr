cmake_minimum_required(VERSION 3.16)

set(PROJECTNAME ${MODULE_NAME})
set(PROJECTDIR "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE inc "${CMAKE_CURRENT_SOURCE_DIR}/../include/*.hpp")
file(GLOB_RECURSE inc_src "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB_RECURSE src "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/../include" FILES ${inc})

if (MSVC)
    add_compile_options(/W3) # warning level 3
	add_compile_options(/MP) # Multi-processor compilation
endif()

add_library(
	${PROJECTNAME}
	INTERFACE
	${inc}
	)

target_include_directories(${PROJECTNAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../include/")