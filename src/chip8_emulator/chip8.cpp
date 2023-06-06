#include "chip8.h"
#include "../displayAndKeyboard/displayAndKeyboard.h"
#include "../timers/timers.h"

Chip8::Chip8::Pixel Chip8::Chip8::Pixel::operator^(uint8_t u) const
{
    if (int(status) ^ u)
    {
        return Chip8::Chip8::Pixel(Chip8::Chip8::Status::on, fadingLevel);
    }
    return Chip8::Chip8::Pixel(Chip8::Chip8::Status::off, fadingLevel);
}

void Chip8::Chip8::Display::decreaseFadingLevel()
{
    for (std::array<Pixel, 64> & row : d)
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

// instruction of draw with wrapping sprites

bool Chip8::Chip8::Display::drw(std::vector<uint8_t>&& sprite, const uint8_t x, const uint8_t y)
{
    bool res = false;
    size_t size = sprite.size();

    assert(x < 64 && y < 32 && size < 16);

    for (size_t row = 0; row < size; ++row)
    {
        size_t rowOffset = (y + row) % 32;

        for (int column = 7; column >= 0; --column)
        {
            size_t columnOffset = (x + column) % 64;

            bool pixelWasOn = (d[rowOffset][columnOffset].status == Chip8::Chip8::Status::on);

            d[rowOffset][columnOffset] = d[rowOffset][columnOffset] ^ static_cast<uint8_t>(sprite[row] & 0b1);
            sprite[row] = sprite[row] >> 1u;

            if (pixelWasOn)
            {
                d[rowOffset][columnOffset].fadingLevel = 500;
            }

            if (!res)
            {
                res = pixelWasOn && (d[rowOffset][columnOffset].status == Chip8::Chip8::Status::off);
            }
        }
    }
    return res;
}


// instruction of draw with clipping sprites
/*
bool Chip8::Chip8::Display::drw(std::vector<uint8_t>&& sprite, const uint8_t x, const uint8_t y)
{
    bool res = false;
    size_t size = sprite.size();

    assert(x<64 && y<32 && size<16);

    int maxWidth = std::min(63, x+7); // necessary for clipping the sprite if the width exceeds the display
    int maxHeight = std::min(32-y, static_cast<int>(size)); // necessary for clipping the sprite if the height exceeds the display

    for (int offset = 0; offset < maxHeight; ++offset)
    {
        int row = y + offset;
        for (int column = x; column <= maxWidth; ++column)
        {
            bool pixelWasOn = (d[row][column].status == Chip8::Chip8::Status::on);

            d[row][column] = d[row][column] ^ static_cast<uint8_t>((sprite[offset] & 0b1000'0000) >> 7u);
            sprite[offset] = static_cast<uint8_t>(sprite[offset] << 1u);

            if (pixelWasOn && d[row][column].status == Status::off)
            {
                d[row][column].fadingLevel = 500;
            }

            if (!res)
            {
                res = pixelWasOn && (d[row][column].status == Chip8::Chip8::Status::off);
            }
        }
    }
    return res;
}
*/

std::string Chip8::Chip8::Display::toString() const
{
    std::string res = "";
    for (const std::array<Pixel, 64>& a : d)
    {
        for (Pixel p : a)
        {
            res += p.toString();
        }
        res += "\n";
    }
    return res;
}

void Chip8::Chip8::jp(const uint16_t nnn)
{

    PC = nnn;
}

void Chip8::Chip8::call(const uint16_t nnn)
{
    ++SP;
    stack[SP] = PC;
    PC = Chip8::Chip8::Address(nnn);
}

void Chip8::Chip8::se(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u; // second 4 bits
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff); // last 8 bits
    if (registers[x] == kk)
    {
        PC = static_cast<Address>(PC + 2);
    }
}

void Chip8::Chip8::sne(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u; // second 4 bits
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff); // last 8 bits
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
    uint16_t kk = static_cast<uint16_t>(xkk & 0xff);
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

    uint8_t val_x = registers[x];
    uint8_t val_y = registers[y];

    registers[x] = static_cast<uint8_t>(val_x - val_y);

    if (val_x > val_y)
    {
        registers[0xf] = 1;
    }
    else
    {
        registers[0xf] = 0;
    }
}

void Chip8::Chip8::shr(const uint8_t xy, Chip8Type flagChip8)
{
    if (int(flagChip8))
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
    else
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

    uint8_t val_x = registers[x];
    uint8_t val_y = registers[y];

    registers[x] = static_cast<uint8_t>(val_y - val_x);

    if (val_y > val_x)
    {
        registers[0xf] = 1;
    }
    else
    {
        registers[0xf] = 0;
    }
}

