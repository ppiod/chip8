#include "chip8.hpp"
#include "raylib.h"
#include <iostream>

const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;
const int SCALE_FACTOR = 10;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <ROM file>" << std::endl;
    return 1;
  }

  InitWindow(SCREEN_WIDTH * SCALE_FACTOR, SCREEN_HEIGHT * SCALE_FACTOR,
             "Krystal CHIP-8 Emulator");
  SetTargetFPS(60);

  Chip8 krystal;
  krystal.loadRom(argv[1]);

  while (!WindowShouldClose()) {
    krystal.handleInput();

    for (int i = 0; i < 10; ++i) {
      krystal.emulateCycle();
    }

    krystal.updateTimers();

    BeginDrawing();
    ClearBackground(BLACK);

    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
      for (int x = 0; x < SCREEN_WIDTH; ++x) {
        if (krystal.display[y * SCREEN_WIDTH + x]) {
          DrawRectangle(x * SCALE_FACTOR, y * SCALE_FACTOR, SCALE_FACTOR,
                        SCALE_FACTOR, RAYWHITE);
        }
      }
    }
    EndDrawing();
  }

  CloseWindow();
  return 0;
}