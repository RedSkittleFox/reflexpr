cmake_minimum_required(VERSION 3.21)

set(MODULE_NAME ruby-reflexpr) # change me

project(${MODULE_NAME} LANGUAGES CXX)

if (PROJECT_IS_TOP_LEVEL)
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	
	set(CMAKE_SYSTEM_VERSION 10.0.19041.0)
	
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin-etc")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
	
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_CXX_STANDARD_REQUIRED)

endif()

add_subdirectory(src)
add_subdirectory(examples)