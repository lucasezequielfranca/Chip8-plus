#include "chip8.h"
#include "display.h"
#include <cstdint>
#include <iostream>

uint8_t exit_program();

int main(int argc, char *argv[]) {
  Chip8 cpu;
  cpu.initialize();
  cpu.load_rom(argv[1]);

  Display display;

  while (true) {
    cpu.execute_cycle();

    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 64; j++) {
        if (cpu.gfx[(i * 64) + j]) {
          std::cout << "▮▮";
        } else {

          std::cout << "  ";
        }
      }
      std::cout << std::endl;
    }
    display.update_screen(cpu);
  }
}

uint8_t exit_program() {
  std::cout << "Exiting program!" << std::endl;
  return 1;
}
