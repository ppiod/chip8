#pragma once
#include <cstdint>
#include <vector>
#include <functional>

class Chip8 {
public:
  static constexpr unsigned int SCREEN_WIDTH = 64;
  static constexpr unsigned int SCREEN_HEIGHT = 32;
  static constexpr unsigned int MEMORY_SIZE = 4096;
  static constexpr unsigned int NUM_REGISTERS = 16;
  static constexpr unsigned int STACK_SIZE = 16;
  static constexpr unsigned int KEYPAD_SIZE = 16;
  static constexpr unsigned int FONTSET_SIZE = 80;
  static constexpr unsigned int ROM_START_ADDRESS = 0x200;

  uint8_t keypad[KEYPAD_SIZE]{};
  std::vector<uint8_t> display;

  Chip8();
  void reset();
  bool loadRom(const char *filename);
  void handleInput();
  void updateTimers();
  void emulateCycle();
  uint8_t getSoundTimer() const;
  bool isRomLoaded() const;

private:
  uint16_t opcode;
  std::vector<uint8_t> memory;
  std::vector<uint8_t> V;
  std::vector<uint16_t> stack;
  uint16_t sp;
  uint16_t pc;
  uint16_t I;
  uint8_t delay_timer;
  uint8_t sound_timer;
  bool romLoaded;

  std::vector<std::function<void()>> opcodeTable;

  void loadFontset();

  void op_00E0_00EE();
  void op_1NNN();
  void op_2NNN();
  void op_3XNN();
  void op_4XNN();
  void op_5XY0();
  void op_6XNN();
  void op_7XNN();
  void op_8XY_();
  void op_9XY0();
  void op_ANNN();
  void op_BNNN();
  void op_CXNN();
  void op_DXYN();
  void op_EX_();
  void op_FX_();
};