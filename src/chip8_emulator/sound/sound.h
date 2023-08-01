#pragma once

#include <SDL.h>

class Sound
{
    /*
    A sound can be either valid or invalid:
    - valid: if it constructed using a valid pointer to memory and a valid length and if the audio device is open
    - invalid: if it constructed using a nullptr or the size of the buffer is 0; in this case no audio device is opened
    */
public:
    // if mem is a nullptr, simply returns;
    // otherwise loads the WAVE file that is stored in memory starting at address mem and opens the default audio device
    explicit Sound(const void* mem, size_t size);

    // safely moves the sound without closing the audio device and freeing the memory of the sound buffer,
    // so that it can be used by the new constructed sound
    explicit Sound(Sound&& sound);

    Sound(Sound&) = delete;

    // similar to move constructor
    Sound& operator=(Sound&& sound);

    Sound& operator=(Sound&) = delete;

    // if the sound is a valid sound, then it closes the audio device and frees the sound buffer memory
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

