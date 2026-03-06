#ifndef CONFIG_H_
#define CONFIG_H_
#include "commons.h"
#include "globals.h"


#define FULLSCREEN 1

// If a specific seed needs testing, this will do the job
#define RSEED time(NULL)

// Turns ons all debug features. OFF for release.
#define DEBUG 1

#define FRAME_RATE 60

// The width the windows will start at (if not fullscreen)
#define DEFAULT_WIDTH 1080

// The height the windows will start at (if not fullscreen)
#define DEFAULT_HEIGHT 720

// Change the small font size
#define SMALL_FONT_SIZE 80

// Font size for More/Less
#define MORE_LESS_SIZE 160

// Font size for timer
#define TIMER_SIZE 240

// This is the width the screen has to be such that the font size is exactly the fonts in the macros
// (makes the font size dynamic)
#define FONT_OFFSET 3840.0f

// How many videos are loaded into the start screen
#define VIDEO_COUNT 10

// How fast does the transition phase last
#define TRANSITION_GUESS_SPEED 150

// The state which the game starts at
#define DEFAULT_GAME_STATE title

// The difficulty which the game starts at.
#define DEFAULT_DIFFICULTY standard

// The minimum score needed to win
#define WINNING_SCORE 20

// Timer for timer <= difficulty < harshTimer.
#define STARTING_TIME 45

// Increased time per question right (non-HarshTimer)
#define TIMER_INCREMENT 2

// Points deducted for difficulty >= pointDeduct
#define POINT_DEDUCT_AMOUNT 2

// Debug feature: Immune to losing lives. (not to time out)
#define LIFE_IMMUNITY 0

#endif // !CONFIG_H_

