#pragma once

#include <iostream>
#include <SDL2/SDL.h>
#include <filesystem>

class Sound
{
public:
    explicit Sound(std::filesystem::path path);

    ~Sound();

    void playSound();

    void pauseSound();

private:
    // device the sound will be played on
    SDL_AudioDeviceID mDevice;

    // properties of the wave file loaded
    SDL_AudioSpec mAudioSpec;
    uint8_t* mWaveStart;
    uint32_t mWaveLength;
};
