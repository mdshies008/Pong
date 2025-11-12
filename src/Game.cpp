#include "Game.h"
#include "Particle.h"
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

Game::Game(int screenWidth, int screenHeight)
        : screenWidth(screenWidth), screenHeight(screenHeight),
            /* order here matches declaration order in Game.h */
            currentState(MENU),
            playerScore(0), playerAnimation(0), cpuScore(0), cpuAnimation(0),
            timerAnimation(0), lastCountdownSecond(-1),
            gameStarted(false), gameOver(false), paused(false),
            winner(""), startAlpha(0.0f), winAlpha(0.0f),
            timePassed(0.0f), ballSpeedMultiplier(1.0f), finalUpdateTime(0.0f), timeTrialDuration(45.0f),
            speedIncrement(0.2f), maxBallSpeedMultiplier(2.0f),
            TIMER_ANIM_FRAMES(18),
            /* Sound members left default; will be loaded in body */
            masterVolume(1.0f),
            ballTrailTimer(0.0f),
            selectedOption(0), menuSelected(0), pauseSelected(0), modeSelected(0),
            timeTrialMode(false) {

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

    InitAudioDevice();
    ballHit = LoadSound("Audio/BallHit.mp3");
    cpuScores = LoadSound("Audio/CpuScoring.wav");
    cpuWinsSound = LoadSound("Audio/CpuWINS.wav");
    playerScores = LoadSound("Audio/PlayerScores.wav");
    gameStart = LoadSound("Audio/GameStart.mp3");
    playerWinsSound = LoadSound("Audio/PlayerWins.mp3");
}

Game::~Game() {
    UnloadSound(ballHit);
    UnloadSound(cpuScores);
    UnloadSound(cpuWinsSound);
    UnloadSound(playerScores);
    UnloadSound(gameStart);
    UnloadSound(playerWinsSound);
    CloseAudioDevice();
}

void Game::run() {
    while (!WindowShouldClose()) {
        update();
        draw();
    }
}

