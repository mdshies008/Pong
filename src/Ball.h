#ifndef BALL_H
#define BALL_H

#include <raylib.h>

class Ball {
public:
    float x, y;
    int speedX, speedY, radius;

    void draw();
    int update(float ballSpeedMultiplier);
    void resetBall();
};

#endif // BALL_H
