set( BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE )

# SDL
# https://github.com/libsdl-org/SDL
set( SDL_LIBC ON CACHE BOOL "" FORCE )
set( SDL_TEST OFF CACHE BOOL "" FORCE )
add_subdirectory( SDL2 )
target_link_libraries( tests SDL2::SDL2main SDL2::SDL2-static )
target_link_libraries( main SDL2::SDL2main SDL2::SDL2-static )