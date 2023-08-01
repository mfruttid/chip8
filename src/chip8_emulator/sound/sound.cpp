#include "sound.h"
#include <iostream>

Sound::Sound(const void* mem, size_t size)
{
    if (!mem || !size)
    {
        return;
    }

    // prepare a read-only memory buffer for the sound to be used to load data in memory
    auto* p_fromConstMem = SDL_RWFromConstMem(mem, static_cast<int>(size));
    if (!p_fromConstMem)
    {
        std::cerr << "Sound loading error: " << SDL_GetError() << "\n";
        return;
    }

    // load the sound buffer into memory of device, possibly decoding it
    auto* p_audioSpec = SDL_LoadWAV_RW(
        p_fromConstMem,
        1,
        &m_audioSpec,
        &m_soundBufferStart,
        &m_soundBufferLength);

    if (!p_audioSpec)
    {
        std::cerr << "Sound loading error: " << SDL_GetError() << "\n";
    }
    else
    {
        // we open the default audio device, checking that this doesn't cause any error
        m_device = SDL_OpenAudioDevice(nullptr, 0, &m_audioSpec, nullptr, 0);

        if (m_device == 0)
        {
            std::cerr << "Sound device error: " << SDL_GetError() << "\n";
        }
    }
}

Sound::Sound(Sound&& sound)
: m_device {sound.m_device},
  m_audioSpec {sound.m_audioSpec},
  m_soundBufferStart {sound.m_soundBufferStart},
  m_soundBufferLength {sound.m_soundBufferLength}
{
    // this is necessary so that the destructor of Sound
    // doesn't close the device when sound is deleted
    sound.m_device = 0;
    sound.m_soundBufferStart = nullptr;
}

Sound::~Sound()
{
    if (m_soundBufferStart)
    {
        SDL_FreeWAV(m_soundBufferStart);
    }
    if (m_device)
    {
        SDL_CloseAudioDevice(m_device);
    }
}

Sound& Sound::operator=(Sound&& sound)
{
    m_device = sound.m_device;
    m_audioSpec = std::move(sound.m_audioSpec);
    m_soundBufferStart = sound.m_soundBufferStart;
    m_soundBufferLength = std::move(sound.m_soundBufferLength);

    // this is necessary so that the destructor of Sound
    // doesn't close the device when sound is deleted
    sound.m_device = 0;
    sound.m_soundBufferStart = nullptr;

    return *this;
}

bool Sound::isValid()
{
    return m_device && m_soundBufferLength && m_soundBufferStart;
}

void Sound::playSound()
{
    if (isValid())
    {
        SDL_QueueAudio(m_device, m_soundBufferStart, m_soundBufferLength);
        SDL_PauseAudioDevice(m_device, 0); // unpauses the audio device
    }
}

void Sound::pauseSound()
{
    if (isValid())
    {
        SDL_PauseAudioDevice(m_device, 1);
        // clear queue to avoid filling it up unnecessarily every time we play a sound
        SDL_ClearQueuedAudio(m_device);
    }
}
