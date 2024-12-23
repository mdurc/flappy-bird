#include "raylib.h"

typedef struct {
    Rectangle top;
    Rectangle bot;
    int v_x;
    int scored;
} Pipe;

typedef struct {
    Rectangle rect;
    float v_y;
    float a_y;
    int frame;
    float frameTime;
    float frameDelay;
    float bird_angle;
} Bird;

static int last_pipe = 2;
static int score = 0;
static int max_score = 0;
static int game_over = 0;
static float texture_scale = 0.5;

static int started_game = 0;

#define screen_width 400
#define screen_height 800
#define pipe_gap 150
#define pipe_length 350

Bird b;
Pipe p[3];
Texture2D background_texture;
Texture2D ground_texture;
Texture2D pipe_texture;
Texture2D bird_texture;
Texture2D intro_texture;
Texture2D best_texture;

void init_game();

void process_input() {
    if (IsKeyPressed(KEY_SPACE)) {
        b.v_y = -8.0f;
    }else if ((!started_game || game_over) && IsKeyPressed(KEY_ENTER)) {
        init_game();
        game_over = 0;
        if (!started_game) started_game = 1;
    }
}

void update() {
    if (!started_game || game_over) return;

    b.v_y += b.a_y;
    b.rect.y += b.v_y;

    b.frameTime += GetFrameTime();
    if (b.frameTime >= b.frameDelay) {
        b.frame = (b.frame + 1) % 3;
        b.frameTime = 0.0f;
    }

    // check collision
    int first_pipe = (last_pipe + 1)%3;
    if (CheckCollisionRecs(b.rect, p[first_pipe].top) || CheckCollisionRecs(b.rect, p[first_pipe].bot)){
        // game over
        if (score > max_score) max_score = score;
        game_over = 1;
        return;
    }
    if (!p[first_pipe].scored && b.rect.x >= p[first_pipe].bot.x + (p[first_pipe].bot.width/2)){
        score += 1;
        p[first_pipe].scored = 1;
    }

    if (b.rect.y + b.rect.height >= (screen_height - ground_texture.height * texture_scale)) {
        b.rect.y = screen_height - b.rect.height - ground_texture.height * texture_scale;
        b.v_y = 0.0f;
    } else if (b.rect.y <= 0) {
        b.rect.y = 1;
        b.v_y = 0.0f;
    }

    for (int i = 0; i < 3; ++i) {
        p[i].top.x += p[i].v_x;
        p[i].bot.x = p[i].top.x;

        if (p[i].top.x + p[i].top.width < 0) {
            p[i].top.x = p[last_pipe].top.x + 200;
            p[i].bot.x = p[i].top.x;

            p[i].top.height = GetRandomValue(pipe_gap, screen_height-(pipe_gap*2));
            p[i].bot.y = p[i].top.height + pipe_gap;
            p[i].bot.height = screen_height - p[i].bot.y;

            p[i].scored = 0;
            last_pipe = i;
        }
    }
}

void draw_text(const char* text, int x, int y, int font_size, Color text_color, Color border_color) {
    int border_offset = 2;

    DrawText(text, x - border_offset, y - border_offset, font_size, border_color);
    DrawText(text, x + border_offset, y - border_offset, font_size, border_color);
    DrawText(text, x - border_offset, y + border_offset, font_size, border_color);
    DrawText(text, x + border_offset, y + border_offset, font_size, border_color);

    DrawText(text, x - border_offset, y, font_size, border_color);
    DrawText(text, x + border_offset, y, font_size, border_color);
    DrawText(text, x, y - border_offset, font_size, border_color);
    DrawText(text, x, y + border_offset, font_size, border_color);

    DrawText(text, x, y, font_size, text_color);
}

