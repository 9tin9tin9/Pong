#include "game.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

const float p0x = 0.1;
const float p1x = 0.9;
const float pdy = 0.015;
const float boardHalfWidth = 0.01;
const float boardHeight = 0.15;
const float ballWidth = boardHalfWidth * 2;
const float ballSpeedSlow = 0.007;
const float ballSpeedNormal = ballSpeedSlow * 2;

// CPU difficulty
const float cpuChaseOffset = boardHeight / 4;
const float cpuSlowMovingDistance = 0.8;
const float cpuSlowMovingFactor = 0.5;

static float hitsoundPitchMultiplier = 1.f;

void Player_update(Player* player, int pn);
void Cpu_update(Cpu* player, Ball* ball);
void Player_render(Player* players[2], int w, int h);

void Ball_update(Ball* ball, Player* players[2], bool* firstHit);
void Ball_render(Ball* ball, int w, int h);

// net and scores and additional stuff if have
void renderNet(int w, int h);
void renderScores(Player* players[2], int w, int h);

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define isInRangeInclusive(x, l, u) ((x) >= (l) && (x) <= (u))
#define isRangeOverlap(x1, x2, y1, y2) ((x1) <= (y2) && (y1) <= (x2))

Game Game_init(enum Mode players_n) {
    Game game;
    game.init = true;
    game.firstHit = false;
    game.ended = false;
    game.ball = (Ball) {
        .pos = { .x = 0.5, .y = 0.5 },
        .vel = { .x = ballSpeedSlow, .y = 0 },
        .hitsound = LoadSound("assets/hitsound.mp3"),
    };
    if (players_n == ONE_PLAYER) {
        game.players[0] = malloc(sizeof(Cpu));
        *(Cpu*)game.players[0] = (Cpu){
            .player = (Player) {
                .isCpu = true,
                .score = 0,
                .y = 0.5,
            },
            .chanceOffset = NAN,
        };
    } else {
        game.players[0] = malloc(sizeof(Player));
        *game.players[0] = (Player) {
            .isCpu = false,
            .score = 0,
            .y = 0.5,
        };
    }
    game.players[1] = malloc(sizeof(Player));
    *game.players[1] = (Player) {
        .isCpu = false,
        .score = 0,
        .y = 0.5,
    };
    return game;
}

void Game_del(Game* game) {
    if (!game->init) {
        return;
    }
    UnloadSound(game->ball.hitsound);
    free(game->players[0]);
    free(game->players[1]);
}

void Game_update(Game* game) {
    if (game->players[0]->isCpu) {
        Cpu_update((Cpu*)game->players[0], &game->ball);
    } else {
        Player_update(game->players[0], 0);
    }
    Player_update(game->players[1], 1);
    Ball_update(&game->ball, game->players, &game->firstHit);

    if (game->players[0]->score == 11 || game->players[1]->score == 11) {
        game->ended = true;
    }
}

void Game_render(Game* state, GameRenderComponents components, int w, int h) {
    components.net ? renderNet(w, h) : 0;
    components.scores ? renderScores(state->players, w, h) : 0;
    components.ball ? Ball_render(&state->ball, w, h) : 0;
    components.boards ? Player_render(state->players, w, h) : 0;
}

void Cpu_update(Cpu* cpu, Ball* ball) {
    if (isnan(cpu->chanceOffset)) {
        float fac = 100000.f;
        cpu->chanceOffset = GetRandomValue(0, cpuChaseOffset * fac) / fac;
    }

    int ballDir = Vector2Angle(Vector2Zero(), ball->vel) > 0 ? 1 : -1;
    float movingFac = ball->pos.x > cpuSlowMovingDistance ?
        cpuSlowMovingFactor : 1;
    float ballGuessY = ball->pos.y + cpu->chanceOffset * ballDir;
    float dy = cpu->player.y < ballGuessY ?
        min(pdy * movingFac, (ballGuessY - cpu->player.y) * movingFac) :
        cpu->player.y > ballGuessY ?
            -min(pdy * movingFac, (cpu->player.y - ballGuessY) * movingFac) :
            0;
    cpu->player.y += dy;
    cpu->player.y = Clamp(cpu->player.y, boardHeight / 2, 1 - boardHeight / 2);
}

void Player_update(Player* player, int pn) {
    #define isUpKeyDown(pn) IsKeyDown((pn) ? KEY_UP : KEY_W)
    #define isDownKeyDown(pn) IsKeyDown((pn) ? KEY_DOWN : KEY_S)

    if (isUpKeyDown(pn)) {
        player->y -= pdy;
    } else if (isDownKeyDown(pn)) {
        player->y += pdy;
    }
    player->y = Clamp(player->y, boardHeight / 2, 1 - boardHeight / 2);

    #undef isUpKeyDown
    #undef isDownKeyDown
}

void Player_render(Player* player[2], int w, int h) {
    // center
    int player0x = p0x * w;
    int player0y = player[0]->y * h;
    int player1x = p1x * w;
    int player1y = player[1]->y * h;

    int boardW = boardHalfWidth * 2 * w;
    int boardH = boardHeight * h;

    DrawRectangle(
        player0x - boardW / 2, player0y - boardH / 2, boardW, boardH, WHITE);
    DrawRectangle(
        player1x - boardW / 2, player1y - boardH / 2, boardW, boardH, WHITE);
}

