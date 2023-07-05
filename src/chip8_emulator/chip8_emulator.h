#pragma once

#include "chip8-core/chip8.h"
#include <cstring>
#include <type_traits>

class Chip8Emulator : private Chip8
{
public:
    Chip8Emulator(
        std::string_view flagChip8Type, 
        std::string_view flagDrawInstruction, 
        std::string_view flagFading
    ) :
    Chip8( flagChip8Type, flagDrawInstruction, flagFading ) {
        SDL_Init(SDL_INIT_EVERYTHING);
    }

    // updates the renderer window frame buffer to show the display of the chip8
    void renderDisplay( SDL_Renderer* renderer );

    // updates isRunning to false if the user clicks to close the window
    // and changes the value of the m_chip8PressedKey if the user presses a valid key on their keyboard
    void handleSystemEvents( SDL_Event ev );

    // creates the window and the renderer and while the program is running,
    // it updates the window and the pressed keys if any
    void renderAndKeyboard( std::promise<bool>& promise_display_initialized );

    // runs the program written in the file specified by the path
    void loadAndRunChip8Program( 
        std::filesystem::path& programPath, 
        std::future<bool>&& futureDisplayInitialized )
    {
        readFromFile( programPath );

        run( futureDisplayInitialized );
    }

    // legend between the keyboard of the computer and the virtual keyboard of the chip8
    // it's scancode-based and not keycode-based
    std::optional<uint8_t> getChip8Key( std::optional<SDL_Scancode> pressedKey ) const;
};

