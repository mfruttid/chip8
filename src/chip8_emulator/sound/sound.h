#pragma once

#include <iostream>
#include <SDL.h>
#include <filesystem>

class Sound
{
public:
    // loads the WAVE file that is stored in memory starting at address mem and opens the default audio device
    explicit Sound(const void* mem, size_t size);

    explicit Sound(Sound&& sound);

    Sound(Sound&) = delete;

    Sound& operator=(Sound&& sound);

    Sound& operator=(Sound&) = delete;

    ~Sound();

    // the sound will not play for more than 8 seconds in a row without stopping
    // i.e. by the way I implemented playSound and pauseSound, after 8 seconds
    // of uninterrupted sound, there will be silence.
    // But this is a safe assumption because this basically never happens in chip8 programs
    void playSound();

    void pauseSound();

private:
    // device the sound will be played on
    SDL_AudioDeviceID m_device {};

    // properties of the wave file loaded
    SDL_AudioSpec m_audioSpec {};
    uint8_t* m_soundBufferStart {};
    uint32_t m_soundBufferLength {};

    bool isValid();
};

