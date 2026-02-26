#include "commons.h"
#include "font.h"
#include "server.h"
#include "config.h"
#include "draw.h"
#include "utilities.h"
#include "globals.h"


int moreOrLess(bool more, Queue* queue, int score, int* state) {

	int viewPublic = queue->front->views;
	YTNode* nextNode = queue->front->next;
	int privateViews = nextNode->views;
	//deQueue(queue, nextNode);
	if (more) {
		if (privateViews >= viewPublic) {
			if (nextNode->next == NULL) {
				return -1;
			}

				
			// buddy won!!!
			if (score + 1 >= WINNING_SCORE) {
				gameAttr->state = justWon;
			}
			else {
				gameAttr->state = moreRight;
			}

			return score + 1;
		}
		else {
			if (nextNode->next == NULL) {
				return -1;
			}

			gameAttr->state = moreWrong;
			gameAttr->health--;

			if (gameAttr->health <= 0) {
				gameAttr->state = justLost;
			}
			
			return INT_MIN;
		}
	}
	else {
		if (privateViews <= viewPublic) {
			if (nextNode->next == NULL) {
				return -1;
			}


			if (score + 1 >= WINNING_SCORE) {
				gameAttr->state = justWon;
			}
			else {
				gameAttr->state = lessRight;
			}

			return score + 1;
		}
		else {
			if (nextNode->next == NULL) {
				return -1;
			}

			gameAttr->state = lessWrong;
			gameAttr->health--;

			if (gameAttr->health <= 0) {
				gameAttr->state = justLost;
			}

			return INT_MIN;
		}
	}
}

bool isPressed(SDL_MouseButtonEvent mouse, SDL_FRect rect) {
	return mouse.x > rect.x && mouse.x < (rect.x + rect.w) && mouse.y > rect.y && mouse.y < (rect.y + rect.h);
}

/*Initializes the video, renderer, window, and font engine
needed for SDL*/
void SDL_Init_All() {

	TTF_Init();
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		//printf("Initialization of video failed: %s\n", SDL_GetError());
		exit(-1);
	}

	if (!SDL_CreateWindowAndRenderer("More Or Less", DEFAULT_WIDTH, DEFAULT_HEIGHT, SDL_WINDOW_HIDDEN, &window, &renderer)) {
		//printf("Initialization of window or renderer failed %s", SDL_GetError());
		exit(-1);
	}

	textEngine = TTF_CreateRendererTextEngine(renderer);
	if (textEngine == NULL) {
		//fprintf(stderr, "%s%s", "The engine failed to load due to: ", SDL_GetError());
		exit(-1);
	}

	if (!SDL_SetWindowFullscreen(window, FULLSCREEN)) {
		//fprintf(stderr, "%s%s", "Could not set the window to full screen", SDL_GetError());
		exit(-1);
	}
	

	screen = malloc(sizeof(Screen));
	if (screen == NULL) {
		//fprintf(stderr, "%s\n", "Malloc for screen struct failed");
		exit(-1);
	}

	
	SDL_DisplayMode* display = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window));
	if (display == NULL) {
		exit(-1);
	}
	// How we get the screen width/height depends on it is fullscreen
	if (FULLSCREEN) {
		screen->w = display->w;
		screen->h = display->h;
	}
	else {
		SDL_GetWindowSize(window, &(screen->w), &(screen->h));
		screen->id = SDL_GetDisplayForWindow(window);
	}

	screen->surface = SDL_CreateSurface(screen->w, screen->h, SDL_PIXELFORMAT_UNKNOWN);

	if (screen->surface == NULL) {
		exit(-1);
	}
}

void hideConsole() {
	HWND consoleWindow = GetConsoleWindow();
	ShowWindow(consoleWindow, SW_HIDE);
}

