#include "sound.h"

Sound::Sound(std::filesystem::path path) :
    mDevice { },
    mAudioSpec { },
    mWaveStart { },
    mWaveLength { }
{
    // we load the WAVE file into memory, checking that this doesn't cause any error
    if ( SDL_LoadWAV( path.c_str(), &mAudioSpec, &mWaveStart, &mWaveLength ) == NULL )
    {
        std::cerr << "Sound loading error: " << SDL_GetError() << "\n";
    }

    else
    {
        // we open the default audio device, checking that this doesn't cause any error
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
    SDL_QueueAudio( mDevice, mWaveStart, mWaveLength );
    SDL_PauseAudioDevice( mDevice, 0 ); // unpauses the audio device
}

void Sound::pauseSound()
{
    SDL_PauseAudioDevice( mDevice, 1 );
}
