#include "../src/chip8.h"

bool check_ret() // prints 2 and 1 (2 is not visible because uint8)
{
    Chip8 c;
    c.stack[0] = Address(1);
    c.SP = 3;
    c.ret();
    return (c.SP == 2) && (c.PC == Address(1));
}

void check_run()
{
    Chip8 chip8;
    std::vector<Instruction> instructions = chip8.readFromFile("/home/martina/cpp/chip8/programs/Breakout [Carmelo Cortez, 1979].ch8");
    std::cout << "\n";
    chip8.run(instructions);
}

void check_execute() // prints 2 and 1 (2 is not visible because uint8)
{
    Chip8 c;
    c.stack[0] = Address(1);
    c.SP = 0;
    Instruction i = Instruction(static_cast<uint16_t>(0x2e45));
    c.execute(i);
    std::cout << std::hex << c.SP << c.PC.toString() << "\n";
    for (size_t j = 0; j <= c.SP; ++j)
    {
        std::cout << c.stack[j].toString() << " ";
    }
}

int main()
{
    check_execute();
}

