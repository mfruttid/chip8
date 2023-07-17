#include <base64.h>
#include <filesystem>
#include <cassert>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <cstdint>
#include <iterator>

std::vector<char> readBynary(std::filesystem::path& soundPath)
{
	assert(std::filesystem::exists(soundPath));

    // file must be opened in read mode and binary mode.
    // If not opened in binary mode, the method read used below
    // will interpret the byte 1a as an end-of-file character,
    // interrupting the copy of the file
    std::ifstream soundFile { soundPath, std::ifstream::in | std::ifstream::binary };

    std::vector<char> res;
    
    if (soundFile.is_open())
    {
        // the following is the best way I found to copy the soundFile in the vector 
        // without skipping white spaces
        soundFile.seekg(0);
        char temp;
        while (soundFile.read(&temp, sizeof(temp)))
        {
            res.push_back(temp);
        }
    }
    else
    {
        std::cout << "Unable to open file.\n";
    }
    return res;
}

std::string base64EncodeSound()
{
    std::filesystem::path soundPath{"C:/cpp/chip8/sounds/beep.wav"}; // non-portable but it doesn't matter
    std::vector<char> soundVector{readBynary(soundPath)};

    return base64_encode(&soundVector[0], soundVector.size());
}

int main()
{
    std::ofstream encoded_sound("encoded_sound.txt");

    std::cout << base64EncodeSound();
    encoded_sound << base64EncodeSound();

    encoded_sound.close();
}

