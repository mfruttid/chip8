#pragma once

#ifndef _BASE64_H_
#define _BASE64_H_

#include <vector>
#include <string>
#include <cstdint>

std::string base64_encode(const char* buf, size_t bufLen);

std::vector<uint8_t> base64_decode(const char* encoded_string, size_t in_len);

#endif
