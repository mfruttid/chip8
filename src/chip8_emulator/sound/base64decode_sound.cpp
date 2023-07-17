#include <base64.h>
#include <base64decode_sound.h>
#include <encoded_sound.inl>

extern const std::vector<uint8_t> Base64Sound::DECODED_SOUND = base64_decode(ENCODED_SOUND, ENCODED_SOUND_FILE_SIZE);
extern const size_t Base64Sound::DECODED_SOUND_SIZE = DECODED_SOUND.size();
