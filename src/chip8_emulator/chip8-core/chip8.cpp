#include "chip8.h"

Chip8::Chip8::Pixel Chip8::Chip8::Pixel::operator^( uint8_t u ) const
{
    assert( u == 0 || u == 1 );
    if ( int(status) ^ u )
    {
        return Chip8::Chip8::Pixel( Chip8::Chip8::Status::on, fadingLevel );
    }
    return Chip8::Chip8::Pixel( Chip8::Chip8::Status::off, fadingLevel );
}

void Chip8::Chip8::Display::decreaseFadingLevel()
{
    for (std::array<Pixel, 64> & row : frame)
    {
        for ( Pixel & pixel : row )
        {
            if (pixel.fadingLevel > 0 && pixel.status == Status::off)
            {
                --pixel.fadingLevel;
            }
        }
    }
}

bool Chip8::Chip8::Display::drwWrap(std::vector<uint8_t>&& sprite, const uint8_t x, const uint8_t y)
{
    bool res = false;

    size_t size = sprite.size();

    // the coordinates (x,y) must represent a point inside the display
    // and the sprite can be maximum 16 lines long by the chip8 documentation
    assert(x < 64 && y < 32 && size < 16);

    for (size_t row = 0; row < size; ++row)
    {
        size_t rowOffset = (y + row) % 32;

        for (int column = 7; column >= 0; --column)
        {
            size_t columnOffset = (x + column) % 64;

            Pixel& pixel { frame[rowOffset][columnOffset] };

            bool pixelWasOn = ( pixel.status == Chip8::Chip8::Status::on );

            pixel = pixel ^ static_cast<uint8_t>(sprite[row] & 0b1);
            sprite[row] = sprite[row] >> 1u;

            if (pixelWasOn)
            {
                pixel.fadingLevel = MAXIMALFADING;
            }

            if (!res)
            {
                res = pixelWasOn && ( pixel.status == Chip8::Chip8::Status::off );
            }
        }
    }
    return res;
}

