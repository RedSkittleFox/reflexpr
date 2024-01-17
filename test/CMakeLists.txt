cmake_minimum_required(VERSION 3.5)

cmake_policy(PUSH)

enable_testing()

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(sources 
    "${CMAKE_CURRENT_SOURCE_DIR}/reflexpr_test.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/reflexpr_test_generator.py"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

add_executable(
    reflexpr-test
    ${sources}
)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	target_compile_options(
        reflexpr-test
		PRIVATE /W4 
		PRIVATE /MP 
		PRIVATE /arch:AVX2
		PRIVATE /WX
        PRIVATE /bigobj
	)

endif()

target_link_libraries(
    reflexpr-test
    GTest::gtest_main
    GTest::gmock_main
    reflexpr
)

find_package(Python COMPONENTS Interpreter Development)
if(NOT ${Python_FOUND})
    message( FATAL_ERROR "Failed to locate python." )
endif()

message("${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reflexpr_test_generator.py")

add_custom_target(
    reflexpr-test-generator 
    COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/reflexpr_test_generator.py" "${CMAKE_CURRENT_SOURCE_DIR}"
    BYPRODUCTS reflexpr_test_types.inl
    COMMENT "RedSkittleFox::Reflexpr: Generating test types"
)

add_dependencies(reflexpr-test reflexpr-test-generator)

include(GoogleTest)
gtest_discover_tests(reflexpr-test)

if (PROJECT_IS_TOP_LEVEL)
    set_target_properties(gtest_main PROPERTIES FOLDER "vendor")
    set_target_properties(gtest PROPERTIES FOLDER "vendor")
    set_target_properties(gmock_main PROPERTIES FOLDER "vendor")
    set_target_properties(gmock PROPERTIES FOLDER "vendor")
endif()

cmake_policy(POP)