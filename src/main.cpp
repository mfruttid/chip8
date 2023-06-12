#include "chip8_emulator/chip8_emulator.h"

// sets up the arguments to construct the emulator taking them as input from the user
// when they started the program
std::pair<const char*, const char*>setUpSettings(int argc, char **argv)
{
    const char* flagChip8;
    const char* flagDrawInstruction;

    // if there are more than two arguments passed by the user and the third is the string schip8
    // then we want to use the settings of the schip8
    if ( argc > 2 && std::strcmp( argv[2], "schip8") == 0 )
    {
        flagChip8 = "schip8";
    }
    // the default option is the settings of the chip8
    else
    {
        flagChip8 = "chip8";
    }

    // if there are more than two arguments passed by the user and the third or fourth of them is
    // the string "wrap", then we want to use the wrapping draw instruction
    if ( argc > 2 && ( std::strcmp( argv[2], "wrap" ) == 0 || std::strcmp( argv[3], "wrap" ) == 0 ) )
    {
        flagDrawInstruction = "wrap";
    }
    // the default option is the clipping draw instruction
    else
    {
        flagDrawInstruction = "clip";
    }
    return std::make_pair( flagChip8, flagDrawInstruction );
}

int main(int argc, char **argv)
{
    if (argc > 1) // check there is a path for a program in input
    {
        std::filesystem::path programPath { argv[1] };

        auto settings = setUpSettings(argc, argv);

        const char* flagChip8 = std::get<0>( settings );
        const char* flagDrawInstruction = std::get<1>( settings );

        Chip8Emulator emulator = Chip8Emulator(flagChip8, flagDrawInstruction);

        std::promise<bool> promiseDisplayInitialized;
        std::future<bool> futureDisplayInitialized = promiseDisplayInitialized.get_future();

        // the chip8 must run the instruction in one thread
        std::thread chip8Thread {
            &Chip8Emulator::runChip8,
            std::ref(emulator),
            std::ref(programPath),
            std::ref(futureDisplayInitialized)};
        chip8Thread.detach();

        // the main thread shows and updates the window and updates the pressed keys in the meantime
        emulator.renderAndKeyboard(promiseDisplayInitialized);
    }

}
