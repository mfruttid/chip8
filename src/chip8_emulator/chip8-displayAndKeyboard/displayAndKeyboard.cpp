#include "../chip8_emulator.h"

// sets up the renderer to show the display of the chip8
void Chip8Emulator::renderDisplay( SDL_Renderer* renderer )
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
            Chip8::Chip8::Pixel pixel = display.frame[row][column];

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
                int32_t n = display.MAXIMALFADING / 125;
                uint8_t colorShade = static_cast<uint8_t>( pixel.fadingLevel / n );
                SDL_SetRenderDrawColor(renderer, colorShade,colorShade,colorShade, 255);

                SDL_Rect pixelRectangle = SDL_Rect( 20*column, 20*row, 20, 20 );
                SDL_RenderFillRect(renderer, & pixelRectangle);
            }
        }
    }
}

void Chip8Emulator::dealKeyboardOrQuitWindow( SDL_Event ev )
{
    while (SDL_PollEvent(&ev) != 0)
        {
            switch (ev.type)
            {

            // if the user closes the window
            case SDL_QUIT:
            {
                std::unique_lock keyboardOrQuitWindowMutexLock {keyboardOrQuitWindowMutex};
                isRunning =  false;
                keyIsPressedOrWindowClosed.notify_one();
                break;
            }

            case SDL_KEYDOWN:
            {
                std::unique_lock keyboardOrQuitWindowMutexLock { keyboardOrQuitWindowMutex };
                pressedKey = std::optional<SDL_Scancode>( ev.key.keysym.scancode );

                keyIsPressedOrWindowClosed.notify_one();
                break;
            }

            case SDL_KEYUP:
            {
                std::unique_lock keyboardOrQuitWindowMutexLock { keyboardOrQuitWindowMutex };
                pressedKey = std::optional<SDL_Scancode>();
                break;
            }

            default:
                break;

            }
        }
}

void Chip8Emulator::renderAndKeyboard( std::promise<bool>& promiseDisplayInitialized )
{
    // set up renderer and window
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window { SDL_CreateWindow(
                            "Chip8",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            1280, 640,
                            SDL_WINDOW_SHOWN ) };
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    promiseDisplayInitialized.set_value(true);

    std::unique_lock displayLock { displayMutex };
    displayLock.unlock();

    SDL_Event ev;

    while ( isRunning )
    {
        dealKeyboardOrQuitWindow(ev);

        displayLock.lock();
        renderDisplay(renderer);
        displayLock.unlock();

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}


