#include "draw_title.h"

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
			xPos[a] /= (w / 2);
		}

		// Correct the w/h variables
		w = (float)screen->w;
		*h = (float)screen->h;

		// We add back the effect
		for (int a = 0; a < VIDEO_COUNT; a++) {
			xPos[a] *= (w / 2);
		}

		for (int a = 0; a < VIDEO_COUNT; a++) {
			xPos[a] += w / 4;
			if (xPos[a] != a % (VIDEO_COUNT / 2)) printf("%f\n", xPos[a]);
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
			xPos[a] -= 4;
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
	static SDL_FRect logoRect;
	static SDL_Texture** thumbnailImgs = NULL;
	static TTF_Font* startTxt = NULL;
	static TTF_Font* quitTxt = NULL;
	static float xPos[VIDEO_COUNT];
	static float vectorFromLogo[VIDEO_COUNT][2];
	static float w, h = 0;
	int imgCount = 1;
	int filesNum = 0;
	static char** files;
	static SDL_FRect* rectArray;
	static char** thumbnailFiles;
	int movePOS[] = { screen->w * 3 / 4, screen->h / 2 };

	static int logoPOS[2];

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

		// This will store all the rectangles which will contains the videos
		rectArray = malloc(sizeof(SDL_FRect) * VIDEO_COUNT);
		if (rectArray == NULL) {
			fprintf(stderr, "%s\n", "Allocation for rect array failed");
			quit(ytQueue);
			exit(1);
		}

		logoPOS[0] = screen->w / 2;
		logoPOS[1] = screen->h / 2;

		// Good way to signal that they are not initialized
		xPos[0] = INT_MAX;
		w = screen->w;
		h = screen->h;

		logoRect = createRect(logoPOS[0], logoPOS[1], w / 2, w / 2 * 9 / 16, false);
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
		free(rectArray);
		// In case we come back here
		pfpImgs = NULL;
		return normal;
	}

	/*if (xOffset == INT_MAX) {
		xOffset = -w;
		yOffset = -h * 3 / 8;
	}*/

	SDL_FRect* pfpRects = malloc(sizeof(SDL_FRect) * VIDEO_COUNT);
	if (pfpRects == NULL) {
		fprintf(stderr, "%s\n", "Allocation for rect array failed");
		quit(ytQueue);
		exit(1);
	}
	SDL_SetRenderDrawColor(renderer, 43, 17, 92, SDL_ALPHA_OPAQUE);

	w = handleXPos(xPos, w, &h);

	static float rectW = 0;
	static float rectH = 0;

	if (rectW == 0) {
		rectW = w / 4;
		rectH = rectW * 9 / 16;
	}
	else {
		rectW = rectArray[0].w;
		rectH = rectArray[0].h;
	}

	for (int a = 0; a < VIDEO_COUNT; a++) {

		float y;
		if (rectW == w / 4) {
			y = h / 4 + (h / 2 * ((int)((float)a / VIDEO_COUNT * 2)));
			rectArray[a] = createRect(xPos[a], y, rectW, rectH, true);
		}
		else {
			y = rectArray[a].y;
			rectArray[a] = createRect(xPos[a], y, rectW, rectH, false);
		}


		vectorFromLogo[a][0] = logoRect.x - rectArray[a].x;
		vectorFromLogo[a][1] = logoRect.y - rectArray[a].y;
	}

	// The location of all the profile pictures
	for (int a = 0; a < VIDEO_COUNT; a++) {
		int x = rectArray[a].x + (h / 16);
		int y = rectArray[a].y + rectArray[a].h + h / 16;
		pfpRects[a] = createRect(x, y, h / 16, h / 16, true);
	}

	// Draw the large logo in the center
	static int frames = 20;
	if (center(logoRect.x, logoRect.w) != movePOS[0] || center(logoRect.y, logoRect.h) != movePOS[1]) {
		float newX = logoRect.x + (movePOS[0] - logoRect.x) / frames;
		float newY = logoRect.y + (movePOS[1] - logoRect.y) / frames;

		printf("%f %f\n", newX, logoRect.x);
		logoRect = zoom(logoRect.x, logoRect.y, newX, newY, logoRect.w, logoRect.h);
		printf("%f\n", logoRect.x);

		/*count++;*/
		// We are where we want to be
		/*if (count == frames) {
			logoRect.x = movePOS[0];
			logoRect.y = movePOS[1];
		}	*/
	}

	for (int a = 0; a < VIDEO_COUNT; a++) {
		float newDistanceX = logoRect.x - rectArray[a].x;
		float newDistanceY = logoRect.y - rectArray[a].y;

		float newX = rectArray[a].x + (newDistanceX - vectorFromLogo[a][0]);
		float newY = rectArray[a].y + (newDistanceY - vectorFromLogo[a][1]);

		if (a == 0) {
			printf("X: %.2f %.2f\n", newX, rectArray[a].x);
			printf("Y: %.2f %.2f\n", newY, rectArray[a].y);
			printf("W: %.2f H: %.2f\n", rectArray[a].w, rectArray[a].h);
		}

		rectArray[a] = zoom(rectArray[a].x, rectArray[a].y, newX, newY, rectArray[a].w, rectArray[a].h);
		xPos[a] = newX;

		if (a == 0) {
			printf("X POS: %f\n", xPos[a]);
		}

		rectArray[a].h = rectArray[a].w * 9 / 16;


	}

	//xOffset++;
	//yOffset++;
	SDL_RenderFillRects(renderer, rectArray, VIDEO_COUNT);
	// Renders all the texture currently in memory
	for (int a = 0; a < VIDEO_COUNT; a++) {
		SDL_RenderTexture(renderer, pfpImgs[a], NULL, &(pfpRects[a]));
		SDL_RenderTexture(renderer, thumbnailImgs[a], NULL, &(rectArray[a]));
	}

	free(pfpRects);

	drawLogo(logoRect.x, logoRect.y, logoRect.w);
	SDL_FRect startLogo = createRect(w / 2 - (w / 8), h * 7 / 8, w / 6, h / 6, true);
	SDL_FRect quitLogo = createRect(w / 2 + (w / 8), h * 7 / 8, w / 6, h / 6, true);
	// The Start and Quit Buttons
	drawSmoothRectagle(startLogo, 100, 27, 0, SDL_ALPHA_OPAQUE, startLogo.w / 6);
	drawSmoothRectagle(quitLogo, 100, 27, 0, SDL_ALPHA_OPAQUE, startLogo.w / 6);
	int x, y;
	displayText(startLogo, startTxt, &x, &y);
	displayText(quitLogo, quitTxt, &x, &y);
	return state;
}