#include <chip8.h>
#include <sound.h>
#include <chrono>

// decreases timer at a rate of 60 per second
// it also plays sound if the flagSound is set to true
void Chip8::decreaseTimer(std::atomic<Register>& timer, bool flagSound)
{
    // wait for the chip8 to start running
    std::unique_lock isRunningMutexLock{m_isRunningMutex};
    m_hasStartedRunning.wait(isRunningMutexLock, [&] { return m_isRunning; });
    isRunningMutexLock.unlock();

    // initialize the appropriate (delay or sound) mutex and condition variables
    std::mutex& timerMutex = (flagSound) ? m_soundTimerMutex : m_delayTimerMutex;
    std::condition_variable& setTimer = (flagSound) ? m_setSoundTimer : m_setDelayTimer;
    std::unique_lock timerMutexLock{timerMutex};
    timerMutexLock.unlock();

    while (m_isRunning)
    {
        // the timers of chip8 must decrease at a rate of 60 per second
        // in order to do that, we let the delay/sound thread sleep 
        // after every tic.
        std::chrono::duration<double, std::milli> sleep_time{ 0 };
        
        // wait until the timer is different from 0
        timerMutexLock.lock();
        setTimer.wait(timerMutexLock, [&] { return (timer != 0); });
        timerMutexLock.unlock();

        if (flagSound)
        {
            m_sound.playSound();
        }

        while (timer != 0)
        {
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

