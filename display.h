#pragma once

#include "chip8.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <array>
#include <cstdint>

class Display {
public:
  Display(Chip8 &chip8);
  ~Display();
  void update_screen(Chip8 &chip8);
  void handle_events(Chip8 &chip8);

private:
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Event event;
  std::array<SDL_Keycode, 16> configurable_keys;
  // this is chip8 display pixel array converted to sdl2 format
  // of pixel in rgba
  std::array<uint32_t, 64 * 32> pixel_data;
};
