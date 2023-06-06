#include "displayAndKeyboard.h"

// sets up the renderer to show the display of the chip8
void renderDisplay( SDL_Renderer* renderer, Chip8::Chip8::Display& display )
{
    // every time we show a new frame, the fading level of the pixels decreases
    display.decreaseFadingLevel();

    // sets the background color to black
    SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
    SDL_RenderClear(renderer);

    for (int row = 0; row < 32; ++row)
    {
        for (int column=0; column<64; ++column)
        {
            Chip8::Chip8::Pixel pixel = display.d[row][column];

            if (pixel.status == Chip8::Chip8::Status::on)
            {
                // sets color to white for on pixels
                SDL_SetRenderDrawColor(renderer, 255,255,255, 255);

                // every chip8 pixel becomes a square of side 20
                SDL_Rect pixelRectangle = SDL_Rect(20*column,20*row, 20,20);
                SDL_RenderFillRect(renderer, & pixelRectangle);
            }

            else if (pixel.fadingLevel > 0)
            {
                // the off pixel gets a color basing on the fading level
                uint8_t colorShade = static_cast<uint8_t>( pixel.fadingLevel / 4 );
                SDL_SetRenderDrawColor(renderer, colorShade,colorShade,colorShade, 255);

                SDL_Rect pixelRectangle = SDL_Rect( 20*column, 20*row, 20, 20 );
                SDL_RenderFillRect(renderer, & pixelRectangle);
            }
        }
    }
}


void renderAndKeyboard(Chip8::Chip8& c, std::promise<bool>& promiseDisplayInitialized, std::promise<bool>& promiseDisplayDone)
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
        renderDisplay(renderer, c.display);
        lck.unlock();

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    promiseDisplayDone.set_value(true);
}


