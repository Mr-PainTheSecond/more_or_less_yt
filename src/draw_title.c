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

/*Responsible for drawing the entire title section of the
game.*/
int drawTitle(int state) {
	static SDL_Texture** pfpImgs = NULL;
	static SDL_FRect logoRect, logoRectProjection;
	static SDL_Texture** thumbnailImgs = NULL;
	static TTF_Font* startTxt = NULL;
	static TTF_Font* quitTxt = NULL;
	static float xPos[2][VIDEO_COUNT];
	static Vector2D vectorFromLogo[VIDEO_COUNT * 2];
	static float w, h = 0;
	static SDL_FRect startLogo, quitLogo;
	static SDL_FRect startLogoProjection, quitLogoProjection;
	int imgCount = 1;
	int filesNum = 0;
	static char** files;

	static SDL_FRect* rectArray;
	static SDL_FRect* rectArrayProjection;

	static SDL_FRect* pfpRects;
	static SDL_FRect* pfpRectsProjection;

	static char** thumbnailFiles;
	float movePOS[] = { screen->w * 3 / 4, screen->h / 2 };

	static float logoPOS[2];

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
		rectArrayProjection = (SDL_FRect*)malloc(sizeof(SDL_FRect) * VIDEO_COUNT);
		if (rectArray == NULL || rectArrayProjection == NULL) {
			fprintf(stderr, "%s\n", "Allocation for rect array failed");
			quit(ytQueue);
			exit(1);
		}

		pfpRects = malloc(sizeof(SDL_FRect) * VIDEO_COUNT);
		pfpRectsProjection = (SDL_FRect*)malloc(sizeof(SDL_FRect) * VIDEO_COUNT);
		if (pfpRects == NULL || pfpRectsProjection == NULL) {
			fprintf(stderr, "%s\n", "Allocation for rect array failed");
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
		logoRect = createRect(logoPOS[0], logoPOS[1], w / 2, w / 2 * 9 / 16, false);
		startLogo = createRect(w / 2 - (w / 8), h * 7 / 8, w / 6, h / 6, true);
		quitLogo = createRect(w / 2 + (w / 8), h * 7 / 8, w / 6, h / 6, true);
		
		logoRectProjection = createRect(logoPOS[0], logoPOS[1], w / 2, w / 2 * 9 / 16, false);
		startLogoProjection = createRect(w / 2 - (w / 8), h * 7 / 8, w / 6, h / 6, true);
		quitLogoProjection = createRect(w / 2 + (w / 8), h * 7 / 8, w / 6, h / 6, true);

	

		float rectW = w / 4;
		float rectH = rectW * 9 / 16;

		w = handleXPos(xPos[0], xPos[1], w, &h, rectW);

		for (int a = 0; a < VIDEO_COUNT; a++) {
			float y = h / 4 + (h / 2 * ((int)((float)a / VIDEO_COUNT * 2)));
			rectArray[a] = createRect(xPos[0][a], y, rectW, rectH, true);
			rectArrayProjection[a] = createRect(xPos[0][a], y, rectW, rectH, true);

			float pfpY = rectArray[a].y + rectArray[a].h + (h / 16);

			pfpRects[a] = createRect(rectArray[a].x + (h / 16), pfpY, h / 16, h / 16, true);
			pfpRectsProjection[a] = createRect(rectArray[a].x + (h / 16), pfpY, h / 16, h / 16, true);
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
		free(rectArrayProjection);
		free(pfpRects);
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
	static int frames = FRAME_RATE / 2;
	static int count = 0;
	float xDifference;
	float yDifference;

	if (count < frames) {
		xDifference = ((movePOS[0]) - logoRectProjection.x) / frames;
		yDifference = (movePOS[1] - logoRectProjection.y) / frames;

		float newX = logoRectProjection.x + (xDifference);
		float newY = logoRectProjection.y + (yDifference);

		logoRectProjection = zoom(logoRect.x, logoRect.y, newX, newY, logoRect.w, logoRect.h);

		Vector2D startVector = { (startLogoProjection.x + (xDifference)) ,
		(startLogoProjection.y + yDifference) };
		Vector2D quitVector = { quitLogoProjection.x + xDifference,
			quitLogoProjection.y + yDifference };

		startLogoProjection = zoom(startLogo.x, startLogo.y, startVector.x, startVector.y, startLogo.w, startLogo.h);
		quitLogoProjection = zoom(quitLogo.x, quitLogo.y, quitVector.x, quitVector.y, quitLogo.w, quitLogo.h);

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
	}


	static float rectW = 0;
	static float rectH = 0;

	if (rectW == 0) {
		rectW = w / 4;
		rectH = rectW * 9 / 16;
	}
	else {
		rectW = rectArrayProjection[0].w;
		rectH = rectArrayProjection[0].h;
	}

	w = handleXPos(xPos[0], xPos[1], w, &h, rectW);

	// This is gonna scale the videos/pfps based on the logo's movement
	for (int a = 0; a < VIDEO_COUNT; a++) {
		float realXShift = xPos[0][a] - rectArray[a].x;

		// xPos changes are real, not projected changes
		rectArray[a].x = xPos[0][a];
		pfpRects[a].x += realXShift;

		if (a == 0) {
			printf("%f %f\n", pfpRectsProjection[a].x, pfpRectsProjection[a].y);
		}

		float newXRects = rectArrayProjection[a].x + xDifference;
		float newYRects = rectArrayProjection[a].y + yDifference;

		float newXpfp = pfpRectsProjection[a].x + xDifference;
		float newYpfp = pfpRectsProjection[a].y + yDifference;

		if (xDifference != 0 || yDifference != 0) {
			rectArrayProjection[a] = zoom(rectArray[a].x, rectArray[a].y, newXRects, newYRects, rectArray[a].w, rectArray[a].h);
			pfpRectsProjection[a] = zoom(pfpRects[a].x, pfpRects[a].y, newXpfp, newYpfp, pfpRects[a].w, pfpRects[a].h);
		}

		// Shifts in here so they don't count towards projection
		rectArrayProjection[a].x += realXShift;
		pfpRectsProjection[a].x += realXShift;

		xPos[1][a] = rectArrayProjection[a].x;
	}

	//xOffset++;
	//yOffset++;
	SDL_RenderFillRects(renderer, rectArrayProjection, VIDEO_COUNT);
	// Renders all the texture currently in memory
	for (int a = 0; a < VIDEO_COUNT; a++) {
		// Offscreen :)
		if (rectArrayProjection[a].x + rectArrayProjection[a].w < 0 || rectArrayProjection[a].x > screen->w) {
			continue;
		}

		if (!SDL_RenderTexture(renderer, pfpImgs[a], NULL, &(pfpRectsProjection[a]))) {
			fprintf(stderr, "%s\n", SDL_GetError());
			quit(ytQueue);
			exit(1);
		}


		
		SDL_RenderTexture(renderer, thumbnailImgs[a], NULL, &(rectArrayProjection[a]));
	}


	drawLogo(logoRectProjection.x, logoRectProjection.y, logoRectProjection.w);

	// The Start and Quit Buttons
	drawSmoothRectagle(startLogoProjection, 100, 27, 0, SDL_ALPHA_OPAQUE, startLogoProjection.w / 6);
	drawSmoothRectagle(quitLogoProjection, 100, 27, 0, SDL_ALPHA_OPAQUE, quitLogoProjection.w / 6);
	int x, y;
	displayText(startLogoProjection, startTxt, &x, &y);
	displayText(quitLogoProjection, quitTxt, &x, &y);
	return state;
}