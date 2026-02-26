#ifndef DRAW_SHAPE_H_
#define DRAW_SHAPE_H_
#include "commons.h"
#include "globals.h"
#include "utilities.h"

void drawSmoothRectagle(SDL_FRect rect, int r, int g, int b, int a, float radius);
SDL_Surface* transformToCircle(SDL_Surface* surf);
void drawRectangle(SDL_FRect* rect, int r, int g, int b, int a, bool border);
void drawCircle(int x, int y, float radius, SDL_Color color);
SDL_Surface* setSurfGrayScale(SDL_Surface* surface);
void drawLogo(int x, int y, float size);

#endif
