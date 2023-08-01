#pragma once

#include <iostream>
#include <array>
#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <vector>
#include <random>
#include <ranges>
#include <stack>
#include <mutex>
#include <future>
#include <condition_variable>
#include <chrono>
#include <SDL.h>
#include <span>
#include <thread>
#include <functional>
#include <cstring>
#include <optional>
#include <sound.h>
#include <base64decode_sound.h>

class Chip8 {

private:

    // some instructions differ and some programs run correctly
    // with one set of instructions and others with the other
    // the default one is chip8, but if you specify "schip8"
    // when you start the program, the set of instructions used changes
    enum class InstructionSet {chip8, schip8};

    // there are two different versions of the draw instruction dxyn
    // one of them clips the pixels that are positioned over the end of the display
    // the other wraps them to the other side of the display
    // the default setting is clipping, but this can be changed specifying "wrap"
    // when starting the program
    enum class DrawBehaviour {clip, wrap};

    using Register = uint8_t;

    using Address = uint16_t;

    // this struct could be an alias of uint16_t
    // it is wrapped only for type safety, so that it is not possible
    // to perform integer operations on the instructions
    struct Instruction {

        explicit Instruction(const uint16_t i) : m_inst {i} {}

        uint16_t m_inst;

    };

    // array of the hexadecimal sprites to be copied in m_ramPtr
    inline constexpr static std::array<std::array<uint8_t, 5>, 16> m_hexadecimalSprites {{
        { 0xf0, 0x90, 0x90, 0x90, 0xf0 },
        { 0x20, 0x60, 0x20, 0x20, 0x70 },
        { 0xf0, 0x10, 0xf0, 0x80, 0xf0 },
        { 0xf0, 0x10, 0xf0, 0x10, 0xf0 },
        { 0x90, 0x90, 0xf0, 0x10, 0x10 },
        { 0xf0, 0x80, 0xf0, 0x10, 0xf0 },
        { 0xf0, 0x80, 0xf0, 0x90, 0xf0 },
        { 0xf0, 0x10, 0x20, 0x40, 0x40 },
        { 0xf0, 0x90, 0xf0, 0x90, 0xf0 },
        { 0xf0, 0x90, 0xf0, 0x10, 0xf0 },
        { 0xf0, 0x90, 0xf0, 0x90, 0x90 },
        { 0xe0, 0x90, 0xe0, 0x90, 0xe0 },
        { 0xf0, 0x80, 0x80, 0x80, 0xf0 },
        { 0xe0, 0x90, 0x90, 0x90, 0xe0 },
        { 0xf0, 0x80, 0xf0, 0x80, 0xf0 },
        { 0xf0, 0x80, 0xf0, 0x80, 0x80 }
     }};

public:
    struct Pixel;
    class Display;

    enum class Status {off, on};
    enum class Fading {on, off};

    std::unique_ptr<std::array<Register, 4096>> m_ramPtr =
        std::make_unique<std::array< Register, 4096>>();

    std::array<Register, 16> m_registers {};

    Address m_I {}; // 16-bits register to store memory address

    bool m_isRunning {false}; // tells when the user closed the window so that the program stops
    std::mutex m_isRunningMutex {};
    std::condition_variable m_hasStartedRunning {};

    std::atomic<Register> m_delayTimer {};
    std::mutex m_delayTimerMutex {};
    std::condition_variable m_setDelayTimer {}; // waits for the delay time to be non zero
    std::jthread m_delayTimerThread {[this] { this->Chip8::decreaseDelayTimer(); }};

    std::atomic<Register> m_soundTimer {};
    std::mutex m_soundTimerMutex {};
    std::condition_variable m_setSoundTimer {}; // waits for the sound timer to be non zero
    std::jthread m_soundTimerThread {[this] { this->Chip8::decreaseSoundTimer(); }};
    std::function<void()> m_playSoundCallback;
    std::function<void()> m_pauseSoundCallback;

    Address m_PC; // program counter

    uint8_t m_SP {}; // 8 bits for pointing to the topmost level of the stack

    std::array<Address, 16> m_stack {};

    Fading m_fadingFlag; // flag saying whether we want to enable the fading effect or not

