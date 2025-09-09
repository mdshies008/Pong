#include <vector>
#include <iostream>
#include <fstream>
#include <raylib.h>
#include <cmath>
#include <algorithm>

using namespace std;

//Particle Stuff
struct Particle {
    float x, y, vx, vy;
    int life;
    Color color;
};

vector<Particle> particles;

void spawnParticles(float x, float y, Color color) {
    for (int i = 0; i < 15; ++i) {
        float angle = GetRandomValue(0, 360) * PI / 180.0f;
        float speed = GetRandomValue(3, 7);
        Particle p;
        p.x = x; p.y = y;
        p.vx = cosf(angle) * speed; p.vy = sinf(angle) * speed;
        p.life = GetRandomValue(15, 30);
        p.color = color;
        particles.push_back(p);
    }
}

void spawnBallTrail(float x, float y, int radius) {
    for (int i = 0; i < 5; ++i) {
        float angle = GetRandomValue(0, 360) * PI / 180.0f;
        float r = GetRandomValue(0, radius);
        Particle p;
        p.x = x + cosf(angle) * r; p.y = y + sinf(angle) * r;
        p.vx = GetRandomValue(-10, 10) / 50.0f; p.vy = GetRandomValue(-10, 10) / 50.0f;
        p.life = GetRandomValue(8, 15);
        p.color = (GetRandomValue(0, 1) == 0) ? RED : BLUE;
        particles.push_back(p);
    }
}

void updateParticles() {
    for (size_t i = 0; i < particles.size(); ) {
        particles[i].x += particles[i].vx; particles[i].y += particles[i].vy;
        particles[i].life--;
        if (particles[i].life <= 0) {
            particles.erase(particles.begin() + i);
        } else {
            ++i;
        }
    }
}

void drawParticles() {
    for (const auto& p : particles) {
        DrawCircle(p.x, p.y, 4, Fade(p.color, p.life / 30.0f));
    }
}

//Global Variables
int playerScore = 0, playerAnimation = 0, cpuScore = 0, cpuAnimation = 0;
int playerWins = 0, cpuWins = 0;
bool gameStarted = false, gameOver = false, paused = false;
string winner = "";
float startAlpha = 0.0f, winAlpha = 0.0f;
float timePassed = 0.0f, ballSpeedMultiplier = 1.0f, finalUpdateTime = 0.0f, timeTrialDuration = 45.0f;
const float speedIncrement = 0.2f, maxBallSpeedMultiplier = 2.0f;
Sound ballHit, cpuScores, cpuWinsSound, playerScores, gameStart, playerWinsSound;

//Game States
enum GameState { MENU, MODE_SELECT, GAME, SETTINGS };
GameState currentState = MENU;

//Settings
float masterVolume = 1.0f;
int selectedOption = 0, menuSelected = 0, pauseSelected = 0, modeSelected = 0;
bool timeTrialMode = false;


void saveStats() {
    ofstream file("Stats.txt");
    if (file.is_open()) {
        file << playerWins << endl;
        file << cpuWins << endl;
        file.close();
    }
}

void loadStats() {
    ifstream file("Stats.txt");
    if (file.is_open()) {
        file >> playerWins;
        file >> cpuWins;
        file.close();
    } else {
        playerWins = 0;
        cpuWins = 0;
    }
}

class Ball {
public:
    float x, y;
    int speedX, speedY, radius;

    void draw() {
        DrawCircle(x, y, radius, WHITE);
    }

    void update() {
        if (gameOver) return;

        x += speedX * ballSpeedMultiplier;
        y += speedY * ballSpeedMultiplier;

        if (y + radius >= GetScreenHeight() || y - radius <= 0) speedY *= -1;

        if (x + radius >= GetScreenWidth()) {
            cpuScore++;
            cpuAnimation = 15;
            PlaySound(cpuScores);
            if (cpuScore >= 7) {
                gameOver = true;
                winner = "CPU";
                PlaySound(cpuWinsSound);
                cpuWins++;
                saveStats();
            }
            resetBall();
        }
        if (x - radius <= 0) {
            playerScore++;
            playerAnimation = 15;
            PlaySound(playerScores);
            if (playerScore >= 7) {
                gameOver = true;
                winner = "Player";
                PlaySound(playerWinsSound);
                playerWins++;
                saveStats();
            }
            resetBall();
        }
    }

