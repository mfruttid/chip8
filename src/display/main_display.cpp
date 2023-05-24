#include "display.h"

int main()
{
    Chip8::Chip8 c = Chip8::Chip8();
    c.readFromFile(std::filesystem::path("/home/martina/cpp/chip8/programs/Breakout [Carmelo Cortez, 1979].ch8"));

    std::promise<bool> promiseDisplayInitialized;
    std::future<bool> futureDisplayInitialized = promiseDisplayInitialized.get_future();

    std::promise<bool> promiseDisplayDone;
    std::future<bool> futureDisplayDone = promiseDisplayDone.get_future();

    std::thread chip8Thread {&Chip8::Chip8::run, std::ref(c), std::ref(futureDisplayInitialized), std::ref(futureDisplayDone), false};
    chip8Thread.detach();

    showDisplay(c, promiseDisplayInitialized, promiseDisplayDone);

}
