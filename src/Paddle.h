#ifndef PADDLE_H
#define PADDLE_H

#include <raylib.h>

class Paddle {
protected:
    void limitMovement();

public:
    float x, y, width, height;
    int speed;
    int flickerTimer = 0;

    void draw();
    void update();
};

class CpuPaddle : public Paddle {
public:
    void update(int ballY);
};

#endif // PADDLE_H