bool Chip8::Chip8::Display::drwClip(std::vector<uint8_t>&& sprite, const uint8_t x, const uint8_t y)
{
    bool res = false;

    size_t size = sprite.size();

    // the coordinates (x,y) must represent a point inside the display
    // and the sprite can be maximum 16 lines long by the chip8 documentation
    assert(x<64 && y<32 && size<16);

    int maxWidth = std::min(63, x+7); // necessary for clipping the sprite if the width exceeds the display
    int maxHeight = std::min(32-y, static_cast<int>(size)); // necessary for clipping the sprite if the height exceeds the display

    for (int offset = 0; offset < maxHeight; ++offset)
    {
        int row = y + offset;
        for (int column = x; column <= maxWidth; ++column)
        {
            Pixel& pixel { frame[row][column] };

            bool pixelWasOn = ( pixel.status == Chip8::Chip8::Status::on );

            pixel = pixel ^ static_cast<uint8_t>((sprite[offset] & 0b1000'0000) >> 7u);
            sprite[offset] = static_cast<uint8_t>(sprite[offset] << 1u);

            if (pixelWasOn && pixel.status == Status::off)
            {
                pixel.fadingLevel = MAXIMALFADING;
            }

            if (!res)
            {
                res = pixelWasOn && ( pixel.status == Chip8::Chip8::Status::off );
            }
        }
    }
    return res;
}


void Chip8::Chip8::readFromFile(const std::filesystem::path path)
{
    assert(exists(path));

    std::ifstream file { path };

    if ( file.is_open() )
    {
        uint8_t byte;
        size_t ramAddress = 0x200;

        while ( !file.eof() )
        {
            byte = static_cast<uint8_t>(file.std::istream::get());

            ram[ramAddress] = byte;

            ++ramAddress;
        }
    }
    else
    {
        std::cout << "Unable to open file.\n";
    }
}

void Chip8::Chip8::run(std::future<bool>& futureDisplayInitialized)
{
    isRunning = true;

    futureDisplayInitialized.wait();

    while (isRunning)
    {
        // chip8 has a clock frequency of 500 Hz
        // we wait the due amount of time after 10 instructions so that
        // it is more precise
        const auto start = std::chrono::high_resolution_clock::now();

        for (int numInstructions = 0; numInstructions < 10; ++numInstructions)
        {
            // one instruction is given by two bytes each
            uint16_t byte1 = static_cast<uint16_t>(ram[PC]);
            byte1 = static_cast<uint16_t>(byte1 << 8u);

            uint16_t byte2 = static_cast<uint16_t>(ram[PC+1]);

            Chip8::Chip8::Instruction instruction { static_cast<uint16_t>(byte1 | byte2) };

            execute(instruction);
        }

        const auto end = std::chrono::high_resolution_clock::now();

        const std::chrono::duration<double, std::milli> sleep_time
                = std::chrono::milliseconds(20) - (end - start); // 2 milliseconds per instruction
        std::this_thread::sleep_for(sleep_time);

    }
}

void Chip8::Chip8::execute(const Chip8::Chip8::Instruction i)
{
    uint16_t instruction = i.inst;

    switch (instruction)
    {
    case static_cast<uint16_t>(0x00ee):
        ret();
        break;

    case static_cast<uint16_t>(0x00e0):
        cls();
        break;

    default:
        break;
    }

    uint16_t first4bits = (instruction & 0xf000) >> 12u;

    switch (first4bits)
    {
    case 1:
        {
            uint16_t nnn = static_cast<uint16_t>(instruction & 0xfff);
            jp(nnn);
            break;
        }

    case 2:
    {
        uint16_t nnn = static_cast<uint16_t>(instruction & 0xfff);
        call(nnn);
        break;
    }

    case 3:
    {
        uint16_t xkk = static_cast<uint16_t>(instruction & 0xfff);
        se(xkk);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 4:
    {
        uint16_t xkk = static_cast<uint16_t>(instruction & 0xfff);
        sne(xkk);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 5:
    {
        uint8_t xy0 = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
        se(xy0);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 6:
    {
        uint16_t xkk = static_cast<uint16_t>(instruction & 0xfff);
        ld(xkk);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 7:
    {
        uint16_t xkk = static_cast<uint16_t>(instruction & 0xfff);
        add(xkk);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 8:
    {
        uint8_t last4bits = instruction & 0xf;
        switch (last4bits)
        {
        case 0:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            ld(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 1:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            bitOr(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 2:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            bitAnd(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 3:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            bitXor(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 4:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            add(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 5:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            sub(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 6:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            shr(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 7:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            subn(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0xe:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            shl(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        default:
            break;
        }
        break;
    }

    case 9:
    {
        uint8_t last4bits = instruction & 0xf;
        switch (last4bits)
        {
            case 0:
            {
                uint8_t xy =  static_cast<uint8_t>((instruction & 0xff0) >> 4u);
                sne(xy);
                PC = static_cast<Address>(PC + 2);
                break;
            }

            default:
                break;
        }
        break;
    }

    case 0xa:
    {
        uint16_t nnn = instruction & 0xfff;
        ldI(nnn);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 0xb:
    {
        uint16_t nnn = instruction & 0xfff;
        jpV0(nnn);
        break;
    }

    case 0xc:
    {
        uint16_t xkk = instruction & 0xfff;
        rnd(xkk);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 0xd:
    {
        uint16_t xyn = instruction & 0xfff;

        std::unique_lock lck{displayMutex};
        drw(xyn);
        lck.unlock();

        std::this_thread::yield();

        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 0xe:
    {
        uint8_t last2bits = static_cast<uint8_t>(instruction & 0xff);
        switch (last2bits)
        {
        case 0x9e:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            skp(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0xa1:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            sknp(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        default:
            break;
        }

        break;
    }

    case 0xf:
    {
        uint8_t last2bits = static_cast<uint8_t>(instruction & 0xff);
        switch (last2bits)
        {
        case 0x07:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldVxDT(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x0a:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldVxK(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x15:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldDTVx(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x18:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldSTVx(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x1e:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            addI(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x29:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldFVx(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x33:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldB(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x55:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldIVx(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x65:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldVxI(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        default:
        {
            PC = static_cast<Address>(PC + 2);
            break;
        }
        }
        break;
    }

    default:
        PC = static_cast<Address>(PC + 2);
        break;
    }
}


void Chip8::Chip8::jp(const uint16_t nnn)
{
    PC = nnn;
}

void Chip8::Chip8::call(const uint16_t nnn)
{
    ++SP;
    stack[SP] = PC;
    PC = Address(nnn);
}

void Chip8::Chip8::se(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u; // leftmost 4 bits of xkk
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff); // rightmost 8 bits of xkk
    if (registers[x] == kk)
    {
        PC = static_cast<Address>(PC + 2);
    }
}

void Chip8::Chip8::sne(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u; // leftmost 4 bits of xkk
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff); // rightmost 8 bits of xkk
    if (registers[x] != kk)
    {
        PC = static_cast<Address>(PC + 2);
    }
}

void Chip8::Chip8::se(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    if (registers[x] == registers[y])
    {
        PC = static_cast<Address>(PC + 2);
    }
}

void Chip8::Chip8::ld(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u;
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff);
    registers[x] = kk;
}

void Chip8::Chip8::add(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u;
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff);
    registers[x] = static_cast<Register>(registers[x] + kk);
}

void Chip8::Chip8::ld(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    registers[x] = registers[y];
}

void Chip8::Chip8::bitOr(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    registers[x]= registers[x] | registers[y];
}

void Chip8::Chip8::bitAnd(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    registers[x] = registers[x] & registers[y];
}

void Chip8::Chip8::bitXor(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    registers[x] = registers[x] ^ registers[y];
}

void Chip8::Chip8::add(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;

    uint8_t sumRegisters = static_cast<uint8_t>(registers[x] + registers[y]);

    registers[x] = sumRegisters;

    if (sumRegisters < registers[y])
    {
        registers[0xf] = 1;
    }
    else
    {
        registers[0xf] = 0;
    }
}

void Chip8::Chip8::sub(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;

    Register val_x = registers[x];
    Register val_y = registers[y];

    registers[x] = static_cast<Register>(val_x - val_y);

    if (val_x > val_y)
    {
        registers[0xf] = 1;
    }
    else
    {
        registers[0xf] = 0;
    }
}

void Chip8::Chip8::shr(const uint8_t xy)
{
    // this instruction differs in chip8 and schip8
    if (int(chip8Type)) // if chip8Type is schip8
    {
        uint8_t x = (xy & 0xf0) >> 4u;
        Register val_x = registers[x];

        if ((val_x & 1) == 1)
        {
            registers[0xf] = 1;
        }
        else
        {
            registers[0xf] = 0;
        }

        registers[x] /= 2;
    }
    else // if chip8Type is chip8
    {
        uint8_t x = (xy & 0xf0) >> 4u;
        uint8_t y = xy & 0xf;

        Register val_y = registers[y];
        registers[x] = val_y >> 1u;

        registers[0xf] = val_y & 0b1;
    }

}

void Chip8::Chip8::subn(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;

    Register val_x = registers[x];
    Register val_y = registers[y];

    registers[x] = static_cast<Register>(val_y - val_x);

    if (val_y > val_x)
    {
        registers[0xf] = 1;
    }
    else
    {
        registers[0xf] = 0;
    }
}

void Chip8::Chip8::shl(const uint8_t xy)
{
    // this instruction differs in chip8 and schip8
    if (int(chip8Type)) // if chip8Type is schip8
    {
        uint8_t x = (xy & 0xf0) >> 4u;
        Register val_x = registers[x];

        if ((val_x >> 7u) == 1)
        {
            registers[0xf] = 1;
        }
        else
        {
            registers[0xf] = 0;
        }

        registers[x] = static_cast<Register>(val_x*2);
    }
    else // if chip8Type is chip8
    {
        uint8_t x = (xy & 0xf0) >> 4u;
        uint8_t y = xy & 0xf;

        Register val_y = registers[y];
        registers[x] = static_cast<Register>(val_y << 1u);

        registers[0xf] = val_y & 0b10000000;
    }

}

void Chip8::Chip8::sne(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;

    if (registers[x] != registers[y])
    {
        PC = static_cast<Address>(PC + 2);
    }
}

void Chip8::Chip8::ldI(const uint16_t nnn)
{
    I = nnn;
}

void Chip8::Chip8::jpV0(const uint16_t nnn)
{
    PC = static_cast<Address>(registers[0] + nnn);
}

void Chip8::Chip8::rnd(const uint16_t xkk)
{
    // generator and distribution are static because they are expensive to create
    // and because otherwise they would generate always the same number,
    // as we are initializing the generator with the default seed
    static std::mt19937 generator { std::random_device{}() };
    static std::uniform_int_distribution<> distribution(0, 255);
    uint8_t randomNumber = static_cast<uint8_t>(distribution(generator));

    uint8_t x = (xkk & 0xf00) >> 8u;
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff);

    registers[x] = randomNumber & kk ;
}

void Chip8::Chip8::drw(const uint16_t xyn)
{
    uint8_t x = (xyn & 0xf00) >> 8u;
    uint8_t y = (xyn & 0xf0) >> 4u;
    uint8_t n = static_cast<uint8_t>(xyn & 0xf); // length of the sprite

    uint8_t coord_x = static_cast<uint8_t>(registers[x] % 64);
    uint8_t coord_y = registers[y] % 32;

    std::vector<uint8_t> sprite;
    for (int i=I; i<I+n; ++i)
    {
        sprite.emplace_back(ram[i]);
    }

    bool pixelWasUnset;

    if (drawInstruction == DrawInstruction::clip)
    {
        pixelWasUnset = display.drwClip(std::move(sprite), coord_x, coord_y);
    }

    else
    {
        pixelWasUnset = display.drwWrap(std::move(sprite), coord_x, coord_y);
    }

    if (pixelWasUnset)
    {
        registers[0xf] = 1;
    }
    else
    {
        registers[0xf] = 0;
    }
}

void Chip8::Chip8::skp(const uint8_t x)
{
    if (chip8PressedKey.has_value())
    {
        if (chip8PressedKey.value() == registers[x])
        {
            PC = static_cast<Address>(PC + 2);
        }
    }
}

void Chip8::Chip8::sknp(const uint8_t x)
{
    if (chip8PressedKey.has_value())
    {
        if (chip8PressedKey.value() != registers[x])
        {
            PC = static_cast<Address>(PC + 2);
        }
    }
    else
    {
        PC = static_cast<Address>(PC + 2);
    }
}

void Chip8::Chip8::ldVxDT(const uint8_t x)
{
    registers[x] = delayTimer;
}

void Chip8::Chip8::ldVxK(const uint8_t x)
{
    std::unique_lock keyboardOrQuitWindowMutexLock { keyboardOrQuitWindowMutex };
    // we wait for the user to either press a valid key or to close the window
    keyIsPressedOrWindowClosed.wait(
        keyboardOrQuitWindowMutexLock,
        [&]{ return (chip8PressedKey.has_value() || !isRunning); }
        );

    if (chip8PressedKey.has_value())
    {
        registers[x] = static_cast<Register>(chip8PressedKey.value());
    }
}

void Chip8::Chip8::ldDTVx(const uint8_t x)
{
    delayTimer = registers[x];

    std::thread delayTimerThread {&Chip8::Chip8::decreaseDelayTimer, std::ref(*this)};
    delayTimerThread.detach();
}

void Chip8::Chip8::ldSTVx(const uint8_t x)
{
    soundTimer = registers[x];

    std::thread soundTimerThread {&Chip8::Chip8::decreaseSoundTimer, std::ref(*this)};
    soundTimerThread.detach();
}

void Chip8::Chip8::addI(const uint8_t x)
{
    I = static_cast<Address>(registers[x] + I);
}

void Chip8::Chip8::ldFVx(const uint8_t x)
{
    Register val_x = registers[x];

    I = static_cast<uint16_t>(val_x * 5);
}

void Chip8::Chip8::ldB(const uint8_t x)
{
    Register val_x = registers[x];
    ram[I] = static_cast<uint8_t>(val_x / 100);

    val_x = static_cast<uint8_t>(val_x - ram[I] * 100);
    ram[I+1] = static_cast<uint8_t>(val_x / 10);

    val_x = static_cast<uint8_t>(val_x - ram[I+1] * 10);
    ram[I+2] = static_cast<uint8_t>(val_x);
}

void Chip8::Chip8::ldIVx(const uint8_t x)
{
    // this instruction differs in chip8 and schip8
    if (int(chip8Type)) // if chip8Type is schip8
    {
        uint16_t J = I;
        for (int i : std::ranges::iota_view(0, x+1))
        {
            ram[J] = registers[i];
            ++J;
        }
    }
    else // if chip8Type is chip8
    {
        for (int i : std::ranges::iota_view(0, x+1))
        {
            ram[I] = registers[i];
            ++I;
        }
    }

}

void Chip8::Chip8::ldVxI(const uint8_t x)
{
    // this instruction differs in chip8 and schip8
    if (int(chip8Type)) // if chip8Type is schip8
    {
        uint16_t J = I;

        for (int i : std::ranges::iota_view(0, x+1))
        {
            registers[i] = ram[J];
            ++J;
        }
    }
    else // if chip8Type is chip8
    {
        for (int i : std::ranges::iota_view(0, x+1))
        {
            registers[i] = ram[I];
            ++I;
        }
    }

}

