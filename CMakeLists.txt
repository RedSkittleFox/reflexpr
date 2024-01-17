cmake_minimum_required(VERSION 3.21)

project(reflexpr)

if (PROJECT_IS_TOP_LEVEL)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    	
    # set(CMAKE_SYSTEM_VERSION 10.0.22000.0)
    
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin-lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin-etc")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
    
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED)
endif()
    
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_NOT_SUPPORTED_ERROR LANGUAGES CXX)

if(NOT ${IPO_SUPPORTED})
    message(STATUS "IPO / LTO not supported: <${IPO_NOT_SUPPORTED_ERROR}>")
else()
    message(STATUS "IPO / LTO supported.")
endif()

include(FetchContent)

add_subdirectory("include")

add_library(fox::reflexpr ALIAS reflexpr)

option(
	FOX_REFLEXPR_BUILD_SAMPLES 
    "If samples are built."
	ON
)

option(
	FOX_REFLEXPR_BUILD_TESTS
    "If unit tests are built"
	ON
)

option(FOX_REFLEXPR_BUILD_SAMPLES ON)
option(FOX_REFLEXPR_BUILD_TESTS ON)

if (FOX_REFLEXPR_BUILD_SAMPLES)
	add_subdirectory("sample")
endif()

if (FOX_REFLEXPR_BUILD_TESTS)
	add_subdirectory("test")
endif()