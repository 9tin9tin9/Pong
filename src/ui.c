#include "ui.h"
#include "raylib.h"

typedef struct {
    UI* ui;
    Game* game;
} ButtonCallbackArgv;
void Button_chooseOnePlayerCallback(Button* button, ButtonCallbackArgv* argv);
void Button_chooseTwoPlayerCallback(Button* button, ButtonCallbackArgv* argv);
void Button_playAgainCallback(Button* button, ButtonCallbackArgv* argv);
void Button_backToMenuCallback(Button* button, ButtonCallbackArgv* argv);
void Button_getFrame(Button* button, int w, int h, Rectangle* frame);
void Button_update(Button* button, UI* ui, Game* game, int w, int h);
void Button_render(Button* button, int w, int h);

Sound* buttonSfx_press = NULL;
Sound* buttonSfx_release = NULL;

void Text_space(Text* text, int w, int h, float* textWidth, float* textHeight);
void Text_render(Text* text, int w, int h, Color color);

UI UI_init(int w, int h) {
    if (!buttonSfx_press) {
        buttonSfx_press = malloc(sizeof(*buttonSfx_press));
        *buttonSfx_press = LoadSound("assets/hitsound.mp3");
        SetSoundPitch(*buttonSfx_press, 0.7);
        SetSoundVolume(*buttonSfx_press, 0.7);
    }
    if (!buttonSfx_release) {
        buttonSfx_release = malloc(sizeof(*buttonSfx_release));
        *buttonSfx_release = LoadSound("assets/hitsound.mp3");
        SetSoundPitch(*buttonSfx_release, 0.6);
        SetSoundVolume(*buttonSfx_release, 0.5);
    }
    Text pongText = {
        .text = "PONG",
        .pos = (Vector2) { .x = 0.5, .y = 0.3 },
        .fontSize = 0.2,
    };

    Button onePlayerButton = {
        .text = (Text) {
            .text = "ONE PLAYER",
            .pos = (Vector2) { .x = 0.5, .y = 0.6 },
            .fontSize = 0.09,
        },
        .state = BUTTONSTATE_INACTIVE,
        .callback = (void(*)(Button*, void*))Button_chooseOnePlayerCallback,
    };
    Button_getFrame(&onePlayerButton, w, h, &onePlayerButton.frame);

    Button twoPlayerButton = {
        .text = (Text) {
            .text = "TWO PLAYERS",
            .pos = (Vector2) { .x = 0.5, .y = 0.8 },
            .fontSize = 0.09,
        },
        .state = BUTTONSTATE_INACTIVE,
        .callback = (void(*)(Button*, void*))Button_chooseTwoPlayerCallback,
    };
    Button_getFrame(&twoPlayerButton, w, h, &twoPlayerButton.frame);

    Text playerOneWinText = {
        .text = "P1 WINS",
        .pos = (Vector2) { .x = 0.5, .y = 0.3 },
        .fontSize = 0.11,
    };

    Text playerTwoWinText = {
        .text = "P2 WINS",
        .pos = (Vector2) { .x = 0.5, .y = 0.3 },
        .fontSize = 0.11,
    };

    Button playAgainButton = {
        .text = (Text) {
            .text = "PLAY AGAIN",
            .pos = (Vector2) { .x = 0.5, .y = 0.6 },
            .fontSize = 0.09,
        },
        .state = BUTTONSTATE_INACTIVE,
        .callback = (void(*)(Button*, void*))Button_playAgainCallback,
    };
    Button_getFrame(&playAgainButton, w, h, &playAgainButton.frame);

    Button backToMenuButton = {
        .text = (Text) {
            .text = "BACK TO MENU",
            .pos = (Vector2) { .x = 0.5, .y = 0.8 },
            .fontSize = 0.09,
        },
        .state = BUTTONSTATE_INACTIVE,
        .callback = (void(*)(Button*, void*))Button_backToMenuCallback,
    };
    Button_getFrame(&backToMenuButton, w, h, &backToMenuButton.frame);

    return (UI) {
        .screen = SCREEN_TITLE,
        .pongText = pongText,
        .onePlayerButton = onePlayerButton,
        .twoPlayerButton = twoPlayerButton,
        .playerOneWinText = playerOneWinText,
        .playerTwoWinText = playerTwoWinText,
        .playAgainButton = playAgainButton,
        .backToMenuButton = backToMenuButton,
    };
}

void UI_del(UI* ui) {
    UnloadSound(*buttonSfx_press);
    UnloadSound(*buttonSfx_release);
    free(buttonSfx_press);
    free(buttonSfx_release);
    buttonSfx_press = NULL;
    buttonSfx_release = NULL;
}

void UI_update(UI* ui, Game* game, int w, int h) {
    if (ui->screen == SCREEN_TITLE) {
        Button_update(&ui->onePlayerButton, ui, game, w, h);
        Button_update(&ui->twoPlayerButton, ui, game, w, h);
    } else if (ui->screen == SCREEN_END) {
        Button_update(&ui->playAgainButton, ui, game, w, h);
        Button_update(&ui->backToMenuButton, ui, game, w, h);
    }
}