/*Creates a  structure that keeps track of important variables
(score, health, difficulty)*/
GameAttributes* initializeAttr(void) {
	GameAttributes* newAttr = malloc(sizeof(GameAttributes));

	if (newAttr == NULL) {
		quit(ytQueue);
		exit(1);
	}

	newAttr->health = 3;
	newAttr->score = 0;
	// This should be title on non-debug versions
	newAttr->state = DEFAULT_GAME_STATE;

	newAttr->frameByFrame = false;

	return newAttr;
}

/*When the program detects a screen change, we need to make sure to update
everything so that the UI remains relatively the same (it should be dynamic)*/
void updateScreen(TTF_Font** moreLessFont, bool fullScreen) {
	// For when the display used is changed
	SDL_DisplayID newId = SDL_GetDisplayForWindow(window);
	SDL_DisplayMode* display = SDL_GetCurrentDisplayMode(newId);
	if (fullScreen) {
		screen->w = display->w;
		screen->h = display->h;
	}
	else {
		SDL_GetWindowSize(window, &(screen->w), &(screen->h));
		screen->id = SDL_GetDisplayForWindow(window);
	}
	// Changes the screen width/height (everthing changes with it)
	screen->id = newId;

	SDL_DestroySurface(screen->surface);
	screen->surface = SDL_CreateSurface(screen->w, screen->h, SDL_PIXELFORMAT_UNKNOWN);

	// Dynamically changes all font sizes
	TTF_SetFontSize(smallFont, (SMALL_FONT_SIZE * ((float)screen->w / FONT_OFFSET)));
	TTF_SetFontSize(*moreLessFont, MORE_LESS_SIZE * ((float)screen->w / FONT_OFFSET));
	TTF_SetFontSize(ytFont, (SMALL_FONT_SIZE * ((float)screen->w / FONT_OFFSET)));
}

/*Will handles all cases whenever the left button is clicked*/
void handleMouseClick(SDL_MouseButtonEvent button, bool* aboutToQuit, int* timeClocked, int* counter) {
	SDL_FRect more = createRect(screen->w / 2, screen->h / 2, screen->w / 4, screen->h / 8, true);
	SDL_FRect less = createRect(screen->w / 2, screen->h * 5 / 8, screen->w / 4, screen->h / 8, true);
	SDL_FRect startLogo = createRect(screen->w / 2 - (screen->w / 8), screen->h * 7 / 8, screen->w / 6, screen->h / 6, true);
	SDL_FRect quitLogo = createRect(screen->w / 2 + (screen->w / 8), screen->h * 7 / 8, screen->w / 6, screen->h / 6, true);
	SDL_FRect quitRectWin = createRect(screen->w * 7 / 8, screen->h / 2 + (screen->h / 6), screen->w / 6, screen->h / 6, true);
	SDL_FRect menuRect = createRect(screen->w * 7 / 8, screen->h / 2 - (screen->h / 6), screen->w / 6, screen->h / 6, true);

	// When state is normal, we will presses as a guess
	if (gameAttr->state == normal) {
		if (isPressed(event.button, more)) {
			gameAttr->score = moreOrLess(true, ytQueue, gameAttr->score, &gameAttr->state);
			*counter = expandQueue(requester, ytQueue, counter);
			*timeClocked = clock();
		}
		else if (isPressed(event.button, less)) {
			gameAttr->score = moreOrLess(false, ytQueue, gameAttr->score, &gameAttr->state);
			*counter = expandQueue(requester, ytQueue, counter);
			*timeClocked = clock();
		}
	}
	// In these state, we have a quit button and a start button
	else if (gameAttr->state == title || gameAttr->state == gameWon || gameAttr->state == gameOver) {
		// Some weird ass coding
		SDL_FRect gameQuit;
		SDL_FRect gameTrans;
		int newState;
		if (gameAttr->state == title) {
			gameTrans = startLogo;
			gameQuit = quitLogo;
			newState = normal;
		}
		else {
			gameTrans = menuRect;
			gameQuit = quitRectWin;
			newState = title;
		}
		if (isPressed(event.button, gameTrans)) {
			*timeClocked = clock();
			// We will go into a transition state (allows us to free mem)
			gameAttr->state = newState;
		}
		else if (isPressed(event.button, gameQuit)) {
			*aboutToQuit = true;
			gameAttr->state = shutDown;
		}
	}
	// Allows user to skip the animantion
	else if (gameAttr->state >= moreRight && gameAttr->state <= lessWrong) {
		gameAttr->state = normal;
	}
}