void Game::update() {
    if (currentState == GAME && gameStarted && !gameOver && IsKeyPressed(KEY_ESCAPE)) {
        paused = !paused;
        if (paused) pauseSelected = 0;
    }

    Vector2 mousePoint = GetMousePosition();

    switch (currentState) {
        case MENU: {
            const char* options[] = {"Start Game", "Settings", "Exit"};
            for (int i = 0; i < 3; i++) {
                float optionX = (float)screenWidth / 2 - (float)MeasureText(options[i], 40) / 2;
                float optionY = (float)screenHeight / 2 - 20 + i * 60;
                Rectangle optionRect = {
                    optionX - 20,
                    optionY - 10,
                    (float)MeasureText(options[i], 40) + 40,
                    60
                };
                if (CheckCollisionPointRec(mousePoint, optionRect)) {
                    menuSelected = i;
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        if (i == 0) currentState = MODE_SELECT;
                        else if (i == 1) currentState = SETTINGS;
                        else exit(0);
                    }
                }
            }

            if (IsKeyPressed(KEY_UP)) menuSelected = (menuSelected + 2) % 3;
            if (IsKeyPressed(KEY_DOWN)) menuSelected = (menuSelected + 1) % 3;
            if (IsKeyPressed(KEY_ENTER)) {
                if (menuSelected == 0) currentState = MODE_SELECT;
                else if (menuSelected == 1) currentState = SETTINGS;
                else exit(0);
            }
            if (startAlpha < 1.0f) startAlpha = min(1.0f, startAlpha + 0.03f);
            break;
        }
        case MODE_SELECT: {
            const char* modes[] = {"Classic", "Time Trial"};
            for (int i = 0; i < 2; i++) {
                float modeX = (float)screenWidth / 2 - (float)MeasureText(modes[i], 40) / 2;
                float modeY = (float)screenHeight / 2 - 20 + i * 60;
                Rectangle modeRect = {
                    modeX - 20,
                    modeY - 10,
                    (float)MeasureText(modes[i], 40) + 40,
                    60
                };
                if (CheckCollisionPointRec(mousePoint, modeRect)) {
                    modeSelected = i;
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        gameStarted = true;
                        currentState = GAME;
                        timeTrialMode = (i == 1);
                        resetGame();
                        PlaySound(gameStart);
                    }
                }
            }

            if (IsKeyPressed(KEY_UP)) modeSelected = (modeSelected + 1) % 2;
            if (IsKeyPressed(KEY_DOWN)) modeSelected = (modeSelected + 1) % 2;
            if (IsKeyPressed(KEY_ENTER)) {
                gameStarted = true;
                currentState = GAME;
                timeTrialMode = (modeSelected == 1);
                resetGame();
                PlaySound(gameStart);
            }
            if (IsKeyPressed(KEY_ESCAPE)) currentState = MENU;
            break;

        }
        case GAME:
            if (!gameOver) {
                if (!paused) {
                    int scoreResult = ball.update(ballSpeedMultiplier);
                    switch (scoreResult) {
                        case 1:
                            cpuScore++;
                            cpuAnimation = 15;
                            PlaySound(cpuScores);
                            if (cpuScore >= 7) {
                                gameOver = true;
                                winner = "CPU";
                                PlaySound(cpuWinsSound);
                            }
                            ball.resetBall();
                            break;
                        case 2:
                            playerScore++;
                            playerAnimation = 15;
                            PlaySound(playerScores);
                            if (playerScore >= 7) {
                                gameOver = true;
                                winner = "Player";
                                PlaySound(playerWinsSound);
                            }
                            ball.resetBall();
                            break;
                    }

                    player.update();
                    cpu.update((int)ball.y);

                    float dt = GetFrameTime();
                    timePassed += dt;

                    // Spawn ball trail particles
                    ballTrailTimer += dt;
                    if (ballTrailTimer >= 0.035f) {  // Spawn particles every 0.035 seconds
                        // Mix of red and blue particles
                        Color trailColor = (GetRandomValue(0, 1) == 0) ? RED : BLUE;
                        spawnParticles(ball.x, ball.y, trailColor);
                        ballTrailTimer = 0.0f;
                    }

                    if (!timeTrialMode && timePassed - finalUpdateTime >= 30.0f) {
                        finalUpdateTime = timePassed;
                        ballSpeedMultiplier += speedIncrement;
                        if (ballSpeedMultiplier > maxBallSpeedMultiplier) ballSpeedMultiplier = maxBallSpeedMultiplier;
                    }

                    if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius, Rectangle{player.x, player.y, player.width, player.height})) {
                        ball.speedX *= -1;
                        player.flickerTimer = 10;
                        spawnParticles(ball.x, ball.y, RED);
                        PlaySound(ballHit);
                    }
                    if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius, Rectangle{cpu.x, cpu.y, cpu.width, cpu.height})) {
                        ball.speedX *= -1;
                        cpu.flickerTimer = 10;
                        spawnParticles(ball.x, ball.y, BLUE);
                        PlaySound(ballHit);
                    }

                    updateParticles();

                    if (timeTrialMode) {
                        float remaining = timeTrialDuration - timePassed;
                        if (remaining < 0) remaining = 0;
                        int remSec = (int)ceil(remaining);

                        if (remSec <= 10) {
                            if (remSec != lastCountdownSecond) {
                                timerAnimation = 18;
                                lastCountdownSecond = remSec;
                            }
                        }

                        if (timePassed >= timeTrialDuration) {
                            gameOver = true;
                            if (playerScore > cpuScore) {
                                winner = "Player";
                                PlaySound(playerWinsSound);
                            } else if (cpuScore > playerScore) {
                                winner = "CPU";
                                PlaySound(cpuWinsSound);
                            } else {
                                winner = "TIE";
                                PlaySound(cpuWinsSound);
                            }
                        }
                    }
                    if (timerAnimation > 0) timerAnimation--;
                } else {
                    const char* pauseOptions[3] = {"Resume", "To Menu", "Exit"};
                    for (int i = 0; i < 3; i++) {
                        float pauseX = (float)screenWidth / 2 - (float)MeasureText(pauseOptions[i], 50) / 2;
                        float pauseY = (float)screenHeight / 2 + i * 60 - 40;
                        Rectangle pauseRect = {
                            pauseX - 20,
                            pauseY - 10,
                            (float)MeasureText(pauseOptions[i], 50) + 40,
                            70
                        };
                        if (CheckCollisionPointRec(mousePoint, pauseRect)) {
                            pauseSelected = i;
                            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                                if (i == 0) paused = false;
                                else if (i == 1) {
                                    paused = false;
                                    gameStarted = false;
                                    currentState = MENU;
                                    resetGame();
                                } else exit(0);
                            }
                        }
                    }

                    if (IsKeyPressed(KEY_UP)) pauseSelected = (pauseSelected + 2) % 3;
                    if (IsKeyPressed(KEY_DOWN)) pauseSelected = (pauseSelected + 1) % 3;
                    if (IsKeyPressed(KEY_ENTER)) {
                        if (pauseSelected == 0) paused = false;
                        else if (pauseSelected == 1) {
                            paused = false;
                            gameStarted = false;
                            currentState = MENU;
                            resetGame();
                        } else exit(0);
                    }
                }
            } else {
                if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    resetGame();
                    gameStarted = false;
                    currentState = MENU;
                }
            }
            break;
        case SETTINGS:
            if (IsKeyPressed(KEY_ESCAPE)) currentState = MENU;

            int barWidth = 400;
            int barX = GetScreenWidth() / 2 - barWidth / 2;
            int barY = 250;
            Rectangle volumeBar = {(float)barX, (float)barY, (float)barWidth, 30};

            if (CheckCollisionPointRec(mousePoint, volumeBar) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                masterVolume = (mousePoint.x - barX) / barWidth;
                if (masterVolume < 0.0f) masterVolume = 0.0f;
                if (masterVolume > 1.0f) masterVolume = 1.0f;
                SetMasterVolume(masterVolume);
            }

            const char* backText = "Back";
            float backX = (float)screenWidth / 2 - (float)MeasureText(backText, 40) / 2;
            float backY = (float)screenHeight - 100;
            Rectangle backRect = { backX - 20, backY - 10, (float)MeasureText(backText, 40) + 40, 60 };
            if (CheckCollisionPointRec(mousePoint, backRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentState = MENU;
            }
            if (IsKeyPressed(KEY_ENTER)) {
                currentState = MENU;
            }
            break;
    }

    if (playerAnimation > 0) playerAnimation--;
    if (cpuAnimation > 0) cpuAnimation--;

    if (currentState == MENU || currentState == MODE_SELECT) {
        if (startAlpha < 1.0f) startAlpha = min(1.0f, startAlpha + 0.03f);
    } else if (gameOver) {
        if (winAlpha < 1.0f) winAlpha = min(1.0f, winAlpha + 0.03f);
    } else {
        winAlpha = 0.0f;
    }
}

