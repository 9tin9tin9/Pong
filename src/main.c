#include <stdio.h>
#include <raylib.h>
#include "game.h"
#include "ui.h"

#define draw(...) \
    do { \
        BeginDrawing(); \
        ClearBackground(BLACK); \
        __VA_ARGS__ \
        EndDrawing(); \
    } while (0)

int main() {
    const int screenWidth = 600;
    const int screenHeight = 400;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Pong");
    InitAudioDevice();
    AttachAudioMixedProcessor(processHitSound);

    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    UI ui = UI_init(screenWidth, screenHeight);
    Game game = { .init = false };
    while (!WindowShouldClose()) {
        int w = GetScreenWidth();
        int h = GetScreenHeight();

        if (ui.screen == SCREEN_GAME) {
            Game_update(&game);
            draw({
                Game_render(&game, GAME_RENDER_ALL, w, h);
            });
            ui.screen =
                (game.players[0]->score == 11 || game.players[1]->score == 11) ?
                    SCREEN_END :
                    SCREEN_GAME;
        } else {
            UI_update(&ui, &game, w, h);
            draw({
                UI_render(&ui, &game, w, h);
            });
        }
    }

    Game_del(&game);
    UI_del(&ui);

    DetachAudioMixedProcessor(processHitSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
