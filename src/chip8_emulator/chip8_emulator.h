#pragma once

#pragma once

#include "chip8-core/chip8.h"
#include <cstring>

class Chip8Emulator : private Chip8::Chip8
{
public:
    Chip8Emulator() = default;

    void renderDisplay( SDL_Renderer* renderer );

    void dealKeyboardOrQuitWindow( SDL_Event ev );

    void renderAndKeyboard( std::promise<bool>& promise_display_initialized );

    void runChip8(
        std::filesystem::path programPath,
        std::future<bool>& futureDisplayInitialized,
        Chip8::Chip8::Chip8Type flagChip8,
        Chip8::Chip8::DrawInstruction drawInstruction
        )
    {
        readFromFile(programPath);
        run( futureDisplayInitialized, flagChip8, drawInstruction );
    }

};

