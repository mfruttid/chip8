#include "../src/chip8_emulator/chip8.h"

bool check_ret() // prints 2 and 1 (2 is not visible because uint8)
{
    Chip8::Chip8 c;
    c.stack[0] = Chip8::Chip8::Address(1);
    c.SP = 3;
    c.ret();
    return (c.SP == 2) && (c.PC == Chip8::Chip8::Address(1));
}

void check_run()
{
    Chip8::Chip8 chip8;
    std::vector<Chip8::Chip8::Instruction> instructions = chip8.readFromFile("/home/martina/cpp/chip8/programs/Breakout [Carmelo Cortez, 1979].ch8");
    std::cout << "\n";
    chip8.run(instructions);
}

void check_execute() // prints 2 and 1 (2 is not visible because uint8)
{
    Chip8::Chip8 c;
    c.stack[0] = Chip8::Chip8::Address(1);
    c.SP = 0;
    //c.registers[0] = Register(0xf2);
    //c.registers[1] = Register(0x11);
    c.ram[0] = 0xf6;
    c.ram[1] = 0x0c;
    Chip8::Chip8::Instruction i = Chip8::Chip8::Instruction(static_cast<uint16_t>(0xf165));
    c.execute(i);
    std::cout << std::hex << c.SP << c.PC.toString() << "\n";
    /*for (size_t j = 0; j <= c.SP; ++j)
    {
        std::cout << c.stack[j].toString() << " ";
    }*/
    std::cout << c.registers[1].toString() << " " << c.registers[0].toString() << "\n";
    std::cout << std::hex << c.I << " " << static_cast<uint32_t>(c.ram[0]) << " " << static_cast<uint32_t>(c.ram[1]) << " " << static_cast<uint32_t>(c.ram[2]);
}

int main()
{
    check_execute();
}

