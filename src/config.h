#ifndef CONFIG_H_
#define CONFIG_H_
#include "commons.h"
#include "globals.h"


#define FULLSCREEN 1

// If a specific seed needs testing, this will do the job
#define RSEED time(NULL)

// Should be used just for debugging
#define SHOW_CONSOLE 1


// The width the windows will start at (if not fullscreen)
#define DEFAULT_WIDTH 1080

// The height the windows will start at (if not fullscreen)
#define DEFAULT_HEIGHT 720

// Change the small font size
#define SMALL_FONT_SIZE 80

#define MORE_LESS_SIZE 160

// This is the width the screen has to be such that the font size is exactly the fonts in the macros
// (makes the font size dynamic)
#define FONT_OFFSET 3840.0f

// How many videos are loaded into the start screen
#define VIDEO_COUNT 10

// How fast does the transition phase last
#define TRANSITION_GUESS_SPEED 300

// The state which the game starts at
#define DEFAULT_GAME_STATE title

// The minimum score needed to win
#define WINNING_SCORE 20

#endif // !CONFIG_H_

