#include "chip8_emulator/chip8_emulator.h"

int main()
{
    Chip8Emulator emulator = Chip8Emulator();
    emulator.chip8.readFromFile(std::filesystem::path("/home/martina/cpp/chip8/programs/slipperyslope.ch8"));

    std::promise<bool> promiseDisplayInitialized;
    std::future<bool> futureDisplayInitialized = promiseDisplayInitialized.get_future();

    std::thread chip8Thread {
        &Chip8::Chip8::run,
        std::ref(emulator.chip8),
        std::ref(futureDisplayInitialized),
        Chip8::Chip8Type::chip8};
    chip8Thread.detach();

    emulator.renderAndKeyboard(promiseDisplayInitialized);

}
