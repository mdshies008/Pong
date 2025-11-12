#include "Paddle.h"
#include <raylib.h>

void Paddle::limitMovement() {
    if (y <= 0) y = 0;
    if (y + height >= GetScreenHeight()) y = GetScreenHeight() - height;
}

void Paddle::draw() {
    Color c = (flickerTimer > 0) ? RED : WHITE;
    DrawRectangle((int)x, (int)y, (int)width, (int)height, c);
}

void Paddle::update() {
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) y -= speed;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) y += speed;
    limitMovement();
    if (flickerTimer > 0) flickerTimer--;
}

void CpuPaddle::update(int ballY) {
    if (y + height / 2 > ballY) y -= speed;
    else y += speed;
    limitMovement();
    if (flickerTimer > 0) flickerTimer--;
}