void Chip8::Chip8::shl(const uint8_t xy, Chip8Type flagChip8)
{
    if (int(flagChip8))
    {
        uint8_t x = (xy & 0xf0) >> 4u;
        uint8_t val_x = registers[x];

        if ((val_x >> 7u) == 1)
        {
            registers[0xf] = 1;
        }
        else
        {
            registers[0xf] = 0;
        }

        registers[x] = static_cast<uint8_t>(val_x*2);
    }
    else
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
    uint8_t n = static_cast<uint8_t>(xyn & 0xf);

    uint8_t coord_x = static_cast<uint8_t>(registers[x] % 64);
    uint8_t coord_y = (registers[y]) % 32;

    std::vector<uint8_t> sprite;
    for (int i=I; i<I+n; ++i)
    {
        sprite.emplace_back(ram[i]);
    }

    bool set = display.drw(std::move(sprite), coord_x, coord_y);

    if (set)
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
    std::optional<Register> chip8Key { getChip8Key() };

    if (chip8Key.has_value())
    {
        if (chip8Key.value() == registers[x])
        {
            PC = static_cast<Address>(PC + 2);
        }
    }
}

void Chip8::Chip8::sknp(const uint8_t x)
{
    std::optional<Register> chip8Key { getChip8Key() };

    if (chip8Key.has_value())
    {
        if (chip8Key.value() != registers[x])
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

std::optional<Chip8::Chip8::Register> Chip8::Chip8::getChip8Key() const
{
    if (pressedKey.has_value())
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wswitch-enum"

        switch (pressedKey.value())
        {
        case SDL_SCANCODE_1:
        {
            return std::optional<Register>(0x1);
        }

        case SDL_SCANCODE_2:
        {
            return std::optional<Register>(0x2);
        }

        case SDL_SCANCODE_3:
        {
            return std::optional<Register>(0x3);
        }

        case SDL_SCANCODE_4:
        {
            return std::optional<Register>(0xc);
        }

        case SDL_SCANCODE_Q:
        {
            return std::optional<Register>(0x4);
        }

        case SDL_SCANCODE_W:
        {
            return std::optional<Register>(0x5);
        }

        case SDL_SCANCODE_E:
        {
            return std::optional<Register>(0x6);
        }

        case SDL_SCANCODE_R:
        {
            return std::optional<Register>(0xd);
        }

        case SDL_SCANCODE_A:
        {
            return std::optional<Register>(0x7);
        }

        case SDL_SCANCODE_S:
        {
            return std::optional<Register>(0x8);
        }

        case SDL_SCANCODE_D:
        {
            return std::optional<Register>(0x9);
        }

        case SDL_SCANCODE_F:
        {
            return std::optional<Register>(0xe);
        }

        case SDL_SCANCODE_Z:
        {
            return std::optional<Register>(0xa);
        }

        case SDL_SCANCODE_X:
        {
            return std::optional<Register>(0x0);
        }

        case SDL_SCANCODE_C:
        {
            return std::optional<Register>(0xb);
        }

        case SDL_SCANCODE_V:
        {
            return std::optional<Register>(0xf);
        }

        default:
            break;
        }

        #pragma GCC diagnostic pop
    }

    return std::optional<Register>();
}

void Chip8::Chip8::ldVxK(const uint8_t x)
{
    std::unique_lock keyboardMutexLock {keyboardMutex};
    keyIsPressed.wait(keyboardMutexLock, [&]{ return (getChip8Key().has_value() || !isRunning); });

    if (getChip8Key().has_value())
    {
        Register chip8PressedKey = getChip8Key().value();

        registers[x] = static_cast<Register>(chip8PressedKey);
    }
}

void Chip8::Chip8::ldDTVx(const uint8_t x)
{
    delayTimer = registers[x];

    std::thread delayTimerThread {decreaseDelayTimer, std::ref(*this)};
    delayTimerThread.detach();
}

void Chip8::Chip8::ldSTVx(const uint8_t x)
{
    soundTimer = registers[x];

    std::thread soundTimerThread {decreaseSoundTimer, std::ref(*this)};
    soundTimerThread.detach();
}

void Chip8::Chip8::addI(const uint8_t x)
{
    I = static_cast<Address>(registers[x] + I);
}

void Chip8::Chip8::ldFVx(const uint8_t x)
{
    uint8_t val_x = registers[x];

    I = static_cast<uint16_t>(val_x * 5);
}

void Chip8::Chip8::ldB(const uint8_t x)
{
    uint8_t val_x = registers[x];
    ram[I] = static_cast<uint8_t>(val_x / 100);

    val_x = static_cast<uint8_t>(val_x - ram[I] * 100);
    ram[I+1] = static_cast<uint8_t>(val_x / 10);

    val_x = static_cast<uint8_t>(val_x - ram[I+1] * 10);
    ram[I+2] = static_cast<uint8_t>(val_x);
}

void Chip8::Chip8::ldIVx(const uint8_t x, Chip8Type flagChip8)
{
    if (int(flagChip8))
    {
        uint16_t J = I;
        for (int i : std::ranges::iota_view(0, x+1))
        {
            ram[J] = registers[i];
            ++J;
        }
    }
    else
    {
        for (int i : std::ranges::iota_view(0, x+1))
        {
            ram[I] = registers[i];
            ++I;
        }
    }

}

void Chip8::Chip8::ldVxI(const uint8_t x, Chip8Type flagChip8)
{
    if (int(flagChip8))
    {
        uint16_t J = I;

        for (int i : std::ranges::iota_view(0, x+1))
        {
            registers[i] = ram[J];
            ++J;
        }
    }
    else
    {
        for (int i : std::ranges::iota_view(0, x+1))
        {
            registers[i] = ram[I];
            ++I;
        }
    }

}

void Chip8::Chip8::readFromFile(const std::filesystem::path path)
{
    assert(exists(path));

    std::ifstream file {path};

    if (file.is_open())
    {
        uint8_t byte;
        size_t ramAddress = 0x200;

        while (!file.eof())
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

void Chip8::Chip8::run(std::future<bool>& futureDisplayInitialized, std::future<bool>& futureDisplayDone, Chip8Type flagChip8)
{
    isRunning = true;

    futureDisplayInitialized.wait();

    //int count {0};

    while (isRunning)
    {
        const auto start = std::chrono::high_resolution_clock::now();

        for (int numInstructions = 0; numInstructions < 10; ++numInstructions)
        {
            uint16_t byte1 = static_cast<uint16_t>(ram[PC]);
            byte1 = static_cast<uint16_t>(byte1 << 8u);

            uint16_t byte2 = static_cast<uint16_t>(ram[PC+1]);
            Chip8::Chip8::Instruction instruction { static_cast<uint16_t>(byte1 | byte2) };

            execute(instruction, flagChip8);

            //++count;

            //if (count > 500)
            //{
                //std::this_thread::sleep_for(std::chrono::duration<double, std::deci>(1));
            //}

        }

        const auto end = std::chrono::high_resolution_clock::now();

        const std::chrono::duration<double, std::milli> sleep_time
                = std::chrono::milliseconds(20) - (end - start); // 2 milliseconds per instruction
        std::this_thread::sleep_for(sleep_time);

    }

    futureDisplayDone.wait();
}

void Chip8::Chip8::execute(const Chip8::Chip8::Instruction i, Chip8Type flagChip8)
{
    uint16_t inst = i.instruction();
    switch (inst)
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

    uint16_t first4bits = (inst & 0xf000) >> 12u;
    switch (first4bits)
    {
    case 1:
        {
            uint16_t nnn = static_cast<uint16_t>(inst & 0xfff);
            jp(nnn);
            break;
        }

    case 2:
    {
        uint16_t nnn = static_cast<uint16_t>(inst & 0xfff);
        call(nnn);
        break;
    }

    case 3:
    {
        uint16_t xkk = static_cast<uint16_t>(inst & 0xfff);
        se(xkk);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 4:
    {
        uint16_t xkk = static_cast<uint16_t>(inst & 0xfff);
        sne(xkk);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 5:
    {
        uint8_t xy0 = static_cast<uint8_t>((inst & 0xff0) >> 4u);
        se(xy0);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 6:
    {
        uint16_t xkk = static_cast<uint16_t>(inst & 0xfff);
        ld(xkk);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 7:
    {
        uint16_t xkk = static_cast<uint16_t>(inst & 0xfff);
        add(xkk);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 8:
    {
        uint8_t last4bits = inst & 0xf;
        switch (last4bits)
        {
        case 0:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            ld(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 1:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            bitOr(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 2:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            bitAnd(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 3:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            bitXor(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 4:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            add(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 5:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            sub(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 6:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            shr(xy, flagChip8);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 7:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            subn(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0xe:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            shl(xy, flagChip8);
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
        uint8_t last4bits = inst & 0xf;
        switch (last4bits)
        {
            case 0:
            {
                uint8_t xy =  static_cast<uint8_t>((inst & 0xff0) >> 4u);
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
        uint16_t nnn = inst & 0xfff;
        ldI(nnn);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 0xb:
    {
        uint16_t nnn = inst & 0xfff;
        jpV0(nnn);
        break;
    }

    case 0xc:
    {
        uint16_t xkk = inst & 0xfff;
        rnd(xkk);
        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 0xd:
    {
        uint16_t xyn = inst & 0xfff;

        std::unique_lock lck{displayMutex};
        drw(xyn);
        lck.unlock();

        std::this_thread::yield();

        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 0xe:
    {
        uint8_t last2bits = static_cast<uint8_t>(inst & 0xff);
        switch (last2bits)
        {
        case 0x9e:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            skp(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0xa1:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
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
        uint8_t last2bits = static_cast<uint8_t>(inst & 0xff);
        switch (last2bits)
        {
        case 0x07:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            ldVxDT(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x0a:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            ldVxK(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x15:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            ldDTVx(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x18:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            ldSTVx(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x1e:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            addI(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x29:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            ldFVx(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x33:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            ldB(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x55:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            ldIVx(x, flagChip8);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x65:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            ldVxI(x, flagChip8);
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
