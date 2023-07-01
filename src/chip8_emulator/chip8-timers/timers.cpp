#include <chip8.h>
#include <sound.h>
#include <chrono>

void Chip8::decreaseTimer(std::atomic<Register>& timer)
{
    while (timer != 0)
    {
        // the timers of the chip8 must decrease at a rate of 60 per second
        const auto start = std::chrono::high_resolution_clock::now();

        --timer;

        const auto end = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double, std::milli> sleep_time = std::chrono::milliseconds(1000/60) - (end - start);
        std::this_thread::sleep_for(sleep_time);
    }
}


void Chip8::decreaseDelayTimer()
{
    decreaseTimer(m_delayTimer);
}

void Chip8::decreaseSoundTimer()
{
    static Sound sound { "../../../sounds/beep.wav" };

    sound.playSound();

    decreaseTimer(m_soundTimer);

    sound.pauseSound();
}
