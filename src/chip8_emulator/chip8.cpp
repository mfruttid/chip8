#include "chip8.h"
#include <cassert>

std::string Chip8_internals::Register::toString() const
{
    std::stringstream stream;
    stream << "0x" << std::hex << static_cast<uint16_t>(reg);
    return stream.str();
}

std::string Chip8_internals::Address::toString() const
{
    std::stringstream stream;
    stream << "0x" << std::hex << address;
    return stream.str();
}

const Chip8_internals::Address Chip8::Chip8::top_stack() const
{
    return stack[SP];
}

Chip8_internals::Pixel Chip8_internals::Pixel::operator^(uint8_t u) const
{
    if (int(status) ^ u)
    {
        return Chip8_internals::Pixel(Chip8_internals::Status::on);
    }
    else
    {
        return Chip8_internals::Pixel(Chip8_internals::Status::off);
    }
}

bool Chip8_internals::Display::drw(auto a, const uint8_t x, const uint8_t y)
{
    bool res = false;
    size_t s = a.size();
    assert(x<64 && y<32 && s<16);
    for (size_t i=0; i<s; ++i)
    {
        int k = i % 32;
        for (int j=0; j<8; ++j)
        {
            int l = j % 64;
            bool r = (d[k][l].status == Chip8_internals::Status::on);
            d[k][l] = d[k][l] ^ static_cast<uint8_t>(a[i] & 1);
            a[i] = a[i] >> 1u;
            if (!res)
            {
                res = r && (d[k][l].status == Chip8_internals::Status::off);
            }
        }
    }
    return res;
}

void Chip8::Chip8::call(const uint16_t nnn)
{
    ++SP;
    stack[SP] = PC;
    PC = Chip8_internals::Address(nnn);
}

void Chip8::Chip8::se(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u; // second 4 bits
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff); // last 8 bits
    if (registers[x] == kk)
    {
        PC += 2;
    }
}

void Chip8::Chip8::sne(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u; // second 4 bits
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff); // last 8 bits
    if (registers[x] != kk)
    {
        PC += 2;
    }
}

void Chip8::Chip8::se(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    if (registers[x] == registers[y])
    {
        PC += 2;
    }
}

void Chip8::Chip8::ld(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u;
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff);
    registers[x].update(kk);
}

void Chip8::Chip8::add(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u;
    uint16_t kk = static_cast<uint16_t>(xkk & 0xff);
    registers[x] += kk;
}

void Chip8::Chip8::ld(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    registers[x].update(registers[y].reg);
}

void Chip8::Chip8::bit_or(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    registers[x].update(registers[x].reg | registers[y].reg);
}

void Chip8::Chip8::bit_and(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    registers[x].update(registers[x].reg & registers[y].reg);
}

void Chip8::Chip8::bit_xor(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    registers[x].update(registers[x].reg ^ registers[y].reg);
}

void Chip8::Chip8::add(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    uint8_t z = static_cast<uint8_t>(registers[x].reg + registers[y].reg);
    registers[x].update(z);
    if (z < registers[y].reg)
    {
        registers[0xf].update(1);
    }
    else
    {
        registers[0xf].update(0);
    }
}

void Chip8::Chip8::sub(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    uint8_t val_x = registers[x].reg;
    uint8_t val_y = registers[y].reg;
    if (val_x > val_y)
    {
        registers[0xf].update(1);
    }
    else
    {
        registers[0xf].update(0);
    }
    registers[x].update(static_cast<uint8_t>(val_x - val_y));
}

void Chip8::Chip8::shr(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t val_x = registers[x].reg;
    if ((val_x & 1) == 1)
    {
        registers[0xf].update(1);
    }
    else
    {
        registers[0xf].update(0);
    }
    registers[x].reg /= 2;
}

void Chip8::Chip8::subn(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    uint8_t val_x = registers[x].reg;
    uint8_t val_y = registers[y].reg;
    if (val_y > val_x)
    {
        registers[0xf].update(1);
    }
    else
    {
        registers[0xf].update(0);
    }
    registers[x].update(static_cast<uint8_t>(val_y - val_x));
}

void Chip8::Chip8::shl(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t val_x = registers[x].reg;
    if ((val_x >> 7u) == 1)
    {
        registers[0xf].update(1);
    }
    else
    {
        registers[0xf].update(0);
    }
    registers[x].update(static_cast<uint8_t>(val_x*2));
}

void Chip8::Chip8::sne(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    if (registers[x] != registers[y])
    {
        PC += 2;
    }
}

void Chip8::Chip8::ld_I(const uint16_t nnn)
{
    I = nnn;
}

