#include "../chip8_emulator/chip8.h"
#include <SDL2/SDL.h>

SDL_Renderer* from_chip8_to_display(SDL_Renderer* renderer, Chip8::Chip8::Display display)
{
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
    return renderer;
}

int main()
{
    Chip8::Chip8 c = Chip8::Chip8();
    c.stack[0] = Chip8::Chip8::Address(1);
    c.SP = 0;
    //c.registers[0] = Register(0xf2);
    //c.registers[1] = Register(0x11);
    c.ram[0] = 0xF0;
    c.ram[1] = 0x10;
    c.ram[2] = 0xf0;
    c.ram[3] = 0x10;
    c.ram[4] = 0xf0;
    c.registers[1].update(0x40);
    c.registers[6].update(0x10);
    Chip8::Chip8::Instruction i = Chip8::Chip8::Instruction(static_cast<uint16_t>(0xd165));
    c.execute(i);

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 640, SDL_WINDOW_SHOWN );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
    SDL_RenderClear(renderer);

    renderer = from_chip8_to_display(renderer, c.display);
    SDL_Delay(200);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);

    SDL_DestroyWindow(window);
    SDL_Quit();
}

/*
int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 640, SDL_WINDOW_SHOWN );

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
*/

