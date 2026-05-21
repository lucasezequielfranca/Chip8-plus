#include "chip8.h"
#include <iostream>

int main(int argc, char *argv[]) {
  Chip8 cpu;
  cpu.initialize();
  cpu.load_rom(argv[1]);
  while (true) {
    cpu.execute_cycle();

    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 64; j++) {
        if (cpu.display[(i * 64) + j]) {
          std::cout << "▮▮";
        } else {

          std::cout << "  ";
        }
      }
      std::cout << std::endl;
    }
  }
}