int main() {
	// Should be off for normal behavior
	if (DEBUG) {
		hideConsole();
	}
	SDL_Init_All();
	createFontArray();
	bool gameRunning = true;
	// Will set gameRunning to false after one loop
	bool aboutToQuit = false;

	bool fullScreen = FULLSCREEN;
	startServer();
	requester =  establishConnection();
	ytQueue = createQueue();

	srand(RSEED);
	if (!getYtData(requester, ytQueue)) {
		zsock_destroy(&requester);
		free(screen);
		freeFontArray();
		SDL_Quit();
		return -1;
	}

	gameAttr = initializeAttr();

	smallFont = createFont("../assets/font/stentiga.TTF", (SMALL_FONT_SIZE * ((float)screen->w / FONT_OFFSET)));
	moreLessFont = createFont("../assets/font/stentiga.TTF", (MORE_LESS_SIZE * ((float)screen->w / FONT_OFFSET)));
	ytFont = createFont("../assets/font/Roboto-Black.TTF", (SMALL_FONT_SIZE * ((float)screen->w / FONT_OFFSET)));
	SDL_FRect more = createRect(screen->w / 2, screen->h / 2, screen->w / 4, screen->h / 8, true);
	SDL_FRect less = createRect(screen->w / 2, screen->h * 5 / 8, screen->w / 4, screen->h / 8, true);
	TTF_Text* moreText = TTF_CreateText(textEngine, moreLessFont, "More", strlen("More") + 1);
	TTF_Text* lessText = TTF_CreateText(textEngine, moreLessFont, "Less", strlen("Less") + 1);
	//{ 255, 0, 51, SDL_ALPHA_OPAQUE };
	ytRed.r = 255;
	ytRed.g = 0;
	ytRed.b = 51;
	ytRed.a = SDL_ALPHA_OPAQUE;
	// Standard unless debugging
	difficulty = DEFAULT_DIFFICULTY;
	if (moreText == NULL || lessText == NULL) {
		printf(SDL_GetError());
		quit(ytQueue);
		return -1;
	}

		
	clock_t timeClocked = clock();
	clock_t cooldown = clock();

	int counter = 0;
	bool canClick = true;

	// Frame by Frame debugging variables
	bool nextFrame = false;
	int frameCount = 0;
	// First Bit: W is allowed; Second Bit: S is allowed;
	// Added as need be.
	int keysAllowed = 7;
	while (gameRunning) {

		// DON'T PUT NON EVENT SHIT INSIDE EVENT LOOP
		clock_t currentTime = clock();
		if (currentTime - timeClocked >= 100) {
			canClick = true;
		}


		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				gameRunning = false;
				break;
			}
			// Key is the code of the key press within the key struct
			if (event.type == SDL_EVENT_KEY_DOWN && canClick) {
				canClick = false;
				cooldown = clock();
				if (event.key.key == SDLK_ESCAPE && (keysAllowed & escOn) != 0) {
					keysAllowed = keysAllowed & disableEsc;
					// States where the game is business as usual
					if (gameAttr->state >= normal && gameAttr->state <= gameWon) {
						gameAttr->state = justQuit;
					}
					else if (gameAttr->state == title) {
						gameAttr->state = shutDown;
						aboutToQuit = true;
					}
				}
				else if (event.key.key == SDLK_W && (keysAllowed & WOn) != 0) {
					keysAllowed = keysAllowed & disableW;
					if (gameAttr->state == normal) {
						gameAttr->score = moreOrLess(true, ytQueue, gameAttr->score, &gameAttr->state);
						counter = expandQueue(requester, ytQueue, counter);
						timeClocked = clock();
						cooldown = clock();
					}

					// If we are in a transtion phase, it will skip it
					else if (gameAttr->state >= moreRight && gameAttr->state <= lessWrong) {
						gameAttr->state = normal;
					}
				}
				else if (event.key.key == SDLK_S && (keysAllowed & SOn) != 0) {
					keysAllowed = keysAllowed & disableS;
					if (gameAttr->state == normal) {
						gameAttr->score = moreOrLess(false, ytQueue, gameAttr->score, &gameAttr->state);
						counter = expandQueue(requester, ytQueue, counter);
						timeClocked = clock();
						cooldown = clock();
					}
					else if (gameAttr->state >= moreRight && gameAttr->state <= lessWrong) {
						gameAttr->state = normal;
					}
				}

				// We shouldn't need keysAllowed for this
				else if (event.key.key == SDLK_F11) {
					if (fullScreen) {
						SDL_SetWindowFullscreen(window, false);
					}
					else {
						SDL_SetWindowFullscreen(window, true);
					}

					// Changes size regardless
					fullScreen = !fullScreen;
					updateScreen(&moreLessFont, fullScreen);
				}

				else if (event.key.key == SDLK_F1 && DEBUG) {
					gameAttr->frameByFrame = !gameAttr->frameByFrame;
					frameCount = 0;
				}

				else if (gameAttr->frameByFrame && event.key.key == SDLK_F2) {
					nextFrame = true;
				}

				// Decrease difficulty
				else if (DEBUG && event.key.key == SDLK_F6) {
					gameAttr->state = justQuit;

					difficulty = (difficulty - 1) % 7;
				}

				// Increases difficulty
				else if (DEBUG && event.key.key == SDLK_F7) {
					gameAttr->state = justQuit;

					difficulty = (difficulty + 1) % 7;
				}
				
			}
			
			if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && canClick) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					canClick = false;
					cooldown = clock();
					// What happens varies a lot of the state/what is clicked
					handleMouseClick(event.button, &aboutToQuit, &timeClocked, &counter);
				}
			}

			if (event.type == SDL_EVENT_KEY_UP) {
				if (event.key.key == SDLK_S) {
					keysAllowed = keysAllowed | enableS;
				}
				else if (event.key.key == SDLK_W) {
					keysAllowed = keysAllowed | enableW;
				}
				else if (event.key.key == SDLK_ESCAPE) {
					keysAllowed = keysAllowed | enableEsc;
				}
			}
		}

		// Handle frame by frame debugging 
		if (gameAttr->frameByFrame) {
			if (!nextFrame) {
				continue;
			}
			else {
				nextFrame = false;
				printf("%d\n", frameCount);
				frameCount++;
			}
		}
		
		gameAttr->state = draw(moreText, lessText, ytQueue);

		if (gameAttr->score < 0) {
			gameAttr->score = 0;
		}

		if (aboutToQuit) {
			gameRunning = false;
		}


		SDL_DisplayID id = SDL_GetDisplayForWindow(window);
		// The screen has changed
		if (id != screen->id) {
			
			updateScreen(&moreLessFont, fullScreen);
			more = createRect(screen->w / 2, screen->h / 2, screen->w / 4, screen->h / 8, true);
			less = createRect(screen->w / 2, screen->h * 5 / 8, screen->w / 4, screen->h / 8, true);
		}

		// Something went wrong
		if (gameAttr->score == -1) {
			printf("%s\n", "Weird Score");
			gameRunning = false;	
		}

		if (counter == -1) {
			printf("%s\n", "Weird Counter");
			gameRunning = false;
		}

		clock_t finishTime = clock();
		// Enforce the FPS
		while (finishTime - currentTime < 3) {
			finishTime = clock();
		}
	}
	quit(ytQueue);

	return 0;
}