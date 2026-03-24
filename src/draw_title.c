#include "draw_title.h"

/*Handles everything for the xPos array, which keeps track of the positions
of the thumbnails which are all offset every frame. Will also change the width and
height if the screen size ever changes.*/
float handleXPos(float* realPOS, float* projectedPOS, float w, float* h, float rectW) {

	// The screen has changed, we need to fix the positions
	if (w != screen->w) {
		// First, we removed the effect the screen width has on the positons
		for (int a = 0; a < VIDEO_COUNT; a++) {
			realPOS[a] -= rectW;
		}

		for (int a = 0; a < VIDEO_COUNT; a++) {
			realPOS[a] /= (w / 2);
		}

		// Correct the w/h variables
		w = (float)screen->w;
		*h = (float)screen->h;

		// We add back the effect
		for (int a = 0; a < VIDEO_COUNT; a++) {
			realPOS[a] *= (w / 2);
		}

		for (int a = 0; a < VIDEO_COUNT; a++) {
			realPOS[a] += rectW;
		}
	}

	// This signals that this is the first iteration, and we need to populate the value
	if (realPOS[0] == INT_MAX) {
		for (int a = 0; a < VIDEO_COUNT; a++) {
			realPOS[a] = rectW + (w / 2 * (a % (VIDEO_COUNT / 2)));
			projectedPOS[a] = realPOS[a];
		}
	}

	// If not, we offset.
	else {
		for (int a = 0; a < VIDEO_COUNT; a++) {
			realPOS[a] -= 4;
			if (projectedPOS[a] + rectW < 0) {
				// This is the right most position the rect can be
				realPOS[a] = rectW + (rectW * 2 * (VIDEO_COUNT / 2 - 1));
			}
		}
	}

	return w;
}

ProjectedObject projectRect(ProjectedObject obj, float xDifference, float yDifference) {
	float newX = obj.projectedRect.x + xDifference;
	float newY = obj.projectedRect.y + yDifference;
	obj.projectedRect = zoom(obj.realRect.x, obj.realRect.y, newX, newY, obj.realRect.w, obj.realRect.h);
	return obj;
}