void Game::draw() {
    BeginDrawing();
    ClearBackground(BLACK);

    Vector2 mousePoint = GetMousePosition();

    if (currentState == MENU) {
        Color fadeWhite = Fade(WHITE, startAlpha);
        Color fadeBlue = Fade(BLUE, startAlpha);
        DrawText("PONG", screenWidth / 2 - MeasureText("PONG", 80) / 2, 80, 80, fadeBlue);

        const char* options[] = {"Start Game", "Settings", "Exit"};
        for (int i = 0; i < 3; i++) {
            Rectangle optionRect = {
                (float)screenWidth / 2 - (float)MeasureText(options[i], 40) / 2,
                (float)screenHeight / 2 - 20 + i * 60,
                (float)MeasureText(options[i], 40),
                40
            };
            bool isSelected = (menuSelected == i);
            bool isHovered = CheckCollisionPointRec(mousePoint, optionRect);
            DrawText(options[i], optionRect.x, optionRect.y, 40, isSelected || isHovered ? RED : fadeWhite);
        }
    } else if (currentState == MODE_SELECT) {
        DrawText("SELECT GAME MODE", screenWidth / 2 - MeasureText("SELECT GAME MODE", 50) / 2, 100, 50, WHITE);
        const char* modes[] = {"Classic", "Time Trial"};
        for (int i = 0; i < 2; i++) {
            Rectangle modeRect = {
                (float)screenWidth / 2 - (float)MeasureText(modes[i], 40) / 2,
                (float)screenHeight / 2 - 20 + i * 60,
                (float)MeasureText(modes[i], 40),
                40
            };
            bool isSelected = (modeSelected == i);
            bool isHovered = CheckCollisionPointRec(mousePoint, modeRect);
            DrawText(modes[i], modeRect.x, modeRect.y, 40, isSelected || isHovered ? RED : WHITE);
        }
    } else if (currentState == GAME) {
        DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);
        for (double r = 148; r <= 152; r += 0.2)
            DrawCircleLines(screenWidth / 2, screenHeight / 2, r, WHITE);

        ball.draw();
        cpu.draw();
        player.draw();
        drawParticles();

        if (timeTrialMode) {
            float remaining = timeTrialDuration - timePassed;
            if (remaining < 0) remaining = 0;
            int remSec = (int)ceil(remaining);

            if (remSec <= 10) {
                float t = (float)timerAnimation / (float)TIMER_ANIM_FRAMES;
                if (t < 0) t = 0;
                int baseFont = 40;
                int extra = (int)round(20.0f * t);
                int fontSize = baseFont + extra;
                string s = TextFormat("%i", remSec);
                DrawText(s.c_str(), screenWidth / 2 - MeasureText(s.c_str(), fontSize) / 2, 20, fontSize, RED);
            } else {
                int minutes = (int)(remaining / 60);
                int seconds = (int)ceil(remaining) % 60;
                string timer = TextFormat("%02i:%02i", minutes, seconds);
                DrawText(timer.c_str(), screenWidth / 2 - MeasureText(timer.c_str(), 40) / 2, 20, 40, WHITE);
            }
        } else {
            int minutes = (int)(timePassed / 60);
            int seconds = (int)ceil(timePassed) % 60;
            string timer = TextFormat("%02i:%02i", minutes, seconds);
            DrawText(timer.c_str(), screenWidth / 2 - MeasureText(timer.c_str(), 40) / 2, 20, 40, WHITE);
        }

        int cpuFontSize = (cpuAnimation > 0) ? 120 : 80;
        Color cpuColor = (cpuAnimation > 0) ? BLUE : WHITE;
        DrawText(TextFormat("%i", cpuScore), screenWidth / 4 - 20, 20, cpuFontSize, cpuColor);

        int playerFontSize = (playerAnimation > 0) ? 120 : 80;
        Color playerColor = (playerAnimation > 0) ? RED : WHITE;
        DrawText(TextFormat("%i", playerScore), 3 * screenWidth / 4 - 20, 20, playerFontSize, playerColor);

        if (paused) {
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.6f));
            const char* pauseOptions[3] = {"Resume", "To Menu", "Exit"};
            for (int i = 0; i < 3; i++) {
                Rectangle pauseRect = {
                    (float)screenWidth / 2 - (float)MeasureText(pauseOptions[i], 50) / 2,
                    (float)screenHeight / 2 + i * 60 - 40,
                    (float)MeasureText(pauseOptions[i], 50),
                    50
                };
                bool isSelected = (pauseSelected == i);
                bool isHovered = CheckCollisionPointRec(mousePoint, pauseRect);
                DrawText(pauseOptions[i], pauseRect.x, pauseRect.y, 50, isSelected || isHovered ? RED : WHITE);
            }
        }

        if (gameOver) {
            Color fadeGreen = Fade(GREEN, winAlpha), fadeWhite = Fade(WHITE, winAlpha);
            if (winner == "TIE") {
                DrawText("DRAW!", screenWidth / 2 - MeasureText("DRAW!", 80) / 2, screenHeight / 2 - 40, 80, RED);
            } else {
                DrawText(TextFormat("%s WON!", winner.c_str()), screenWidth / 2 - 200, screenHeight / 2 - 40, 80, fadeGreen);
            }
            DrawText("Press SPACE to restart", screenWidth / 2 - 200, screenHeight / 2 + 60, 40, fadeWhite);
        }
    } else if (currentState == SETTINGS) {
        DrawText("---SETTINGS---", GetScreenWidth() / 2 - MeasureText("---SETTINGS---", 50) / 2, 100, 50, WHITE);

        int barWidth = 400, barHeight = 30;
        int barX = GetScreenWidth() / 2 - barWidth / 2;
        int barY = 250;
        DrawRectangle(barX, barY, barWidth, barHeight, GRAY);
        DrawRectangle(barX, barY, (int)(barWidth * masterVolume), barHeight, RED);

        DrawText(TextFormat("Volume: %i%%", (int)(masterVolume * 100)), screenWidth / 2 - 200, 200, 40, GREEN);
        
        const char* backText = "Back";
        Rectangle backRect = { (float)screenWidth / 2 - (float)MeasureText(backText, 40) / 2, (float)screenHeight - 100, (float)MeasureText(backText, 40), 40 };
        bool isHovered = CheckCollisionPointRec(mousePoint, backRect);
        DrawText(backText, backRect.x, backRect.y, 40, isHovered ? RED : WHITE);
    }
    EndDrawing();
}

void Game::resetGame() {
    timePassed = 0.0f;
    ballSpeedMultiplier = 1.0f;
    playerScore = 0;
    cpuScore = 0;
    gameOver = false;
    paused = false;
    winner = "";
    startAlpha = 0.0f;
    finalUpdateTime = 0.0f;
    timerAnimation = 0;
    lastCountdownSecond = -1;
    ballTrailTimer = 0.0f;
    ball.resetBall();
}
