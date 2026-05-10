#include "draw_title.h"


/*MAIN FUNCTION*/

/*Responsible for drawing the entire title section of the
game.*/
int drawTitle(int state) {
	static SDL_Texture** pfpImgs = NULL;
	static SDL_Texture** thumbnailImgs = NULL;
	static SDL_Texture* difficultyImgs[DIFFICULTY_COUNT];

	static TTF_Text* startTxt = NULL;
	static TTF_Text* quitTxt = NULL;
	static TTF_Text* backTxt = NULL;
	static TTF_Text* selectTxt = NULL;
	static TTF_Text* playTxt = NULL;
	static TTF_Font* firstStopFont = NULL;

	// Stores the explanation for each difficulty
	static MultiLineText explanationTxt[DIFFICULTY_COUNT + 1];
	static TTF_Text* difficultyNameTxt[DIFFICULTY_COUNT + 1];

	static float xPos[2][VIDEO_COUNT];
	static Vector2D vectorFromLogo[VIDEO_COUNT * 2];
	static float w, h = 0;

	static ProjectedObject startLogo, quitLogo, backLogo;

	SDL_Color wineColor = { 100, 27, 0, SDL_ALPHA_OPAQUE };
	SDL_Color greenColor = { 0, 102, 27, SDL_ALPHA_OPAQUE };
	SDL_Color orangeColor = { 238, 63, 0, SDL_ALPHA_OPAQUE };
	SDL_Color blueColor = { 8, 39, 245, SDL_ALPHA_OPAQUE };
	SDL_Color goldColor = { 255, 233, 0, SDL_ALPHA_OPAQUE };
	SDL_Color whiteColor = { 255, 255, 255, SDL_ALPHA_OPAQUE };

	int imgCount = 1;
	int filesNum = 0;
	static char** files;
	static bool isDiff = false;

	// The Projected Objects helps with the zoom effect
	static ProjectedObject logoRect;
	static ProjectedObject explanationRect;
	static ProjectedObject beginGameLogo;
	static ProjectedObject difficultyName;

	static ProjectedObject* rectArray;
	static ProjectedObject* pfpRects;
	static ProjectedObject* difficultyRects;
	static ProjectedObject* buttonDifficulty;

	static char** thumbnailFiles;
	float movePOS[] = { screen->w * 2, screen->h / 2 };

	static float logoPOS[2];
	static float firstStopRatio;
	static int firstStopDistanceX;
	static int firstStopDistanceY;
	const float yDisToH = 0.787f;
	
	static float screenWrap = -1;

	float expOffset = screen->h * 7 / 4;
	const int selectScreenRects = DIFFICULTY_COUNT + 1;

	const int frames = FRAME_RATE * 3 / 2;

	/*INITIALIZATION*/

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

		logoPOS[0] = screen->w / 2;
		logoPOS[1] = screen->h / 2;

		w = screen->w;
		h = screen->h;

		logoRect = createProjectedObject(w / 2, h / 2, w / 2, w / 2 * 9 / 16, false);
		// Use this to calculate ratio
		for (int a = 0; a < frames; a++) {
			logoRect = projectRect(logoRect, (movePOS[0] - logoRect.projectedRect.x) / frames, 0);
		}

		firstStopRatio = logoRect.realRect.w / logoRect.projectedRect.w;
		firstStopDistanceX = logoRect.projectedRect.x - (logoRect.realRect.x);
		firstStopDistanceY = screen->h * -yDisToH;

		// The point where the thumbnails will wrap around
		screenWrap = firstStopDistanceX;
		
		// Now that math is done, we need to reset the logo to its original position
		logoRect = unprojectObject(logoRect);

		// Font for left side
		firstStopFont = TTF_CopyFont(smallFont);
		TTF_SetFontSize(firstStopFont, TTF_GetFontSize(smallFont) * firstStopRatio);

		startTxt = TTF_CreateText(textEngine, smallFont, "Start", strlen("Start"));
		quitTxt = TTF_CreateText(textEngine, smallFont, "Quit", strlen("Quit"));
		selectTxt = TTF_CreateText(textEngine, firstStopFont, "Select", strlen("Select"));
		backTxt = TTF_CreateText(textEngine, firstStopFont, "Back", strlen("Back"));
		playTxt = TTF_CreateText(textEngine, firstStopFont, "Play", strlen("Play"));

		// We are gonna document what we have found since I/O is a big bottleneck
		SDL_Surface* thumbnails[UNIQUE_VIDEOS];
		SDL_Surface* pfps[UNIQUE_VIDEOS];
		bool found[UNIQUE_VIDEOS];
		for (int a = 0; a < UNIQUE_VIDEOS; a++) {
			found[a] = false;
			thumbnails[a] = NULL;
			pfps[a] = NULL;
		}

		for (int a = 0; a < VIDEO_COUNT; a++) {
			int imgIndex = rand() % filesNum;
			// No need to load it from storage
			if (found[imgIndex]) {
				thumbnailImgs[a] = SDL_CreateTextureFromSurface(renderer, thumbnails[imgIndex]);
				pfpImgs[a] = SDL_CreateTextureFromSurface(renderer, pfps[imgIndex]);
				continue;
			}

			SDL_Surface* pfpSurf = IMG_Load(files[imgIndex]);
			SDL_Surface* thumbnailSurf = IMG_Load(thumbnailFiles[imgIndex]);
			pfpSurf = transformToCircle(pfpSurf);
			pfpImgs[a] = SDL_CreateTextureFromSurface(renderer, pfpSurf);
			thumbnailImgs[a] = SDL_CreateTextureFromSurface(renderer, thumbnailSurf);

			thumbnails[imgIndex] = thumbnailSurf;
			pfps[imgIndex] = pfpSurf;
			found[imgIndex] = true;
		}

		for (int a = 0; a < filesNum; a++) {
			free(files[a]);
			free(thumbnailFiles[a]);
		}

		free(files);
		free(thumbnailFiles);

		for (int a = 0; a < UNIQUE_VIDEOS; a++) {
			SDL_DestroySurface(pfps[a]);
			SDL_DestroySurface(thumbnails[a]);
		}

		char** diffLocations = malloc(sizeof(char*) * DIFFICULTY_COUNT);

		int objs;
		int* items;


		for (int a = 0; a < DIFFICULTY_COUNT; a++) {
			// One for the digit, one for the null terminator
			diffLocations[a] = malloc(sizeof(char) * (strlen("..\\assets\\images\\perm\\other\\difficulty_.PNG")) + (sizeof(char) * 2));

			diffLocations[a] = converToStr(a);
		}

		if (diffLocations == NULL) {
			fprintf(stderr, "%s\n", "Allocation for difficulty imgs failed :(");
			quit(ytQueue);
			exit(1);
		}
		// All the difficulty imgs follow same format
		formatAsFileLocation("..\\assets\\images\\perm\\other\\difficulty_", ".PNG", diffLocations, DIFFICULTY_COUNT);


		for (int a = 0; a < DIFFICULTY_COUNT; a++) {
			SDL_Surface* diffSurf = IMG_Load(diffLocations[a]);
			
			difficultyImgs[a] = SDL_CreateTextureFromSurface(renderer, diffSurf);


			SDL_DestroySurface(diffSurf);
		}
		
		for (int a = 0; a < DIFFICULTY_COUNT; a++) {
			free(diffLocations[a]);
		}

		free(diffLocations);

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

		difficultyRects = malloc(sizeof(ProjectedObject) * selectScreenRects);
		buttonDifficulty = malloc(sizeof(ProjectedObject) * selectScreenRects);

		if (difficultyRects == NULL || buttonDifficulty == NULL) {
			fprintf(stderr, "%s\n", "Allocation for difficulties failed");
			quit(ytQueue);
			exit(1);
		}


		// Good way to signal that they are not initialized
		xPos[0][0] = INT_MAX;

		// Starting values of the logos
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


		float width = w / 6 * firstStopRatio;
		float height = width * 9 / 16;
		// The difficulty select assets
		for (int a = 0; a < selectScreenRects; a++) {
			float x = w * 3 / 10 + (w / 3 * (a % 2)) - ((firstStopDistanceX));
			float y =  (h * 5 / 8 * ((int)((float)a / selectScreenRects * ceil(selectScreenRects / 2.0f))) - firstStopDistanceY);

			difficultyRects[a] = createProjectedObject(x, y, width, height, true);

			float buttonX = difficultyRects[a].realRect.x + (difficultyRects[a].realRect.w + w / 8) / firstStopRatio;
			float buttonY = difficultyRects[a].realRect.y + difficultyRects[a].realRect.h / firstStopRatio;

			buttonDifficulty[a] = createProjectedObject(buttonX, buttonY, width / 2, height / 2, false);
		}

		// Display name of selected difficulty
		float diffNameX = difficultyRects[selectScreenRects - 1].realRect.x - (width * 2.0f / firstStopRatio / 2) - (width / 8);
		float diffNameY = difficultyRects[selectScreenRects - 1].realRect.y - (h * 3 / 4.0f * firstStopRatio) - expOffset;

		difficultyName = createProjectedObject(diffNameX, diffNameY, width * 2.0f, height * 3 / 4.0f, false);

		// Explanation is formatted with the difficulties
		float expX = (difficultyRects[1].realRect.x + (w * 17 / 8) / firstStopRatio);
		float expY = (difficultyRects[1].realRect.y  + (h * 7 / 2) - firstStopDistanceY + expOffset) / firstStopRatio;
		//printf("%f %f\n", expX, expY);
		explanationRect = createProjectedObject(expX, expY, (w / 4) * firstStopRatio, h * 3 / 4 * firstStopRatio, true);

		float beginX = explanationRect.realRect.x +  ((w / 8) / firstStopRatio);
		float beginY = explanationRect.realRect.y + (explanationRect.realRect.h) + (h / 16);
		beginGameLogo = createProjectedObject(beginX, beginY, width, height, false);

		char*** jsonData = readJSONArray("..\\assets\\data\\description.json", "descriptions", &objs, &items);

		createExplanationTxt(explanationTxt, firstStopFont, jsonData, objs, explanationRect.realRect);

		int titleLocation = 1;
		int lineCountLocation = 2;

		TTF_Font** newFonts = malloc(sizeof(TTF_Font*) * (DIFFICULTY_COUNT + 1));
		if (newFonts == NULL) {
			errorExit("Allocation for the new fonts failed");
		}

		for (int a = 0; a <= DIFFICULTY_COUNT; a++) {
			char* diffName = jsonData[a][titleLocation];
			int lineCount = convertToInt(jsonData[a][lineCountLocation]);
			// JSON is formatted as lines -> first -> hasFont
			int hasNewFont = convertToInt(jsonData[a][lineCountLocation + 2 + lineCount]);
			if (hasNewFont) {
				// Font location is after font count
				char* fontLocation = jsonData[a][lineCountLocation + 3 + lineCount];
				// Some fonts need to be made bigger, while others smaller
				float scaleFactor = convertToFloat(jsonData[a][lineCountLocation + 7 + lineCount]);
				printf("Index %d Font location %s\n", a, fontLocation);
				newFonts[a] = createFont(fontLocation, TTF_GetFontSize(firstStopFont) * scaleFactor);
			}
			else {
				newFonts[a] = copyFont(firstStopFont);
			}
			difficultyNameTxt[a] = TTF_CreateText(textEngine, newFonts[a], diffName, strlen(diffName));
			if (hasNewFont) {
				// After font location, it has the following value of the font color
				int red = convertToInt(jsonData[a][lineCountLocation + 4 + lineCount]);
				int green = convertToInt(jsonData[a][lineCountLocation + 5 + lineCount]);
				int blue = convertToInt(jsonData[a][lineCountLocation + 6 + lineCount]);

				TTF_SetTextColor(difficultyNameTxt[a], red, green, blue, SDL_ALPHA_OPAQUE);
			}
		}
		
		free(newFonts);
		freeJSONArray(jsonData, objs, items);
	}

	/*RESET STATE*/

	// Turns the projected rects back to their real size
	if (state == titleToNormal) {
		startLogo = unprojectObject(startLogo);
		quitLogo = unprojectObject(quitLogo);
		backLogo = unprojectObject(backLogo);
		logoRect = unprojectObject(logoRect);

		// Also gotta undo real movements for these ones
		explanationRect.realRect.y += expOffset / 2;
		beginGameLogo.realRect.y += expOffset / 2;
		difficultyName.realRect.y -= expOffset;

		// Important: Undo real movement first then unproject (can't do other way around)
		explanationRect = unprojectObject(explanationRect);
		beginGameLogo = unprojectObject(beginGameLogo);
		difficultyName = unprojectObject(difficultyName);

		// The rare i lol
		for (int i = 0; i < VIDEO_COUNT; i++) {
			rectArray[i] = unprojectObject(rectArray[i]);
			pfpRects[i] = unprojectObject(pfpRects[i]);
		}

		for (int i = 0; i < selectScreenRects; i++) {
			difficultyRects[i] = unprojectObject(difficultyRects[i]);
			buttonDifficulty[i] = unprojectObject(buttonDifficulty[i]);
		}

		for (int a = 0; a < selectScreenRects; a++) {
			for (int b = 0; b < explanationTxt[a].lineCount; b++) {
				// Similar to explanationRect
				explanationTxt[a].lineRects[b].realRect.y += expOffset / 2;
				explanationTxt[a].lineRects[b] = unprojectObject(explanationTxt[a].lineRects[b]);
			}
		}

		isDiff = false;
		screenWrap = firstStopDistanceX;

		gameAttr->state = normal;
		return normal;
	}

	/*SHUTDOWN*/
	
	// The transition from title to main game.
	// Destroys all the assets in the meanwhile
	if (state == shutDown) {
		if (pfpImgs != NULL) {
			for (int a = 0; a < VIDEO_COUNT; a++) {
				SDL_DestroyTexture(pfpImgs[a]);
				SDL_DestroyTexture(thumbnailImgs[a]);
			}
		}

		for (int a = 0; a < DIFFICULTY_COUNT; a++) {
			for (int b = 0; b < explanationTxt[a].lineCount; b++) {
				TTF_DestroyText(explanationTxt[a].lines[b]);
			}

			TTF_DestroyText(difficultyNameTxt[a]);		
			free(explanationTxt[a].lineRects);
			free(explanationTxt[a].lines);
		}

		// Get the last since loop doesn't cover it
		TTF_DestroyText(difficultyNameTxt[DIFFICULTY_COUNT]);
		TTF_DestroyText(startTxt);
		TTF_DestroyText(quitTxt);
		TTF_DestroyText(selectTxt);
		TTF_DestroyText(backTxt);	

		TTF_CloseFont(firstStopFont);
		
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
		// Zoom out just undoes what the zoom in did
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
			beginGameLogo.realRect.y -= expOffset / frames / 2;
			difficultyName.realRect.y += expOffset / frames;
		}
		else {
			explanationRect.realRect.y += expOffset / frames / 2;
			beginGameLogo.realRect.y += expOffset / frames / 2;
			difficultyName.realRect.y -= expOffset / frames;
		}

		startLogo = projectRect(startLogo, xDifference, yDifference);
		quitLogo = projectRect(quitLogo, xDifference, yDifference);
		explanationRect = projectRect(explanationRect, xDifference, yDifference);
		beginGameLogo = projectRect(beginGameLogo, xDifference, yDifference);
		difficultyName = projectRect(difficultyName, xDifference, yDifference);
		/*zoomOutTxt(changingSmallFont, logoRect.realRect.x, logoRect.projectedRect.x + xDifference);*/


		screenWrap += xDifference;
		count++;
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

	int haltCond;
	// If transition, difficulty assets are also affected, if not only videos matter
	if (state == titleAni) {
		haltCond = max(VIDEO_COUNT, selectScreenRects);
	}
	else {
		haltCond = VIDEO_COUNT;
	}


	/*ARRAY PROJECTION*/
	// This is gonna scale the videos/pfps based on the logo's movement
	for (int a = 0; a < haltCond; a++) {
		float realXShift = xPos[0][a] - rectArray[a].realRect.x;

		// xPos changes are real, not projected changes
		rectArray[a].realRect.x = xPos[0][a];
		pfpRects[a].realRect.x += realXShift;

		// Need to calculate the new y, w, and h based on the logo's movement
		if (state == titleAni) {
			if (a < VIDEO_COUNT) {
				rectArray[a] = projectRect(rectArray[a], xDifference, yDifference);
				pfpRects[a] = projectRect(pfpRects[a], xDifference, yDifference);
			}
			
			if (a < selectScreenRects) {
				for (int b = 0; b < explanationTxt[a].lineCount; b++) {
					if (!isDiff) {
						explanationTxt[a].lineRects[b].realRect.y -= expOffset / frames / 2;
					}
					else {
						explanationTxt[a].lineRects[b].realRect.y += expOffset / frames / 2;
					}

					explanationTxt[a].lineRects[b] = projectRect(explanationTxt[a].lineRects[b], xDifference, yDifference);
					
				}	
			}

			if (a < selectScreenRects) {
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


	/*RENDERING*/

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

	for (int a = 0; a < selectScreenRects; a++) {
		// Final entry is a logo 
		if (a != selectScreenRects - 1) {
			if (inBounds(difficultyRects[a].projectedRect)) {
				SDL_RenderTexture(renderer, difficultyImgs[a], NULL, &(difficultyRects[a].projectedRect));	
			}

			if (inBounds(buttonDifficulty[a].projectedRect)) {
				drawSmoothRectagle(buttonDifficulty[a].projectedRect, wineColor.r, wineColor.g, wineColor.b, wineColor.a, buttonDifficulty[a].projectedRect.w / 6);
				if (gameAttr->difficulty == a) {
					// Makes it a little clearer that this is the selected difficulty
					TTF_SetTextColor(selectTxt, goldColor.r, goldColor.g, goldColor.b, goldColor.a);
				} else {
					TTF_SetTextColor(selectTxt, whiteColor.r, whiteColor.g, whiteColor.b, whiteColor.a);
				}
				displayTextAsSurface(buttonDifficulty[a], selectTxt);
			}
		}
		// All have in bound checks interally
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
	drawSmoothRectagle(difficultyName.projectedRect, wineColor.r, wineColor.g, wineColor.b, wineColor.a, difficultyName.projectedRect.w / 6);
	/*displayText(startLogo.projectedRect, startTxt, &x, &y);
	displayText(quitLogo.projectedRect, quitTxt, &x, &y);*/

	displayTextAsSurface(startLogo, startTxt);
	displayTextAsSurface(quitLogo, quitTxt);
	// Explanins how currently selected difficulty works
	if (gameAttr->difficulty != -1 && inBounds(explanationRect.projectedRect)) {
		for (int a = 0; a < explanationTxt[gameAttr->difficulty].lineCount; a++) {
			displayTextAsSurface(explanationTxt[gameAttr->difficulty].lineRects[a], explanationTxt[gameAttr->difficulty].lines[a]);
		}
	}
	// This prompts user to select a difficulty
	else if (inBounds(explanationRect.projectedRect)) {
		for (int a = 0; a < explanationTxt[selectScreenRects - 1].lineCount; a++) {
			displayTextAsSurface(explanationTxt[selectScreenRects - 1].lineRects[a], explanationTxt[selectScreenRects - 1].lines[a]);
		}
	}
	
	// Can only start once difficulty selected
	if (difficulty != -1)  {
		drawSmoothRectagle(beginGameLogo.projectedRect, blueColor.r, blueColor.g, blueColor.b, blueColor.a, beginGameLogo.projectedRect.w / 6);
		displayTextAsSurface(beginGameLogo, playTxt);
		displayTextAsSurface(difficultyName, difficultyNameTxt[difficulty]);
	}
	else {
		// Title when no difficulty selected
		displayTextAsSurface(difficultyName, difficultyNameTxt[DIFFICULTY_COUNT]);
	}

	// Button that takes us back to the main menu from the difficulty select
	diffToTitle = buttonDifficulty[selectScreenRects - 1].projectedRect;
	diffToPlay = beginGameLogo.projectedRect;
	diffSelect = buttonDifficulty;
	return state;
}