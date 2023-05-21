#include "display.h"

void fromChip8ToDisplay(SDL_Renderer* renderer, const Chip8::Chip8::Display& display)
{
    SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
    SDL_RenderClear(renderer);

    for (int i =0; i<32; ++i)
    {
        for (int j=0; j<64; ++j)
        {
            if (display.d[i][j].status == Chip8::Chip8::Status::on)
            {
                SDL_Rect rectangle = SDL_Rect(20*j,20*i, 20,20);
                SDL_SetRenderDrawColor(renderer, 255,255,255, 255);
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

    std::unique_lock lck{c.display_mutex};
    lck.unlock();

    SDL_Event ev;

    while (c.isRunning)
    {
        while (SDL_PollEvent(&ev) != 0)
        {
            if (ev.type == SDL_QUIT)
            {
                c.isRunning =  false;
            }
        }

        lck.lock();
        fromChip8ToDisplay(renderer, c.display);
        lck.unlock();

        SDL_Delay(200);
        SDL_RenderPresent(renderer);
    }

    //SDL_Delay(2000);
    SDL_DestroyWindow(window);
    SDL_Quit();

    promiseDisplayDone.set_value(true);
}


