#include "chip8.hpp"
#include "raylib.h"
#include <fstream>
#include <iostream>

Chip8::Chip8() {
  pc = 0x200;
  I = 0;
  sp = 0;
  delay_timer = 0;
  sound_timer = 0;

  memory.assign(4096, 0);
  V.assign(16, 0);
  display.assign(64 * 32, 0);
  stack.assign(16, 0);

  uint8_t fontset[80] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF0, 0x10,
      0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10,
      0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0,
      0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0,
      0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
      0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80,
      0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80};
  for (int i = 0; i < 80; ++i)
    memory[i] = fontset[i];
}

void Chip8::loadRom(const char *filename) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    std::cerr << "Failed to open ROM: " << filename << std::endl;
    exit(1);
  }
  std::streampos size = file.tellg();
  if (size > (4096 - 0x200)) {
    std::cerr << "ROM is too large for memory." << std::endl;
    exit(1);
  }
  file.seekg(0, std::ios::beg);
  file.read(reinterpret_cast<char *>(&memory[0x200]), size);
  file.close();
}

void Chip8::handleInput() {
  for (int i = 0; i < 16; ++i) {
    keypad[i] = 0;
  }
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
  uint16_t opcode = memory[pc] << 8 | memory[pc + 1];
  pc += 2;

  uint16_t NNN = opcode & 0x0FFF;
  uint8_t NN = opcode & 0x00FF;
  uint8_t N = opcode & 0x000F;
  uint8_t X = (opcode & 0x0F00) >> 8;
  uint8_t Y = (opcode & 0x00F0) >> 4;

  switch (opcode & 0xF000) {
  case 0x0000:
    switch (NN) {
    case 0xE0:
      display.assign(64 * 32, 0);
      break;
    case 0xEE:
      pc = stack[--sp];
      break;
    default:
      break;
    }
    break;
  case 0x1000:
    pc = NNN;
    break;
  case 0x2000:
    stack[sp++] = pc;
    pc = NNN;
    break;
  case 0x3000:
    if (V[X] == NN)
      pc += 2;
    break;
  case 0x4000:
    if (V[X] != NN)
      pc += 2;
    break;
  case 0x5000:
    if (V[X] == V[Y])
      pc += 2;
    break;
  case 0x6000:
    V[X] = NN;
    break;
  case 0x7000:
    V[X] += NN;
    break;
  case 0x8000:
    switch (N) {
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
    break;
  case 0x9000:
    if (V[X] != V[Y])
      pc += 2;
    break;
  case 0xA000:
    I = NNN;
    break;
  case 0xB000:
    pc = NNN + V[0];
    break;
  case 0xC000:
    V[X] = (rand() % 256) & NN;
    break;
  case 0xD000: {
    uint8_t coordX = V[X] % 64;
    uint8_t coordY = V[Y] % 32;
    V[0xF] = 0;
    for (int row = 0; row < N; ++row) {
      uint8_t spriteByte = memory[I + row];
      for (int col = 0; col < 8; ++col) {
        if ((spriteByte & (0x80 >> col)) != 0) {
          size_t index = (coordY + row) * 64 + (coordX + col);
          if (index < display.size()) {
            if (display[index] == 1)
              V[0xF] = 1;
            display[index] ^= 1;
          }
        }
      }
    }
    break;
  }
  case 0xE000:
    switch (NN) {
    case 0x9E:
      if (keypad[V[X]])
        pc += 2;
      break;
    case 0xA1:
      if (!keypad[V[X]])
        pc += 2;
      break;
    }
    break;
  case 0xF000:
    switch (NN) {
    case 0x07:
      V[X] = delay_timer;
      break;
    case 0x0A: {
      bool keyPressed = false;
      for (int i = 0; i < 16; ++i) {
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
    break;
  default:
    break;
  }
}