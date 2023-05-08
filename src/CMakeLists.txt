target_sources( main PRIVATE
    "chip8_emulator/chip8.cpp"
    "../tests/test.cpp"
    )

target_sources( display PRIVATE
    "chip8_emulator/chip8.cpp"
    "display/display.cpp"
    )