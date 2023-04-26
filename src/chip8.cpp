#include "chip8.h"

std::vector<Instruction> Chip8::readFromFile(std::filesystem::path path)
    {
        assert(exists(path));
        std::vector<Instruction> output;
        std::ifstream file {path};
        if (file.is_open())
        {
            uint8_t byte1, byte2;
            while (!file.eof())
            {
                byte1 = (uint8_t) file.std::istream::get();
                byte2 = (uint8_t) file.std::istream::get();
                uint16_t i = (uint16_t) (((uint16_t)(byte1) * 16 * 16) + (uint16_t)(byte2));
                output.emplace_back(Instruction(i));
            }

            return output;
        }
        else
        {
            std::cout << "Unable to open file.\n";
            return {};
        }
    }

void Chip8::run(std::vector<Instruction> instructions)
    {
        for (Instruction instruction : instructions)
        {
            instruction.run();
        }
    }

int main() {
    Chip8 chip8;
    std::vector<Instruction> instructions = chip8.readFromFile("/home/martina/cpp/chip8/programs/Breakout [Carmelo Cortez, 1979].ch8");
    std::cout << "\n";
    chip8.run(instructions);
}
