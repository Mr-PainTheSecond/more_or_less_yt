
#ifndef GLOBALS_H_
#define GLOBALS_H_
#include "commons.h"
#include "config.h"
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
	int fontSize;
} Fonts;

typedef struct ytNode {
	struct ytNnode* next;
	float views;
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
	int difficulty;
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

typedef struct multiLineText {
	TTF_Text** lines;
	ProjectedObject* lineRects;
	int lineCount;
} MultiLineText;

enum states {title, titleAni, titleDiff, titleToNormal, normal,
	moreRight, moreWrong, lessRight, lessWrong, 
	justQuit, justLost, justWon, gameOver, gameWon, shutDown};
enum toggles {enableW = 1, enableS, disableEsc, enableEsc, disableS, disableW};
enum isOn {WOn = 1, SOn, escOn = 4};
enum difficulties {standard, noSubs, timer, pointDeduct, lessHeart, noMil, harshTimer};
enum saveInfo {stars, highScore};

SDL_Color ytRed;
Fonts* fontArray;
char** offlineVideos;
int offlineVideoCount;
TTF_Font* smallFont;
TTF_Font* moreLessFont;
TTF_Font* timerFont;
TTF_Font* ytFont;


SDL_FRect diffToTitle;
SDL_FRect diffToPlay;
ProjectedObject* diffSelect;
bool offline;
bool connected;
int* saveData;
int savePoints;
int difficulty;
TTF_TextEngine* textEngine;
Queue* ytQueue;
GameAttributes* gameAttr;
SDL_Event event;
zsock_t* requester;
HANDLE hMutex;
#endif


