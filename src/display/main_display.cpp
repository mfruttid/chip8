#include "display.h"

int main()
{
    Chip8::Chip8 c = Chip8::Chip8();
    c.readFromFile(std::filesystem::path("/home/martina/cpp/chip8/programs/octojam1title.ch8"));
    c.run();

}
