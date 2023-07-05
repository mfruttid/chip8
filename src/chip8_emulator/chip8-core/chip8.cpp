#include "chip8.h"

Chip8::Pixel Chip8::Pixel::operator^( Status s )
{
    return (int(m_status) ^ int(s))? Chip8::Pixel(Chip8::Status::on, m_fadingLevel) 
        : Chip8::Pixel(Chip8::Status::off, m_fadingLevel);
}

void Chip8::Display::decreaseFadingLevel()
{
    for (std::array<Pixel, DISPLAY_WIDTH>& row : m_frame)
    {
        for ( Pixel& pixel : row )
        {
            if (pixel.m_fadingLevel > 0 && pixel.m_status == Status::off)
            {
                --pixel.m_fadingLevel;
            }
        }
    }
}

// drwWrap takes ownership of the sprite because it modifies it
// and it shouldn't be used again after the modifications
bool Chip8::Display::drwWrap( std::vector<uint8_t>&& sprite, const uint8_t x, const uint8_t y )
{
    bool res = false;

    size_t size = sprite.size();

    // the coordinates (x,y) must represent a point inside the display
    assert(x < 64 && y < 32);
    // the sprite can be maximum 16 lines long by the chip8 documentation
    assert(size < 16);

    for (size_t row = 0; row < size; ++row)
    {
        size_t rowOffset = (y + row) % 32;

        for (int column = 7; column >= 0; --column)
        {
            size_t columnOffset = (x + column) % 64;

            Pixel& pixel { m_frame[rowOffset][columnOffset] };

            bool pixelWasOn = ( pixel.m_status == Chip8::Status::on );

            pixel = pixel ^ static_cast<Status>(sprite[row] & 0b1);
            sprite[row] = sprite[row] >> 1u;

            if (pixelWasOn)
            {
                pixel.m_fadingLevel = m_maximalFading;
            }

            if (!res)
            {
                res = pixelWasOn && ( pixel.m_status == Chip8::Status::off );
            }
        }
    }
    return res;
}

