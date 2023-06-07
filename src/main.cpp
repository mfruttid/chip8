#include "chip8_emulator/chip8_emulator.h"


std::pair<Chip8::Chip8::Chip8Type, Chip8::Chip8::DrawInstruction> setUpSettings(int argc, char **argv)
{
    Chip8::Chip8::Chip8Type flagChip8;
    Chip8::Chip8::DrawInstruction drawInstruction;

    if ( argc > 2 && std::strcmp( argv[2], "schip8") == 0 )
    {
        flagChip8 = Chip8::Chip8::Chip8Type::schip8;
    }

    if ( argc > 2 && ( std::strcmp( argv[2], "wrap" ) == 0 || std::strcmp( argv[3], "wrap" ) == 0 ))
    {
        drawInstruction = Chip8::Chip8::DrawInstruction::wrap;
    }
    else
    {
        flagChip8 = Chip8::Chip8::Chip8Type::chip8;
        drawInstruction = Chip8::Chip8::DrawInstruction::clip;
    }

    return std::make_pair(flagChip8, drawInstruction);
}

int main(int argc, char **argv)
{
    if (argc > 1) // check there is a path for a program in input
    {
        std::filesystem::path programPath { argv[1] };

        Chip8Emulator emulator = Chip8Emulator();

        auto settings = setUpSettings(argc, argv);

        Chip8::Chip8::Chip8Type flagChip8 = std::get<0>( settings );
        Chip8::Chip8::DrawInstruction drawInstruction = std::get<1>( settings );

        std::promise<bool> promiseDisplayInitialized;
        std::future<bool> futureDisplayInitialized = promiseDisplayInitialized.get_future();

        std::thread chip8Thread {
            &Chip8Emulator::runChip8,
            std::ref(emulator),
            std::ref(programPath),
            std::ref(futureDisplayInitialized),
            flagChip8, drawInstruction };
        chip8Thread.detach();

        emulator.renderAndKeyboard(promiseDisplayInitialized);
    }

}
