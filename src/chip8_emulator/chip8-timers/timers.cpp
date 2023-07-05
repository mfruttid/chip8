#include <chip8.h>
#include <sound.h>
#include <chrono>

// decreases timer at a rate of 60 per second
// it also plays sound if the flagSound is set to true
void Chip8::decreaseTimer(std::atomic<Register>& timer, bool flagSound)
{
    while (!m_isRunning)
    {
        continue;
    }

    while (m_isRunning)
    {
        // the timers of chip8 must decrease at a rate of 60 per second
        // in order to do that, we let the delay/sound thread sleep 
        // after every tic.
        std::chrono::duration<double, std::milli> sleep_time{ 0 };

        while (timer != 0)
        {
            if (flagSound)
            {
                m_sound.playSound();
            }

            // the timers of the chip8 must decrease at a rate of 60 per second
            auto start = std::chrono::high_resolution_clock::now();

            std::this_thread::sleep_for(sleep_time);

            --timer;

            auto end = std::chrono::high_resolution_clock::now();
            sleep_time = std::chrono::milliseconds(1000 / 60) - (end - start);
        }

        if (flagSound)
        {
            m_sound.pauseSound();
        }
    }
}

