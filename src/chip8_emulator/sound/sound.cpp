#include "sound.h"

Sound::Sound(const void* mem, size_t size)
{
    //mAudioSpec.callback = &feedAudioDeviceCallbackFunction;
    //mAudioSpec.userdata = this;
    // load the sound buffer into memory of device, checking that this doesn't cause any error
    if (SDL_LoadWAV_RW(SDL_RWFromConstMem(mem, static_cast<int>(size)), 1, &mAudioSpec, 
                                            &mSoundBufferStart, &mSoundBufferLength) == nullptr)
    {
        std::cerr << "Sound loading error: " << SDL_GetError() << "\n";
    }

    else
    {
        // we open the default audio device, checking that this doesn't cause any error
        mDevice = SDL_OpenAudioDevice(nullptr, 0, &mAudioSpec, nullptr, 0);

        if (mDevice == 0)
        {
            std::cerr << "Sound device error: " << SDL_GetError() << "\n";
        }
    }
}

Sound::~Sound()
{
    SDL_FreeWAV(mSoundBufferStart);
    SDL_CloseAudioDevice(mDevice);
}

void Sound::playSound()
{
    SDL_QueueAudio(mDevice, mSoundBufferStart, mSoundBufferLength);
    SDL_PauseAudioDevice(mDevice, 0); // unpauses the audio device
}

void Sound::pauseSound()
{
    SDL_PauseAudioDevice(mDevice, 1);
    SDL_ClearQueuedAudio(mDevice);
}
