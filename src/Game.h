#ifndef GAME_H
#define GAME_H

#include "Ball.h"
#include "Paddle.h"
#include <string>
#include <raylib.h>

enum GameState { MENU, MODE_SELECT, GAME, SETTINGS };

class Game {
public:
    Game(int screenWidth, int screenHeight);
    ~Game();

    void run();

private:
    void update();
    void draw();
    void resetGame();

    int screenWidth, screenHeight;
    Ball ball;
    Paddle player;
    CpuPaddle cpu;

    GameState currentState;
    int playerScore, playerAnimation, cpuScore, cpuAnimation;
    int timerAnimation, lastCountdownSecond;
    bool gameStarted, gameOver, paused;
    std::string winner;
    float startAlpha, winAlpha;
    float timePassed, ballSpeedMultiplier, finalUpdateTime, timeTrialDuration;
    const float speedIncrement, maxBallSpeedMultiplier;
    const int TIMER_ANIM_FRAMES;

    Sound ballHit, cpuScores, cpuWinsSound, playerScores, gameStart, playerWinsSound;

    float masterVolume;
    float ballTrailTimer;
    int selectedOption, menuSelected, pauseSelected, modeSelected;
    bool timeTrialMode;
};

#endif // GAME_H
