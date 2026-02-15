#ifndef DRAW_H_
#define DRAW_H_
#define TXT_COUNT 4

#include "commons.h"
#include "config.h"
#include "globals.h"
#include "draw_shape.h"
#include "draw_game.h"
#include "utilities.h"

int draw(TTF_Text* more, TTF_Text* less, Queue* queue);
float center(float pos, float size);
#endif