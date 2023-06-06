target_sources( tests PRIVATE
    "chip8_emulator/chip8.cpp"
    "../tests/test.cpp"
    "displayAndKeyboard/displayAndKeyboard.cpp"
    "timers/timers.cpp"
    "sound/sound.cpp"
    )

target_sources( main PRIVATE
    "chip8_emulator/chip8.cpp"
    "displayAndKeyboard/displayAndKeyboard.cpp"
    "main.cpp"
    "timers/timers.cpp"
    "sound/sound.cpp"
    )