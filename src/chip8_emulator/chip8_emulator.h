#pragma once

#include "chip8-core/chip8.h"
#include <cstring>

inline constexpr auto testLambda = []()
    {
        std::cout << "I'm a side effect!" << std::endl;
    };

class Chip8Emulator
{
public:
    // The constructor of Chip8Emulator spawns two threads: m_delayTimerThread and m_soundTimerThread
    // (look at the Chip8 constructor for more info).
    Chip8Emulator(
        std::string_view flagChip8Type,
        std::string_view flagDrawInstruction,
        std::string_view flagFading
    )
    : m_chip8 {
        flagChip8Type,
        flagDrawInstruction,
        flagFading,
        testLambda,
        testLambda
        }
    {
        SDL_Init(SDL_INIT_EVERYTHING);

        m_sound = Sound(reinterpret_cast<const void*>(Base64Sound::DECODED_SOUND.data()),
                    Base64Sound::DECODED_SOUND_SIZE);

        m_chip8.m_playSoundCallback = [this]{ this->m_sound.playSound(); };
        m_chip8.m_pauseSoundCallback = [this]{ this->m_sound.pauseSound(); };
    }

    ~Chip8Emulator()
    {
        // necessary because the distructor of a valid Sound uses SDL
        m_sound = Sound(nullptr, 0);

        SDL_Quit();
    }

    // creates the window and the renderer and while the program is running,
    // it updates the window and the pressed keys if any
    void renderAndKeyboard(std::promise<bool>& promise_display_initialized);

    // This function spawns a new thread, where the instructions of the rom are executed.
    // This thread communicates with the main thread through a future and a promise, which
    // gets set when the display in the main thread has finished its initialization.
    void runEmulator(std::filesystem::path&& programPath)
    {
        std::promise<bool> promiseDisplayInitialized;
        std::future<bool> futureDisplayInitialized = promiseDisplayInitialized.get_future();

        // the chip8 must run the instruction in one thread
        std::thread chip8Thread {
            &Chip8Emulator::loadAndRunChip8Program,
            std::ref(*this),
            std::move(programPath),
            std::move(futureDisplayInitialized)};
        chip8Thread.detach();

        // the main thread shows and updates the window and updates the pressed keys in the meantime
        renderAndKeyboard(promiseDisplayInitialized);
    }

private:
    Sound m_sound {nullptr, 0};
    Chip8 m_chip8;

    // updates the renderer window frame buffer to show the display of the chip8
    void renderDisplay(SDL_Renderer* renderer);

    // updates isRunning to false if the user clicks to close the window
    // and changes the value of the m_chip8PressedKey if the user presses a valid key on their keyboard
    void handleSystemEvents(SDL_Event ev);

    // runs the program written in the file specified by the path
    void loadAndRunChip8Program(
        std::filesystem::path&& programPath,
        std::future<bool>&& futureDisplayInitialized)
    {
        m_chip8.readFromFile(programPath);

        m_chip8.run(std::move(futureDisplayInitialized));
    }

    // legend between the keyboard of the computer and the virtual keyboard of the chip8
    // it's scancode-based and not keycode-based
    std::optional<uint8_t> getChip8Key(SDL_Scancode pressedKey) const;
};

