#include "display.h"

int main()
{
    Chip8::Chip8 c = Chip8::Chip8();
    c.stack[0] = Chip8::Chip8::Address(1);
    c.SP = 0;
    //c.registers[0] = Register(0xf2);
    //c.registers[1] = Register(0x11);
    c.ram[0] = 0x90;
    c.ram[1] = 0x90;
    c.ram[2] = 0xf0;
    c.ram[3] = 0x10;
    c.ram[4] = 0x10;
    c.registers[1].update(0x82);
    c.registers[6].update(0x10);
    c.registers[3].update(0x57);
    c.registers[7].update(0xd9);
    c.registers[0xa].update(0xa1);
    Chip8::Chip8::Instruction i = Chip8::Chip8::Instruction(static_cast<uint16_t>(0xd165));
    Chip8::Chip8::Instruction j = Chip8::Chip8::Instruction(static_cast<uint16_t>(0xd365));
    Chip8::Chip8::Instruction l = Chip8::Chip8::Instruction(static_cast<uint16_t>(0xda75));
    Chip8::Chip8::Instruction k = Chip8::Chip8::Instruction(static_cast<uint16_t>(0xd115));
    c.run(std::vector<Chip8::Chip8::Instruction> {i,k,j,l});



    /*c.execute(i);

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
    SDL_Quit();*/
}
