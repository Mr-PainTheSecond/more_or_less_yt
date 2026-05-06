#ifndef FONT_H_
#define FONT_H_

#include "commons.h"
#include "globals.h"
#include "utilities.h"
#include "draw_shape.h"

void createFontArray();
char* format(char* original, const char* suffix);
void freeFontArray();
TTF_Font* createFont(char* file_name, float size);
TTF_Font* copyFont(TTF_Font* oldFont);
void renderRotatedText(const char* txt, double angle, SDL_Color color, SDL_FRect dstRect, float size,
	int x, int y);
DynamicText* createDynamicText(const char* str, TTF_Font* font);
void destroyDynamicText(DynamicText* dmTxt);
void get_text_center(SDL_FRect rect, TTF_Text* text, int* x, int* y);

void displayText(SDL_FRect rect, TTF_Text* txt, int* x, int* y);
TTF_Text** wrapText(TTF_Text* msg, float containerW, int* count);
void zoomOutTxt(TTF_Font* oldFont, float oldX, float newX);

void displayTextAsSurface(ProjectedObject obj, TTF_Text* txt);
void expandFontArray();
#endif