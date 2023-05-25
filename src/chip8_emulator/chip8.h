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
#include <mutex>
#include <future>
#include <condition_variable>
#include <chrono>
#include <SDL2/SDL.h>

//namespace Chip8{

//enum class Chip8Type {chip8, schip8};

//class Chip8;
//}


namespace Chip8 {

    enum class Chip8Type {chip8, schip8};

    class Chip8 {

    public: // to be changed to private in the future

        using Register = uint8_t;
        using Address = uint16_t;

        class Instruction {
        public:
            explicit Instruction(const uint16_t i) : inst{ i } {}

            uint16_t instruction() const {return inst;}

            uint16_t inst;

        };

        enum class Status {off, on};

        class Pixel {
        public:
            Pixel() : status{ Status::off } { }
            Pixel(Status s) : status{ s } { }

            Pixel operator^(uint8_t u) const;

            std::string toString() const
            {
                std::string res = (status == Status::on)? "#" : ".";
                return res;
            }

            Status status;
        };

        class Display {
        public:
            Display() :
                d{ std::array<std::array<Pixel, 64>, 32>() } {}

            // takes the vector v and does xor with the pixels at starting at coordinate (x,y)
            // returns true if this causes any pixel to be unset and false otherwise
            bool drw(std::vector<uint8_t>&& a, const uint8_t x, const uint8_t y);

            std::string toString() const;

            std::array<std::array<Pixel, 64>, 32> d;
        };

        class HexadecimalSprite {
        public:
            HexadecimalSprite(uint8_t u) : sprite { std::array<uint8_t, 5>() }
            {
                assert((0 <= u) && (u <= 0xf));

                switch (u)
                {
                case 0x0:
                {
                    sprite[0] = 0xf0;
                    sprite[1] = 0x90;
                    sprite[2] = 0x90;
                    sprite[3] = 0x90;
                    sprite[4] = 0xf0;
                    break;
                }

                case 0x1:
                {
                    sprite[0] = 0x20;
                    sprite[1] = 0x60;
                    sprite[2] = 0x20;
                    sprite[3] = 0x20;
                    sprite[4] = 0x70;
                    break;
                }

                case 0x2:
                {
                    sprite[0] = 0xf0;
                    sprite[1] = 0x10;
                    sprite[2] = 0xf0;
                    sprite[3] = 0x80;
                    sprite[4] = 0xf0;
                    break;
                }

                case 0x3:
                {
                    sprite[0] = 0xf0;
                    sprite[1] = 0x10;
                    sprite[2] = 0xf0;
                    sprite[3] = 0x10;
                    sprite[4] = 0xf0;
                    break;
                }

                case 0x4:
                {
                    sprite[0] = 0x90;
                    sprite[1] = 0x90;
                    sprite[2] = 0xf0;
                    sprite[3] = 0x10;
                    sprite[4] = 0x10;
                    break;
                }

                case 0x5:
                {
                    sprite[0] = 0xf0;
                    sprite[1] = 0x80;
                    sprite[2] = 0xf0;
                    sprite[3] = 0x10;
                    sprite[4] = 0xf0;
                    break;
                }

                case 0x6:
                {
                    sprite[0] = 0xf0;
                    sprite[1] = 0x80;
                    sprite[2] = 0xf0;
                    sprite[3] = 0x90;
                    sprite[4] = 0xf0;
                    break;
                }

                case 0x7:
                {
                    sprite[0] = 0xf0;
                    sprite[1] = 0x10;
                    sprite[2] = 0x20;
                    sprite[3] = 0x40;
                    sprite[4] = 0x40;
                    break;
                }

                case 0x8:
                {
                    sprite[0] = 0xf0;
                    sprite[1] = 0x90;
                    sprite[2] = 0xf0;
                    sprite[3] = 0x90;
                    sprite[4] = 0xf0;
                    break;
                }

                case 0x9:
                {
                    sprite[0] = 0xf0;
                    sprite[1] = 0x90;
                    sprite[2] = 0xf0;
                    sprite[3] = 0x10;
                    sprite[4] = 0xf0;
                    break;
                }

                case 0xa:
                {
                    sprite[0] = 0xf0;
                    sprite[1] = 0x90;
                    sprite[2] = 0xf0;
                    sprite[3] = 0x90;
                    sprite[4] = 0x90;
                    break;
                }

                case 0xb:
                {
                    sprite[0] = 0xe0;
                    sprite[1] = 0x90;
                    sprite[2] = 0xe0;
                    sprite[3] = 0x90;
                    sprite[4] = 0xe0;
                    break;
                }

                case 0xc:
                {
                    sprite[0] = 0xf0;
                    sprite[1] = 0x80;
                    sprite[2] = 0x80;
                    sprite[3] = 0x80;
                    sprite[4] = 0xf0;
                    break;
                }

                case 0xd:
                {
                    sprite[0] = 0xe0;
                    sprite[1] = 0x90;
                    sprite[2] = 0x90;
                    sprite[3] = 0x90;
                    sprite[4] = 0xe0;
                    break;
                }

                case 0xe:
                {
                    sprite[0] = 0xf0;
                    sprite[1] = 0x80;
                    sprite[2] = 0xf0;
                    sprite[3] = 0x80;
                    sprite[4] = 0xf0;
                    break;
                }

                case 0xf:
                {
                    sprite[0] = 0xf0;
                    sprite[1] = 0x80;
                    sprite[2] = 0xf0;
                    sprite[3] = 0x80;
                    sprite[4] = 0x80;
                    break;
                }

                default:
                    break;
                }
            }

