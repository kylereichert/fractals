#include "SDL3/SDL_events.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_stdinc.h"
#include "SDL3/SDL_timer.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include "math.h"

double map_pixel(double new_lbound, double new_ubound, int current_pixel, double size) {
   double new_mapping  = new_lbound + ((double)current_pixel / size) * (new_ubound - new_lbound);
   return new_mapping;
}

std::vector<uint32_t> mandelbrot(int WIDTH, 
                                 int HEIGHT,
                                 std::vector<uint32_t>& pixels,
                                 double x_lbound = -2, double x_ubound = 1,
                                 double y_lbound = -1.5, double y_ubound = 1.5
                                 ) {
  double SCALE = 50;
  int max_iters = 600;

  for (int x = 0; x < WIDTH; x++) {
      for (int y = 0; y < HEIGHT; y++) {
          double zr = 0.0, zi = 0.0;
          int iters = 0;

          // Maps the coordinates to the window's pixel counts
          double cr = map_pixel(x_lbound, x_ubound, x, WIDTH);
          double ci = map_pixel(y_lbound, y_ubound, y, HEIGHT);

          // The actual Mandelbrot algorithm
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
  // Window size 
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

  // Initial Mandelbrot texture
  std::vector<uint32_t> pixels_init(WIDTH * HEIGHT, 0);
  std::vector<uint32_t> pixels = mandelbrot(WIDTH, HEIGHT, pixels_init);
  SDL_UpdateTexture(texture, NULL, pixels.data(), WIDTH * sizeof(uint32_t));

  double x_lbound = -2, x_ubound = 1;
  double y_lbound = -1.5, y_ubound = 1.5;

  // Declarations for mouse location info
  float mouse_x, mouse_y;
  double mouse_real, mouse_imag;
  // double cr = x_lbound + ((double)x / WIDTH)  * (x_ubound - x_lbound);

  bool dragging = false;
  bool drag_release = false;
  double drag_start_x, drag_start_y;
  double drag_end_x, drag_end_y;


  // 3. The Main Loop
  while (!quit) {
    // Event Handling
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
          quit = true;
      }

      // Grabs mouse events for creating the zoom box
      if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        dragging = true;
        drag_start_x = event.button.x;
        drag_start_y = event.button.y;
      }
      if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        drag_end_x = event.button.x;
        drag_end_y = event.button.y;
        dragging = false;
        drag_release = true;
      }

    }
    
    SDL_SetRenderVSync(renderer, 1); // Set the framerate, PC goin cray cray

    // Coordinate locations at mouse cursor
    SDL_GetMouseState(&mouse_x, &mouse_y);
    mouse_real = map_pixel(x_lbound, x_ubound, mouse_x, WIDTH);
    mouse_imag = map_pixel(y_lbound, y_ubound, mouse_y, HEIGHT);

    // Clear old frame -> render new frame -> render over top of that frame
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugTextFormat(renderer, 10.0f, 10.0f, "Location: %4.2f, %4.2f", mouse_real, -mouse_imag);

    if (dragging) {
      // New boundaries for rectangle and zoom
      double new_x_lbound = drag_start_x;
      double new_x_ubound = mouse_x - drag_start_x;
      double new_y_lbound = drag_start_y;
      double new_y_ubound = mouse_y - drag_start_y;

      // Actually draws the box
      SDL_FRect select_box = {(float)drag_start_x, (float)drag_start_y, 
                              (float)mouse_x-(float)drag_start_x,
                              (float)mouse_y-(float)drag_start_y};
      SDL_RenderRect(renderer, &select_box);

    }

    // Zoom function. Will zoom using selected coordinates as new boundaries.
    if (drag_release) {
      // Match the pixel counts in the boundaries so the image doesn't skew
      int x_pixel_diff = abs(drag_end_x - drag_start_x);
      int y_pixel_diff = abs(drag_end_y - drag_start_y);

      // Find which direction is larger and subtract that from the larger axis
      if (x_pixel_diff > y_pixel_diff) {
        int offset = x_pixel_diff - y_pixel_diff;
        drag_end_x -= offset;
      } else {
        int offset = y_pixel_diff - x_pixel_diff;
        drag_end_y -= offset;
      }

      // Boundary recomputation, accounting for squaring
      double c_real_start= map_pixel(x_lbound, x_ubound, drag_start_x, WIDTH);
      double c_imag_start= map_pixel(y_lbound, y_ubound, drag_start_y, HEIGHT);
      double c_real_end= map_pixel(x_lbound, x_ubound, drag_end_x, WIDTH);
      double c_imag_end= map_pixel(y_lbound, y_ubound, drag_end_y, HEIGHT);

      /*
        Boundary Setup
        Need to consider endpoints. If the square starts bottom right and moves
        up and left, need to swap the upper and lower bounds so the make sense
      */

      auto result_real = std::minmax(c_real_start, c_real_end);
      auto result_imag = std::minmax(c_imag_start, c_imag_end);

      double new_x_lbound = result_real.first;
      double new_x_ubound = result_real.second;
      double new_y_lbound = result_imag.first;
      double new_y_ubound = result_imag.second;

      std::vector<uint32_t> pixels_zoom(WIDTH * HEIGHT, 0);
      // std::vector<uint32_t> pixels = mandelbrot(WIDTH, HEIGHT, pixels_init);
      std::vector<uint32_t> pixels = mandelbrot(WIDTH, HEIGHT,
                                      pixels_init,
                                      new_x_lbound, new_x_ubound,
                                      new_y_lbound, new_y_ubound);


      double mouse_real_zoom = map_pixel(new_x_lbound, new_x_ubound, mouse_x, WIDTH);
      double mouse_imag_zoom = map_pixel(new_y_lbound, new_y_ubound, mouse_y, HEIGHT);

      SDL_RenderClear(renderer);
      SDL_UpdateTexture(texture, NULL, pixels.data(), WIDTH * sizeof(uint32_t));
      // SDL_RenderDebugTextFormat(renderer, 10.0f, 10.0f, "Location: %4.2f, %4.2f", mouse_real_zoom, -mouse_imag_zoom);
      x_lbound = new_x_lbound;
      x_ubound = new_x_ubound;
      y_lbound = new_y_lbound;
      y_ubound = new_y_ubound;



      // std::cout << "Its working?:" << mouse_real << " " << drag_end_y << '\n';
      drag_release = false;
    }


    SDL_RenderPresent(renderer);
  }

  // 5. Cleanup
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
