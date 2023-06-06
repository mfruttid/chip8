#pragma once

#include "chip8-core/chip8.h"

class Chip8Emulator
{
public:
    Chip8::Chip8 chip8;

public:
    Chip8Emulator() :
    chip8 { Chip8::Chip8() } {}

    void renderDisplay( SDL_Renderer* renderer );

    void dealKeyboardOrQuitWindow( SDL_Event ev );

    void renderAndKeyboard( std::promise<bool>& promise_display_initialized );

};