// drwClip takes ownership of the sprite because it modifies it
// and it shouldn't be used again after the modifications
bool Chip8::Display::drwClip( std::vector<uint8_t>&& sprite, const uint8_t x, const uint8_t y )
{
    bool res = false;

    size_t size = sprite.size();

    // the coordinates (x,y) must represent a point inside the display
    assert(x < 64 && y < 32);
    // the sprite can be maximum 16 lines long by the chip8 documentation
    assert(size < 16);

    int maxWidth = std::min(63, x+7); // necessary for clipping the sprite if the width exceeds the display
    int maxHeight = std::min(32-y, static_cast<int>(size)); // necessary for clipping the sprite if the height exceeds the display

    for ( int offset = 0; offset < maxHeight; ++offset )
    {
        int row = y + offset;
        for ( int column = x; column <= maxWidth; ++column )
        {
            Pixel& pixel { m_frame[row][column] };

            bool pixelWasOn = ( pixel.m_status == Chip8::Status::on );

            pixel = pixel ^ static_cast<Status>( ( sprite[offset] & 0b1000'0000 ) >> 7u );
            sprite[offset] = static_cast<uint8_t>( sprite[offset] << 1u );

            if ( pixelWasOn && pixel.m_status == Status::off )
            {
                pixel.m_fadingLevel = m_maximalFading;
            }

            if (!res)
            {
                res = pixelWasOn && ( pixel.m_status == Chip8::Status::off );
            }
        }
    }
    return res;
}

Chip8::Chip8(
    std::string_view flagChip8Type,
    std::string_view flagDrawInstruction,
    std::string_view flagFading
) :
    m_PC{ Address(0x200) }, // the first 0x200 addresses in m_ramPtr are not used by the program
    m_fadingFlag{ (flagFading == "-f") ? Fading::off : Fading::on },
    m_display{ std::make_unique<Display>( m_fadingFlag ) },
    m_instructionSet{ (flagChip8Type == "-s") ? InstructionSet::schip8 : InstructionSet::chip8 },
    m_drawBehaviour{ (flagDrawInstruction == "-w") ? DrawBehaviour::wrap
                        : DrawBehaviour::clip }
{
    // the first addresses of the m_ramPtr are used for the hexadecimal sprites
    uint8_t ramIndex = 0;
    for (uint8_t u = 0x0; u <= 0xf; ++u)
    {
        std::array<uint8_t, 5> hexadecimalSprite { m_hexadecimalSprites[u] };

        for (uint8_t line : hexadecimalSprite)
        {
            (*m_ramPtr)[ramIndex] = line;
            ++ramIndex;
        }
    }
}

void Chip8::readFromFile( const std::filesystem::path& path )
{
    assert(exists(path));

    // file must be opened in read mode and binary mode.
    // If not opened in binary mode, the method read used below 
    // will interpret the byte 1a as an end-of-file character,
    // interrupting the copy of the program in ram
    std::ifstream file { path , std::ifstream::in | std::ifstream::binary };

    if ( file.is_open() )
    {
        // the program must be copied in ram starting from address 0x200, 
        // which is exactly the address of m_PC
        char* startProgramAddress = &(reinterpret_cast<char&>((*m_ramPtr)[m_PC]));

        // get length of file
        file.seekg(0, std::ios_base::end); // sets input position indicator at the end of file
        int length = file.tellg(); // says the position of the input indicator 
        file.seekg(0, std::ios_base::beg); // resets the input indicator at the beginning of file

        file.read(startProgramAddress, length); // copies the file in ram
    }
    else
    {
        std::cout << "Unable to open file.\n";
    }
}

void Chip8::run( std::future<bool>& futureDisplayInitialized )
{
    m_isRunning = true;

    futureDisplayInitialized.wait();

    // the internal clock of the Chip8 is slower than the internal clock 
    // of a modern computer.
    // Thus we need to let the execution thread sleep for some time in between instructions
    // (more below)
    std::chrono::duration<double, std::milli> sleep_time{ 0 };

    while (m_isRunning)
    {
        // chip8 has a clock frequency of 500 Hz, so we have to let this thread sleep.
        // We use the function std::this_tread::sleep_for(sleep_time)
        // This function grants that the thread will sleep for AT LEAST 
        // sleep_time, but it could sleep a bit more, since it is not super precise.
        // In order to make the Chip8 clock more precise, I adopted two strategies:
        // 1) sleep after executing ten instructions at a time, so that 
        // the overall sleeping time in excess is less significant;
        // 2) take into account the effective time slept in the previous iteration
        // when computing the sleep_time. I achieved this by starting to 
        // measure the time before the thread sleeps.
        const auto start = std::chrono::high_resolution_clock::now();

        std::this_thread::sleep_for(sleep_time);

        for (int numInstructions = 0; numInstructions < 10; ++numInstructions)
        {
            // one instruction is given by two bytes each
            uint16_t byte1 = static_cast<uint16_t>((*m_ramPtr)[m_PC]);
            byte1 = static_cast<uint16_t>(byte1 << 8u);

            uint16_t byte2 = static_cast<uint16_t>((*m_ramPtr)[m_PC+1]);

            Chip8::Instruction instruction { static_cast<uint16_t>(byte1 | byte2) };

            execute(instruction);
        }

        const auto end = std::chrono::high_resolution_clock::now();

        sleep_time = std::chrono::milliseconds(20) - (end - start - sleep_time); // 2 milliseconds per instruction
        
    }
}

void Chip8::execute( const Chip8::Instruction i )
{
    uint16_t instruction = i.m_inst;

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
        m_PC = static_cast<Address>(m_PC + 2);
        break;
    }

    case 4:
    {
        uint16_t xkk = static_cast<uint16_t>(instruction & 0xfff);
        sne(xkk);
        m_PC = static_cast<Address>(m_PC + 2);
        break;
    }

    case 5:
    {
        uint8_t xy0 = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
        se(xy0);
        m_PC = static_cast<Address>(m_PC + 2);
        break;
    }

    case 6:
    {
        uint16_t xkk = static_cast<uint16_t>(instruction & 0xfff);
        ld(xkk);
        m_PC = static_cast<Address>(m_PC + 2);
        break;
    }

    case 7:
    {
        uint16_t xkk = static_cast<uint16_t>(instruction & 0xfff);
        add(xkk);
        m_PC = static_cast<Address>(m_PC + 2);
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
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 1:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            bitOr(xy);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 2:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            bitAnd(xy);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 3:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            bitXor(xy);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 4:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            add(xy);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 5:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            sub(xy);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 6:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            shr(xy);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 7:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            subn(xy);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 0xe:
        {
            uint8_t xy = static_cast<uint8_t>((instruction & 0xff0) >> 4u);
            shl(xy);
            m_PC = static_cast<Address>(m_PC + 2);
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
                m_PC = static_cast<Address>(m_PC + 2);
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
        m_PC = static_cast<Address>(m_PC + 2);
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
        m_PC = static_cast<Address>(m_PC + 2);
        break;
    }

    case 0xd:
    {
        uint16_t xyn = instruction & 0xfff;

        std::unique_lock lck{m_displayMutex};
        drw(xyn);
        lck.unlock();

        std::this_thread::yield();

        m_PC = static_cast<Address>(m_PC + 2);
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

            std::unique_lock lck{m_eventMutex};
            skp(x);
            lck.unlock();

            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 0xa1:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;

            std::unique_lock lck{m_eventMutex};
            sknp(x);
            lck.unlock();

            m_PC = static_cast<Address>(m_PC + 2);
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
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 0x0a:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldVxK(x);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 0x15:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldDTVx(x);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 0x18:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldSTVx(x);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 0x1e:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            addI(x);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 0x29:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldFVx(x);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 0x33:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldB(x);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 0x55:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldIVx(x);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        case 0x65:
        {
            uint8_t x = (instruction & 0xf00) >> 8u;
            ldVxI(x);
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }

        default:
        {
            m_PC = static_cast<Address>(m_PC + 2);
            break;
        }
        }
        break;
    }

    default:
        m_PC = static_cast<Address>(m_PC + 2);
        break;
    }
}