void Ball_resetVel(Ball* ball) {
    float angle = GetRandomValue(110, 135) / 180.f * 2 - 1;
    Vector2 v = { .x = 1, .y = angle };
    ball->vel = Vector2Scale(v, ballSpeedSlow);
}

void Ball_checkOutOfBounce(Ball* ball, Player* players[2]) {
    if (ball->pos.x > 1) {
        players[0]->score++;
        ball->pos = (Vector2){ .x = 0.5, .y = GetRandomValue(4, 6) / 10.f };
        Ball_resetVel(ball);
    } else if (ball->pos.x < 0) {
        players[1]->score++;
        ball->pos = (Vector2){ .x = 0.5, .y = GetRandomValue(4, 6) / 10.f };
        Ball_resetVel(ball);
        ball->vel.x = -ball->vel.x;
    }
}

void Ball_checkCollisionWithBoard(Ball* ball, Player* players[2], bool* firstHit) {
    #define xCollideWithP0(bx) \
        isRangeOverlap( \
            bx - ballWidth / 2, bx + ballWidth / 2, \
            p0x - boardHalfWidth / 2, p0x + boardHalfWidth)
    #define yCollideWithP0(by) \
        isRangeOverlap( \
            by - ballWidth / 2, by + ballWidth / 2, \
            players[0]->y - boardHeight / 2, players[0]->y + boardHeight / 2)
    #define collideWithP0(ball) \
        ball->vel.x < 0 && \
        xCollideWithP0(ball->pos.x) && yCollideWithP0(ball->pos.y)

    #define xCollideWithP1(bx) \
        isRangeOverlap( \
            bx - ballWidth / 2, bx + ballWidth / 2, \
            p1x - boardHalfWidth, p1x + boardHalfWidth / 2)
    #define yCollideWithP1(by) \
        isRangeOverlap( \
            by - ballWidth / 2, by + ballWidth / 2, \
            players[1]->y - boardHeight / 2, players[1]->y + boardHeight / 2)
    #define collideWithP1(ball) \
        ball->vel.x > 0 && \
        xCollideWithP1(ball->pos.x) && yCollideWithP1(ball->pos.y)

    if (collideWithP0(ball)) {
        float dis = (ball->pos.y - players[0]->y) / (boardHeight);
        Vector2 v = { .x = 1, .y = dis * 4 };
        ball->vel = Vector2Scale(v, ballSpeedNormal);
        if (players[0]->isCpu) {
            ((Cpu*)players[0])->chanceOffset = NAN;
        }

        float diff = Vector2Length(ball->vel) - ballSpeedNormal;
        hitsoundPitchMultiplier = exp(diff * 150);
        PlaySound(ball->hitsound);
    } else if (collideWithP1(ball)) {
        float dis = (ball->pos.y - players[1]->y) / (boardHeight);
        Vector2 v = { .x = -1, .y = dis * 4 };
        ball->vel = Vector2Scale(v, ballSpeedNormal);

        float diff = Vector2Length(ball->vel) - ballSpeedNormal;
        hitsoundPitchMultiplier = exp(diff * 150);
        PlaySound(ball->hitsound);
    }

    if (!IsSoundPlaying(ball->hitsound)) {
        hitsoundPitchMultiplier = 1.f;
    }

    #undef xCollideWithP0
    #undef yCollideWithP0
    #undef collideWithP0
    #undef xCollideWithP1
    #undef yCollideWithP1
    #undef collideWithP1
}

void Ball_checkCollisionWithWall(Ball* ball) {
    Vector2 future = Vector2Add(ball->pos, ball->vel);
    if (!isInRangeInclusive(future.y, 0, 1)) {
        ball->vel.y = -ball->vel.y;
    }
}

void Ball_update(Ball* ball, Player* players[2], bool* firstHit) {
    Ball_checkOutOfBounce(ball, players);
    Ball_checkCollisionWithBoard(ball, players, firstHit);
    Ball_checkCollisionWithWall(ball);
    ball->pos = Vector2Add(ball->pos, ball->vel);
}

void Ball_render(Ball* ball, int w, int h) {
    int x = ball->pos.x * w;
    int y = ball->pos.y * h;
    int width = ballWidth * w;
    DrawRectangle(x - width / 2, y - width / 2, width, width, WHITE);
}

void renderNet(int w, int h) {
    for (float y = 0; y < h; y += (ballWidth * 2) * w) {
        float x = (0.5 - ballWidth / 2) * w;
        int width = ballWidth * w;
        DrawRectangle(x, y, width, width, WHITE);
    }
}

void renderScores(Player* players[2], int w, int h) {
    float fontSize = 0.13 * h;
    {
        char text[20];
        sprintf(text, "%zu", players[0]->score);
        int textwidth = MeasureText(text, fontSize);
        DrawText(text, (0.5 - 0.1) * w - textwidth, 0.1 * h, fontSize, WHITE);
    }
    {
        char text[20];
        sprintf(text, "%zu", players[1]->score);
        DrawText(text, (0.5 + 0.1) * w, 0.1 * h, fontSize, WHITE);
    }
}

void processHitSound(void* buffer, unsigned int frames) {
    float* samples = buffer;
    for (unsigned int f = 0; f < frames; f++) {
        samples[f] = samples[f] * hitsoundPitchMultiplier;
    }
}
