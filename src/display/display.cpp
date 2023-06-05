#include "display.h"

// copied from https://github.com/Grieverheart/sdl_tone_generator/blob/main/main.cpp
void audio_callback(void* userdata, uint8_t* stream, int len)
{
    uint64_t* samples_played = (uint64_t*)userdata;

    float f = static_cast<float>(*stream);
    float* fstream = &f;

    static const double volume = 0.2;
    static const double frequency = 200.0;

    for(int sid = 0; sid < (len / 8); ++sid)
    {
        double time = static_cast<double>((*samples_played + sid) / 44100);
        double x = static_cast<double>(2.0 * M_PI * time * frequency);
        fstream[2 * sid + 0] = static_cast<float>(volume * sin(x)); /* L */
        fstream[2 * sid + 1] = static_cast<float>(volume * sin(x)); /* R */
    }

    *samples_played += (len / 8);
}

void fromChip8ToDisplay(SDL_Renderer* renderer, Chip8::Chip8::Display& display)
{
    display.clearFadingLevel();

    SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
    SDL_RenderClear(renderer);

    for (int row = 0; row < 32; ++row)
    {
        for (int column=0; column<64; ++column)
        {
            Chip8::Chip8::Pixel pixel = display.d[row][column];

            if (pixel.status == Chip8::Chip8::Status::on)
            {
                SDL_SetRenderDrawColor(renderer, 255,255,255, 255);

                SDL_Rect rectangle = SDL_Rect(20*column,20*row, 20,20);
                SDL_RenderFillRect(renderer, &rectangle);
            }
            else if (pixel.fadingLevel > 0)
            {
                uint8_t colorShade = static_cast<uint8_t>( pixel.fadingLevel / 4 );
                SDL_SetRenderDrawColor(renderer, colorShade,colorShade,colorShade, 255);

                SDL_Rect rectangle = SDL_Rect( 20*column, 20*row, 20, 20 );
                SDL_RenderFillRect(renderer, &rectangle);
            }
        }
    }
}

void showDisplay(Chip8::Chip8& c, std::promise<bool>& promiseDisplayInitialized, std::promise<bool>& promiseDisplayDone)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow( "Chip8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 640, SDL_WINDOW_SHOWN );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    promiseDisplayInitialized.set_value(true);

    std::unique_lock lck{c.displayMutex};
    lck.unlock();

    SDL_Event ev;

    /*
    uint64_t samples_played = 0;

    SDL_AudioSpec audio_spec_want, audio_spec;
    SDL_memset(&audio_spec_want, 0, sizeof(audio_spec_want));

    audio_spec_want.freq     = 44100;
    audio_spec_want.format   = AUDIO_F32;
    audio_spec_want.channels = 2;
    audio_spec_want.samples  = 512;
    audio_spec_want.callback = audio_callback;
    audio_spec_want.userdata = (void*)&samples_played;
    */

    while (c.isRunning)
    {
        while (SDL_PollEvent(&ev) != 0)
        {
            switch (ev.type)
            {

            case SDL_QUIT:
            {
                std::unique_lock keyboardMutexLock {c.keyboardMutex};
                c.isRunning =  false;
                c.keyIsPressed.notify_one();
                break;
            }

            case SDL_KEYDOWN:
            {
                std::unique_lock keyboardMutexLock {c.keyboardMutex};
                c.pressedKey = std::optional<SDL_Scancode>(ev.key.keysym.scancode);

                c.keyIsPressed.notify_one();
                break;
            }

            case SDL_KEYUP:
            {
                std::unique_lock keyboardMutexLock {c.keyboardMutex};
                c.pressedKey = std::optional<SDL_Scancode>();
                break;
            }

            default:
                break;

            }
        }

        lck.lock();
        fromChip8ToDisplay(renderer, c.display);
        lck.unlock();

        SDL_RenderPresent(renderer);

        /*
        SDL_AudioDeviceID audio_device_id;

        while (c.soundTimer != 0)
        {
            audio_device_id = SDL_OpenAudioDevice(
            NULL, 0,
            &audio_spec_want, &audio_spec,
            SDL_AUDIO_ALLOW_FORMAT_CHANGE );
        }

        SDL_PauseAudioDevice(audio_device_id, 0);
        */

    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    promiseDisplayDone.set_value(true);
}