void Chip8::jp(const uint16_t nnn)
{
    m_PC = nnn;
}

void Chip8::call(const uint16_t nnn)
{
    ++m_SP;
    m_stack[m_SP] = m_PC;
    m_PC = Address(nnn);
}

void Chip8::se(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u; // leftmost 4 bits of xkk
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff); // rightmost 8 bits of xkk
    if ( m_registers[x] == kk )
    {
        m_PC = static_cast<Address>(m_PC + 2);
    }
}

void Chip8::sne(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u; // leftmost 4 bits of xkk
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff); // rightmost 8 bits of xkk
    if (m_registers[x] != kk)
    {
        m_PC = static_cast<Address>(m_PC + 2);
    }
}

void Chip8::se(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    if (m_registers[x] == m_registers[y])
    {
        m_PC = static_cast<Address>(m_PC + 2);
    }
}

void Chip8::ld(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u;
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff);
    m_registers[x] = kk;
}

void Chip8::add(const uint16_t xkk)
{
    uint8_t x = (xkk & 0xf00) >> 8u;
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff);
    m_registers[x] = static_cast<Register>(m_registers[x] + kk);
}

void Chip8::ld(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    m_registers[x] = m_registers[y];
}

void Chip8::bitOr(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    m_registers[x]= m_registers[x] | m_registers[y];
}

void Chip8::bitAnd(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    m_registers[x] = m_registers[x] & m_registers[y];
}

void Chip8::bitXor(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;
    m_registers[x] = m_registers[x] ^ m_registers[y];
}

void Chip8::add(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;

    uint8_t sumRegisters = static_cast<uint8_t>(m_registers[x] + m_registers[y]);

    m_registers[x] = sumRegisters;

    if (sumRegisters < m_registers[y])
    {
        m_registers[0xf] = 1;
    }
    else
    {
        m_registers[0xf] = 0;
    }
}

