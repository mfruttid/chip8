#include "../chip8-core/chip8.h"
#include "../sound/sound.h"
#include <chrono>

void Chip8::Chip8::decreaseTimer(std::atomic<Register>& timer)
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


void Chip8::Chip8::decreaseDelayTimer()
{
    decreaseTimer(delayTimer);
}

void Chip8::Chip8::decreaseSoundTimer()
{
    static Sound sound { "/home/martina/cpp/chip8/sounds/test.wav" };
    sound.playSound();

    decreaseTimer(soundTimer);

    sound.pauseSound();
}
