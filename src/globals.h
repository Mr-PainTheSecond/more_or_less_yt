
#ifndef GLOBALS_H_
#define GLOBALS_H_
#include "commons.h"
// FPS is about 3ms
#define FPS_TIME 3

SDL_Renderer* renderer;
SDL_Window* window;
typedef struct screen {
	float w;
	float h;
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
	int timer;
	int health;
	int state;
	// Debug exclusive variables
	bool frameByFrame;
	int lifeImmunity;
} GameAttributes;

typedef struct dText {
	TTF_Text* text;
	char* str;
} DynamicText;

typedef struct vector2D {
	float x;
	float y;
} Vector2D;

typedef struct queue {
	YTNode* front;
	YTNode* back;
	int size;
} Queue;

typedef struct projectedObject {
	SDL_FRect realRect;
	SDL_FRect projectedRect;
} ProjectedObject;

enum states {title, titleAni, titleDiff, titleToNormal, normal,
	moreRight, moreWrong, lessRight, lessWrong, 
	justQuit, justLost, justWon, gameOver, gameWon, shutDown};
enum toggles {enableW = 1, enableS, disableEsc, enableEsc, disableS, disableW};
enum isOn {WOn = 1, SOn, escOn = 4};
enum difficulties {standard, noSubs, timer, pointDeduct, lessHeart, noMil, harshTimer};

SDL_Color ytRed;
Fonts* fontArray;
TTF_Font* smallFont;
TTF_Font* moreLessFont;
TTF_Font* timerFont;
TTF_Font* ytFont;

SDL_FRect diffToTitle;
SDL_FRect diffToPlay;
ProjectedObject* diffSelect;
int difficulty;
TTF_TextEngine* textEngine;
Queue* ytQueue;
GameAttributes* gameAttr;
SDL_Event event;
zsock_t* requester;
#endif


