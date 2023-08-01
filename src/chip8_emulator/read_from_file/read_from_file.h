#pragma once

#include <filesystem>

// returns the length in bytes of a binary file that is already open
size_t lengthOfBinaryFile(std::ifstream& file);

size_t openAndComputeLength(const std::filesystem::path& path);

// copies the file starting at memory address startCopyAddress
// IMPORTANT: this function assumes that there is enough space
// in memory to copy the whole file without overwriting memory
// that is still in use
void copyFromBinaryFile(const std::filesystem::path& path, char* startCopyAddress);
