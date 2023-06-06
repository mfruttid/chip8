#include "sound.h"

Sound::Sound(std::filesystem::path path) :
    mDevice { },
    mAudioSpec { },
    mWaveStart { },
    mWaveLength { }
{
    if ( SDL_LoadWAV( path.c_str(), &mAudioSpec, &mWaveStart, &mWaveLength ) == NULL )
    {
        std::cerr << "Sound loading error: " << SDL_GetError() << "\n";
    }

    else
    {
        mDevice = SDL_OpenAudioDevice( nullptr, 0, &mAudioSpec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE );

        if (mDevice == 0)
        {
            std::cerr << "Sound device error: " << SDL_GetError() << "\n";
        }
    }
}

Sound::~Sound()
{
    SDL_FreeWAV(mWaveStart);
    SDL_CloseAudioDevice(mDevice);
}

void Sound::playSound()
{
    [[maybe_unused]] int status { SDL_QueueAudio( mDevice, mWaveStart, mWaveLength ) };
    SDL_PauseAudioDevice( mDevice, 0 );
}

void Sound::pauseSound()
{
    SDL_PauseAudioDevice( mDevice, 1 );
}
