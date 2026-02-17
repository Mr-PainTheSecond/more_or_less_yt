#include "draw.h"
#include "font.h"


/*Handles everything for the xPos array, which keeps track of the positions
of the thumbnails which are all offset every frame. Will also change the width and
height if the screen size ever changes.*/
float handleXPos(float* xPos, float w, float* h) {
	// The screen has changed, we need to fix the positions
	if (w != screen->w) {
		// First, we removed the effect the screen width has on the positons
		for (int a = 0; a < VIDEO_COUNT; a++) {
			xPos[a] -= w / 4;
		}

		for (int a = 0; a < VIDEO_COUNT; a++) {
			xPos[a] /= w / 2;
		}

		// Correct the w/h variables
		w = (float)screen->w;
		*h = (float)screen->h;

		// We add back the effect
		for (int a = 0; a < VIDEO_COUNT; a++) {
			xPos[a] *= w / 2;
		}

		for (int a = 0; a < VIDEO_COUNT; a++) {
			xPos[a] += w / 4;
		}
	}

	float rectW = w / 4;
	// This signals that this is the first iteration, and we need to populate the value
	if (xPos[0] == INT_MAX) {
		for (int a = 0; a < VIDEO_COUNT; a++) {
			xPos[a] = w / 4 + (w / 2 * (a % (VIDEO_COUNT / 2)));
		}
	}
	// If not, we offset.
	else {
		for (int a = 0; a < VIDEO_COUNT; a++) {
			xPos[a]--;
			if (xPos[a] + (rectW / 2) < 0) {
				// This is the right most position the rect can be
				xPos[a] = w / 4 + (w / 2 * (VIDEO_COUNT / 2 - 1));
			}
		}
	}

	return w;
}

