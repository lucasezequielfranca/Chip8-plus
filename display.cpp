#include "display.h"
#include "chip8.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>

Display::Display() {

  pixel_data.fill(0); // fill pixel array with black

  // will try to initialize sdl
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    std::cout << "Error: " << SDL_GetError() << std::endl;
    throw std::runtime_error("SDL initialization failed");
  }

  // will try to create a sdl2 window
  window = SDL_CreateWindow("Chip8", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, Chip8::GFX_WIDTH * 10,
                            Chip8::GFX_HEIGHT * 10, SDL_WINDOW_OPENGL);
  if (!window) {
    std::cout << "Error: " << SDL_GetError() << std::endl;
    throw std::runtime_error("Could not create a window");
  }

  // will try to create a sdl2 render, necessary to render to the screen
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cout << "Error: " << SDL_GetError() << std::endl;
    throw std::runtime_error("Could not create renderer");
  }

  // will try to create a sdl2 texture where we put our pixel array
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                              SDL_TEXTUREACCESS_STREAMING, Chip8::GFX_WIDTH,
                              Chip8::GFX_HEIGHT);
}
Display::~Display() {}

void Display::update_screen(Chip8 &chip8) {
  // converts my chip8 gfx array to a pixel data in rgba format for sdl2
  for (uint16_t i = 0, size = chip8.gfx.size(); i < size; i++) {
    chip8.gfx[i] ? pixel_data[i] = 0xFFFFFFFF : 0x000000FF;
  }
  // clear the renderer for the next screen
  SDL_RenderClear(renderer);

  // update the texture by using the pixel data of the screen, and cutting it in
  // rows every GFX_WIDTH * sizer o pixel uint32 in formtat 0xFFFFFFF
  SDL_UpdateTexture(texture, NULL, pixel_data.data(),
                    Chip8::GFX_WIDTH * sizeof(uint32_t));

  // copy the texture into the render, the null bellow is to use whole texture
  // mapped to whole render
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  // render the preser renderer to the screen
  SDL_RenderPresent(renderer);
}
