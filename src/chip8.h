//#pragma once

#include <iostream>
#include <array>
#include  <cstddef>
#include <bitset>

class Register{
public:
    Register() : reg{std::bitset<8>()} {}
private:
    std::bitset<8> reg; // the usual register has 8 bits
};

class Address{
public:
    Address() : address{ std::bitset<16>() } {}
private:
    std::bitset<16> address; // an address consists of 12 bits, so only the rightmost 12 bits are gonna be filled
};

class Chip8{
public:
    Chip8() :
    ram{ std::array<std::bitset<8>, 4096>() },
    registers{ std::array<Register, 16>() },
    I{ std::bitset<16>() },
    timer{ Register() },
    sound{ Register() },
    PC{ std::bitset<16>() },
    stackPointer{ std::bitset<8>() },
    stack{ std::array<Address, 16>() }  {}

private:
    // the ram consists of register 0 to 4095 and each register has 8 bits
    std::array<std::bitset<8>, 4096> ram;

    std::array<Register, 16> registers; // chip-8 has 16 registers of 8 bits
    std::bitset<16> I; // 16-bits register to store memory address
    Register timer; // 8-bits register for delay
    Register sound; // 8-bits register for sound
    std::bitset<16> PC; // program counter
    std::bitset<8> stackPointer; // 8 bits for pointing to the topmost level of the stack
    std::array<Address, 16> stack; // the stack is an array of 16 16-bits values to store addresses
};