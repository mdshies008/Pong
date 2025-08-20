#include <vector>
#include <iostream>
#include <raylib.h>
#include <cmath>

using namespace std;

struct Particle 
{
    float x, y; 
    float vx, vy;
    int life;
    Color color;
};

std::vector<Particle> particles;

// Particle Stuff
void SpawnParticles(float x, float y, Color color){
    for (int i = 0; i < 15; i++)
    {
        float angle = GetRandomValue(0, 360) * PI / 180.0f;
        float speed = GetRandomValue(3, 7);
        Particle p;
        p.x = x;
        p.y = y;
        p.vx = cosf(angle) * speed;
        p.vy = sinf(angle) * speed;
        p.life = GetRandomValue(15, 30);
        p.color = color;
        particles.push_back(p);
    }
    
}
void SpawnBallTrail(float x, float y, int radius) {
    for (int i = 0; i < 5; i++) {  
        float angle = GetRandomValue(0, 360) * PI / 180.0f;
        float r = GetRandomValue(0, radius);
        Particle p;
        p.x = x + cosf(angle) * r;
        p.y = y + sinf(angle) * r;
        p.vx = GetRandomValue(-10, 10) / 50.0f;
        p.vy = GetRandomValue(-10, 10) / 50.0f;
        p.life = GetRandomValue(8, 15);
        
        // Randomly choose color
        if (GetRandomValue(0, 1) == 0)
            p.color = RED;
        else
            p.color = BLUE;

        particles.push_back(p);
    }
}
void UpdateParticles() {
    for (size_t i = 0; i < particles.size(); )
    {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].life--;
        if (particles[i].life <= 0)
        {
            particles.erase(particles.begin() + i);
        } else
        {
            i++;
        }        
    }
}
void DrawParticles() {
    for (const auto& p : particles)
    {
        DrawCircle(p.x, p.y, 4, Fade(p.color, p.life / 30.0f));
    }
    
}

int player_score = 0; int player_animation = 0;
int cpu_score = 0; int cpu_animation = 0;
bool game_started = false; bool game_over = false; bool paused = false;
string winner = "";
float start_alpha = 0.0f, win_alpha = 0.0f;
Sound BallHit, Cpu_Scores, Cpu_Wins, Player_Scores, Game_Start, Player_Wins;

class Ball{
    public:
        float x, y;
        int speed_x, speed_y;
        int radius;

    void draw(){
        DrawCircle(x, y, radius, WHITE);
    }

    void update(){
        if (game_over) return;
        
        x += speed_x;
        y += speed_y;

        if (y + radius >= GetScreenHeight() || y - radius <= 0)
        {
            speed_y *= -1;
        }
        if (x + radius >= GetScreenWidth())
        {
            cpu_score++;
            cpu_animation = 15;
            PlaySound(Cpu_Scores);
            if (cpu_score >= 25)
            {
                game_over = true;
                winner = "CPU";
                PlaySound(Cpu_Wins);
            }
            ResetBall();
        }
        if (x - radius <= 0)
        {
            player_score++;
            player_animation = 15;
            PlaySound(Player_Scores);
            if (player_score >= 25)
            {
                game_over = true;
                winner = "Player";
                PlaySound(Player_Wins);
            }
            ResetBall();
        }        
    }

    void ResetBall(){
        x = GetScreenWidth()/2;
        y = GetScreenHeight()/2;

        int speed_choices[2] = {-1, 1};
        speed_x = abs(speed_x) * speed_choices[GetRandomValue(0,1)];
        speed_y = abs(speed_y) * speed_choices[GetRandomValue(0,1)];
    }
};

class Paddle {
    protected:
    void LimitMovment(){
        if (y <= 0)
        {
            y = 0;
        }
        if (y + height >= GetScreenHeight())
        {
            y = GetScreenHeight() - height;
        }
    }

    public:
        float x, y, width, height;
        int speed; int flicker_timer = 0;

    void Draw(){
        Color paddle_color = (flicker_timer > 0) ? RED : WHITE;
        DrawRectangle(x,y, width, height, paddle_color);
    }

    void Update(){
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        {
            y -= speed;
        }
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        {
            y += speed;
        }

        LimitMovment();
        if (flicker_timer > 0) flicker_timer--;
    }

};

class CpuPaddle: public Paddle{

    public:
    void Update(int ball_y){
        if (y + height/2 > ball_y)
        {
            y = y - speed;
        }
        if (y + height/2 <= ball_y)
        {
            y = y + speed;
        }
        LimitMovment();
        if (flicker_timer > 0) flicker_timer--;
    }

};

Ball ball;
Paddle player;
CpuPaddle cpu;

