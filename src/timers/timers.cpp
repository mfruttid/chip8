#include "timers.h"
#include "../sound/sound.h"
#include <chrono>

void decreaseTimer(std::atomic<Chip8::Chip8::Register>& timer)
{
    while (timer != 0)
    {
        const auto start = std::chrono::high_resolution_clock::now();

        --timer;

        const auto end = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double, std::milli> sleep_time = std::chrono::milliseconds(1000/60) - (end - start);
        std::this_thread::sleep_for(sleep_time);
    }
}


void decreaseDelayTimer(Chip8::Chip8& c)
{
    decreaseTimer(c.delayTimer);
}

void decreaseSoundTimer(Chip8::Chip8& c)
{
    static Sound sound { "/home/martina/cpp/chip8/sounds/test.wav" };
    sound.playSound();

    decreaseTimer(c.soundTimer);

    sound.pauseSound();
}
