#pragma once

#include "../chip8_emulator/chip8.h"

void decreaseTimer(std::atomic<Chip8::Chip8::Register>& timer);

void decreaseDelayTimer(Chip8::Chip8& c);

void decreaseSoundTimer(Chip8::Chip8& c);