    std::unique_ptr<Display> m_display {};
    mutable std::mutex m_displayMutex {};

    // every uint8_t corresponds to a key:
    // false = not pressed
    // true = pressed
    std::array<std::atomic<bool>, 16> m_chip8Keys {};

    // key used for instruction ldVxK
    std::optional<Register> m_lastPressedKey {};

    // this mutex protects m_chip8PressedKey and m_isRunning
    std::mutex m_eventMutex {};
    // locks the m_eventMutex and checks that a key has been pressed or that m_isRunning is false
    std::condition_variable m_eventHappened {};

    // specifies the settings with which we want to run the program
    InstructionSet m_instructionSet; // set of instructions
    DrawBehaviour m_drawBehaviour; // drawing sprites using clipping or wrapping

public:

    Chip8(
        std::string_view flagChip8Type,
        std::string_view flagDrawInstruction,
        std::string_view flagFading,
        std::function<void()> playSoundCallback,
        std::function<void()> pauseSoundCallback
        );

    // reads instructions from file and copies them in ram
    void readFromFile(const std::filesystem::path& path);

    // runs the program that has been copied in ram
    void run(std::future<bool>&& futureDisplayInitialized);

private:
    void decreaseDelayTimer() { decreaseTimer(m_delayTimer, 0); }

    void decreaseSoundTimer() { decreaseTimer(m_soundTimer, 1); }

    void execute(const Instruction i);

    void decreaseTimer(std::atomic<Register>& timer, bool flagSound);

    // instruction 00e0
    void cls() { m_display = std::make_unique<Display>(m_fadingFlag); }

    // instruction 00ee
    void ret() { m_PC = m_stack[m_SP]; --m_SP; }

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
    void jpV0(const uint16_t nnm);

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

struct Chip8::Pixel {
    constexpr Pixel() : m_status {Status::off}, m_fadingLevel {0} {}
    Pixel(Status s, int32_t fadinglev) : m_status {s}, m_fadingLevel {fadinglev} {}

    // xor between pixel and status where status = off = 0 or on = 1
    // keeps the same fading level
    Chip8::Pixel operator^(Status s);

    // says if a pixel is on or off
    Chip8::Status m_status;

    // if a pixel turns from on to off, it doesn't completely go black,
    // simulating an old phosphorous CRT-style effect.
    // The fadingLevel takes care of how much the pixel is faded:
    // the higher it is, the lighter the color of the pixel
    int32_t m_fadingLevel;
};

class Chip8::Display {
public:
    Display(Chip8::Fading fadingFlag) :
        m_maximalFading {(fadingFlag == Fading::on) ? MAXIMAL_FADING_VALUE : 0}
    {}

    // decrease fading level by 1 (if > 0) for each pixel in the frame
    void decreaseFadingLevel();

    // does xor of the sprite with the pixels starting at coordinate (x,y)
    // returns true if this causes any pixel to be unset and false otherwise
    // clips the pixels over the end of the screen
    bool drwClip(std::vector<uint8_t>&& sprite, const uint8_t x, const uint8_t y);

    // does xor of the sprite with the pixels starting at coordinate (x,y)
    // returns true if this causes any pixel to be unset and false otherwise
    // wraps the pixels over the end of the screen
    bool drwWrap(std::vector<uint8_t>&& sprite, const uint8_t x, const uint8_t y);

    static constexpr int DISPLAY_WIDTH {64};
    static constexpr int DISPLAY_HEIGHT {32};
    static constexpr int MAXIMAL_FADING_VALUE {500};

private:
    std::array<std::array<Pixel, DISPLAY_WIDTH>, DISPLAY_HEIGHT> m_frame {};

    // it's the maximal value the fadingLevel of a pixel can have
    // The higher the MAXIMALFADING, the longer it will take for a pixel
    // to go completely black.
    // It has default value of 500, which results to the display showing
    // two different tones of grey every time a pixel is turned off
    int32_t m_maximalFading;

public:
    const std::array<std::array<Pixel, DISPLAY_WIDTH>, DISPLAY_HEIGHT>* getDisplayFrame()
    {
        return &m_frame;
    }
};
