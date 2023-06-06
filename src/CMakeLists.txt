target_sources( tests PRIVATE
    "chip8_emulator/chip8-core/chip8.cpp"
    "../tests/test.cpp"
    "chip8_emulator/chip8-displayAndKeyboard/displayAndKeyboard.cpp"
    "chip8_emulator/chip8-timers/timers.cpp"
    "chip8_emulator/sound/sound.cpp"
    )

target_sources( main PRIVATE
    "chip8_emulator/chip8-core/chip8.cpp"
    "chip8_emulator/chip8-displayAndKeyboard/displayAndKeyboard.cpp"
    "main.cpp"
    "chip8_emulator/chip8-timers/timers.cpp"
    "chip8_emulator/sound/sound.cpp"
    )