void Chip8::sub(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;

    Register val_x = m_registers[x];
    Register val_y = m_registers[y];

    m_registers[x] = static_cast<Register>(val_x - val_y);

    if (val_x > val_y)
    {
        m_registers[0xf] = 1;
    }
    else
    {
        m_registers[0xf] = 0;
    }
}

void Chip8::shr(const uint8_t xy)
{
    // this instruction differs in chip8 and schip8
    if (int(m_instructionSet))
    {
        uint8_t x = (xy & 0xf0) >> 4u;
        Register val_x = m_registers[x];

        if ((val_x & 1) == 1)
        {
            m_registers[0xf] = 1;
        }
        else
        {
            m_registers[0xf] = 0;
        }

        m_registers[x] /= 2;
    }
    else
    {
        uint8_t x = (xy & 0xf0) >> 4u;
        uint8_t y = xy & 0xf;

        Register val_y = m_registers[y];
        m_registers[x] = val_y >> 1u;

        m_registers[0xf] = val_y & 0b1;
    }

}

void Chip8::subn(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;

    Register val_x = m_registers[x];
    Register val_y = m_registers[y];

    m_registers[x] = static_cast<Register>(val_y - val_x);

    if (val_y > val_x)
    {
        m_registers[0xf] = 1;
    }
    else
    {
        m_registers[0xf] = 0;
    }
}

void Chip8::shl(const uint8_t xy)
{
    // this instruction differs in chip8 and schip8
    if (int(m_instructionSet)) // if chip8Type is schip8
    {
        uint8_t x = (xy & 0xf0) >> 4u;
        Register val_x = m_registers[x];

        if ((val_x >> 7u) == 1)
        {
            m_registers[0xf] = 1;
        }
        else
        {
            m_registers[0xf] = 0;
        }

        m_registers[x] = static_cast<Register>(val_x*2);
    }
    else // if chip8Type is chip8
    {
        uint8_t x = (xy & 0xf0) >> 4u;
        uint8_t y = xy & 0xf;

        Register val_y = m_registers[y];
        m_registers[x] = static_cast<Register>(val_y << 1u);

        m_registers[0xf] = val_y & 0b10000000;
    }

}

void Chip8::sne(const uint8_t xy)
{
    uint8_t x = (xy & 0xf0) >> 4u;
    uint8_t y = xy & 0xf;

    if (m_registers[x] != m_registers[y])
    {
        m_PC = static_cast<Address>(m_PC + 2);
    }
}

void Chip8::ldI(const uint16_t nnn)
{
    m_I = nnn;
}

void Chip8::jpV0(const uint16_t nnn)
{
    m_PC = static_cast<Address>(m_registers[0] + nnn);
}

void Chip8::rnd(const uint16_t xkk)
{
    // generator and distribution are static because they are expensive to create
    // and because otherwise they would generate always the same number,
    // as we are initializing the generator with the default seed
    static std::mt19937 generator { std::random_device{}() };
    static std::uniform_int_distribution<> distribution(0, 255);
    uint8_t randomNumber = static_cast<uint8_t>(distribution(generator));

    uint8_t x = (xkk & 0xf00) >> 8u;
    uint8_t kk = static_cast<uint8_t>(xkk & 0xff);

    m_registers[x] = randomNumber & kk ;
}

void Chip8::drw(const uint16_t xyn)
{
    uint8_t x = (xyn & 0xf00) >> 8u;
    uint8_t y = (xyn & 0xf0) >> 4u;
    uint8_t n = static_cast<uint8_t>(xyn & 0xf); // length of the sprite

    uint8_t coord_x = static_cast<uint8_t>(m_registers[x] % 64);
    uint8_t coord_y = m_registers[y] % 32;

    std::vector<uint8_t> sprite;
    for (int i=m_I; i<m_I+n; ++i)
    {
        sprite.emplace_back((*m_ramPtr)[i]);
    }

    bool pixelWasUnset;

    if (m_drawBehaviour == DrawBehaviour::clip)
    {
        pixelWasUnset = m_display->drwClip(std::move(sprite), coord_x, coord_y);
    }

    else
    {
        pixelWasUnset = m_display->drwWrap(std::move(sprite), coord_x, coord_y);
    }

    // the variable sprite is now invalid
    if (pixelWasUnset)
    {
        m_registers[0xf] = 1;
    }
    else
    {
        m_registers[0xf] = 0;
    }
}

