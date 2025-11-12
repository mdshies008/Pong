#include "Particle.h"
#include <cmath>
#include <algorithm>

std::vector<Particle> particles;

void spawnParticles(float x, float y, Color color) {
    for (int i = 0; i < 8; i++) {  // Reduced from 20 to 8 particles
        float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
        float speed = (float)GetRandomValue(40, 100);
        particles.push_back({
            {x, y},
            {cosf(angle) * speed, sinf(angle) * speed},
            (float)GetRandomValue(2, 4),  // Bigger particles for better visibility
            color,
            0.4f  // Shorter lifespan for compact trail
        });
    }
}

void updateParticles() {
    float dt = GetFrameTime();
    for (auto& p : particles) {
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;
        p.life -= dt;
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p) {
        return p.life <= 0;
    }), particles.end());
}

void drawParticles() {
    for (const auto& p : particles) {
        DrawCircleV(p.position, p.radius, Fade(p.color, p.life));
    }
}