target_sources( main PRIVATE
    "chip8_emulator/chip8.cpp"
    "../tests/test.cpp"
    "display/display.cpp"
    )

target_sources( display PRIVATE
    "chip8_emulator/chip8.cpp"
    "display/display.cpp"
    "display/main_display.cpp"
    )