/*Responsible for drawing the entire title section of the
game.*/
int drawTitle(int state) {
	static SDL_Texture** pfpImgs = NULL;
	static ProjectedObject logoRect;
	static SDL_Texture** thumbnailImgs = NULL;
	static TTF_Font* startTxt = NULL;
	static TTF_Font* quitTxt = NULL;
	static float xPos[2][VIDEO_COUNT];
	static Vector2D vectorFromLogo[VIDEO_COUNT * 2];
	static float w, h = 0;
	static ProjectedObject startLogo, quitLogo;
	int imgCount = 1;
	int filesNum = 0;
	static char** files;

	static ProjectedObject* rectArray;

	static ProjectedObject* pfpRects;

	static ProjectedObject* difficultyRects;
	static ProjectedObject* buttonDifficulty;

	static char** thumbnailFiles;
	float movePOS[] = { screen->w * 7 / 4, screen->h / 2 };

	static float logoPOS[2];
	const float firstStopRatio = 2.69f;
	const int firstStopDistanceX = 3049;
	const int firstStopDistanceY = -1519;

	const int difficultyCount = 8;
	const int frames = FRAME_RATE;

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
		rectArray = malloc(sizeof(ProjectedObject) * VIDEO_COUNT);
		if (rectArray == NULL) {
			fprintf(stderr, "%s\n", "Allocation for rect array failed");
			quit(ytQueue);
			exit(1);
		}

		pfpRects = malloc(sizeof(ProjectedObject) * VIDEO_COUNT);
		if (pfpRects == NULL) {
			fprintf(stderr, "%s\n", "Allocation for pfps failed");
			quit(ytQueue);
			exit(1);
		}

		difficultyRects = malloc(sizeof(ProjectedObject) * difficultyCount);
		buttonDifficulty = malloc(sizeof(ProjectedObject )* difficultyCount);

		if (difficultyRects == NULL || buttonDifficulty == NULL) {
			fprintf(stderr, "%s\n", "Allocation for difficulties failed");
			quit(ytQueue);
			exit(1);
		}

		logoPOS[0] = screen->w / 2;
		logoPOS[1] = screen->h / 2;

		// Good way to signal that they are not initialized
		xPos[0][0] = INT_MAX;
		w = screen->w;
		h = screen->h;

		// Starting values of the logos
		logoRect.realRect = createRect(logoPOS[0], logoPOS[1], w / 2, w / 2 * 9 / 16, false);
		startLogo.realRect = createRect(w / 2 - (w / 8), h * 7 / 8, w / 6, h / 6, true);
		quitLogo.realRect = createRect(w / 2 + (w / 8), h * 7 / 8, w / 6, h / 6, true);
		
		logoRect.projectedRect = createRect(logoPOS[0], logoPOS[1], w / 2, w / 2 * 9 / 16, false);
		startLogo.projectedRect = createRect(w / 2 - (w / 8), h * 7 / 8, w / 6, h / 6, true);
		quitLogo.projectedRect = createRect(w / 2 + (w / 8), h * 7 / 8, w / 6, h / 6, true);

	

		float rectW = w / 4;
		float rectH = rectW * 9 / 16;

		w = handleXPos(xPos[0], xPos[1], w, &h, rectW);

		for (int a = 0; a < VIDEO_COUNT; a++) {
			float y = h / 4 + (h / 2 * ((int)((float)a / VIDEO_COUNT * 2)));
			rectArray[a].realRect = createRect(xPos[0][a], y, rectW, rectH, true);
			rectArray[a].projectedRect = createRect(xPos[0][a], y, rectW, rectH, true);

			float pfpY = rectArray[a].realRect.y + rectArray[a].realRect.h + (h / 16);

			pfpRects[a].realRect = createRect(rectArray[a].realRect.x + (h / 16), pfpY, h / 16, h / 16, true);
			pfpRects[a].projectedRect = createRect(rectArray[a].realRect.x + (h / 16), pfpY, h / 16, h / 16, true);
		}

		for (int a = 0; a < difficultyCount; a++) {

			float width = w / 6 * firstStopRatio;
			float x = w / 4 + (w / 3 * (a % 2)) - ((firstStopDistanceX));
			float y = -(h / 4) + (h / 2 * ((int)((float)a / difficultyCount * 4))) - firstStopDistanceY;
			float height = width * 9 / 16;

			difficultyRects[a].realRect = createRect(x, y, width, height, true);
			difficultyRects[a].projectedRect = createRect(x, y, width, height, true);

			float buttonX = difficultyRects[a].realRect.x + (difficultyRects[a].realRect.w + w / 8) / firstStopRatio;
			float buttonY = difficultyRects[a].realRect.y + difficultyRects[a].realRect.h / firstStopRatio;

			buttonDifficulty[a].realRect = createRect(buttonX, buttonY, width / 2, height / 2, false);
			buttonDifficulty[a].projectedRect = createRect(buttonX, buttonY, width / 2, height / 2, false);
		}
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
		free(pfpRects);
		free(difficultyRects);
		// In case we come back here
		pfpImgs = NULL;
		return normal;
	}

	/*if (xOffset == INT_MAX) {
		xOffset = -w;
		yOffset = -h * 3 / 8;
	}*/

	SDL_SetRenderDrawColor(renderer, 43, 17, 92, SDL_ALPHA_OPAQUE);

	// Draw the large logo in the center
	static int count = 0;
	float xDifference;
	float yDifference;

	if (count < frames) {
		xDifference = ((movePOS[0]) - logoRect.projectedRect.x) / frames;
		yDifference = (movePOS[1] - logoRect.projectedRect.y) / frames;

		float newX = logoRect.projectedRect.x + (xDifference);
		float newY = logoRect.projectedRect.y + (yDifference);

		float oldH = logoRect.projectedRect.h;

		logoRect.projectedRect = zoom(logoRect.realRect.x, logoRect.realRect.y, newX, newY, logoRect.realRect.w, logoRect.realRect.h);

		//yDifference += logoRectProjection.h - oldH;

		startLogo = projectRect(startLogo, xDifference, yDifference);
		quitLogo = projectRect(quitLogo, xDifference, yDifference);

		count++;
		/*count++;*/
		// We are where we want to be
		/*if (count == frames) {
			logoRect.x = movePOS[0];
			logoRect.y = movePOS[1];
		}	*/
	}
	else {
		xDifference = 0;
		yDifference = 0;
		printf("%f\n", buttonDifficulty[0].projectedRect.x);
		printf("%f\n", logoRect.projectedRect.x - logoRect.realRect.x);
		printf("distance y: %f\n", logoRect.projectedRect.y - logoRect.realRect.x);
	}


	static float rectW = 0;
	static float rectH = 0;

	if (rectW == 0) {
		rectW = w / 4;
		rectH = rectW * 9 / 16;
	}
	else {
		rectW = rectArray[0].projectedRect.w;
		rectH = rectArray[0].projectedRect.h;
	}

	w = handleXPos(xPos[0], xPos[1], w, &h, rectW);

	// This is gonna scale the videos/pfps based on the logo's movement
	for (int a = 0; a < VIDEO_COUNT; a++) {
		float realXShift = xPos[0][a] - rectArray[a].realRect.x;

		// xPos changes are real, not projected changes
		rectArray[a].realRect.x = xPos[0][a];
		pfpRects[a].realRect.x += realXShift;

		if (a == 0) {
			printf("%f %f\n", rectArray[a].projectedRect.x, rectArray[a].projectedRect.y);
		}

		if (xDifference != 0 || yDifference != 0) {
			rectArray[a] = projectRect(rectArray[a], xDifference, yDifference);
			pfpRects[a] = projectRect(pfpRects[a], xDifference, yDifference);

			if (a < difficultyCount) {
				difficultyRects[a] = projectRect(difficultyRects[a], xDifference, yDifference);
				buttonDifficulty[a] = projectRect(buttonDifficulty[a], xDifference, yDifference);
			}

		}

		// Shifts in here so they don't count towards projection
		rectArray[a].projectedRect.x += realXShift;
		pfpRects[a].projectedRect.x += realXShift;

		pfpRects[a].projectedRect.y = rectArray[a].projectedRect.y + rectArray[a].projectedRect.h + (pfpRects[a].projectedRect.h);

		xPos[1][a] = rectArray[a].projectedRect.x;
	}

	//xOffset++;
	//yOffset++;
	// Renders all the texture currently in memory
	for (int a = 0; a < VIDEO_COUNT; a++) {
		// Offscreen :)
		if (inBounds(pfpRects[a].projectedRect)) {
			SDL_RenderTexture(renderer, pfpImgs[a], NULL, &(pfpRects[a].projectedRect));
		}

		// Offscreen :)
		if (inBounds(rectArray[a].projectedRect)) {
			SDL_RenderTexture(renderer, thumbnailImgs[a], NULL, &(rectArray[a].projectedRect));
		}
		
	}

	SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);

	for (int a = 0; a < difficultyCount; a++) {
		SDL_RenderRect(renderer, &(difficultyRects[a].projectedRect));

		SDL_RenderRect(renderer, &(buttonDifficulty[a].projectedRect));
	}

	drawLogo(logoRect.projectedRect.x, logoRect.projectedRect.y, logoRect.projectedRect.w);

	// The Start and Quit Buttons
	drawSmoothRectagle(startLogo.projectedRect, 100, 27, 0, SDL_ALPHA_OPAQUE, startLogo.projectedRect.w / 6);
	drawSmoothRectagle(quitLogo.projectedRect, 100, 27, 0, SDL_ALPHA_OPAQUE, quitLogo.projectedRect.w / 6);
	int x, y;
	displayText(startLogo.projectedRect, startTxt, &x, &y);
	displayText(quitLogo.projectedRect, quitTxt, &x, &y);
	return state;
}