/*Responsible for drawing the entire title section of the
game.*/
int drawTitle(int state) {
	static SDL_Texture** pfpImgs = NULL;
	static SDL_Texture** thumbnailImgs = NULL;
	static TTF_Font* startTxt = NULL;
	static TTF_Font* quitTxt = NULL;
	static float xPos[VIDEO_COUNT];
	static int w, h = 0;
	int imgCount = 1;
	int filesNum = 0;
	static char** files;
	static char** thumbnailFiles;
	if (pfpImgs == NULL) {
		files = readAndSplit("..\\assets\\data\\pfp.txt", '\n', &filesNum);
		formatAsFileLocation("..\\assets\\images\\perm\\pfp\\", NULL, files, filesNum);
		thumbnailFiles = readAndSplit("..\\assets\\data\\pfp.txt", '\n', &filesNum);
		formatAsFileLocation("..\\assets\\images\\perm\\thumbnail\\", NULL, thumbnailFiles, filesNum);
		pfpImgs = malloc(sizeof(SDL_Texture*) * VIDEO_COUNT);
		if (pfpImgs == NULL) {
			fprintf(stderr, "%s\n", "Could not allocate memory for PFPs");
			quit(ytQueue);
			exit(1);
		}

		thumbnailImgs = malloc(sizeof(SDL_Texture*) * VIDEO_COUNT);
		if (thumbnailImgs == NULL) {
			fprintf(stderr, "%s\n", "Could not allocate memory for thumbnails");
			quit(ytQueue);
			exit(1);
		}

		startTxt = TTF_CreateText(textEngine, smallFont, "Start", strlen("Start"));
		quitTxt = TTF_CreateText(textEngine, smallFont, "Quit", strlen("Quit"));
		for (int a = 0; a < VIDEO_COUNT; a++) {
			int imgIndex = rand() % filesNum;
			SDL_Surface* pfpSurf = IMG_Load(files[imgIndex]);
			SDL_Surface* thumbnailSurf = IMG_Load(thumbnailFiles[imgIndex]);
			pfpSurf = transformToCircle(pfpSurf);
			pfpImgs[a] = SDL_CreateTextureFromSurface(renderer, pfpSurf);
			thumbnailImgs[a] = SDL_CreateTextureFromSurface(renderer, thumbnailSurf);
			SDL_DestroySurface(pfpSurf);
			SDL_DestroySurface(thumbnailSurf);
		}

		// Good way to signal that they are not initialized
		xPos[0] = INT_MAX;
		w = screen->w;
		h = screen->h;
	}

	// The transition from title to main game.
	// Destroys all the assets in the meanwhile
	if (state == shutDown) {
		if (pfpImgs != NULL) {
			for (int a = 0; a < VIDEO_COUNT; a++) {
				SDL_DestroyTexture(pfpImgs[a]);
				SDL_DestroyTexture(thumbnailImgs[a]);
			}
		}

		for (int a = 0; a < filesNum; a++) {
			free(files[a]);
			free(thumbnailFiles[a]);
		}

		TTF_DestroyText(startTxt);
		TTF_DestroyText(quitTxt);
		free(files);
		free(thumbnailFiles);
		free(pfpImgs);
		free(thumbnailImgs);
		// In case we come back here
		pfpImgs = NULL;
		return normal;
	}

	/*if (xOffset == INT_MAX) {
		xOffset = -w;
		yOffset = -h * 3 / 8;
	}*/
	// This will store all the rectangles which will contains the videos
	SDL_FRect* rectArray = malloc(sizeof(SDL_FRect) * VIDEO_COUNT);
	if (rectArray == NULL) {
		fprintf(stderr, "%s\n", "Allocation for rect array failed");
		quit(ytQueue);
		exit(1);
	}


	SDL_FRect* pfpRects = malloc(sizeof(SDL_FRect) * VIDEO_COUNT);
	if (pfpRects == NULL) {
		fprintf(stderr, "%s\n", "Allocation for rect array failed");
		quit(ytQueue);
		exit(1);
	}
	SDL_SetRenderDrawColor(renderer, 43, 17, 92, SDL_ALPHA_OPAQUE);
	
	w = handleXPos(xPos, w, &h);

	float rectW = w / 4;
	float rectH = h / 4;

	for (int a = 0; a < VIDEO_COUNT; a++) {
		float y = h / 4 + (h / 2 * ((int)((float)a / VIDEO_COUNT * 2)));
		rectArray[a] = createRect(xPos[a], y, rectW, rectH, true);
	}
	
	// The location of all the profile pictures
	for (int a = 0; a < VIDEO_COUNT; a++) {
		int x = rectArray[a].x + (h / 16);
		int y = rectArray[a].y + rectArray[a].h + h / 16;
		pfpRects[a] = createRect(x, y, h / 16, h / 16, true);
	}
	//xOffset++;
	//yOffset++;
	SDL_RenderFillRects(renderer, rectArray, VIDEO_COUNT);
	// Renders all the texture currently in memory
	for (int a = 0; a < VIDEO_COUNT; a++) {
		SDL_RenderTexture(renderer, pfpImgs[a], NULL, &(pfpRects[a]));
		SDL_RenderTexture(renderer, thumbnailImgs[a], NULL, &(rectArray[a]));
	}
	free(rectArray);
	free(pfpRects);
	// Draw the large logo in the center
	drawLogo(w / 2, h / 2, w / 2);
	SDL_FRect startLogo = createRect(w / 2 - (w / 8), h * 7 / 8, w / 6, h / 6, true);
	SDL_FRect quitLogo = createRect(w / 2 + (w / 8), h * 7 / 8, w / 6, h / 6, true);
	// The Start and Quit Buttons
	drawSmoothRectagle(startLogo, 100, 27, 0, startLogo.w / 6);
	drawSmoothRectagle(quitLogo, 100, 27, 0, startLogo.w / 6);
	int x, y;
	displayText(startLogo, startTxt, &x, &y);
	displayText(quitLogo, quitTxt, &x, &y);
	return state;
}

int finalConditionProperties(TTF_Text** videoTxt, TTF_Text** stateTxt, SDL_Texture** background, SDL_Texture** pfp) {
	SDL_Surface* winSurf;
	SDL_Surface* pfpSurf;
	if (gameAttr->state == justWon || gameAttr->state == gameWon) {
		char videoMsg[] = "Thumbs up crew CONGRAGULATES you";
		char stateMsg[] = "WINNER!!!";

		*videoTxt = TTF_CreateText(textEngine, ytFont, videoMsg, strlen(videoMsg));
		*stateTxt = TTF_CreateText(textEngine, moreLessFont, stateMsg, strlen(stateMsg));

		winSurf = IMG_Load("..\\assets\\images\\perm\\other\\winner.JPG");
		pfpSurf = IMG_Load("..\\assets\\images\\perm\\other\\winner_channel.PNG");
	}
	else {
		char videoMsg[] = "Markiplier is disappointed (ASMR)";
		char stateMsg[] = "GAME OVER";

		*videoTxt = TTF_CreateText(textEngine, ytFont, videoMsg, strlen(videoMsg));
		*stateTxt = TTF_CreateText(textEngine, moreLessFont, stateMsg, strlen(stateMsg));

		winSurf = IMG_Load("..\\assets\\images\\perm\\other\\loser.JPG");
		pfpSurf = IMG_Load("..\\assets\\images\\perm\\other\\loser_channel.PNG");
	}

	*background = SDL_CreateTextureFromSurface(renderer, winSurf);
	*pfp = SDL_CreateTextureFromSurface(renderer, pfpSurf);

	SDL_DestroySurface(winSurf);
	SDL_DestroySurface(pfp);
}

