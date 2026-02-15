
#ifndef GLOBALS_H_
#define GLOBALS_H_
#include "commons.h"
// FPS is about 3ms
#define FPS_TIME 3

SDL_Renderer* renderer;
SDL_Window* window;
typedef struct screen {
	int w;
	int h;
	SDL_DisplayID id;
	SDL_Surface* surface;
} Screen;
Screen* screen;
typedef struct fonts {
	TTF_Font** fonts;
	int fontIndex;
	float fontSize;
} Fonts;

typedef struct ytNode {
	struct ytNnode* next;
	int views;
	char* filePath;
	char* sViews;
	char* subs;
	SDL_Texture* img;
} YTNode;

typedef struct gameAttributes {
	int score;
	int difficulty;
	int health;
	int state;
} GameAttributes;

typedef struct dText {
	TTF_Text* text;
	char* str;
} DynamicText;

typedef struct queue {
	YTNode* front;
	YTNode* back;
	int size;
} Queue;

enum states {title, normal,
	moreRight, moreWrong, lessRight, lessWrong, 
	justQuit, justLost, justWon, gameOver, gameWon, shutDown};
enum toggles {enableW = 1, enableS, disableEsc, enableEsc, disableS, disableW};
enum isOn {WOn = 1, SOn, escOn = 4};
enum difficulties {standard, noSubs, timer, pointDeduct, lessHeart, noMil, harshTimer};

SDL_Color ytRed;
Fonts* fontArray;
TTF_Font* smallFont;
TTF_Font* moreLessFont;
TTF_Font* ytFont;
int difficulty;
TTF_TextEngine* textEngine;
Queue* ytQueue;
GameAttributes* gameAttr;
SDL_Event event;
zsock_t* requester;
#endif


