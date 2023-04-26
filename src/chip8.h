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

class Instruction{
public:
    Instruction(uint16_t i) : inst{ i } {}

    void run()
    {
        std::cout << std::hex << inst << " ";
    }

    void operator+=(uint16_t x) { inst = (uint16_t) (inst + x); }

private:
    uint16_t inst;
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

    // read instructions from file and return a vector with instructions in hexadecimal ints
    std::vector<Instruction> readFromFile(std::filesystem::path path);

    // runs the instructions
    void run(std::vector<Instruction> instructions);

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