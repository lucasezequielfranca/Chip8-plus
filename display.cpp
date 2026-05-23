#pragma once
#include "display.h"
#include "chip8.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>

Display::Display() {

  pixel_data.fill(0);
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    std::cout << "Error: " << SDL_GetError() << std::endl;
    throw std::runtime_error("SDL initialization failed");
  }

  window = SDL_CreateWindow("Chip8", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);

  if (!window) {
    std::cout << "Error: " << SDL_GetError() << std::endl;
    throw std::runtime_error("Could not create a window");
  }
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cout << "Error: " << SDL_GetError() << std::endl;
    throw std::runtime_error("Could not create renderer");
  }
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                              SDL_TEXTUREACCESS_STREAMING, 64, 32);
}
Display::~Display() {}

uint8_t Display::update_screen(Chip8 &chip8) {
  for (uint16_t i = 0, size = chip8.gfx.size(); i++;) {
    chip8.gfx[i] ? pixel_data[i] = 0xFFFFFFFF : 0x000000FF;
  }
  surface =
      SDL_CreateRGBSurfaceFrom(pixel_data.data(), 64, 32, 8, 64, 0, 0, 0, 0);
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);
  return 0;
}
