// this is just a test file
// to build:
// cls && clang-cl main.c raylib/src/libraylib.lib winmm.lib shell32.lib Gdi32.lib -Iraylib/src
#include "raylib.h"

int main() {
  const int screenWidth = 800;
  const int screenHeight = 600;
  InitWindow(screenWidth, screenHeight, "Raylib basic window");
  SetTargetFPS(60);
  Camera2D camera = {0};
  camera.zoom = 1.f;

  while (!WindowShouldClose()) {
    BeginDrawing();
    BeginMode2D(camera);
    ClearBackground(RAYWHITE);
    DrawText("It works!", 20, 20, 20, BLACK);
    DrawRectangle(0,0,200,200,RED);
    EndMode2D();
    EndDrawing();
  }
  CloseWindow();
  return 0;
}