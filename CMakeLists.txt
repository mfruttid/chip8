cmake_minimum_required( VERSION 3.26.0 )

# the following block is for using clang
#set( CMAKE_CXX_COMPILER  "/usr/bin/clang++"  )
#set( CMAKE_C_COMPILER  "/usr/bin/clang"  )
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
#set(CMAKE_C_COMPILER_WORKS TRUE CACHE INTERNAL "")
#set(CMAKE_CXX_COMPILER_WORKS TRUE CACHE INTERNAL "")



project( Chip8 )

set( CMAKE_CXX_STANDARD 20 CACHE STRING "" FORCE )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE )


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


set( CMAKE_CXX_FLAGS "-fdiagnostics-show-template-tree ")
# flags for gcc
set( CMAKE_CXX_FLAGS_DEBUG "-ggdb -Werror -Wextra -Wall -Wpedantic -Wconversion -Warith-conversion -Wwrite-strings -Warray-bounds=2 -Wcast-align=strict -Wcast-qual -Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Wformat-overflow=2 -Wnull-dereference -Wimplicit-fallthrough=5 -Wshift-negative-value -Wswitch-default -Wswitch-enum -Wuninitialized -Wfloat-equal -Wshadow -Wlogical-op -Wvla -Wno-unknown-pragmas")

# flags for clang
#set( CMAKE_CXX_FLAGS_DEBUG "-ggdb -fdiagnostics-show-template-tree -Wextra -Wall -Wpedantic -Wconversion -Wwrite-strings -Warray-bounds -Wcast-align -Wcast-qual -Wdouble-promotion -Wnull-dereference -Wimplicit-fallthrough -Wshift-negative-value -Wswitch-default -Wswitch-enum -Wuninitialized -Wfloat-equal -Wshadow -Wvla -Wno-unknown-pragmas")

set( CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")


add_executable( tests )
set_target_properties( tests PROPERTIES OUTPUT_NAME tests.bin )

add_executable( main )
set_target_properties( main PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME} )

add_subdirectory( "src" )
add_subdirectory( "external" )

if( ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set( CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" )
    set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT ${CMAKE_PROJECT_NAME} )
    set_property( TARGET main PROPERTY WIN32_EXECUTABLE true )
endif()