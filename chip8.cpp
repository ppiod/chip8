#include "chip8.hpp"
#include "raylib.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

void Chip8::op_00E0_00EE() {
  switch (opcode & 0x00FF) {
  case 0x00E0:
    display.assign(SCREEN_WIDTH * SCREEN_HEIGHT, 0);
    break;
  case 0x00EE:
    pc = stack[--sp];
    break;
  }
}
void Chip8::op_1NNN() { pc = opcode & 0x0FFF; }
void Chip8::op_2NNN() {
  stack[sp++] = pc;
  pc = opcode & 0x0FFF;
}
void Chip8::op_3XNN() {
  if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
    pc += 2;
}
void Chip8::op_4XNN() {
  if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
    pc += 2;
}
void Chip8::op_5XY0() {
  if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
    pc += 2;
}
void Chip8::op_6XNN() { V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF); }
void Chip8::op_7XNN() { V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF); }
void Chip8::op_8XY_() {
  uint8_t X = (opcode & 0x0F00) >> 8;
  uint8_t Y = (opcode & 0x00F0) >> 4;
  switch (opcode & 0x000F) {
  case 0x0:
    V[X] = V[Y];
    break;
  case 0x1:
    V[X] |= V[Y];
    break;
  case 0x2:
    V[X] &= V[Y];
    break;
  case 0x3:
    V[X] ^= V[Y];
    break;
  case 0x4: {
    uint16_t sum = V[X] + V[Y];
    V[0xF] = sum > 255;
    V[X] = sum & 0xFF;
    break;
  }
  case 0x5: {
    V[0xF] = V[X] > V[Y];
    V[X] -= V[Y];
    break;
  }
  case 0x6: {
    V[0xF] = V[X] & 0x1;
    V[X] >>= 1;
    break;
  }
  case 0x7: {
    V[0xF] = V[Y] > V[X];
    V[X] = V[Y] - V[X];
    break;
  }
  case 0xE: {
    V[0xF] = V[X] >> 7;
    V[X] <<= 1;
    break;
  }
  }
}
void Chip8::op_9XY0() {
  if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
    pc += 2;
}
void Chip8::op_ANNN() { I = opcode & 0x0FFF; }
void Chip8::op_BNNN() { pc = (opcode & 0x0FFF) + V[0]; }
void Chip8::op_CXNN() {
  V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
}
void Chip8::op_DXYN() {
  uint8_t X = (opcode & 0x0F00) >> 8;
  uint8_t Y = (opcode & 0x00F0) >> 4;
  uint8_t N = opcode & 0x000F;
  uint8_t coordX = V[X] % SCREEN_WIDTH;
  uint8_t coordY = V[Y] % SCREEN_HEIGHT;
  V[0xF] = 0;
  for (int row = 0; row < N; ++row) {
    uint8_t spriteByte = memory[I + row];
    for (int col = 0; col < 8; ++col) {
      if ((spriteByte & (0x80 >> col)) != 0) {
        size_t index = (coordY + row) * SCREEN_WIDTH + (coordX + col);
        if (index < display.size()) {
          if (display[index] == 1)
            V[0xF] = 1;
          display[index] ^= 1;
        }
      }
    }
  }
}
void Chip8::op_EX_() {
  uint8_t X = (opcode & 0x0F00) >> 8;
  switch (opcode & 0x00FF) {
  case 0x9E:
    if (keypad[V[X]])
      pc += 2;
    break;
  case 0xA1:
    if (!keypad[V[X]])
      pc += 2;
    break;
  }
}
void Chip8::op_FX_() {
  uint8_t X = (opcode & 0x0F00) >> 8;
  switch (opcode & 0x00FF) {
  case 0x07:
    V[X] = delay_timer;
    break;
  case 0x0A: {
    bool keyPressed = false;
    for (unsigned int i = 0; i < KEYPAD_SIZE; ++i) {
      if (keypad[i]) {
        V[X] = i;
        keyPressed = true;
        break;
      }
    }
    if (!keyPressed)
      pc -= 2;
    break;
  }
  case 0x15:
    delay_timer = V[X];
    break;
  case 0x18:
    sound_timer = V[X];
    break;
  case 0x1E:
    I += V[X];
    break;
  case 0x29:
    I = V[X] * 5;
    break;
  case 0x33:
    memory[I] = V[X] / 100;
    memory[I + 1] = (V[X] / 10) % 10;
    memory[I + 2] = V[X] % 10;
    break;
  case 0x55:
    for (int i = 0; i <= X; ++i)
      memory[I + i] = V[i];
    break;
  case 0x65:
    for (int i = 0; i <= X; ++i)
      V[i] = memory[I + i];
    break;
  }
}

