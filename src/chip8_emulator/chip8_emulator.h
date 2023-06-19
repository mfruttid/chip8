#pragma once

#pragma once

#include "chip8-core/chip8.h"
#include <cstring>

class Chip8Emulator : private Chip8
{
public:
    Chip8Emulator(const std::string& flagChip8Type, const std::string& flagDrawInstruction, const std::string& flagFading) :
    Chip8(flagChip8Type, flagDrawInstruction, flagFading) {}

    // modifies the renderer to show the display of the chip8
    void renderDisplay( SDL_Renderer* renderer );

    // updates isRunning to false if the user clicks to close the window
    // and changes the value of the chip8PressedKey if the user presses a valid key on their keyboard
    void dealKeyboardOrQuitWindow( SDL_Event ev );

    // creates the window and the renderer and while the program is running,
    // it updates the window and the pressed keys if any
    void renderAndKeyboard( std::promise<bool>& promise_display_initialized );

    // runs the program written in the file specified by the path
    void runChip8( std::filesystem::path programPath, std::future<bool>& futureDisplayInitialized )
    {
        readFromFile(programPath);
        run( futureDisplayInitialized );
    }

    // legend between the keyboard of the computer and the virtual keyboard of the chip8
    // The keys are used to simulate the keyboard of the chip8 are the following:
    // 1 = 0x1; 2 = 0x2; 3 = 0x3; 4 = 0xc; q = 0x4; w = 0x5; e = 0x6; r = 0xd;
    // a = 0x7; s = 0x8; d = 0x9; f = 0xe; z =0xa; x = 0x0; c =0xb; v = 0xf
    // If one of the above key is pressed, then it returns the associated value,
    // otherwise it returns nullptr
    std::optional<uint8_t> getChip8Key(std::optional<SDL_Scancode> pressedKey) const;
};

