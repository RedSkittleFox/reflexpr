cmake_minimum_required(VERSION 3.16)

function(CREATE_DEMO DEMO_NAME)
	message(STATUS "Generating project for example '${DEMO_NAME}.cpp'")

	set(PROJECTNAME ${DEMO_NAME})
	set(PROJECTDIR "${CMAKE_CURRENT_SOURCE_DIR}/examples")
	
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CMAKE_CURRENT_SOURCE_DIR}/${DEMO_NAME}.cpp)
	
	if (MSVC)
		add_compile_options(/W3) # warning level 3
		add_compile_options(/MP) # Multi-processor compilation
	endif()
	
	add_executable(
		${PROJECTNAME}
		${CMAKE_CURRENT_SOURCE_DIR}/${DEMO_NAME}.cpp
		)
	
	set_property(TARGET ${PROJECTNAME} PROPERTY CXX_STANDARD 23)
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include/")

	target_link_libraries(${PROJECTNAME} ${MODULE_NAME})

endfunction()

set(PROJECTDIR "${CMAKE_CURRENT_SOURCE_DIR}/examples")
file(GLOB demos "*.cpp")

foreach(demo ${demos})
	get_filename_component(demo_name ${demo} NAME_WE)
	create_demo(${demo_name})
endforeach()