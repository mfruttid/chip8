#include "../chip8_emulator/chip8.h"
#include <SDL2/SDL.h>

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_SetRenderDrawColor(renderer, 255,255,255, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0,0,255, 255);
    SDL_RenderDrawLine(renderer, 5,5, 100,100);

    SDL_Rect rectangle = SDL_Rect(100,60, 200,200);
    SDL_RenderFillRect(renderer, &rectangle);

    SDL_Delay(200);

    SDL_RenderPresent(renderer);

    SDL_Delay(3000);

    SDL_GetError();
    SDL_DestroyWindow(window);
    SDL_Quit();

}