int main() 
{
    cout << "Pong" << endl;

    const int screen_height = 800;
    const int screen_width = 1280;

    ball.x = screen_width/2; ball.y = screen_height/2; ball.radius = 20; ball.speed_x = 7; ball.speed_y = 7;

    player.width = 25; player.height = 120;
    player.x = screen_width - player.width - 50; player.y = screen_height/2 - player.height/2;
    player.speed = 6;

    cpu.width = 25; cpu.height = 120;
    cpu.x = 50; cpu.y = screen_height/2 - cpu.height/2;
    cpu.speed = 6;

    
    InitWindow(screen_width, screen_height, "Pong");
    SetExitKey(0);
    SetTargetFPS(60);

    // Load Sound
    InitAudioDevice();
    BallHit    = LoadSound("Audio/BallHit.mp3");
    Cpu_Scores   = LoadSound("Audio/CpuScoring.wav");
    Cpu_Wins     = LoadSound("Audio/CpuWINS.wav");
    Player_Scores= LoadSound("Audio/PlayerScores.wav");
    Game_Start  = LoadSound("Audio/GameStart.mp3");
    Player_Wins  = LoadSound("Audio/PlayerWins.mp3");

    while(!WindowShouldClose()){
        //Pause/Resume 
        if (game_started && !game_over && (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE))) {
            paused = !paused;
        }

        //Update
        if (!game_started)
        {
            if (IsKeyPressed(KEY_SPACE))
            {
                game_started = true;
                PlaySound(Game_Start);
            }
            
        }
        else if (!game_over)
        {
            if (!paused)
            {
                ball.update();
                SpawnBallTrail(ball.x, ball.y, ball.radius);
                player.Update();
                cpu.Update(ball.y);

                //Collison Check
                if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius, Rectangle{player.x, player.y, player.width, player.height}))
                {
                    ball.speed_x *= -1;
                    player.flicker_timer = 10;
                    SpawnParticles(ball.x, ball.y, RED);
                    PlaySound(BallHit);
                }
                if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius, Rectangle{cpu.x, cpu.y, cpu.width, cpu.height}))
                {   
                    ball.speed_x *= -1;
                    cpu.flicker_timer = 10;
                    SpawnParticles(ball.x, ball.y, BLUE);
                    PlaySound(BallHit);
                }
                UpdateParticles();
            }
        }
        else
        {
            if (IsKeyPressed(KEY_SPACE))
            {
                player_score = 0; cpu_score = 0; game_over = false; winner = ""; ball.ResetBall();
                game_started = false;
                start_alpha = 0.0f;
            }
        }
        // Scoreboard Animation#1
        if (player_animation > 0) player_animation--;
        if (cpu_animation > 0) cpu_animation--;

        // Animate start and win screens
        if (!game_started)
        {
            if (start_alpha < 1.0f) start_alpha = min(1.0f, start_alpha + 0.03f);
        }
        else if (game_over)
        {
            if (win_alpha < 1.0f) win_alpha = min(1.0f, win_alpha + 0.03f);
        }
        else
        {
            win_alpha = 0.0f;
        }

        //Drawing
        ClearBackground(BLACK);
        BeginDrawing();
        if (!game_started)
        {
            Color fade_white = Fade(WHITE, start_alpha);
            Color fade_blue = Fade(BLUE, start_alpha);

            DrawText("PONG", screen_width/2 - MeasureText("PONG", 80)/2, 80, 80, fade_blue);
            DrawText("Press SPACE to Start", screen_width/2 - MeasureText("Press SPACE to Start", 40)/2, screen_height/2 - 20, 40, fade_white);
        }
        else{
            DrawLine(screen_width/2, 0, screen_width/2, screen_height, WHITE);
            for (double r = 148; r <= 152; r += 0.2) 
            {
                DrawCircleLines(screen_width/2, screen_height/2, r, WHITE);
            }
            ball.draw();
            cpu.Draw();
            player.Draw();
            DrawParticles();

            // Scoreboard Animation#2
            int cpu_font_size = (cpu_animation > 0) ? 120 : 80;
            Color cpu_color = (cpu_animation > 0) ? BLUE : WHITE;
            DrawText(TextFormat("%i", cpu_score), screen_width/4 - 20, 20, cpu_font_size, cpu_color);

            int player_font_size = (player_animation > 0) ? 120 : 80;
            Color player_color = (player_animation > 0) ? RED : WHITE;
            DrawText(TextFormat("%i", player_score), 3 * screen_width/4 - 20, 20, player_font_size, player_color);

            if (game_over)
            {
                Color fade_green = Fade(GREEN, win_alpha);
                Color fade_white = Fade(WHITE, win_alpha);

                DrawText(TextFormat("%s WON!", winner.c_str()), screen_width/2 - 200, screen_height/2 - 40, 80, fade_green);
                DrawText("Press SPACE to restart", screen_width/2 - 200, screen_height/2 + 60, 40, fade_white);
            }
            else
            {
                DrawText("First to 25 wins!", screen_width/2 - MeasureText("First to 25 wins!", 40)/2, screen_height - 60, 40, RED);
                if (paused)
                {
                    DrawText("PAUSED", screen_width/2 - MeasureText("PAUSED", 80)/2, screen_height/2 - 40, 80, BLUE);
                }
            }

        }
        EndDrawing();
    }

    // Memory Clean-up
    UnloadSound(BallHit);
    UnloadSound(Cpu_Scores);
    UnloadSound(Cpu_Wins);
    UnloadSound(Player_Scores);
    UnloadSound(Game_Start);
    UnloadSound(Player_Wins);
    CloseAudioDevice();

    CloseWindow();
    return 0;
}