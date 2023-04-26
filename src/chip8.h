#pragma once

#include <iostream>
#include <array>
#include <string>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <vector>


class Register{
public:
    Register() : reg{ 0 } {}
private:
    uint8_t reg; // the usual register has 8 bits
};

class Address{
public:
    Address() : address{ 0 } {}
private:
    uint16_t address; // an address consists of 12 bits, so only the rightmost 12 bits are gonna be filled
};

class Chip8{
public:
    Chip8() :
    ram{ std::array<uint8_t, 4096>() },
    registers{ std::array<Register, 16>() },
    I{ 0 },
    timer{ Register() },
    sound{ Register() },
    PC{ 0 },
    stackPointer{ 0 },
    stack{ std::array<Address, 16>() }  {}

    std::vector<uint8_t> readFromFile(std::filesystem::path path)
    {
        assert(exists(path));
        std::vector<uint8_t> output;
        std::ifstream file {path};
        if (file.is_open())
        {
            uint8_t byte;
            while (!file.eof())
            {
                file >> byte;
                output.push_back(byte);
            }
            for (uint8_t v : output)
            {
                std::cout << std::hex << (uint16_t) v << " ";
            }
            return output;
        }
        else
        {
            std::cout << "Unable to open file.\n";
            return {};
        }
    }

private:
    // the ram consists of register 0 to 4095 and each register has 8 bits
    std::array<uint8_t, 4096> ram;

    std::array<Register, 16> registers; // chip-8 has 16 registers of 8 bits
    uint16_t I; // 16-bits register to store memory address
    Register timer; // 8-bits register for delay
    Register sound; // 8-bits register for sound
    uint16_t PC; // program counter
    uint8_t stackPointer; // 8 bits for pointing to the topmost level of the stack
    std::array<Address, 16> stack; // the stack is an array of 16 16-bits values to store addresses
};