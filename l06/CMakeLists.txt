cmake_minimum_required(VERSION 3.13)
project(l06 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for LSP / IDEs (clangd)
add_compile_options(-Wall -Werror -g)

# Comment these out to disable address sanitizer
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

find_package(Threads REQUIRED)

# Modify these if you add more source files
add_executable(hello hello.cpp)
add_executable(echo echo.cpp)
target_link_libraries(hello Threads::Threads)
target_link_libraries(echo Threads::Threads)

# Tests start ./hello and ./echo and talk to them over TCP, so they have to
# run in the directory where those programs are built.
enable_testing()

# Compile Catch's main only once for all the test programs
add_library(testMain OBJECT testMain.cpp)
target_include_directories(testMain PUBLIC ../test)

# GCC 13 gives false "maybe uninitialized" errors inside <regex> (used by Catch)
# when optimizing (e.g. -DCMAKE_BUILD_TYPE=Release)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(testMain PRIVATE -Wno-error=maybe-uninitialized)
endif()

foreach(prog Hello Echo)
	string(TOLOWER ${prog} server)
	add_executable(Test${prog} Test${prog}.cpp $<TARGET_OBJECTS:testMain>)
	target_include_directories(Test${prog} PRIVATE ../test)
	target_link_libraries(Test${prog} Threads::Threads)
	set_property(TARGET Test${prog} APPEND PROPERTY SOURCES ../test/TestHelpers.h ../test/TestNet.h)
	add_dependencies(Test${prog} ${server}) # build the tested program too
	add_test(NAME test-${server} COMMAND Test${prog} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endforeach()
