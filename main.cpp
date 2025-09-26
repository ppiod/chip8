#include "chip8.hpp"
#include "raylib.h"
#include <iostream>

const int SCALE_FACTOR = 10;
const int INITIAL_CYCLES_PER_FRAME = 10;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <ROM file>" << std::endl;
    return 1;
  }

  InitWindow(Chip8::SCREEN_WIDTH * SCALE_FACTOR,
             Chip8::SCREEN_HEIGHT * SCALE_FACTOR, "Krystal CHIP-8 Emulator");
  InitAudioDevice();

  Sound beep = LoadSound("beep.wav");
  SetTargetFPS(60);

  Chip8 krystal;
  if (!krystal.loadRom(argv[1])) {
    UnloadSound(beep);
    CloseAudioDevice();
    CloseWindow();
    return 1;
  }

  int cyclesPerFrame = INITIAL_CYCLES_PER_FRAME;

  while (!WindowShouldClose()) {
    krystal.handleInput();

    if (IsKeyPressed(KEY_O))
      cyclesPerFrame += 1;
    if (IsKeyPressed(KEY_P) && cyclesPerFrame > 1)
      cyclesPerFrame -= 1;

    for (int i = 0; i < cyclesPerFrame; ++i) {
      krystal.emulateCycle();
    }

    krystal.updateTimers();

    if (krystal.getSoundTimer() > 0) {
      PlaySound(beep);
    }

    BeginDrawing();
    ClearBackground(BLACK);

    for (unsigned int y = 0; y < Chip8::SCREEN_HEIGHT; ++y) {
      for (unsigned int x = 0; x < Chip8::SCREEN_WIDTH; ++x) {
        if (krystal.display[y * Chip8::SCREEN_WIDTH + x]) {
          DrawRectangle(x * SCALE_FACTOR, y * SCALE_FACTOR, SCALE_FACTOR,
                        SCALE_FACTOR, RAYWHITE);
        }
      }
    }
    EndDrawing();
  }

  UnloadSound(beep);
  CloseAudioDevice();
  CloseWindow();
  return 0;
}