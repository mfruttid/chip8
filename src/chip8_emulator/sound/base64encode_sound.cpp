#include <base64.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <read_from_file.h>

std::vector<char> readBynary(std::filesystem::path& soundPath)
{
    std::vector<char> res;

    size_t fileLength {openAndComputeLength(soundPath)};
    res.reserve(fileLength);

    copyFromBinaryFile(soundPath, res.data());

    return res;
}

std::string base64EncodeSound()
{
    std::filesystem::path soundPath {"C:/cpp/chip8/sounds/beep.wav"}; // non-portable but it doesn't matter
    std::vector<char> soundVector {readBynary(soundPath)};

    return base64_encode(&soundVector[0], soundVector.size());
}

int main()
{
    std::ofstream encoded_sound("encoded_sound.txt");

    std::cout << base64EncodeSound();
    encoded_sound << base64EncodeSound();

    encoded_sound.close();
}