void Chip8::loadFontset() {
  uint8_t fontset[FONTSET_SIZE] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF0, 0x10,
      0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10,
      0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0,
      0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0,
      0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
      0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80,
      0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80};

  for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
    memory[i] = fontset[i];
  }
}

Chip8::Chip8() {
  opcodeTable = {{[this]() { op_00E0_00EE(); }}, {[this]() { op_1NNN(); }},
                 {[this]() { op_2NNN(); }},      {[this]() { op_3XNN(); }},
                 {[this]() { op_4XNN(); }},      {[this]() { op_5XY0(); }},
                 {[this]() { op_6XNN(); }},      {[this]() { op_7XNN(); }},
                 {[this]() { op_8XY_(); }},      {[this]() { op_9XY0(); }},
                 {[this]() { op_ANNN(); }},      {[this]() { op_BNNN(); }},
                 {[this]() { op_CXNN(); }},      {[this]() { op_DXYN(); }},
                 {[this]() { op_EX_(); }},       {[this]() { op_FX_(); }}};
  reset();
}

void Chip8::reset() {
  opcode = 0;
  pc = ROM_START_ADDRESS;
  I = 0;
  sp = 0;
  delay_timer = 0;
  sound_timer = 0;
  romLoaded = false;

  memory.assign(MEMORY_SIZE, 0);
  V.assign(NUM_REGISTERS, 0);
  display.assign(SCREEN_WIDTH * SCREEN_HEIGHT, 0);
  stack.assign(STACK_SIZE, 0);

  loadFontset();
}

bool Chip8::loadRom(const char *filename) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    std::cerr << "Failed to open ROM: " << filename << std::endl;
    return false;
  }

  std::streampos size = file.tellg();
  if (size > (MEMORY_SIZE - ROM_START_ADDRESS)) {
    std::cerr << "ROM is too large for memory." << std::endl;
    return false;
  }

  file.seekg(0, std::ios::beg);
  file.read(reinterpret_cast<char *>(&memory[ROM_START_ADDRESS]), size);
  file.close();
  romLoaded = true;
  return true;
}

bool Chip8::isRomLoaded() const { return romLoaded; }
uint8_t Chip8::getSoundTimer() const { return sound_timer; }

void Chip8::handleInput() {
  for (unsigned int i = 0; i < KEYPAD_SIZE; ++i)
    keypad[i] = 0;
  if (IsKeyDown(KEY_W))
    keypad[0x1] = 1;
  if (IsKeyDown(KEY_S))
    keypad[0x4] = 1;
  if (IsKeyDown(KEY_A))
    keypad[0x7] = 1;
  if (IsKeyDown(KEY_D))
    keypad[0x8] = 1;
  if (IsKeyDown(KEY_UP))
    keypad[0xC] = 1;
  if (IsKeyDown(KEY_DOWN))
    keypad[0xD] = 1;
  if (IsKeyDown(KEY_LEFT))
    keypad[0x9] = 1;
  if (IsKeyDown(KEY_RIGHT))
    keypad[0xE] = 1;
  if (IsKeyDown(KEY_SPACE))
    keypad[0x5] = 1;
  if (IsKeyDown(KEY_ENTER))
    keypad[0x6] = 1;
}

void Chip8::updateTimers() {
  if (delay_timer > 0)
    --delay_timer;
  if (sound_timer > 0)
    --sound_timer;
}

void Chip8::emulateCycle() {
  opcode = memory[pc] << 8 | memory[pc + 1];
  pc += 2;

  (opcodeTable[(opcode & 0xF000) >> 12])();
}