#include "read_from_file.h"
#include <cassert>
#include <fstream>
#include <iostream>

size_t lengthOfBinaryFile(std::ifstream& file)
{
    // get length of file
    file.seekg(0, std::ios_base::end); // sets input position indicator at the end of file
    size_t length = file.tellg(); // says the position of the input indicator
    file.seekg(0, std::ios_base::beg); // resets the input indicator at the beginning of file

    return length;
}

size_t openAndComputeLength(const std::filesystem::path& path)
{
    assert(exists(path));

    std::ifstream file {path , std::ifstream::in | std::ifstream::binary};

    if (file.is_open())
    {
        size_t length {lengthOfBinaryFile(file)};
        return length;
    }
    else
    {
        std::cout << "Unable to open file.\n";
        return 0;
    }
}

void copyFromBinaryFile(const std::filesystem::path& path, char* startCopyAddress)
{
    assert(exists(path));

    // file must be opened in read mode and binary mode.
    // If not opened in binary mode, the method read used below
    // will interpret the byte 1a as an end-of-file character,
    // interrupting the copy of the program in ram
    std::ifstream file {path , std::ifstream::in | std::ifstream::binary};

    if (file.is_open())
    {
        size_t length {lengthOfBinaryFile(file)};

        file.read(startCopyAddress, length);
    }
    else
    {
        std::cout << "Unable to open file.\n";
    }
}