/*Draws the screen which informs the user that they have
won the game*/
void drawFinalScreen() {
	static SDL_Texture* background = NULL;
	static SDL_Texture* pfp = NULL;
	// A rect depicting the whole display
	static TTF_Text* videoTxt;
	static TTF_Text* stateTxt;
	static TTF_Text* menuTxt;
	static TTF_Text* quitTxt;
	// Keep track of previous state to swap assets
	static int previousState = 0;
	int w = screen->w;
	int h = screen->h;
	if (gameAttr->state == shutDown) {
		if (background != NULL) {
			SDL_DestroyTexture(background);
			SDL_DestroyTexture(pfp);

			TTF_DestroyText(videoTxt);
			TTF_DestroyText(stateTxt);
			TTF_DestroyText(menuTxt);
			TTF_DestroyText(quitTxt);
		}

		return;
	}

	if (background == NULL) {
		SDL_Surface* winSurf;
		SDL_Surface* pfpSurf;
		
		finalConditionProperties(&videoTxt, &stateTxt, &background, &pfp);

		menuTxt = TTF_CreateText(textEngine, smallFont, "MENU", strlen("MENU"));
		quitTxt = TTF_CreateText(textEngine, smallFont, "QUIT", strlen("QUIT"));

		// Saves so we can change it later
		previousState = gameAttr->state;
	}
	
	// This part of the game is initialzied, but we need new assets
	if (gameAttr->state != previousState) {
		finalConditionProperties(&videoTxt, &stateTxt, &background, &pfp);

		previousState = gameAttr->state;
	}

	int x = 0;
	int y = 0;
	
	SDL_FRect winRect = createRect(w / 64, h / 4, w / 2, h / 2, false);
	SDL_FRect pfpRect = createRect(winRect.x + (h / 16), winRect.y + winRect.h + (h / 16), h / 10, h / 10, true);
	SDL_FRect videoTxtRect = createRect(pfpRect.x + (winRect.w / 2), pfpRect.y + (pfpRect.h / 2), (winRect.w + winRect.x) - (pfpRect.x + pfpRect.w), h / 16, true);
	SDL_FRect announceRect = createRect(w / 2, h / 8, w / 4, h / 6, true);
	SDL_FRect quitRect = createRect(w * 7 / 8, h / 2 + (h / 6), w / 6, h / 6, true);
	SDL_FRect menuRect = createRect(w * 7 / 8, h / 2 - (h / 6), w / 6, h / 6, true);

	drawSmoothRectagle(announceRect, 21, 71, 52, 20.0f);
	drawSmoothRectagle(videoTxtRect, 39, 39, 39, 30.0f);
	drawSmoothRectagle(menuRect, 100, 27, 0, menuRect.w / 6);
	drawSmoothRectagle(quitRect, 100, 27, 0, menuRect.w / 6);

	SDL_RenderTexture(renderer, background, NULL, &winRect);
	SDL_RenderTexture(renderer, pfp, NULL, &pfpRect);

	displayText(videoTxtRect, videoTxt, &x, &y);
	displayText(announceRect, stateTxt, &x, &y);
	displayText(quitRect, quitTxt, &x, &y);
	displayText(menuRect, menuTxt, &x, &y);
}

int draw(TTF_Text* more, TTF_Text* less, Queue* queue) {
	// Clears the renderer
	static bool firstIter = true;
	SDL_SetRenderDrawColor(renderer, 128, 0, 32, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);

	if (gameAttr->state == title  || gameAttr->state == shutDown) {
		gameAttr->state = drawTitle(gameAttr->state);
	}
	// There is a LOT of states within the regular game logic
	if (gameAttr->state >= normal && gameAttr->state <= justWon) {
		gameAttr->state = drawMoreOrLess(more, less, queue);
	}
	if (gameAttr->state >= justLost && gameAttr->state <= gameWon) {
		drawFinalScreen();
	}
	if (firstIter) {
		SDL_ShowWindow(window);
		firstIter = false;
	}

	SDL_RenderPresent(renderer);
	return gameAttr->state;
}