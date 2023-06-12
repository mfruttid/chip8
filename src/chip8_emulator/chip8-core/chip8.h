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
#include <span>
#include <thread>
#include <functional>
#include <cstring>


namespace Chip8 {

    class Chip8 {

    private:

        // some instructions differ and some programs run correctly
        // with one set of instructions and others with the other
        // the default one is chip8, but if you specify "schip8"
        // when you start the program, the set of instructions used changes
        enum class Chip8Type { chip8, schip8 };

        // there are two different versions of the draw instruction dxyn
        // one of them clips the pixels that are positioned over the end of the display
        // the other wraps them to the other side of the display
        // the default setting is clipping, but this can be changed specifying "wrap"
        // when starting the program
        enum class DrawInstruction { clip, wrap };

        using Register = uint8_t;

        using Address = uint16_t;

        class Instruction {
        public:

            explicit Instruction( const uint16_t i ) : inst{ i } {}

            uint16_t inst;

        };

        // font sprites that represent numbers from 0x0 to 0xf
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

    protected:

        enum class Status { off, on };

        class Pixel {
        public:

            Pixel() : status{ Status::off }, fadingLevel{ 0 } { }
            Pixel(Status s, int32_t fadinglev) : status{ s }, fadingLevel{ fadinglev } { }

            // xor between pixel and u where u = 0 or 1
            // keeps the same fading level
            Pixel operator^(uint8_t u) const;

            // says if a pixel is on or off
            Status status;

            // if a pixel turns from on to off, it doesn't completely go black,
            // simulating an old phosphorous CRT-style effect.
            // The fadingLevel takes care of how much the pixel is faded:
            // the higher it is, the lighter the color of the pixel
            int32_t fadingLevel;
        };

        class Display {
        public:
            Display() :
                frame{ std::array< std::array< Pixel, 64 >, 32 >() },
                MAXIMALFADING{ 500 } {}

            // decrease fading level by 1 for each pixel in the frame
            void decreaseFadingLevel();

            // takes the vector v and does xor with the pixels starting at coordinate (x,y)
            // returns true if this causes any pixel to be unset and false otherwise
            // clips the pixels over the end of the screen
            bool drwClip(std::vector<uint8_t>&& a, const uint8_t x, const uint8_t y);

            // takes the vector v and does xor with the pixels starting at coordinate (x,y)
            // returns true if this causes any pixel to be unset and false otherwise
            // wraps the pixels over the end of the screen
            bool drwWrap(std::vector<uint8_t>&& a, const uint8_t x, const uint8_t y);

            std::array< std::array< Pixel, 64 >, 32 > frame;

            // it's the maximal value the fadingLevel of a pixel can have
            // The higher the MAXIMALFADING, the longer it will take for a pixel
            // to go completely black.
            // It has default value of 500, which results to the display showing
            // two different tones of grey every time a pixel is turned off
            int32_t MAXIMALFADING;
        };

    protected:

        // the ram consists of register 0 to 4095 and each register has 8 bits
        std::array< Register, 4096 > ram;

        std::array< Register, 16 > registers; // chip-8 has 16 registers of 8 bits

        Address I; // 16-bits register to store memory address

        std::atomic< Register > delayTimer; // 8-bits register for delay
        std::atomic< Register > soundTimer; // 8-bits register for sound

        Address PC; // program counter

        uint8_t SP; // 8 bits for pointing to the topmost level of the stack

        std::array< Address, 16 > stack; // the stack is an array of 16 16-bits values to store addresses

        Display display;
        std::mutex displayMutex;

        bool isRunning; // tells when the user closed the window so that the program stops

        std::optional<Register> chip8PressedKey;
        std::mutex keyboardOrQuitWindowMutex;
        std::condition_variable keyIsPressedOrWindowClosed;

        // specifies the settings with which we want to run the program
        Chip8Type chip8Type;
        DrawInstruction drawInstruction;

    public:

        Chip8(const char* whichChip8Type, const char* whichDrawInstruction) :
            ram{ std::array<Register, 4096>() },
            registers{ std::array<Register, 16>() },
            I{ 0 },
            delayTimer{ std::atomic<Register>() },
            soundTimer{ std::atomic<Register>() },
            PC{ Address(0x200) }, // usually the first 0x200 addresses in ram are not used by the program
            SP{ 0 },
            stack{ std::array<Address, 16>() },
            display{ Display() },
            displayMutex{ std::mutex() },
            isRunning { false },
            chip8PressedKey { std::optional<Register>() },
            keyboardOrQuitWindowMutex { std::mutex() },
            keyIsPressedOrWindowClosed { std::condition_variable() },
            // if whichChip8Type is "schip8", then we set the chip8Type to be schip8,
            // otherwise it is chip8
            chip8Type { (std::strcmp( whichChip8Type, "schip8" ) == 0)? Chip8Type::schip8 : Chip8Type::chip8 },
            // if whichDrawInstruction is "wrap", then we set drawInstruction to be wrap
            // otherwise it is clip
            drawInstruction { (std::strcmp( whichDrawInstruction, "wrap" ) == 0)?  DrawInstruction::wrap
                                : DrawInstruction::clip }
        {
            // the first addresses of the ram are used for the hexadecimal sprites
            uint8_t ramIndex = 0;
            for (uint8_t u = 0x0; u <= 0xf; ++u)
            {
                const HexadecimalSprite& hexSprite { u };

                for (uint8_t line : hexSprite.sprite)
                {
                    ram[ramIndex] = line;
                    ++ramIndex;
                }
            }
        }

    protected:
        // read instructions from file and copies them in ram starting at address 0x200
        void readFromFile(const std::filesystem::path path);

        // runs the program that has been copied in ram
        void run( std::future<bool>& futureDisplayInitialized );

    private:
        void execute(const Instruction i);

        static void decreaseTimer( std::atomic<Register>& timer );

        void decreaseDelayTimer();

        void decreaseSoundTimer();

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
        void shr(const uint8_t xy);

        // instruction 8xy7
        void subn(const uint8_t xy);

        // instruction 8xye
        void shl(const uint8_t xy);

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

        // instruction fx18
        void ldSTVx(const uint8_t x);

        // instruction fx1e
        void addI(const uint8_t x);

        // instruction fx29
        void ldFVx(const uint8_t x);

        // instruction fx33
        void ldB(const uint8_t x);

        // instruction fx55
        void ldIVx(const uint8_t x);

        // instruction fx65
        void ldVxI(const uint8_t x);
    };
}
