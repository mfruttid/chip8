#include "display.h"

void fromChip8ToDisplay(SDL_Renderer* renderer, const Chip8::Chip8::Display& display)
{
    SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255,255,255, 255);

    for (int row = 0; row < 32; ++row)
    {
        for (int column=0; column<64; ++column)
        {
            if (display.d[row][column].status == Chip8::Chip8::Status::on)
            {
                SDL_Rect rectangle = SDL_Rect(20*column,20*row, 20,20);
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

    while (c.isRunning)
    {
        while (SDL_PollEvent(&ev) != 0)
        {
            switch (ev.type)
            {

            case SDL_QUIT:
            {
                c.isRunning =  false;
                break;
            }

            case SDL_KEYDOWN:
            {
                std::unique_lock keyboardMutexLock {c.keyboardMutex};
                c.pressedKey = std::optional<SDL_Keycode>(ev.key.keysym.sym);

                c.keyIsPressed.notify_one();
                break;
            }

            case SDL_KEYUP:
            {
                std::unique_lock keyboardMutexLock {c.keyboardMutex};
                c.pressedKey = std::optional<SDL_Keycode>();
                break;
            }

            default:
                break;

            }
        }

        lck.lock();
        fromChip8ToDisplay(renderer, c.display);
        lck.unlock();

        std::this_thread::yield();

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    promiseDisplayDone.set_value(true);
}


