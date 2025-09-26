#pragma once
#include <cstdint>
#include <vector>

class Chip8 {
public:
  uint8_t keypad[16]{};
  std::vector<uint8_t> display;

  Chip8();
  void loadRom(const char *filename);
  void handleInput();
  void updateTimers();
  void emulateCycle();

private:
  std::vector<uint8_t> memory;
  std::vector<uint8_t> V;
  std::vector<uint16_t> stack;

  uint16_t sp;
  uint16_t pc;
  uint16_t I;

  uint8_t delay_timer;
  uint8_t sound_timer;
};