#pragma once

#include "../chip8_emulator/chip8.h"
#include <SDL2/SDL.h>

void from_chip8_to_display(SDL_Renderer* renderer, const Chip8::Chip8::Display& display);

void show(Chip8::Chip8& c, std::promise<bool>& promise_display_initialized, std::promise<bool>& promise_display_done);

