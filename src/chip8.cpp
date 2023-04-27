#include "chip8.h"

std::string Address::toString() const
{
    std::stringstream stream;
    stream << "0x" << std::hex << address;
    return stream.str();
}

const Address Chip8::top_stack() const
{
    return stack[SP];
}

void Chip8::call(const uint16_t u)
{
    ++SP;
    stack[SP] = PC;
    PC = Address(u);
}

std::vector<Instruction> Chip8::readFromFile(const std::filesystem::path path) const
    {
        assert(exists(path));

        std::vector<Instruction> output;
        std::ifstream file {path};

        if (file.is_open())
        {
            uint16_t byte1, byte2;
            while (!file.eof())
            {
                byte1 = static_cast<uint16_t>(file.std::istream::get());
                byte1 = static_cast<uint16_t>(byte1 << 8u);
                byte2 = static_cast<uint16_t>(file.std::istream::get());

                uint16_t i =static_cast<uint16_t>(byte1 | byte2);
                output.emplace_back(i); // construct Instruction(i) in-place
            }

            return output;
        }
        else
        {
            std::cout << "Unable to open file.\n";
            return {};
        }
    }

void Chip8::run(const std::vector<Instruction> instructions)
    {
        for (Instruction instruction : instructions)
        {
            execute(instruction);
        }
    }

void Chip8::execute(const Instruction i)
{
    uint16_t inst = i.instruction();
    switch (inst)
    {
    case static_cast<uint16_t>(0x00ee):
        ret();
        break;

    default:
        break;
    }

    uint16_t first4bits = (inst & 0xf000) >> 12u;
    switch (first4bits)
    {
    case 1:
        {
            uint16_t nnn = static_cast<uint16_t>(inst & 0xfff);
            jp(nnn);
            break;
        }

    case 2:
    {
        uint16_t nnn = static_cast<uint16_t>(inst & 0xfff);
        call(nnn);
        break;
    }

    default:
        break;
    }
}
