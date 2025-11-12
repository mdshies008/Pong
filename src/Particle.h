#ifndef PARTICLE_H
#define PARTICLE_H

#include <raylib.h>
#include <vector>

struct Particle {
    Vector2 position;
    Vector2 velocity;
    float radius;
    Color color;
    float life;
};

void spawnParticles(float x, float y, Color color);
void updateParticles();
void drawParticles();

#endif // PARTICLE_H