            std::array<uint8_t, 5> sprite;
        };

    public:
        Chip8() :
        ram{ std::array<uint8_t, 4096>() },
        registers{ std::array<Register, 16>() },
        I{ 0 },
        delayTimer{ std::atomic<Register>() },
        soundTimer{ Register() },
        PC{ Address(0x200) },
        SP{ 0 },
        stack{ std::array<Address, 16>() },
        display{ Display() },
        displayMutex{ std::mutex() },
        isRunning { false },
        pressedKey { std::optional<SDL_Keycode>() },
        keyboardMutex { std::mutex() },
        keyIsPressed { std::condition_variable() }
        {
            uint8_t ramIndex = 0;
            for (uint8_t u = 0x0; u <= 0xf; ++u)
            {
                HexadecimalSprite hexSprite { u };

                for (uint8_t x : hexSprite.sprite)
                {
                    ram[ramIndex] = x;
                    ++ramIndex;
                }
            }
        }
        //end_program{ false },
        //display_initialized{ false }  {}

        // read instructions from file and return a vector with instructions in hexadecimal ints
        void readFromFile(const std::filesystem::path path);

        // runs the instructions
        // flagChip8 is 0 if we run the Chip8 instructions and 1 for the SChip8
        void run(std::future<bool>& futureDisplayInitialized, std::future<bool>& futureDisplayDone, Chip8Type flagChip8);

        // instruction 00e0
        void cls() { display = Display(); }

        // instruction 00ee
        void ret() { PC = stack[SP]; --SP; }

        // instrucion 1nnn
        void jp(const uint16_t nnn);

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
        void bitOr(const uint8_t xy);

        // instruction 8xy2
        void bitAnd(const uint8_t xy);

        // instruction 8xy3
        void bitXor(const uint8_t xy);

        // instruction 8xy4
        void add(const uint8_t xy);

        // instruction 8xy5
        void sub(const uint8_t xy);

        // instruction 8xy6
        void shr(const uint8_t xy, Chip8Type flagChip8);

        // instruction 8xy7
        void subn(const uint8_t xy);

        // instruction 8xye
        void shl(const uint8_t xy, Chip8Type flagChip8);

        // instruction 9xy0
        void sne(const uint8_t xy);

        // instruction annn
        void ldI(const uint16_t nnn);

        // instruction bnnn
        void jpV0(const uint16_t nnn);

        // instruction cxkk
        void rnd(const uint16_t xkk);

        // instruction dxyn
        void drw(const uint16_t xyn);

        // instruction ex9e
        void skp(const uint8_t x);

        // instruction exa1
        void sknp(const uint8_t x);

        // instruction fx07
        void ldVxDT(const uint8_t x);

        // instruction fx0a
        void ldVxK(const uint8_t x);

        // instruction fx15
        void ldDTVx(const uint8_t x);

        // instruction fx1e
        void addI(const uint8_t x);

        // instruction fx29
        void ldFVx(const uint8_t x);

        // instruction fx33
        void ldB(const uint8_t x);

        // instruction fx55
        void ldIVx(const uint8_t x, Chip8Type flagChip8);

        // instruction fx65
        void ldVxI(const uint8_t x, Chip8Type flagChip8);

        void execute(const Instruction i, Chip8Type flagChip8);

        std::optional<Register> getChip8Key() const;


    //private:
        // the ram consists of register 0 to 4095 and each register has 8 bits
        std::array<uint8_t, 4096> ram;

        std::array<Register, 16> registers; // chip-8 has 16 registers of 8 bits
        uint16_t I; // 16-bits register to store memory address
        std::atomic<Register> delayTimer; // 8-bits register for delay
        Register soundTimer; // 8-bits register for sound
        Address PC; // program counter
        uint8_t SP; // 8 bits for pointing to the topmost level of the stack
        std::array<Address, 16> stack; // the stack is an array of 16 16-bits values to store addresses
        Display display;
        std::mutex displayMutex;
        bool isRunning;
        std::optional<SDL_Keycode> pressedKey;
        std::mutex keyboardMutex;
        std::condition_variable keyIsPressed;
    };
}
