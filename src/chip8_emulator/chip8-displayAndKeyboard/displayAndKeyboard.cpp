#include <chip8_emulator.h>

std::optional<uint8_t> Chip8Emulator::getChip8Key( std::optional<SDL_Scancode> pressedKey ) const
// The keys are used to simulate the keyboard of the chip8 are the following:
// 1 = 0x1; 2 = 0x2; 3 = 0x3; 4 = 0xc; q = 0x4; w = 0x5; e = 0x6; r = 0xd;
// a = 0x7; s = 0x8; d = 0x9; f = 0xe; z =0xa; x = 0x0; c =0xb; v = 0xf
// If one of the above key is pressed, then it returns the associated value,
// otherwise it returns nullopt
{
    if ( pressedKey.has_value() )
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wswitch-enum"

        switch ( pressedKey.value() )
        {
        case SDL_SCANCODE_1:
        {
            return std::optional<uint8_t>(0x1);
        }

        case SDL_SCANCODE_2:
        {
            return std::optional<uint8_t>(0x2);
        }

        case SDL_SCANCODE_3:
        {
            return std::optional<uint8_t>(0x3);
        }

        case SDL_SCANCODE_4:
        {
            return std::optional<uint8_t>(0xc);
        }

        case SDL_SCANCODE_Q:
        {
            return std::optional<uint8_t>(0x4);
        }

        case SDL_SCANCODE_W:
        {
            return std::optional<uint8_t>(0x5);
        }

        case SDL_SCANCODE_E:
        {
            return std::optional<uint8_t>(0x6);
        }

        case SDL_SCANCODE_R:
        {
            return std::optional<uint8_t>(0xd);
        }

        case SDL_SCANCODE_A:
        {
            return std::optional<uint8_t>(0x7);
        }

        case SDL_SCANCODE_S:
        {
            return std::optional<uint8_t>(0x8);
        }

        case SDL_SCANCODE_D:
        {
            return std::optional<uint8_t>(0x9);
        }

        case SDL_SCANCODE_F:
        {
            return std::optional<uint8_t>(0xe);
        }

        case SDL_SCANCODE_Z:
        {
            return std::optional<uint8_t>(0xa);
        }

        case SDL_SCANCODE_X:
        {
            return std::optional<uint8_t>(0x0);
        }

        case SDL_SCANCODE_C:
        {
            return std::optional<uint8_t>(0xb);
        }

        case SDL_SCANCODE_V:
        {
            return std::optional<uint8_t>(0xf);
        }

        default:
            break;
        }

        #pragma GCC diagnostic pop
    }

    return std::optional<uint8_t>();
}

void Chip8Emulator::renderDisplay( SDL_Renderer* renderer )
{
    // every time we show a new frame, the fading level of the pixels decreases
    if (m_fadingFlag == Fading::on)
    {
        m_display.decreaseFadingLevel();
    }

    // sets the background color to black
    SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
    SDL_RenderClear(renderer);

    const std::array < std::array<Pixel, Display::DISPLAY_WIDTH>, Display::DISPLAY_HEIGHT>* frame =
        m_display.getDisplayFrame();

    for (int row = 0; row < Display::DISPLAY_HEIGHT; ++row)
    {
        for (int column=0; column< Display::DISPLAY_WIDTH; ++column)
        {
            Chip8::Chip8::Pixel pixel = (*frame)[row][column];

            if (pixel.m_status == Chip8::Chip8::Status::on)
            {
                // sets color to white for on pixels
                SDL_SetRenderDrawColor(renderer, 255,255,255, 255);

                // every chip8 pixel becomes a square of side 20
                SDL_Rect pixelRectangle = SDL_Rect(20*column,20*row, 20,20);
                SDL_RenderFillRect(renderer, & pixelRectangle);
            }

            else if (pixel.m_fadingLevel > 0)
            {
                // the lightest grey we want to show has rgb code (125,125,125)
                constexpr int32_t lightestGrey { 125 };

                // the off pixel gets a color basing on the fading level
                // we want the color to be given by the rgb code
                // (colorShade,colorShade,colorShade), which is grey
                int32_t n = Display::MAXIMAL_FADING_VALUE / lightestGrey;
                uint8_t colorShade = static_cast<uint8_t>( pixel.m_fadingLevel / n );

                SDL_SetRenderDrawColor(renderer, colorShade,colorShade,colorShade, 255);

                SDL_Rect pixelRectangle = SDL_Rect( 20*column, 20*row, 20, 20 );
                SDL_RenderFillRect(renderer, & pixelRectangle);
            }
        }
    }
}

void Chip8Emulator::handleSystemEvents( SDL_Event ev )
{
    while (SDL_PollEvent(&ev) != 0)
        {
            switch (ev.type)
            {

            // if the user closes the window
            case SDL_QUIT:
            {
                std::unique_lock keyboardOrQuitWindowMutexLock {m_keyboardOrQuitWindowMutex};
                m_isRunning =  false;
                m_keyIsPressedOrWindowClosed.notify_one();
                break;
            }

            case SDL_KEYDOWN:
            {
                std::unique_lock keyboardOrQuitWindowMutexLock { m_keyboardOrQuitWindowMutex };
                std::optional<SDL_Scancode> pressedKey { ev.key.keysym.scancode };

                m_chip8PressedKey = getChip8Key(pressedKey);
                m_keyIsPressedOrWindowClosed.notify_one();
                break;
            }

            case SDL_KEYUP:
            {
                std::unique_lock keyboardOrQuitWindowMutexLock { m_keyboardOrQuitWindowMutex };
                std::optional<SDL_Scancode> pressedKey { };

                m_chip8PressedKey = getChip8Key(pressedKey);
                break;
            }

            default:
                break;

            }
        }
}

void Chip8Emulator::renderAndKeyboard( std::promise<bool>& promiseDisplayInitialized )
{
    SDL_Init(SDL_INIT_EVERYTHING);

    // set up renderer and window
    SDL_Window* window { SDL_CreateWindow(
                            "Chip8",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            1280, 640,
                            SDL_WINDOW_SHOWN ) };
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    promiseDisplayInitialized.set_value(true);

    std::unique_lock displayLock { m_displayMutex };
    displayLock.unlock();

    SDL_Event ev;
    ev.type = 0;

    while ( m_isRunning )
    {
        handleSystemEvents(ev);

        displayLock.lock();
        renderDisplay(renderer);
        displayLock.unlock();

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}


