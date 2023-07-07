cmake_minimum_required( VERSION 3.26.0 )
add_executable( main )
set_target_properties( main PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME} )

target_compile_definitions( main PUBLIC "SDL_MAIN_HANDLED" )

target_include_directories( main PUBLIC  "chip8_emulator/sound/"
                                         "chip8_emulator/chip8-core/"
                                         "chip8_emulator/"
                                         "../external/SDL2/")

target_sources( main PRIVATE
    "chip8_emulator/chip8-core/chip8.cpp"
    "chip8_emulator/chip8-displayAndKeyboard/displayAndKeyboard.cpp"
    "main.cpp"
    "chip8_emulator/chip8-timers/timers.cpp"
    "chip8_emulator/sound/sound.cpp"
    "chip8_emulator/chip8-core/chip8.h"
    "chip8_emulator/chip8_emulator.h"
    "chip8_emulator/sound/sound.h"
    )

add_executable( tests )
set_target_properties( tests PROPERTIES OUTPUT_NAME tests.bin )

target_compile_definitions( tests PUBLIC "SDL_MAIN_HANDLED" )

target_include_directories( tests PUBLIC  "chip8_emulator/sound/"
                                         "chip8_emulator/chip8-core/"
                                         "chip8_emulator/")
target_sources( tests PRIVATE
    "chip8_emulator/chip8-core/chip8.cpp"
    "../tests/test.cpp"
    "chip8_emulator/chip8-displayAndKeyboard/displayAndKeyboard.cpp"
    "chip8_emulator/chip8-timers/timers.cpp"
    "chip8_emulator/sound/sound.cpp"
    )


if( ${CMAKE_SYSTEM_NAME} MATCHES "Windows")

    set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT main )
    set( SDL_AUDIODRIVER=directsound )

endif()


