#include<base64.h>
#include<filesystem>
#include<cassert>
#include<fstream>
#include<algorithm>
#include<iostream>

std::vector<BYTE> readBynary(std::filesystem::path& soundPath)
{
	assert(exists(soundPath));
    
    // file must be opened in read mode and binary mode.
    // If not opened in binary mode, the method read used below
    // will interpret the byte 1a as an end-of-file character,
    // interrupting the copy of the file
    std::ifstream soundFile { soundPath, std::ifstream::in | std::ifstream::binary };

    std::vector<BYTE> res{};

    if (soundFile.is_open())
    {
        std::copy(std::istream_iterator<BYTE>(soundFile), std::istream_iterator<BYTE>(), std::back_inserter(res));
    }
    else
    {
        std::cout << "Unable to open file.\n";
    }
    return res;
}

std::string base64EncodeSound()
{
    std::filesystem::path soundPath{"../../sounds/beep.wav"};
    std::vector<BYTE> soundVector{readBynary(soundPath)};

    return base64_encode(&soundVector[0], soundVector.size());
}

int main()
{
    std::ofstream encoded_sound("../../sounds/encoded_sound.txt");

    encoded_sound << base64EncodeSound() << '\n';

    encoded_sound.close();
}