    void resetBall() {
        x = GetScreenWidth() / 2.0f;
        y = GetScreenHeight() / 2.0f;
        int choices[2] = { -1, 1 };
        speedX = 7 * choices[GetRandomValue(0, 1)];
        speedY = 7 * choices[GetRandomValue(0, 1)];
    }
};

class Paddle {
protected:
    void limitMovement() {
        if (y <= 0) y = 0;
        if (y + height >= GetScreenHeight()) y = GetScreenHeight() - height;
    }

public:
    float x, y, width, height;
    int speed;
    int flickerTimer = 0;

    void draw() {
        Color c = (flickerTimer > 0) ? RED : WHITE;
        DrawRectangle((int)x, (int)y, (int)width, (int)height, c);
    }

    void update() {
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) y -= speed;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) y += speed;
        limitMovement();
        if (flickerTimer > 0) flickerTimer--;
    }
};

class CpuPaddle : public Paddle {
public:
    void update(int ballY) {
        if (y + height / 2 > ballY) y -= speed;
        else y += speed;
        limitMovement();
        if (flickerTimer > 0) flickerTimer--;
    }
};
// ---------------- Main Game Loop ----------------
Ball ball;
Paddle player;
CpuPaddle cpu;

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 800;

    ball.x = screenWidth / 2.0f;
    ball.y = screenHeight / 2.0f;
    ball.radius = 20;
    ball.speedX = 7;
    ball.speedY = 7;

    player.width = 25;
    player.height = 120;
    player.x = screenWidth - player.width - 50;
    player.y = screenHeight / 2 - player.height / 2;
    player.speed = 6;

    cpu.width = 25;
    cpu.height = 120;
    cpu.x = 50;
    cpu.y = screenHeight / 2 - cpu.height / 2;
    cpu.speed = 7;

    InitWindow(screenWidth, screenHeight, "Pong");
    SetExitKey(0);
    SetTargetFPS(60);
    InitAudioDevice();

    // Load sounds
    ballHit         = LoadSound("Audio/BallHit.mp3");
    cpuScores       = LoadSound("Audio/CpuScoring.wav");
    cpuWinsSound    = LoadSound("Audio/CpuWINS.wav");
    playerScores    = LoadSound("Audio/PlayerScores.wav");
    gameStart       = LoadSound("Audio/GameStart.mp3");
    playerWinsSound = LoadSound("Audio/PlayerWins.mp3");

    loadStats();

    while (!WindowShouldClose()) {
        // Pause/Resume
        if (currentState == GAME && gameStarted && !gameOver && IsKeyPressed(KEY_ESCAPE)) {
            paused = !paused;
            if (paused) pauseSelected = 0;
        }

        //Update
        if (currentState == MENU) {
            if (IsKeyPressed(KEY_UP)) menuSelected = (menuSelected + 2) % 3;
            if (IsKeyPressed(KEY_DOWN)) menuSelected = (menuSelected + 1) % 3;
            if (IsKeyPressed(KEY_ENTER)) {
                if (menuSelected == 0) currentState = MODE_SELECT;
                else if (menuSelected == 1) currentState = SETTINGS;
                else break;
            }
            if (startAlpha < 1.0f) startAlpha = min(1.0f, startAlpha + 0.03f);
        }
        else if (currentState == MODE_SELECT) {
            if (IsKeyPressed(KEY_UP)) modeSelected = (modeSelected + 1) % 2;
            if (IsKeyPressed(KEY_DOWN)) modeSelected = (modeSelected + 1) % 2;
            if (IsKeyPressed(KEY_ENTER)) {
                gameStarted = true;
                currentState = GAME;
                timeTrialMode = (modeSelected == 1);
                timePassed = 0.0f; ballSpeedMultiplier = 1.0f; 
                playerScore = 0; cpuScore = 0;
                gameOver = false; paused = false;
                winner = "";
                startAlpha = 0.0f; finalUpdateTime = 0.0f;
                ball.resetBall();
                PlaySound(gameStart);
            }
        }
        else if (currentState == GAME) {
            if (!gameOver) {
                if (!paused) {
                    ball.update();
                    spawnBallTrail(ball.x, ball.y, ball.radius);
                    player.update();
                    cpu.update((int)ball.y);

                    float dt = GetFrameTime();
                    timePassed += dt;

                    // Ball speed increases every 30 seconds (classic mode)
                    if (!timeTrialMode && timePassed - finalUpdateTime >= 30.0f) {
                        finalUpdateTime = timePassed;
                        ballSpeedMultiplier += speedIncrement;
                        if (ballSpeedMultiplier > maxBallSpeedMultiplier) ballSpeedMultiplier = maxBallSpeedMultiplier;
                    }

                    // Collision Detection
                    if (CheckCollisionCircleRec(Vector2{ ball.x, ball.y }, ball.radius, Rectangle{ player.x, player.y, player.width, player.height })) {
                        ball.speedX *= -1;
                        player.flickerTimer = 10;
                        spawnParticles(ball.x, ball.y, RED);
                        PlaySound(ballHit);
                    }
                    if (CheckCollisionCircleRec(Vector2{ ball.x, ball.y }, ball.radius, Rectangle{ cpu.x, cpu.y, cpu.width, cpu.height })) {
                        ball.speedX *= -1;
                        cpu.flickerTimer = 10;
                        spawnParticles(ball.x, ball.y, BLUE);
                        PlaySound(ballHit);
                    }

                    updateParticles();

                    // Time Trial Mode check
                    if (timeTrialMode && timePassed >= timeTrialDuration) {
                        gameOver = true;
                        winner = (playerScore > cpuScore) ? "Player" : "CPU";
                        PlaySound(cpuWinsSound);
                    }
                }
                else {
                    if (IsKeyPressed(KEY_UP)) pauseSelected = (pauseSelected + 2) % 3;
                    if (IsKeyPressed(KEY_DOWN)) pauseSelected = (pauseSelected + 1) % 3;
                    if (IsKeyPressed(KEY_ENTER)) {
                        if (pauseSelected == 0) paused = false;
                        else if (pauseSelected == 1) {
                            paused = false; gameStarted = false;
                            currentState = MENU;
                            startAlpha = 0.0f; timePassed = 0.0f; ballSpeedMultiplier = 1.0f; finalUpdateTime = 0.0f;
                            playerScore = 0; cpuScore = 0;
                            gameOver = false;
                            winner = "";
                            ball.resetBall();
                        }
                        else break;
                    }
                }
            }
            else {
                if (IsKeyPressed(KEY_SPACE)) {
                    playerScore = 0; cpuScore = 0;
                    gameOver = false;
                    winner = "";
                    ball.resetBall();
                    gameStarted = false;
                    currentState = MENU;
                    startAlpha = 0.0f; timePassed = 0.0f; ballSpeedMultiplier = 1.0f; finalUpdateTime = 0.0f;
                }
            }
        }
        else if (currentState == SETTINGS) {
            if (IsKeyPressed(KEY_ESCAPE)) currentState = MENU;
            if (selectedOption == 0) {
                if (IsKeyPressed(KEY_LEFT)) {
                    masterVolume -= 0.1f;
                    if (masterVolume < 0.0f) masterVolume = 0.0f;
                    SetMasterVolume(masterVolume);
                }
                if (IsKeyPressed(KEY_RIGHT)) {
                    masterVolume += 0.1f;
                    if (masterVolume > 1.0f) masterVolume = 1.0f;
                    SetMasterVolume(masterVolume);
                }
            }
        }

        // ----------- Animation Logic -----------
        if (playerAnimation > 0) playerAnimation--;
        if (cpuAnimation > 0) cpuAnimation--;

        if (currentState == MENU || currentState == MODE_SELECT) {
            if (startAlpha < 1.0f) startAlpha = min(1.0f, startAlpha + 0.03f);
        }
        else if (gameOver) {
            if (winAlpha < 1.0f) winAlpha = min(1.0f, winAlpha + 0.03f);
        }
        else {
            winAlpha = 0.0f;
        }

        //Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        if (currentState == MENU) {
            Color fadeWhite = Fade(WHITE, startAlpha);
            Color fadeBlue = Fade(BLUE, startAlpha);
            DrawText("PONG", screenWidth / 2 - MeasureText("PONG", 80) / 2, 80, 80, fadeBlue);

            DrawText("Start Game", screenWidth / 2 - MeasureText("Start Game", 40) / 2, screenHeight / 2 - 20, 40, menuSelected == 0 ? RED : fadeWhite);
            DrawText("Settings", screenWidth / 2 - MeasureText("Settings", 40) / 2, screenHeight / 2 + 40, 40, menuSelected == 1 ? RED : fadeWhite);
            DrawText("Exit", screenWidth / 2 - MeasureText("Exit", 40) / 2, screenHeight / 2 + 100, 40, menuSelected == 2 ? RED : fadeWhite);
        }
        else if (currentState == MODE_SELECT) {
            DrawText("SELECT GAME MODE", screenWidth / 2 - MeasureText("SELECT GAME MODE", 50) / 2, 100, 50, WHITE);
            DrawText("Classic", screenWidth / 2 - MeasureText("Classic", 40) / 2, screenHeight / 2 - 20, 40, modeSelected == 0 ? RED : WHITE);
            DrawText("Time Trial", screenWidth / 2 - MeasureText("Time Trial", 40) / 2, screenHeight / 2 + 40, 40, modeSelected == 1 ? RED : WHITE);
        }
        else if (currentState == GAME) {
            DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);
            for (double r = 148; r <= 152; r += 0.2)
                DrawCircleLines(screenWidth / 2, screenHeight / 2, r, WHITE);

            ball.draw();
            cpu.draw();
            player.draw();
            drawParticles();

            // Timer
            int minutes = (int)(timePassed / 60);
            int seconds = (int)(timePassed) % 60;
            string timer = TextFormat("%02i:%02i", minutes, seconds);
            DrawText(timer.c_str(), screenWidth / 2 - MeasureText(timer.c_str(), 40) / 2, 10, 40, WHITE);

            // Scoreboard
            int cpuFontSize = (cpuAnimation > 0) ? 120 : 80;
            Color cpuColor = (cpuAnimation > 0) ? BLUE : WHITE;
            DrawText(TextFormat("%i", cpuScore), screenWidth / 4 - 20, 20, cpuFontSize, cpuColor);

            int playerFontSize = (playerAnimation > 0) ? 120 : 80;
            Color playerColor = (playerAnimation > 0) ? RED : WHITE;
            DrawText(TextFormat("%i", playerScore), 3 * screenWidth / 4 - 20, 20, playerFontSize, playerColor);

            if (paused) {
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.6f));
                const char* pauseOptions[3] = { "Resume", "To Menu", "Exit" };
                for (int i = 0; i < 3; i++)
                    DrawText(pauseOptions[i], screenWidth / 2 - MeasureText(pauseOptions[i], 50) / 2, screenHeight / 2 + i * 60 - 40, 50, pauseSelected == i ? RED : WHITE);
            }

            if (gameOver) {
                Color fadeGreen = Fade(GREEN, winAlpha), fadeWhite = Fade(WHITE, winAlpha);
                DrawText(TextFormat("%s WON!", winner.c_str()), screenWidth / 2 - 200, screenHeight / 2 - 40, 80, fadeGreen);
                DrawText("Press SPACE to restart", screenWidth / 2 - 200, screenHeight / 2 + 60, 40, fadeWhite);
            }
        }
        else if (currentState == SETTINGS) {
            DrawText("---SETTINGS---", GetScreenWidth() / 2 - MeasureText("---SETTINGS---", 50) / 2, 100, 50, WHITE);

            int barWidth = 400, barHeight = 30;
            int barX = GetScreenWidth() / 2 - barWidth / 2;
            int barY = 250;
            DrawRectangle(barX, barY, barWidth, barHeight, GRAY);
            DrawRectangle(barX, barY, (int)(barWidth * masterVolume), barHeight, RED);

            DrawText(TextFormat("Volume: %i%%", (int)(masterVolume * 100)), screenWidth / 2 - 200, 200, 40, GREEN);
        }
        EndDrawing();
    }

    // ----------- Cleanup -----------
    UnloadSound(ballHit);
    UnloadSound(cpuScores);
    UnloadSound(cpuWinsSound);
    UnloadSound(playerScores);
    UnloadSound(gameStart);
    UnloadSound(playerWinsSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
