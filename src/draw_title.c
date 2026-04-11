#include "draw_title.h"

/*Handles everything for the xPos array, which keeps track of the positions
of the thumbnails which are all offset every frame. Will also change the width and
height if the screen size ever changes.*/
float handleXPos(float* realPOS, float* projectedPOS, float wrapPoint, float w, float* h, float rectW) {

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
			realPOS[a] = -(rectW * 2) + (w / 2 * (a % (VIDEO_COUNT / LEVEL_COUNT)));
			projectedPOS[a] = realPOS[a];
		}
	}

	// If not, we offset.
	else {
		for (int a = 0; a < VIDEO_COUNT; a++) {
			realPOS[a] -= 4;
			if (projectedPOS[a] + rectW < -wrapPoint) {
				// This is the right most position the rect can be
				realPOS[a] = -(rectW * 2) + (rectW * 2 * (VIDEO_COUNT / LEVEL_COUNT - 1));
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

/*Given the x, y, w, and h, returns a projectedObject which has both the realRect and the projectedRect
Start out the same, but in zooms projected changes while real stays the same*/
ProjectedObject createProjectedObject(float x, float y, float w, float h, bool centered) {
	ProjectedObject newObj;
	
	newObj.realRect = createRect(x, y, w, h, centered);
	newObj.projectedRect = createRect(x, y, w, h, centered);

	return newObj;
}

/*Responsible for drawing the entire title section of the
game.*/
int drawTitle(int state) {
	static SDL_Texture** pfpImgs = NULL;
	static SDL_Texture** thumbnailImgs = NULL;

	static TTF_Text* startTxt = NULL;
	static TTF_Text* quitTxt = NULL;
	static TTF_Text* backTxt = NULL;
	static TTF_Text* selectTxt = NULL;
	static TTF_Font* firstStopFont = NULL;

	static TTF_Text** explanationTxt = NULL;

	static float xPos[2][VIDEO_COUNT];
	static Vector2D vectorFromLogo[VIDEO_COUNT * 2];
	static float w, h = 0;

	static ProjectedObject startLogo, quitLogo, backLogo;

	SDL_Color wineColor = { 100, 27, 0, SDL_ALPHA_OPAQUE };
	SDL_Color greenColor = { 0, 83, 10, SDL_ALPHA_OPAQUE };

	int imgCount = 1;
	int filesNum = 0;
	static char** files;
	static bool isDiff = false;

	static ProjectedObject logoRect;
	static ProjectedObject explanationRect;

	static ProjectedObject* rectArray;
	static ProjectedObject* pfpRects;
	static ProjectedObject* difficultyRects;
	static ProjectedObject* buttonDifficulty;

	static char** thumbnailFiles;
	float movePOS[] = { screen->w * 2, screen->h / 2 };

	static float logoPOS[2];
	const float firstStopRatio = 3.03f;
	const int firstStopDistanceX = 3658;
	const int firstStopDistanceY = -1886;
	
	static float screenWrap = 0;

	// The point where the thumbnails will wrap around
	if (screenWrap == 0) {
		screenWrap = firstStopDistanceX;
	}

	float expOffset = screen->h * 7 / 4;

	const int difficultyCount = 8;
	const int frames = FRAME_RATE * 3 / 2;

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


		// Font for left side
		firstStopFont = TTF_CopyFont(smallFont);
		TTF_SetFontSize(firstStopFont, TTF_GetFontSize(smallFont) * firstStopRatio);

		startTxt = TTF_CreateText(textEngine, smallFont, "Start", strlen("Start"));
		quitTxt = TTF_CreateText(textEngine, smallFont, "Quit", strlen("Quit"));
		selectTxt = TTF_CreateText(textEngine, firstStopFont, "Select", strlen("Select"));
		backTxt = TTF_CreateText(textEngine, firstStopFont, "Back", strlen("Back"));

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
		buttonDifficulty = malloc(sizeof(ProjectedObject) * difficultyCount);

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
		logoRect = createProjectedObject(logoPOS[0], logoPOS[1], w / 2, w / 2 * 9 / 16, false);
		startLogo = createProjectedObject(w / 2 - (w / 8), h * 7 / 8, w / 6, h / 6, true);
		quitLogo = createProjectedObject(w / 2 + (w / 8), h * 7 / 8, w / 6, h / 6, true);
		backLogo = createProjectedObject(w / 2 - (w / 8) - firstStopDistanceX, h * 7 / 8 - firstStopDistanceY, w / 6 * firstStopRatio, h / 6 * firstStopRatio, true);
		
		
		float rectW = w / 4;
		float rectH = rectW * 9 / 16;

		w = handleXPos(xPos[0], xPos[1], screenWrap, w, &h, rectW);

		// These are thumbnails/video floating around
		for (int a = 0; a < VIDEO_COUNT; a++) {
			float y = h / 4 + (h / 2 * ((int)((float)a / VIDEO_COUNT * LEVEL_COUNT)));
			rectArray[a] = createProjectedObject(xPos[0][a], y, rectW, rectH, true);

			float pfpY = rectArray[a].realRect.y + rectArray[a].realRect.h + (h / 16);

			pfpRects[a]= createProjectedObject(rectArray[a].realRect.x + (h / 16), pfpY, h / 16, h / 16, true);
		}

		// The difficulty select assets
		for (int a = 0; a < difficultyCount; a++) {

			float width = w / 6 * firstStopRatio;
			float x = w * 3 / 10 + (w / 3 * (a % 2)) - ((firstStopDistanceX));
			float y =  (h * 5 / 8 * ((int)((float)a / difficultyCount * ceil(difficultyCount / 2.0f))) - firstStopDistanceY);
			float height = width * 9 / 16;

			difficultyRects[a] = createProjectedObject(x, y, width, height, true);

			float buttonX = difficultyRects[a].realRect.x + (difficultyRects[a].realRect.w + w / 8) / firstStopRatio;
			float buttonY = difficultyRects[a].realRect.y + difficultyRects[a].realRect.h / firstStopRatio;

			buttonDifficulty[a] = createProjectedObject(buttonX, buttonY, width / 2, height / 2, false);
		}

		// Explanation is formatted with the difficulties
		float expX = (difficultyRects[1].realRect.x + (w / 3 * 2) + (w / 8) - firstStopDistanceX) / firstStopRatio;
		float expY = (difficultyRects[1].realRect.y - firstStopDistanceY + expOffset) / firstStopRatio;
		//printf("%f %f\n", expX, expY);
		explanationRect = createProjectedObject(expX, expY, (w / 4) * firstStopRatio, h * 3 / 4 * firstStopRatio, false);

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
		free(buttonDifficulty);
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
	static float xDifference;
	static float yDifference;
	static float allXDifference[FRAME_RATE * 3 / 2];

	if (count < frames && state == titleAni) {
		if (!isDiff) {
			xDifference = ((movePOS[0]) - logoRect.projectedRect.x) / frames;
	/*		printf("%f\n", xDifference);*/
			yDifference = (movePOS[1] - logoRect.projectedRect.y) / frames;
			allXDifference[count] = xDifference;
		}
		else {
			xDifference = -allXDifference[count];
		}

		float newX = logoRect.projectedRect.x + (xDifference);
		float newY = logoRect.projectedRect.y + (yDifference);

		float oldH = logoRect.projectedRect.h;

		logoRect.projectedRect = zoom(logoRect.realRect.x, logoRect.realRect.y, newX, newY, logoRect.realRect.w, logoRect.realRect.h);

		//yDifference += logoRectProjection.h - oldH;

		// We move this for real cause u can see it at start otherwise
		if (!isDiff) {
			explanationRect.realRect.y -= expOffset / frames / 2;
		}
		else {
			explanationRect.realRect.y += expOffset / frames / 2;
		}

		startLogo = projectRect(startLogo, xDifference, yDifference);
		quitLogo = projectRect(quitLogo, xDifference, yDifference);
		explanationRect = projectRect(explanationRect, xDifference, yDifference);

		/*zoomOutTxt(changingSmallFont, logoRect.realRect.x, logoRect.projectedRect.x + xDifference);*/


		screenWrap += xDifference;
		count++;
		/*count++;*/
		// We are where we want to be
		/*if (count == frames) {
			logoRect.x = movePOS[0];
			logoRect.y = movePOS[1];
		}	*/


	}
	else {
		/*xDifference = 0;
		yDifference = 0;*/
		// Animation is done :)
		if (state == titleAni) {
			if (!isDiff) state = titleDiff;
			else state = title;

			isDiff = !isDiff;
		}

		count = 0;
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
		/*printf("%f %f %f\n", logoRect.realRect.w / logoRect.projectedRect.w, logoRect.projectedRect.x - logoRect.realRect.x, logoRect.projectedRect.y - logoRect.realRect.y);*/
	}

	w = handleXPos(xPos[0], xPos[1], screenWrap, w, &h, rectW);

	// This is gonna scale the videos/pfps based on the logo's movement
	for (int a = 0; a < VIDEO_COUNT; a++) {
		float realXShift = xPos[0][a] - rectArray[a].realRect.x;

		// xPos changes are real, not projected changes
		rectArray[a].realRect.x = xPos[0][a];
		pfpRects[a].realRect.x += realXShift;

		if (state == titleAni) {
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

	int x, y;

	for (int a = 0; a < difficultyCount; a++) {
		// Final entry is a logo 
		if (a != 7) {
			SDL_RenderRect(renderer, &(difficultyRects[a].projectedRect));
			drawSmoothRectagle(buttonDifficulty[a].projectedRect, wineColor.r, wineColor.g, wineColor.b, wineColor.a, buttonDifficulty[a].projectedRect.w / 6);
			displayTextAsSurface(buttonDifficulty[a], selectTxt);
		}
		else {
			drawLogo((difficultyRects[a].projectedRect.x), (difficultyRects[a].projectedRect.y), (difficultyRects[a].projectedRect.w), false);
			// Giving it a different color so it stands out
			drawSmoothRectagle(buttonDifficulty[a].projectedRect, greenColor.r, greenColor.g, greenColor.b, greenColor.a, buttonDifficulty[a].projectedRect.w / 6);
			displayTextAsSurface(buttonDifficulty[a], backTxt);
		}
	}

	drawLogo(logoRect.projectedRect.x, logoRect.projectedRect.y, logoRect.projectedRect.w, true);

	// The Start and Quit Buttons (smooth rect handles out of bounds internally)
	drawSmoothRectagle(startLogo.projectedRect, wineColor.r, wineColor.g, wineColor.b, wineColor.a, startLogo.projectedRect.w / 6);
	drawSmoothRectagle(quitLogo.projectedRect, wineColor.r, wineColor.g, wineColor.b, wineColor.a, quitLogo.projectedRect.w / 6);
	drawSmoothRectagle(explanationRect.projectedRect, wineColor.r, wineColor.g, wineColor.b, wineColor.a, explanationRect.projectedRect.w / 6);
	/*displayText(startLogo.projectedRect, startTxt, &x, &y);
	displayText(quitLogo.projectedRect, quitTxt, &x, &y);*/

	displayTextAsSurface(startLogo, startTxt);
	displayTextAsSurface(quitLogo, quitTxt);

	// Button that takes us back to the main menu from the difficulty select
	diffToTitle = buttonDifficulty[7].projectedRect;
	return state;
}