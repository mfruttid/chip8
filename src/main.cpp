#include "chip8_emulator/chip8_emulator.h"
#include <tuple>

// sets up the arguments to construct the emulator taking them as input from the user
// when they started the program
const std::array<std::string ,3> setUpSettings(int argc, char** argv)
{
    std::string flagChip8 {"-chip8"}; // default is chip8 instructions
    std::string flagDrawInstruction {"-clipping"}; // default is clipping
    std::string flagFading {"-fading"}; // default is fading simulating the phosphorus screen

    for ( int i { 0 }; i < argc; ++i )
    {
        if ( argv[i][0] == '-' )
        {
            switch ( argv[i][1] )
            {
            case 'f':
                flagFading = "-f"; // flag for flickering pixels
                break;

            case 's':
                flagChip8 = "-s"; // flag for superchip8
                break;

            case 'w':
                flagDrawInstruction = "-w"; // flag for wrapping sprites
                break;

            case 'h': // in case user is asking for help on how to use the program
                std::cout << "Emulator of a chip8:" << '\n';
                std::cout << "type the absolute path of a chip8 program to start" << '\n';
                std::cout <<
                    "-s : flag for using the set of instructions of the super chip8 (default: use instructions of chip8)" << '\n';
                std::cout <<
                    "-w : the drawing instruction wraps the sprites (default: the drawing instruction clips the sprites)" << '\n';
                std::cout <<
                    "-f : disables the fading effect, making the pixels flicker " <<
                    "(default: unset pixels slowly fade to black, simulating the old phosphorus screens effect)" << '\n';
                break;

            default:
                break;
            }
        }
    }
    const std::array<std::string ,3> res {flagChip8, flagDrawInstruction, flagFading};
    return res;
}


int main(int argc, char** argv)
{
    //SDL_setenv("SDL_AUDIODRIVER","directsound",1);
    SDL_Init(SDL_INIT_EVERYTHING);

    if (argc > 1) // check there is a path for a program in input
    {
        auto settings = setUpSettings(argc, argv);

        // if the first argument is "-h", then we print the useful info during setUpSetting
        // so we have nothing left to do
        if (std::strcmp(argv[1], "-h") == 0)
        {
            return 0;
        }

        std::filesystem::path programPath { argv[1] };

        const std::string_view flagChip8 = settings[0];
        const std::string_view flagDrawInstruction = settings[1];
        const std::string_view fadingFlag = settings[2];

        Chip8Emulator emulator{ flagChip8, flagDrawInstruction, fadingFlag };

        std::promise<bool> promiseDisplayInitialized;
        std::future<bool> futureDisplayInitialized = promiseDisplayInitialized.get_future();

        // the chip8 must run the instruction in one thread
        std::thread chip8Thread {
            &Chip8Emulator::loadAndRunChip8Program,
            std::ref(emulator),
            std::move(programPath),
            std::move(futureDisplayInitialized)};
        chip8Thread.detach();

        // the main thread shows and updates the window and updates the pressed keys in the meantime
        emulator.renderAndKeyboard(promiseDisplayInitialized);
    }

}
