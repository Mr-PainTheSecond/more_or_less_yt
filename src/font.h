#ifndef FONT_H_
#define FONT_H_

#include "commons.h"
#include "globals.h"
#include "utilities.h"
void createFontArray();
char* format(char* original, const char* suffix);
void freeFontArray();
TTF_Font* createFont(char* file_name, float size);
void renderRotatedText(const char* txt, double angle, SDL_Color color, SDL_FRect dstRect, float size,
	int x, int y);
DynamicText* createDynamicText(const char* str, TTF_Font* font);
void destroyDynamicText(DynamicText* dmTxt);
void get_text_center(SDL_FRect rect, TTF_Text* text, int* x, int* y);

void displayText(SDL_FRect rect, TTF_Text* txt, int* x, int* y);
#endif