void Chip8::Chip8::jp_v0(const uint16_t nnn)
{
    PC += registers[0] + nnn;
}

void Chip8::Chip8::rnd(const uint16_t xkk)
{
    static std::mt19937 generator = std::mt19937();
    static std::uniform_int_distribution<> distrib(0, 255);
    uint8_t rd = static_cast<uint8_t>(distrib(generator));
    uint8_t x = (xkk & 0xf00) >> 8u;
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff);
    registers[x].update( rd & kk );
}

void Chip8::Chip8::drw(const uint16_t xyn)
{
    uint8_t x = (xyn & 0xf00) >> 8u;
    uint8_t y = (xyn & 0xf0) >> 4u;
    uint8_t n = static_cast<uint8_t>(xyn & 0xf);
    uint8_t coord_x = (registers[x].reg) % 64;
    uint8_t coord_y = (registers[y].reg) % 32;
    auto a = std::ranges::take_view(std::ranges::drop_view(ram, I-1),n);
    bool set = display.drw(a, coord_x, coord_y);
    if (set)
    {
        registers[0xf].update(1);
    }
    else
    {
        registers[0xf].update(0);
    }
}

void Chip8::Chip8::add_I(const uint8_t x)
{
    I = registers[x] + I;
}

void Chip8::Chip8::ld_B(const uint8_t x)
{
    uint8_t val_x = registers[x].reg;
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
        ram[J] = registers[i].reg;
        ++J;
    }
}

void Chip8::Chip8::ldVxI(const uint8_t x)
{
    uint16_t J = I;
    for (int i : std::ranges::iota_view(0, x+1))
    {
        registers[i].update(ram[J]);
        ++J;
    }
}

std::vector<Chip8_internals::Instruction> Chip8::Chip8::readFromFile(const std::filesystem::path path) const
    {
        assert(exists(path));

        std::vector<Chip8_internals::Instruction> output;
        std::ifstream file {path};

        if (file.is_open())
        {
            uint16_t byte1, byte2;
            while (!file.eof())
            {
                byte1 = static_cast<uint16_t>(file.std::istream::get());
                byte1 = static_cast<uint16_t>(byte1 << 8u);
                byte2 = static_cast<uint16_t>(file.std::istream::get());

                uint16_t i =static_cast<uint16_t>(byte1 | byte2);
                output.emplace_back(i); // construct Instruction(i) in-place
            }

            return output;
        }
        else
        {
            std::cout << "Unable to open file.\n";
            return {};
        }
    }

void Chip8::Chip8::run(const std::vector<Chip8_internals::Instruction> instructions)
    {
        for (Chip8_internals::Instruction instruction : instructions)
        {
            execute(instruction);
        }
    }

void Chip8::Chip8::execute(const Chip8_internals::Instruction i)
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
        break;
    }

    case 4:
    {
        uint16_t xkk = static_cast<uint16_t>(inst & 0xfff);
        sne(xkk);
        break;
    }

    case 5:
    {
        uint8_t xy0 = static_cast<uint8_t>((inst & 0xff0) >> 4u);
        se(xy0);
        break;
    }

    case 6:
    {
        uint16_t xkk = static_cast<uint16_t>(inst & 0xfff);
        ld(xkk);
        break;
    }

    case 7:
    {
        uint16_t xkk = static_cast<uint16_t>(inst & 0xfff);
        add(xkk);
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
            break;
        }

        case 1:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            bit_or(xy);
            break;
        }

        case 2:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            bit_and(xy);
            break;
        }

        case 3:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            bit_xor(xy);
            break;
        }

        case 4:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            add(xy);
            break;
        }

        case 5:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            sub(xy);
            break;
        }

        case 6:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            shr(xy);
            break;
        }

        case 7:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            subn(xy);
            break;
        }

        case 0xe:
        {
            uint8_t xy = static_cast<uint8_t>((inst & 0xff0) >> 4u);
            shl(xy);
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
        break;
    }

    case 0xb:
    {
        uint16_t nnn = inst & 0xfff;
        jp_v0(nnn);
        break;
    }

    case 0xc:
    {
        uint16_t xkk = inst & 0xfff;
        rnd(xkk);
        break;
    }

    case 0xd:
    {
        uint16_t xyn = inst & 0xfff;
        drw(xyn);
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
            add_I(x);
            break;
        }

        case 0x33:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            ld_B(x);
            break;
        }

        case 0x55:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            ldIVx(x);
            break;
        }

        case 0x65:
        {
            uint8_t x = (inst & 0xf00) >> 8u;
            ldVxI(x);
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