void UI_render(UI* ui, Game* game, int w, int h) {
    if (ui->screen == SCREEN_TITLE) {
        Text_render(&ui->pongText, w, h, WHITE);
        Button_render(&ui->onePlayerButton, w, h);
        Button_render(&ui->twoPlayerButton, w, h);
    } else if (ui->screen == SCREEN_END) {
        Text* text = game->players[0]->score == 11 ?
            &ui->playerOneWinText :
            &ui->playerTwoWinText;
        Text_render(text, w, h, WHITE);
        Button_render(&ui->playAgainButton, w, h);
        Button_render(&ui->backToMenuButton, w, h);
    }
}

void Button_chooseOnePlayerCallback(Button* button, ButtonCallbackArgv* argv) {
    argv->ui->screen = SCREEN_GAME;
    *argv->game = Game_init(ONE_PLAYER);
}

void Button_chooseTwoPlayerCallback(Button* button, ButtonCallbackArgv* argv) {
    argv->ui->screen = SCREEN_GAME;
    *argv->game = Game_init(TWO_PLAYERS);
}

void Button_playAgainCallback(Button* button, ButtonCallbackArgv* argv) {
    argv->ui->screen = SCREEN_GAME;
    enum Mode mode = argv->game->players[0]->isCpu ? ONE_PLAYER : TWO_PLAYERS;
    Game_del(argv->game);
    *argv->game = Game_init(mode);
}

void Button_backToMenuCallback(Button* button, ButtonCallbackArgv* argv) {
    Game_del(argv->game);
    argv->ui->screen = SCREEN_TITLE;
}

void Button_update(Button* button, UI* ui, Game* game, int w, int h) {
    Button_getFrame(button, w, h, &button->frame);
    Vector2 mousePoint = GetMousePosition();
    if (CheckCollisionPointRec(mousePoint, button->frame)) {
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            button->state = BUTTONSTATE_INACTIVE;
                ButtonCallbackArgv argv = {
                    .ui = ui,
                    .game = game,
                };
            button->callback(button, &argv);
            if (buttonSfx_release) {
                PlaySound(*buttonSfx_release);
            }
        } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (button->state != BUTTONSTATE_PRESSING && buttonSfx_press) {
                PlaySound(*buttonSfx_press);
            }
            button->state = BUTTONSTATE_PRESSING;
        } else {
            button->state = BUTTONSTATE_ACTIVE;
        }
    } else {
        button->state = BUTTONSTATE_INACTIVE;
    }
}

void Button_getFrame(Button* button, int w, int h, Rectangle* frame) {
    float textWidth, textHeight;
    Text_space(&button->text, w, h, &textWidth, &textHeight);
    Vector2 pos = {
        .x = button->text.pos.x * w,
        .y = button->text.pos.y * h,
    };
    Vector2 frameDim = {
        .x = textWidth + 0.03 * 2 * w,
        .y = textHeight + 0.03 * 2 * h,
    };
    *frame = (Rectangle){
        .x = pos.x - frameDim.x / 2,
        .y = pos.y - frameDim.y / 2,
        .width = frameDim.x,
        .height = frameDim.y,
    };
}

void drawButtonFrame(
    Rectangle* rec,
    enum ButtonState state,
    int w,
    int h)
{
    DrawRectangleRec(*rec, WHITE);
    if (state != BUTTONSTATE_ACTIVE) {
        float borderWidth = 0.01;
        Rectangle inner = {
            .x = rec->x + borderWidth * w,
            .y = rec->y + borderWidth * h,
            .width = rec->width - borderWidth * 2 * w,
            .height = rec->height - borderWidth * 2 * h,
        };
        Color color = state == BUTTONSTATE_INACTIVE ?
            BLACK :
            state == BUTTONSTATE_ACTIVE ?
                WHITE :
                DARKGRAY;
        DrawRectangleRec(inner, color);
    }
}

void Button_render(Button* button, int w, int h) {
    drawButtonFrame(&button->frame, button->state, w, h);
    Color color = button->state == BUTTONSTATE_INACTIVE ?
        WHITE :
        button->state == BUTTONSTATE_ACTIVE ?
            BLACK :
            LIGHTGRAY;
    Text_render(&button->text, w, h, color);
}

void Text_space(Text* text, int w, int h, float* textWidth, float* textHeight) {
    float fontSize = text->fontSize * h;
    *textWidth = MeasureText(text->text, fontSize);
    *textHeight = fontSize;
}

void Text_render(Text* text, int w, int h, Color color) {
    float fontSize = text->fontSize * h;
    float textWidth, textHeight;
    Text_space(text, w, h, &textWidth, &textHeight);
    float x = text->pos.x * w - textWidth / 2.f;
    float y = text->pos.y * h - textHeight / 2.f;
    DrawText(text->text, x, y, fontSize, color);
}
