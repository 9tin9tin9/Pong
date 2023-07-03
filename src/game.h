#pragma once

#include <stdlib.h>
#include <raylib.h>

// Coordinates 0 to 1
// Scale with screen size

typedef struct {
    bool isCpu;
    size_t score;
    float y;
} Player;

typedef struct {
    Player player;
    float chanceOffset;
} Cpu;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    Sound hitsound;
} Ball;

typedef struct {
    bool init;
    bool firstHit;
    bool ended;
    Ball ball;
    Player* players[2];
} Game;

typedef struct {
    bool boards;
    bool scores;
    bool ball;
    bool net;
} GameRenderComponents;

static const GameRenderComponents GAME_RENDER_ALL = {
    .boards = true,
    .scores = true, 
    .ball = true,
    .net = true,
};

enum Mode : bool {
    ONE_PLAYER,
    TWO_PLAYERS
};

Game Game_init(enum Mode players_n);
void Game_del(Game* game); // Doesn't free the game pointer
void Game_update(Game* game);
void Game_render(Game* state, GameRenderComponents components, int w, int h);
void processHitSound(void* buffer, unsigned int frames);
