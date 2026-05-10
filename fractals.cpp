#include "SDL3/SDL_events.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_stdinc.h"
#include "SDL3/SDL_timer.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstdint>
#include <iostream>
#include <vector>
#include "math.h"

std::vector<uint32_t> mandelbrot(int WIDTH, int HEIGHT, std::vector<uint32_t>& pixels) {
  double SCALE = 50;
  int max_iters = 100;
  double x_lbound = -2, x_ubound = 1;
  double y_lbound = -1.5, y_ubound = 1.5;

  for (int x = 0; x < WIDTH; x++) {
      for (int y = 0; y < HEIGHT; y++) {
          double zr = 0.0, zi = 0.0;
          int iters = 0;

          double cr = x_lbound + ((double)x / WIDTH)  * (x_ubound - x_lbound);
          double ci = y_lbound + ((double)y / HEIGHT) * (y_ubound - y_lbound);

          while (zr*zr + zi*zi < 4.0 && iters < max_iters) {
              double zr_next = zr*zr - zi*zi + cr;
              double zi_next = 2*zr*zi + ci;
              zr = zr_next;
              zi = zi_next;
              iters++;
          }

          uint8_t r, g, b;
          if (iters == max_iters) {
              r = g = b = 0;
          } else {
              r = (iters * 9) % 256;
              g = (iters * 5) % 256;
              b = (iters * 3) % 256;
          }

          pixels[y * WIDTH + x] = ((uint32_t)r << 24) |
                                   ((uint32_t)g << 16) |
                                   ((uint32_t)b <<  8) |
                                   0xFF;
      }
  }
  return pixels;
}

int main(int argc, char* argv[]) {
  
  const int WIDTH = 800;
  const int HEIGHT = 800;

  // 1. Initialize SDL subsystems (Video in this case)
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  // 2. Create a Window and Renderer in one go
  // SDL3 simplifies this process significantly.
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;

  if (!SDL_CreateWindowAndRenderer("Mandelbrot", WIDTH, HEIGHT, 0, &window, &renderer)) {
    std::cerr << "Window/Renderer Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

  SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
  SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255); // Dark Background


  bool quit = false;
  SDL_Event event;

  // double SCALE = 50;
  // int max_iters = 100;
  // double x_lbound = -2, x_ubound = 1;
  // double y_lbound = -1.5, y_ubound = 1.5;

  // double x_lbound = 15.0/SCALE, x_ubound = 17.0/SCALE;
  // double y_lbound = 0.5/SCALE, y_ubound = 2.5/SCALE;


  std::vector<uint32_t> pixels_init(WIDTH * HEIGHT, 0);
  std::vector<uint32_t> pixels = mandelbrot(WIDTH, HEIGHT, pixels_init);
  SDL_UpdateTexture(texture, NULL, pixels.data(), WIDTH * sizeof(uint32_t));

  double x_lbound = -2, x_ubound = 1;
  double y_lbound = -1.5, y_ubound = 1.5;

  // Declarations for mouse location info
  float mouse_x, mouse_y;
  float mouse_real, mouse_imag;
  // double cr = x_lbound + ((double)x / WIDTH)  * (x_ubound - x_lbound);

  bool dragging = false;
  float drag_start_x, drag_start_y;
  float drag_end_x, drag_end_y;




  // 3. The Main Loop
  while (!quit) {
    // Event Handling
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
          quit = true;
      }

      // Testing mouse button presses
      if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        dragging = true;
        drag_start_x = event.button.x;
        drag_start_y = event.button.y;
        // SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255); // Dark Background
      }
      if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        dragging = false;
      }

      }
    // This is outside the polling event

    
    // Coordinate locations at mouse pointer 
    SDL_GetMouseState(&mouse_x, &mouse_y);
    mouse_real = x_lbound + (mouse_x / WIDTH) * (x_ubound - x_lbound);
    mouse_imag = y_lbound + (mouse_y / HEIGHT) * (y_ubound - y_lbound);


    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // SDL_RenderDebugTextFormat(renderer, 10.0f, 10.0f, "Location: %.0f, %.0f", *mouse_x, *mouse_y);
    SDL_RenderDebugTextFormat(renderer, 10.0f, 10.0f, "Location: %4.2f, %4.2f", mouse_real, -mouse_imag);

    if (dragging) {
      SDL_RenderDebugTextFormat(renderer, 10.0f, 10.0f, "Draggin: %4.2f, %4.2f", mouse_real, -mouse_imag);
    }

    SDL_RenderPresent(renderer);
    // 4. Rendering
    // SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255); // Dark Background
    // SDL_RenderClear(renderer);

    // // Draw a simple white rectangle (your "player" or "particle")
    // SDL_FRect rect = { 350.0f, 250.0f, 100.0f, 100.0f };
    // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // SDL_RenderFillRect(renderer, &rect);

    // SDL_RenderPresent(renderer);
  }

  // 5. Cleanup
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
