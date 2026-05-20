#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <iterator>
#include <ostream>
#include <thread>

typedef struct chip8 {
  // memory is set to 4096 bytes
  uint8_t memory[4096];
  uint16_t pc;
  // 16 stack with 16 bits or 2 bytes each
  uint16_t stack[16];
  uint8_t sp;
  // 16 bits register i
  uint16_t i;
  // v0 to vF register multiple pourpose
  uint16_t vRegister[16];

  // timers run down at 60hz
  uint8_t delay_timer;
  uint8_t sound_timer;

  uint8_t display[64 * 32];

  const uint8_t FONT_SET[80] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };

  uint8_t keypad[16];

} chip8;

void initChip8(chip8 &cpu) {
  // set program counter to 512 or 0x200
  cpu.pc = 0x200;
  // initialize the font_set at 0x50 in memory
  for (int i = 0; i < 80; i++) {
    cpu.memory[0x50 + i] = cpu.FONT_SET[i];
  }
}

// need to check for 0 return before continuing with the cpu cycle, if return is
// 1 stop the program
int loadRom(chip8 &cpu, char *filename) {
  // inicia um cursor que vai receber o tamanho da rom
  std::streampos size;
  // opens the file in if mode (read mode) deafult ios is in for ifstream, opens
  // in binary, set the pointer then to the end of the file with ios::ate, this
  // is needed for dinamicly getting the size for the size var above
  std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    std::cout << "file could not be open" << std::endl;
    return 1;
  }
  // pass the size of the file to size cursor
  size = file.tellg();
  // set the file cursor to the beggining again
  // for the copy to work
  file.seekg(0, std::ios::beg);
  // copy the contents of the rom to the cpu memory starting at pc
  // counter (0x200), aos need to rcast the uint8_t to char* using
  // reinterpret_cast<char*>() as its expected for the read method
  file.read(reinterpret_cast<char *>(&cpu.memory[cpu.pc]), size);

  file.close();
  return 0;
}

void executeCicle(chip8 &cpu) {
  uint16_t opcode = (cpu.memory[cpu.pc] << 8) | (cpu.memory[cpu.pc + 1]);
  std::cout << "Executing Opcode: 0x" << std::hex << opcode << " at PC: 0x"
            << cpu.pc << std::dec << std::endl;
  cpu.pc += 2;

  uint8_t type = (opcode & 0xF000) >> 12; // first nibble (4bits) type of opcode
  uint8_t X = (opcode & 0x0F00) >> 8;     // second nibble
  uint8_t Y = (opcode & 0x00F0) >> 4;     // third nibble
  uint8_t N = (opcode & 0x000F);          // last nibble
  uint8_t NN = (opcode & 0x00FF);         // third and last nibble
  uint16_t NNN = (opcode & 0x0FFF);       // second, third and last nibble

  switch (type) {
  case 0x0: {
    switch (NN) {
      // clear the display buffer
    case 0xE0: {
      std::fill(std::begin(cpu.display), std::end(cpu.display), 0);
      break;
    } break;
    }
    break;
  }
    // jump, set pc to NNN just it
  case 0x1: {
    cpu.pc = NNN;
    break;
  }
  case 0x2: {
  }
  case 0x3: {
  }
  case 0x4: {
  }
  case 0x5: {
  }
    // set VX to NN
  case 0x6: {
    cpu.vRegister[X] = NN;
    break;
  }
    // ADD to VX the value of NN
  case 0x7: {
    cpu.vRegister[X] += NN;
    break;
  }
  case 0x8: {
  }
  case 0x9: {
  }
    // set register i to NNN
  case 0xA: {
    cpu.i = NNN;
    break;
  }
  case 0xB: {
  }
  case 0xC: {
  }
  // quit complex to comment here the DXYN instruction;
  case 0xD: {
    // get the modulo (the "resto" da divisao), pra pegar o quanto sobra apos
    // "contornar" a tela algumas vezes, apos o wrap around e pega a cordenada
    // de inicio e a cordenada de fim do sprite
    uint8_t XCoord = cpu.vRegister[X] & 0x3F;
    uint8_t YCoord = cpu.vRegister[Y] & 0x1F;

    cpu.vRegister[0xF] = 0;

    // counts from 0 to N to drawn a sprite of pixels from top (0), to bottom at
    // row N
    for (uint8_t row = 0; row < N; row++) {

      // check if coord initial Y + the row to drawn now is still inside the
      // screen, else breaks
      if ((YCoord + row) >= 32) {
        break;
      }

      // sprite data of the row is loaded from starting at i register and going
      // up for every row, i does not increment
      uint8_t spriteData = cpu.memory[cpu.i + row];

      // will iterate 8 times for every row aka for every bit in the row
      for (uint8_t collum = 0; collum < 8; collum++) {
        // same check of above but now checks if the sprite is inside the screen
        // to the right
        if (XCoord + collum >= 64) {
          break;
        }
        // screen index for the display array will be, deafult x,y value +
        // collum for x (aka default x value + offset to the right for every
        // collum (bit)), and deafult Y coord + (jumps 64 indexs for every row
        // not used, ex. for row at coord 10 out of 32, will jump 10 * 64, as
        // every horizontal line have 64 spaces)
        uint16_t screenIndex = (XCoord + collum) + ((YCoord + row) * 64);
        // mask sprite data ex 01010101 with 0x80 (10000000) bitshifted by
        // collum number ex. if collum is 5 0x80 will be shifted to right to
        // become (00000100) this get the 6th (0-7 so 5 is actualy the 6th), bit
        // data so is true if anything above 0 ie. 1 or above
        uint8_t bit = spriteData & (0x80 >> collum);

        // if bit from collum is 0 do nothing as theres nothing to drawn
        if (!bit) {
          continue;
        }
        // if bit is true and screenIndex aka bit in the display buffer is 1,
        // set VF register to 1 (aka colision register)
        if (cpu.display[screenIndex]) {
          cpu.vRegister[0xF] = 1;
        }
        // will perform a xor operation in the display with bit on or off if the
        // bit is off in display and on 1 sprite, display gets on. if display is
        // on and sprite bit is on, display gets off, and trigger above the
        // collision register
        cpu.display[screenIndex] ^= bit;
      }
    }
    break;
  }
  case 0xE: {
  }
  case 0xF: {
  }
  }
}

int main(int argc, char *argv[]) {
  // creates a sdl2 window
  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_Window *window =
      SDL_CreateWindow("Chip8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       640, 320, SDL_WINDOW_SHOWN);

  // check if windows is created, if not cloases the program
  if (!window) {
    std::cout << "Could not create window, ending program, error: "
              << SDL_GetError() << std::endl;
    return 1;
  }

  // initialize chip8 as CHIP8 and run initialize func
  chip8 CHIP8;
  initChip8(CHIP8);
  // run check if rom is loaded correctly
  int isNotLoaded = loadRom(CHIP8, argv[1]);
  if (isNotLoaded) {
    std::cout << "Ending program!" << std::endl;
    return 1;
  }
  std::cout << "Rom loaded with sucess" << std::endl;

  while (true) {
    executeCicle(CHIP8);

    std::cout << "\x1B[2J\x1B[H";

    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 64; j++) {
        if (CHIP8.display[j + (i * 64)]) {
          std::cout << "█";
        } else {

          std::cout << " ";
        }
      }
      std::cout << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
}
