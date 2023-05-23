#include "timers.h"
#include <chrono>

void decreaseDelayTimer(Chip8::Chip8& c)
{
    while (c.delayTimer != 0)
    {
        const auto start = std::chrono::high_resolution_clock::now();

        --c.delayTimer;

        const auto end = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double, std::milli> sleep_time = std::chrono::seconds(1/60) - (end - start);
        std::this_thread::sleep_for(sleep_time);
    }
}