void render() {
    BeginDrawing();

    ClearBackground(BLUE);

    //DrawTexture(background_texture, 0, 0, WHITE);
    // scale height but not width
    DrawTexturePro(
        background_texture,
        (Rectangle){0, 0, background_texture.width, background_texture.height},
        (Rectangle){0, 0, background_texture.width, screen_height},
        (Vector2){0, 0},
        0.0f,
        WHITE
    );

    for (int i = 0; i < 3; i++) {
        // top extend pipe
        DrawTexturePro(
                pipe_texture,
                (Rectangle){0, 0, pipe_texture.width, pipe_texture.height},
                (Rectangle){p[i].top.x, p[i].top.y, p[i].top.width, pipe_length},
                (Vector2){0, (pipe_length-p[i].top.height) + pipe_length},
                0.0f,
                WHITE
                );

        // top pipe bulb
        DrawTexturePro(
                pipe_texture,
                (Rectangle){0, 0, pipe_texture.width, -pipe_texture.height},
                (Rectangle){p[i].top.x, p[i].top.y, p[i].top.width, pipe_length},
                (Vector2){0, pipe_length - p[i].top.height},
                0.0f,
                WHITE
                );

        // bottom pipe bulb
        DrawTexturePro(
                pipe_texture,
                (Rectangle){0, 0, pipe_texture.width, pipe_texture.height},
                (Rectangle){p[i].bot.x, p[i].bot.y, p[i].bot.width, pipe_length},
                (Vector2){0, 0},
                0.0f,
                WHITE
                );
        
        // bottom extend pipe
        DrawTexturePro(
                pipe_texture,
                (Rectangle){0, 0, pipe_texture.width, -pipe_texture.height},
                (Rectangle){p[i].bot.x, p[i].bot.y, p[i].bot.width, pipe_length},
                (Vector2){0, -pipe_length},
                0.0f,
                WHITE
                );
    }

    // draw ground
    float ground_height = ground_texture.height * texture_scale;
    for (int x = 0; x < screen_width; x += ground_texture.width * texture_scale) {
        DrawTexturePro(
            ground_texture,
            (Rectangle){0, 0, ground_texture.width, ground_texture.height},
            (Rectangle){x, screen_height - ground_height, ground_texture.width * texture_scale, ground_height},
            (Vector2){0, 0},
            0.0f,
            WHITE
        );
    }

    // draw bird
    float target_angle = b.v_y * 3.0f;
    if (target_angle > 45.0f) target_angle = 45.0f;
    if (target_angle < -45.0f) target_angle = -45.0f;
    b.bird_angle += (target_angle - b.bird_angle) * 0.1f;

    DrawTexturePro(
        bird_texture,
        (Rectangle){b.frame * (bird_texture.width/3.0), 0, (bird_texture.width/3.0), bird_texture.height},
        b.rect,
        (Vector2){0,0},
        b.bird_angle,
        WHITE
    );

    if (started_game && !game_over){
        const char* score_text = TextFormat("%d", score);
        draw_text(score_text, (screen_width/2)-(MeasureText(score_text, 40)/2), 80, 40, WHITE, BLACK);
    }

    if (!started_game){
        float intro_scaler = 2.0;
        DrawTexturePro(
            intro_texture, (Rectangle){0, 0, intro_texture.width, intro_texture.height},
            (Rectangle){(screen_width / 2.0) - ((intro_texture.width * intro_scaler) / 2.0), 150,
                            intro_texture.width * intro_scaler, intro_texture.height * intro_scaler},
            (Vector2){0, 0}, 0.0f, WHITE);
        draw_text("PRESS [ENTER] TO START", (screen_width/2)-(MeasureText("PRESS [ENTER] TO START", 20)/2), (screen_height/2), 20, WHITE, BLACK);
    }else if (game_over) {
        float best_scaler = 0.6;
        DrawTexturePro(
            best_texture, (Rectangle){0, 0, best_texture.width, best_texture.height},
            (Rectangle){(screen_width / 2.0) - ((best_texture.width * best_scaler) / 2.0), 150,
                            best_texture.width * best_scaler, best_texture.height * best_scaler},
            (Vector2){0, 0}, 0.0f, WHITE);
        const char* max_text = TextFormat("%d", max_score);
        const char* score_text = TextFormat("%d", score);
        draw_text(score_text, (screen_width/2)-(MeasureText(score_text, 30)/2), 190, 30, WHITE, BLACK);
        draw_text(max_text, (screen_width/2)-(MeasureText(max_text, 35)/2), 242, 35, WHITE, BLACK);
        draw_text("PRESS [ENTER] TO PLAY AGAIN", (screen_width/2)-(MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2), (screen_height/2)-50, 20, WHITE, BLACK);
    }

    EndDrawing();
}

void init_game(){
    texture_scale = 0.5;
    b.rect = (Rectangle){50, 300, (bird_texture.width / 3.0) * texture_scale, bird_texture.height * texture_scale};
    b.v_y = 0.0f;
    b.a_y = 0.5f;
    b.frame = 0;
    b.frameTime = 0.0f;
    b.frameDelay = 0.1f;

    for (int i = 0; i < 3; ++i) {
        int height = GetRandomValue(pipe_gap, screen_height - (pipe_gap*2));
        p[i] = (Pipe) {
            .top = (Rectangle){screen_width + i * 200, 0, pipe_texture.width * texture_scale, height},
            .bot = (Rectangle){screen_width + i * 200, height + pipe_gap, pipe_texture.width * texture_scale, screen_height - (height+pipe_gap)},
            .v_x = -2,
            .scored = 0
        };
    }

    last_pipe = 2;
    score = 0;
}

int main() {
    //SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(60);
    InitWindow(screen_width, screen_height, "flappy");

    bird_texture = LoadTexture("imgs/bird.png");
    background_texture = LoadTexture("imgs/background.png");
    ground_texture = LoadTexture("imgs/ground.png");
    pipe_texture = LoadTexture("imgs/pipe.png");
    intro_texture = LoadTexture("imgs/logo.png");
    best_texture = LoadTexture("imgs/score.png");

    init_game();
    while (!WindowShouldClose()) {
        process_input();
        update();
        render();
    }

    UnloadTexture(bird_texture);
    UnloadTexture(background_texture);
    UnloadTexture(ground_texture);
    UnloadTexture(pipe_texture);
    UnloadTexture(intro_texture);
    UnloadTexture(best_texture);

    CloseWindow();
    return 0;
}
