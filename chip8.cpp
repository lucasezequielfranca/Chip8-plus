#include "chip8.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <ostream>
#include <sys/types.h>

Chip8::Chip8() {
  memory.fill(0);
  pc = 0;
  i = 0;
  stack.fill(0);
  stack_pointer = 0;
  v_register.fill(0);
  delay_timer = 0;
  sound_timer = 0;
  gfx.fill(0);
  update_screen_flag = 0;
  instructions_per_second = 700;
  running_flag = 1;
  opcode = 0;
  type = 0;
  X = 0;
  Y = 0;
  N = 0;
  NN = 0;
  NNN = 0;
  basic_emulator_flag = 0;
  keypad.fill(0);
}
Chip8::~Chip8() {}

void Chip8::initialize() {
  pc = 0x200;
  std::copy(std::begin(FONT_SET), std::end(FONT_SET), &memory[0x50]);

  std::cout << "Interpreter initialized with sucess" << std::endl;
}

uint8_t Chip8::load_rom(char *filename) {
  std::ifstream rom(filename, std::ios::in | std::ios::binary | std::ios::ate);
  if (!rom.is_open()) {
    std::cout << "Could not open rom" << filename << std::endl;
    return 1;
  }
  uint16_t size = rom.tellg();
  rom.seekg(std::ios::beg);

  rom.read(reinterpret_cast<char *>(&memory[pc]), size);

  rom.close();
  std::cout << "Rom loaded with sucess" << std::endl;
  return 0;
}

void Chip8::execute_cycle() {
  opcode = (memory[pc] << 8) | memory[pc + 1];
  // std::cout << "Decoding opcode: " << std::hex << opcode << std::endl;

  pc += 2;

  type = (opcode & 0xF000) >> 12;

  X = (opcode & 0x0F00) >> 8;
  Y = (opcode & 0x00F0) >> 4;
  N = opcode & 0x000F;
  NN = opcode & 0x00FF;
  NNN = opcode & 0x0FFF;

  switch (type) {
  default:
    std::cout << "Could Not decode opcode: " << std::hex << (opcode)
              << std::endl;
    break;
  case 0x0:
    switch (NN) {
    default:
      std::cout << "Could Not decode opcode: " << std::hex << (opcode)
                << std::endl;
      break;
    case 0xE0:
      gfx.fill(0);
      break;
    case 0xEE:
      pc = stack[stack_pointer];
      stack[stack_pointer] = 0;
      stack_pointer--;
      break;
    case 0x00:
      break;
    }
    break;
  case 0x1:
    pc = NNN;
    break;
  case 0x2:
    stack[stack_pointer] = pc;
    stack_pointer++;
    pc = NNN;
    break;
  case 0x3:
    if (v_register[X] == NN) {
      pc += 2;
    }
    break;
  case 0x4:
    if (v_register[X] != NN) {
      pc += 2;
    }
    break;
  case 0x5:
    if (v_register[X] == v_register[Y]) {
      pc += 2;
    }
    break;
  case 0x6:
    v_register[X] = NN;
    break;
  case 0x7:
    v_register[X] += NN;
    break;
  case 0x8:
    switch (N) {
    default:
      std::cout << "Could Not decode opcode: " << std::hex << (opcode)
                << std::endl;
      break;
    case 0x0:
      v_register[X] = v_register[Y];
      break;
    case 0x1:
      v_register[X] = (v_register[X] | v_register[Y]);
      break;
    case 0x2:
      v_register[X] = (v_register[X] & v_register[Y]);
      break;
    case 0x3:
      v_register[X] = (v_register[X] ^ v_register[Y]);
      break;
    case 0x4: {
      uint8_t sum = (v_register[X] + v_register[Y]);
      uint8_t carry = (v_register[X] > sum) ? 1 : 0;

      v_register[X] = sum;
      v_register[0xF] = carry;
      break;
    }
    case 0x5: {
      uint8_t vf_flag = (v_register[X] >= v_register[Y]) ? 1 : 0;
      v_register[X] = (v_register[X] - v_register[Y]);
      v_register[0xF] = vf_flag;
      break;
    }
    case 0x6: {
      if (basic_emulator_flag) {
        v_register[X] = v_register[Y];
      }
      uint8_t carry = (v_register[X] & 0x1);
      v_register[X] >>= 1;
      v_register[0xF] = carry;
      break;
    }
    case 0x7: {
      uint8_t vf_flag = (v_register[Y] >= v_register[X]) ? 1 : 0;
      v_register[X] = (v_register[Y] - v_register[X]);
      v_register[0xF] = vf_flag;
      break;
    }
    case 0xE: {
      if (basic_emulator_flag) {
        v_register[X] = v_register[Y];
      }
      uint8_t carry = (v_register[X] & 0x80) >> 7;
      v_register[X] <<= 1;
      v_register[0xF] = carry;
      break;
    }
    }
    break;
  case 0x9:
    if (v_register[X] != v_register[Y]) {
      pc += 2;
    }
    break;
  case 0xA:
    i = NNN;
    break;
  case 0xB:
    if (basic_emulator_flag) {
      pc = NNN + v_register[0];
    } else {
      pc = ((X << 8) | NN) + v_register[X];
    }
    break;
  case 0xC:
    v_register[X] = (rand() & NN);
    break;
  case 0xD: {
    uint8_t base_coord_x = v_register[X] & 63;
    uint8_t base_coord_y = v_register[Y] & 31;
    v_register[0xF] = 0;

    for (uint8_t row = 0; row < N; row++) {
      uint8_t sprite_data = memory[i + row];

      if ((base_coord_y + row) >= 32) {
        break;
      }
      for (int collum = 0; collum < 8; collum++) {
        if ((base_coord_x + collum) >= 64) {
          continue;
        }
        uint8_t bit = (0x80 >> collum) & sprite_data;

        uint16_t display_index =
            (base_coord_x + collum) + (((base_coord_y + row) * 64));
        if (gfx[display_index] && bit) {
          v_register[0xF] = 1;
        }
        gfx[display_index] ^= bit;
        continue;
      }
      update_screen_flag = 1;
    }
    break;
  }
  case 0xE:
    switch (NN) {
    default:
      std::cout << "Could Not decode opcode: " << std::hex << (opcode)
                << std::endl;
      break;
    }
  case 0x9E:
    if (keypad[v_register[X]]) {
      pc += 2;
    }
    break;
  case 0xA1:
    if (!keypad[v_register[X]]) {
      pc += 2;
    }
    break;
  }
}
