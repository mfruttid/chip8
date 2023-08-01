#include "chip8_emulator/chip8_emulator.h"

// sets up the arguments to construct the emulator taking them as input from the user
// when they started the program
const std::array<std::string ,3> processArguments(int argc, char** argv)
{
    // default options
    std::string flagChip8 {"-chip8"}; // default is chip8 instructions
    std::string flagDrawInstruction {"-clipping"}; // default is clipping
    std::string flagFading {"-fading"}; // default is fading simulating the phosphorus screen

    for (int i {0}; i<argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
            case 'n':
                flagFading = "-n"; // flag for flickering pixels
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
                    "-n : disables the fading effect, making the pixels flicker " <<
                    "(default: unset pixels slowly fade to black, simulating the old phosphorus screens effect)" << '\n';
                break;

            default:
                std::cout << "Invalid argument" << "\n";
                break;
            }
        }
    }
    const std::array<std::string ,3> res {flagChip8, flagDrawInstruction, flagFading};
    return res;
}


/*
    The main structure of this program is the following:
    - in the main thread the display and keyboard are handled;
    - the main thread spawns two more threads (for delay and sound timer)
      at the creation of the Chip8Emulator;
      these two threads are joined at the distruction of the emulator;
    - the main thread spawns another thread when executing the member function
      runEmulator of Chip8Emulator: this last thread runs the instructions of
      the Chip8 rom and is detached.
*/
int main(int argc, char** argv)
{
    if (argc > 1) // check there is a path for a program in input
    {
        auto settings = processArguments(argc, argv);

        // if the first argument is "-h", then we print the useful info during setUpSetting
        // so we have nothing left to do
        if (std::strcmp(argv[1], "-h") == 0)
        {
            return 0;
        }

        std::filesystem::path programPath {argv[1]};

        const std::string_view flagChip8 = settings[0];
        const std::string_view flagDrawInstruction = settings[1];
        const std::string_view fadingFlag = settings[2];

        Chip8Emulator emulator{flagChip8, flagDrawInstruction, fadingFlag};

        emulator.runEmulator(std::move(programPath));
    }

    else
    {
        std::cout << "Pass the path of a Chip8 rom as argument." << "\n";
        return 0;
    }
}
