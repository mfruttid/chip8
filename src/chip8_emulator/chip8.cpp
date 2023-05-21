#include "chip8.h"
#include "../display/display.h"
#include <cassert>
#include <span>
#include <thread>
#include <functional>


Chip8::Chip8::Pixel Chip8::Chip8::Pixel::operator^(uint8_t u) const
{
    u = u >> 7u;
    if (status == Chip8::Chip8::Status::off)
    {
        if (0 ^ u)
        {
            return Chip8::Chip8::Pixel(Chip8::Chip8::Status::on);
        }
        else
        {
            return Chip8::Chip8::Pixel(Chip8::Chip8::Status::off);
        }
    }
    else
    {
        if (1 ^ u)
        {
            return Chip8::Chip8::Pixel(Chip8::Chip8::Status::on);
        }
        else
        {
            return Chip8::Chip8::Pixel(Chip8::Chip8::Status::off);
        }
    }
}

// instruction of draw with wrapping sprites
/*
bool Chip8::Chip8::Display::drw(auto a, const uint8_t x, const uint8_t y)
{
    bool res = false;
    size_t s = a.size();
    assert(x<64 && y<32 && s<16);
    for (size_t i=0; i<32; ++i)
    {
        int k = (y + i) % 32;
        for (int j=7; j>=0; --j)
        {
            int l = (x + j) % 64;
            bool r = (d[k][l].status == Chip8::Chip8::Status::on);
            d[k][l] = d[k][l] ^ static_cast<uint8_t>(a[i] & 1);
            a[i] = a[i] >> 1u;
            if (!res)
            {
                res = r && (d[k][l].status == Chip8::Chip8::Status::off);
            }
        }
    }
    return res;
}
*/

bool Chip8::Chip8::Display::drw(std::vector<uint8_t>&& a, const uint8_t x, const uint8_t y)
{
    bool res = false;
    size_t s = a.size();
    assert(x<64 && y<32 && s<16);
    int m = std::min(63, x+7);
    int n = std::min(32-y, static_cast<int>(s));

    for (int i=0; i<n; ++i)
    {
        int k = y + i;
        for (int j=x; j<=m; ++j)
        {
            bool r = (d[k][j].status == Chip8::Chip8::Status::on);
            d[k][j] = d[k][j] ^ static_cast<uint8_t>(a[i] & 0x80); //0x80 = 1000'0000 in binary
            a[i] = static_cast<uint8_t>(a[i] << 1u);
            if (!res)
            {
                res = r && (d[k][j].status == Chip8::Chip8::Status::off);
            }
        }
    }
    return res;
}

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

void Chip8::Chip8::bit_or(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    registers[x]= registers[x] | registers[y];
}

void Chip8::Chip8::bit_and(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    registers[x] = registers[x] & registers[y];
}

void Chip8::Chip8::bit_xor(const uint8_t xy)
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

    if (val_x > val_y)
    {
        registers[0xf] = 1;
    }
    else
    {
        registers[0xf] = 0;
    }

    registers[x] = static_cast<uint8_t>(val_x - val_y);
}

void Chip8::Chip8::shr(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t val_x = registers[x];

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

void Chip8::Chip8::subn(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;

    uint8_t val_x = registers[x];
    uint8_t val_y = registers[y];

    if (val_y > val_x)
    {
        registers[0xf] = 1;
    }
    else
    {
        registers[0xf] = 0;
    }

    registers[x] = static_cast<uint8_t>(val_y - val_x);
}

void Chip8::Chip8::shl(const uint8_t xy)
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

void Chip8::Chip8::sne(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;

    if (registers[x] != registers[y])
    {
        PC = static_cast<Address>(PC + 2);
    }
}

void Chip8::Chip8::ld_I(const uint16_t nnn)
{
    I = nnn;
}

void Chip8::Chip8::jpV0(const uint16_t nnn)
{
    PC = static_cast<Address>(registers[0] + nnn);
}

void Chip8::Chip8::rnd(const uint16_t xkk)
{
    static std::mt19937 generator = std::mt19937();
    static std::uniform_int_distribution<> distrib(0, 255);
    uint8_t randomNumber = static_cast<uint8_t>(distrib(generator));

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

    std::vector<uint8_t> a;
    for (int i=I; i<I+n; ++i)
    {
        a.emplace_back(ram[i]);
    }

    bool set = display.drw(std::move(a), coord_x, coord_y);

    if (set)
    {
        registers[0xf] = 1;
    }
    else
    {
        registers[0xf] = 0;
    }
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

void Chip8::Chip8::ldIVx(const uint8_t x)
{
    uint16_t J = I;
    for (int i : std::ranges::iota_view(0, x+1))
    {
        ram[J] = registers[i];
        ++J;
    }
}

void Chip8::Chip8::ldVxI(const uint8_t x)
{
    uint16_t J = I;

    for (int i : std::ranges::iota_view(0, x+1))
    {
        registers[i] = ram[J];
        ++J;
    }
}

void Chip8::Chip8::readFromFile(const std::filesystem::path path)
{
    assert(exists(path));

    std::ifstream file {path};

    if (file.is_open())
    {
        uint8_t byte1;
        size_t ramAddress = 0x200;

        while (!file.eof())
        {
            byte1 = static_cast<uint8_t>(file.std::istream::get());

            ram[ramAddress] = byte1;

            ++ramAddress;
        }
    }
    else
    {
        std::cout << "Unable to open file.\n";
    }
}

void Chip8::Chip8::run()
{
    isRunning = true;

    std::promise<bool> promiseDisplayInitialized;
    std::future<bool> futureDisplayInitialized = promiseDisplayInitialized.get_future();

    std::promise<bool> promiseDisplayDone;
    std::future<bool> futureDisplayDone = promiseDisplayDone.get_future();

    std::thread displayThread {show, std::ref(*this), std::ref(promiseDisplayInitialized), std::ref(promiseDisplayDone)};
    displayThread.detach();

    futureDisplayInitialized.wait();

    while (isRunning)
    {
        const auto start = std::chrono::high_resolution_clock::now();

        uint16_t byte1 = static_cast<uint16_t>(ram[PC]);
        byte1 = static_cast<uint16_t>(byte1 << 8u);

        uint16_t byte2 = static_cast<uint16_t>(ram[PC+1]);
        Chip8::Chip8::Instruction instruction { static_cast<uint16_t>(byte1 | byte2) };

        execute(instruction);

        const auto end = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double, std::milli> sleep_time = std::chrono::seconds(1/500) - (end - start);
        std::this_thread::sleep_for(sleep_time);
    }

    futureDisplayDone.wait();
}

void Chip8::Chip8::execute(const Chip8::Chip8::Instruction i)
{
    uint16_t inst = i.instruction();
    switch (inst)
    {
    case static_cast<uint16_t>(0x00ee):
        ret();
        break;

    case static_cast<uint16_t>(0x00e0):
        cls();
        PC = static_cast<Address>(PC + 2);
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
            bit_or(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 2:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            bit_and(xy);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 3:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            bit_xor(xy);
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
            shr(xy);
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
        ld_I(nnn);
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

        std::unique_lock lck{display_mutex};
        drw(xyn);
        lck.unlock();

        std::this_thread::yield();

        PC = static_cast<Address>(PC + 2);
        break;
    }

    case 0xf:
    {
        uint8_t last2bits = static_cast<uint8_t>(inst & 0xff);
        switch (last2bits)
        {
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
            ldIVx(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        case 0x65:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            ldVxI(x);
            PC = static_cast<Address>(PC + 2);
            break;
        }

        default:
            break;
        }
        break;
    }

    default:
        break;
    }
}
