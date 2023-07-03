#pragma once

#include "game.h"

typedef struct {
    const char* text;
    Vector2 pos;
    float fontSize;
} Text;

typedef struct Button {
    Text text;
    enum ButtonState {
        BUTTONSTATE_INACTIVE,
        BUTTONSTATE_ACTIVE,
        BUTTONSTATE_PRESSING,
    } state;
    Rectangle frame;
    void (*callback)(struct Button*, void*);
} Button;

typedef struct {
    enum Screen {
        SCREEN_TITLE,
        SCREEN_GAME,
        SCREEN_END,
    } screen;
    Text pongText;
    Button onePlayerButton;
    Button twoPlayerButton;

    Text playerOneWinText;
    Text playerTwoWinText;
    Button playAgainButton;
    Button backToMenuButton;
} UI;

UI UI_init(int w, int h);
void UI_del(UI* ui);
void UI_update(UI* ui, Game* game, int w, int h);
void UI_render(UI* ui, Game* game, int w, int h);
