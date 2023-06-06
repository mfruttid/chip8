#include "../src/chip8_emulator/chip8.h"
# include "../src/displayAndKeyboard/displayAndKeyboard.h"

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
    chip8.readFromFile("/home/martina/cpp/chip8/programs/Breakout [Carmelo Cortez, 1979].ch8");
    std::cout << "\n";
//    chip8.run();
}

void check_execute() // prints 2 and 1 (2 is not visible because uint8)
{
    Chip8::Chip8 c;
    c.stack[0] = Chip8::Chip8::Address(1);
    c.SP = 0;
    //c.registers[0] = Register(0xf2);
    //c.registers[1] = Register(0x11);
    c.ram[0] = 0xF0;
    c.ram[1] = 0x10;
    c.ram[2] = 0xf0;
    c.ram[3] = 0x10;
    c.ram[4] = 0xf0;
    c.registers[1] = 0x61;
    c.registers[6] = 0x10;
    Chip8::Chip8::Instruction i = Chip8::Chip8::Instruction(static_cast<uint16_t>(0xe19e));
    c.execute(i,Chip8::Chip8Type::chip8);
    std::cout << std::hex << c.SP << c.PC << "\n";
    /*for (size_t j = 0; j <= c.SP; ++j)
    {
        std::cout << c.stack[j].toString() << " ";
    }*/
    std::cout << c.registers[1] << " " << c.registers[0] << "\n";
    std::cout << std::hex << c.I << " " << static_cast<uint32_t>(c.ram[0]) << " " << static_cast<uint32_t>(c.ram[1]) << " " << static_cast<uint32_t>(c.ram[2]);
    std::cout << "\n";
    std::cout << c.display.toString();
}
/*
void run(Chip8::Chip8& c)
{
    const auto start = std::chrono::high_resolution_clock::now();

    uint16_t byte1 = static_cast<uint16_t>(c.ram[c.PC]);
    byte1 = static_cast<uint16_t>(byte1 << 8u);

    uint16_t byte2 = static_cast<uint16_t>(c.ram[c.PC+1]);
    Chip8::Chip8::Instruction instruction { static_cast<uint16_t>(byte1 | byte2) };

    c.execute(instruction);

    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double, std::milli> sleep_time = std::chrono::seconds(1/500) - (end - start);
    std::this_thread::sleep_for(sleep_time);
}*/

int main()
{
    Chip8::Chip8 c;

    c.ram[0x200] = 0xe1;
    c.ram[0x201] = 0x9e;
    c.registers[1] = 0x61;

    std::promise<bool> promiseDisplayInitialized;
    std::future<bool> futureDisplayInitialized = promiseDisplayInitialized.get_future();

    std::promise<bool> promiseDisplayDone;
    std::future<bool> futureDisplayDone = promiseDisplayDone.get_future();

//    std::thread chip8Thread {&Chip8::Chip8::run, std::ref(c), std::ref(futureDisplayInitialized), std::ref(futureDisplayDone)};
//    chip8Thread.detach();

//    showDisplay(c, promiseDisplayInitialized, promiseDisplayDone);

}


