#include "Ball.h"
#include <raylib.h>

void Ball::draw() {
    DrawCircle(x, y, radius, WHITE);
}

int Ball::update(float ballSpeedMultiplier) {
    x += speedX * ballSpeedMultiplier;
    y += speedY * ballSpeedMultiplier;

    if (y + radius >= GetScreenHeight() || y - radius <= 0) speedY *= -1;

    if (x + radius >= GetScreenWidth()) {
        return 1; // CPU scored
    }
    if (x - radius <= 0) {
        return 2; // Player scored
    }
    return 0; // No one scored
}

void Ball::resetBall() {
    x = GetScreenWidth() / 2.0f;
    y = GetScreenHeight() / 2.0f;
    int choices[2] = { -1, 1 };
    speedX = 7 * choices[GetRandomValue(0, 1)];
    speedY = 7 * choices[GetRandomValue(0, 1)];
}
