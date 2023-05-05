#pragma once

#include <iostream>
#include <array>
#include <string>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <vector>
#include <random>
#include <ranges>

namespace Chip8{
class Chip8;
}

namespace Chip8_internals {
    class Register {
    public:
        Register() : reg{ 0 } {}
        Register(uint8_t u) : reg{ u } {}

        bool operator==(const uint8_t u) const { return reg ==u; }

        bool operator==(const Register r) const { return reg == r.reg; }

        bool operator!=(const uint8_t u) const { return reg != u; }

        void update(const uint8_t u) { reg = u; }

        void operator+=(const uint16_t u) { reg = static_cast<uint8_t>(reg + u); }

        uint16_t operator+(const uint16_t u) { return static_cast<uint16_t>(u + reg); }

        std::string toString() const;

        friend class Chip8::Chip8;

    private:
        uint8_t reg; // the usual register has 8 bits
    };

    class Address {
    public:
        Address() : address{ 0 } {}
        Address(const uint16_t u) : address{ u } {}

        bool isEmpty() const { return address == 0; }

        std::string toString() const;

        bool operator==(Address a) const { return address == a.address; }

        void operator+=(int i) { address = static_cast<uint16_t>(address+i); }

    private:
        uint16_t address; // an address consists of 12 bits, so only the rightmost 12 bits are gonna be filled
    };

    class Instruction {
    public:
        explicit Instruction(const uint16_t i) : inst{ i } {}

        void operator+=(const uint16_t x) { inst = static_cast<uint16_t>(inst + x); }

        uint16_t instruction() const {return inst;}

    private:
        uint16_t inst;

    };

    enum class Status {off, on};

    class Pixel {
    public:
        Pixel() : status{ Status::off } { }
        Pixel(Status s) : status{ s } { }
        Pixel operator^(uint8_t u) const;

    //private:
        Status status;
    };

    class Display {
    public:
        Display() : d{ std::array<std::array<Pixel, 64>, 32>() } {}

        // takes the vector v and does xor with the pixels at starting at coordinate (x,y)
        // returns true if this causes any pixel to be unset and false otherwise
        bool drw(auto a, const uint8_t x, const uint8_t y);

    private:
        std::array<std::array<Pixel, 64>, 32> d;
    };

    /*template<uint8_t n>
        requires (n > 0) && (n < 16)
    class Sprite {
    public:
        Sprite() : sprite { std::array<std::array<Pixel, 8>, n>() } {}
        Sprite(std::array< uint8_t, n> a) : sprite { std::array<std::array<Pixel, 8>, n>() }
        {
            for (int i : std::ranges::iota_view{0,n})
            {
                uint8_t u = a[i];
                for (int j=0; j<8; ++j)
                {
                    if (u & 1 == 1)
                    {
                        sprite[i][j] = Pixel(Status::on);
                    }
                    u >> 1u;
                }
            }
        }
    private:
        std::array<std::array<Pixel, 8>, n> sprite;
    };*/
}

namespace Chip8 {
    class Chip8 {
    public:
        Chip8() :
        ram{ std::array<uint8_t, 4096>() },
        registers{ std::array<Chip8_internals::Register, 16>() },
        I{ 0 },
        timer{ Chip8_internals::Register() },
        sound{ Chip8_internals::Register() },
        PC{ Chip8_internals::Address() },
        SP{ 0 },
        stack{ std::array<Chip8_internals::Address, 16>() },
        display{ Chip8_internals::Display() }  {}

        // read instructions from file and return a vector with instructions in hexadecimal ints
        std::vector<Chip8_internals::Instruction> readFromFile(const std::filesystem::path path) const;

        // runs the instructions
        void run(const std::vector<Chip8_internals::Instruction> instructions);

        // instruction 00e0
        void cls() { display = Chip8_internals::Display(); }

        // instruction 00ee
        void ret() { PC = top_stack(); --SP; }

        // instrucion 1nnn
        void jp(const uint16_t nnn) { PC = nnn; }

        // instruction 2nnn
        void call(const uint16_t nnn);

        // instruction 3xkk
        void se(const uint16_t xkk);

        // instruction 4xkk
        void sne(const uint16_t xkk);

        // instruction 5xy0
        void se(const uint8_t xy);

        // instruction 6xkk
        void ld(const uint16_t xkk);

        // instruction 7xkk
        void add(const uint16_t xkk);

        // instruction 8xy0
        void ld(const uint8_t xy);

        // instruction 8xy1
        void bit_or(const uint8_t xy);

        // instruction 8xy2
        void bit_and(const uint8_t xy);

        // instruction 8xy3
        void bit_xor(const uint8_t xy);

        // instruction 8xy4
        void add(const uint8_t xy);

        // instruction 8xy5
        void sub(const uint8_t xy);

        // instruction 8xy6
        void shr(const uint8_t xy);

        // instruction 8xy7
        void subn(const uint8_t xy);

        // instruction 8xye
        void shl(const uint8_t xy);

        // instruction 9xy0
        void sne(const uint8_t xy);

        // instruction annn
        void ld_I(const uint16_t nnn);

        // instruction bnnn
        void jp_v0(const uint16_t nnn);

        // instruction cxkk
        void rnd(const uint16_t xkk);

        // instruction dxyn
        void drw(const uint16_t xyn);

        // instruction fx1e
        void add_I(const uint8_t x);

        // instruction fx33
        void ld_B(const uint8_t x);

        // instruction fx55
        void ldIVx(const uint8_t x);

        // instruction fx65
        void ldVxI(const uint8_t x);

        void execute(const Chip8_internals::Instruction i);


    //private:
        // the ram consists of register 0 to 4095 and each register has 8 bits
        std::array<uint8_t, 4096> ram;

        std::array<Chip8_internals::Register, 16> registers; // chip-8 has 16 registers of 8 bits
        uint16_t I; // 16-bits register to store memory address
        Chip8_internals::Register timer; // 8-bits register for delay
        Chip8_internals::Register sound; // 8-bits register for sound
        Chip8_internals::Address PC; // program counter
        uint8_t SP; // 8 bits for pointing to the topmost level of the stack
        std::array<Chip8_internals::Address, 16> stack; // the stack is an array of 16 16-bits values to store addresses
        Chip8_internals::Display display;

        const Chip8_internals::Address top_stack() const;

    };
}
