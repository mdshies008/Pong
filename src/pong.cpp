#include "Game.h"
#include <raylib.h>

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Pong");
    SetExitKey(0);
    SetTargetFPS(60);

    Game game(screenWidth, screenHeight);
    game.run();

    CloseWindow();
    return 0;
}