void Chip8::skp(const uint8_t x)
{
    if (m_chip8PressedKey.has_value())
    {
        if (m_chip8PressedKey.value() == m_registers[x])
        {
            m_PC = static_cast<Address>(m_PC + 2);
        }
    }
}

void Chip8::sknp(const uint8_t x)
{
    if (m_chip8PressedKey.has_value())
    {
        if (m_chip8PressedKey.value() != m_registers[x])
        {
            m_PC = static_cast<Address>(m_PC + 2);
        }
    }
    else
    {
        m_PC = static_cast<Address>(m_PC + 2);
    }
}

void Chip8::ldVxDT(const uint8_t x)
{
    m_registers[x] = m_delayTimer;
}

void Chip8::ldVxK(const uint8_t x)
{
    std::unique_lock eventMutexLock { m_eventMutex };
    // we wait for the user to either press a valid key or to close the window
    m_eventHappened.wait(eventMutexLock,
        [&]{ return (m_chip8PressedKey.has_value() || !m_isRunning); }
        );

    if (m_chip8PressedKey.has_value())
    {
        m_registers[x] = static_cast<Register>(m_chip8PressedKey.value());
    }
}

void Chip8::ldDTVx(const uint8_t x)
{
    m_delayTimer = m_registers[x];
    m_setDelayTimer.notify_one();
}

void Chip8::ldSTVx(const uint8_t x)
{
    m_soundTimer = m_registers[x];
    m_setSoundTimer.notify_one();
}

void Chip8::addI(const uint8_t x)
{
    m_I = static_cast<Address>(m_registers[x] + m_I);
}

void Chip8::ldFVx(const uint8_t x)
{
    Register val_x = m_registers[x];

    m_I = static_cast<uint16_t>(val_x * 5);
}

void Chip8::ldB(const uint8_t x)
{
    Register val_x = m_registers[x];
    (*m_ramPtr)[m_I] = static_cast<uint8_t>(val_x / 100);

    val_x = static_cast<uint8_t>(val_x - (*m_ramPtr)[m_I] * 100);
    (*m_ramPtr)[m_I+1] = static_cast<uint8_t>(val_x / 10);

    val_x = static_cast<uint8_t>(val_x - (*m_ramPtr)[m_I+1] * 10);
    (*m_ramPtr)[m_I+2] = static_cast<uint8_t>(val_x);
}

void Chip8::ldIVx(const uint8_t x)
{
    // this instruction differs in chip8 and schip8
    if (int(m_instructionSet)) // if chip8Type is schip8
    {
        uint16_t J = m_I;
        for (int i : std::ranges::iota_view(0, x+1))
        {
            (*m_ramPtr)[J] = m_registers[i];
            ++J;
        }
    }
    else // if chip8Type is chip8
    {
        for (int i : std::ranges::iota_view(0, x+1))
        {
            (*m_ramPtr)[m_I] = m_registers[i];
            ++m_I;
        }
    }

}

void Chip8::ldVxI(const uint8_t x)
{
    // this instruction differs in chip8 and schip8
    if (int(m_instructionSet)) // if chip8Type is schip8
    {
        uint16_t J = m_I;

        for (int i : std::ranges::iota_view(0, x+1))
        {
            m_registers[i] = (*m_ramPtr)[J];
            ++J;
        }
    }
    else // if chip8Type is chip8
    {
        for (int i : std::ranges::iota_view(0, x+1))
        {
            m_registers[i] = (*m_ramPtr)[m_I];
            ++m_I;
        }
    }

}

