#include "draw_game.h"


/*Handles any change which occur to the text. This would happen if we are either calling this function
for the first time, the user recently submitted their guess, or the transitions
animations finished*/
int handleTextChange(DynamicText* views, DynamicText* txtScore, DynamicText* mysTxt, DynamicText** subsTxts, int score) {
	static int data;
	static int lastScore = 0;
	// Reveals the view count during the transition phase.
	if (gameAttr->score != lastScore) {
		// The Score is incremented by one
		if (gameAttr->score >= 0) {
			lastScore = gameAttr->score;
		}
		else {
			// The score stays the same
			gameAttr->score = lastScore;
		}
		YTNode* revealedView = ytQueue->front->next;
		TTF_DestroyText(mysTxt->text);
		mysTxt->str = format(revealedView->sViews, " Views");
		mysTxt->text = TTF_CreateText(textEngine, smallFont, mysTxt->str, strlen(mysTxt->str) + 1);
		free(txtScore->str);
		// log10 can be used to figure out how many digits a positive integer has.
		// Since 0 is not positive, it is a special case
		if (gameAttr->score != 0) {
			txtScore->str = malloc(sizeof(char) * (int)log10((gameAttr->score)) + 2);
			if (txtScore->str == NULL) {
				fprintf(stderr, "%s\n", "Alloc for txtScore String failed");
				quit(ytQueue);
				exit(1);
			}
			int temp = gameAttr->score;
			// Extracts each digit from a number, and turns it into a char
			for (int a = log10(gameAttr->score); a >= 0; a--) {
				int digit = (int)(temp / pow(10, a)) % 10;
				txtScore->str[(int)log10(gameAttr->score) - a] = (int)(digit + 48);
				temp -= pow(10, a) * digit;
			}

			txtScore->str[(int)log10(gameAttr->score) + 1] = '\0';
			TTF_DestroyText(txtScore->text);
			txtScore->text = TTF_CreateText(textEngine, smallFont, txtScore->str, strlen(txtScore->str) + 1);
		}
		else {
			// Edge case for 0, nothing need to be calculated really
			txtScore->str = malloc(sizeof(char) * 2);
			if (txtScore->str == NULL) {
				fprintf(stderr, "%s\n", "Alloc for txtScore String failed");
				quit(ytQueue);
				exit(1);
			}
			TTF_DestroyText(txtScore->text);
			txtScore->text = TTF_CreateText(textEngine, smallFont, "0", strlen("0") + 1);
		}
	}

	// Signifies this is out first iteration
	if (views->text == NULL) {
		txtScore->str = malloc(sizeof(char) * 2);
		if (txtScore->str == NULL) {
			fprintf(stderr, "%s\n", "Alloc for txtScore String failed");
			quit(ytQueue);
			exit(1);
		}
		views->str = format(ytQueue->front->sViews, " Views");
		views->text = TTF_CreateText(textEngine, smallFont, views->str, strlen(views->str) + 1);

		YTNode* currentNode = ytQueue->front;
		for (int a = 0; a < 3; a++) {
			subsTxts[a]->str = format(currentNode->subs, " Subs");
			subsTxts[a]->text = TTF_CreateText(textEngine, ytFont, subsTxts[a]->str, strlen(subsTxts[a]->str));
			currentNode = currentNode->next;
		}
		// We always start at 0, mysTxt always starts at 
		txtScore->text = TTF_CreateText(textEngine, smallFont, "0", strlen("0") + 1);
		mysTxt->text = TTF_CreateText(textEngine, smallFont, "???", strlen("???") + 1);
		data = ytQueue->front->views;
	}
	// Signifies that the transition is over, and so we need to change text accordingly
	if (data != ytQueue->front->views) {
		free(views->str);
		views->str = format(ytQueue->front->sViews, " Views");
		TTF_DestroyText(views->text);
		TTF_DestroyText(mysTxt->text);
		views->text = TTF_CreateText(textEngine, smallFont, views->str, strlen(views->str) + 1);
		mysTxt->text = TTF_CreateText(textEngine, smallFont, "???", strlen("???") + 1);
		if (views->text == NULL || mysTxt->text == NULL) {
			fprintf(stderr, "%s\n", "Creation of mysTxt/view TTF txt failed");
			quit(ytQueue);
			exit(1);
		}


		// All elements go down by one
		copyDymTxt(subsTxts[0], subsTxts[1]); // Takes care of erasing leftmost
		copyDymTxt(subsTxts[1], subsTxts[2]);

		YTNode* node = ytQueue->front;
		// For some reason doing this in one line is not allowed
		node = node->next;
		node = node->next;
		subsTxts[2]->str = format(node->subs, " Subs");
		subsTxts[2]->text = TTF_CreateText(textEngine, ytFont, subsTxts[2]->str, strlen(subsTxts[2]->str));
		data = ytQueue->front->views;
	}

	// Useless, but wtv
	return data;
}


void moreLessTxt(TTF_Text* moreTxt, SDL_FRect moreRect, TTF_Text* lessTxt, SDL_FRect lessRect, int state) {
	if (state == moreRight || state == moreWrong) {
		TTF_SetTextColor(moreTxt, 225, 215, 0, SDL_ALPHA_OPAQUE);
	}
	else if (state == lessRight || state == lessWrong) {
		TTF_SetTextColor(lessTxt, 225, 215, 0, SDL_ALPHA_OPAQUE);
	}
	else {
		TTF_SetTextColor(moreTxt, 255, 255, 255, SDL_ALPHA_OPAQUE);
		TTF_SetTextColor(lessTxt, 255, 255, 255, SDL_ALPHA_OPAQUE);
	}

	// Pointers neeeded for the display function
	int x = 0;
	int y = 0;
	displayText(lessRect, lessTxt, &x, &y);
	displayText(moreRect, moreTxt, &x, &y);
}

/*Draw the main component of the more or less game.*/
int drawMoreOrLess(TTF_Text* moreTxt, TTF_Text* lessTxt, Queue* queue) {
	static DynamicText* views = NULL;
	static DynamicText* txtScore = NULL;
	static DynamicText* mysteryText = NULL;
	static TTF_Text* nextTxt = NULL;
	static DynamicText** subsTxts = NULL;
	static SDL_Texture* mrBeastHeart = NULL;
	static SDL_Texture* deadBeast = NULL;
	// The array of positions of Mr. Beast
	static SDL_FRect* mrBeastPos = NULL;
	static int heartCount;
	// The offset for screen transition
	static float delX = 0;
	static float delY = -1;
	static int data;

	// The state that will remove all the elements from here
	if (gameAttr->state >= justQuit && gameAttr->state <= justWon || gameAttr->state == shutDown) {
		if (views != NULL) {
			// We have to free/reset everything
			destroyDynamicText(views);
			destroyDynamicText(txtScore);
			destroyDynamicText(mysteryText);

			for (int a = 0; a < 3; a++) {
				destroyDynamicText(subsTxts[a]);
			}

			free(subsTxts);
			free(mrBeastPos);

			// RIP to the beasts
			SDL_DestroyTexture(mrBeastHeart);
			SDL_DestroyTexture(deadBeast);

			TTF_DestroyText(nextTxt);
			delX = 0;

			// Dequeue twice so the elements don't repeat w/ new game
			deQueue(ytQueue, ytQueue->front->next);
			deQueue(ytQueue, ytQueue->front->next);

		}

		// Signifies that we will need to remake everything
		views = NULL;
		nextTxt = NULL;
		gameAttr->score = 0;
		if (gameAttr->state == justWon) {
			gameAttr->state = gameWon;
		}
		else if (gameAttr->state == justLost) {
			gameAttr->state = gameOver;	
		}
		else {
			gameAttr->state = title;
		}
		gameAttr->health = 3;
		return gameAttr->state;
	}

	// Create text for first round
	if (views == NULL) {
		views = createDynamicText("", NULL);
		txtScore = createDynamicText("", NULL);
		mysteryText = createDynamicText("", NULL);
		subsTxts = malloc(sizeof(DynamicText*) * 3);

		if (subsTxts == NULL) {
			fprintf(stderr, "%s\n", "Failled to alloc for subsTxts");
			quit(ytQueue);
			exit(1);
		}

		for (int a = 0; a < 3; a++) {
			subsTxts[a] = createDynamicText("", NULL);
		}

		// Load the Mr Beast img int memory
		SDL_Surface* beastSurf = IMG_Load("..\\assets\\images\\perm\\other\\mr_beast_life.PNG");
		beastSurf = transformToCircle(beastSurf);
		// Normal Version
		mrBeastHeart = SDL_CreateTextureFromSurface(renderer, beastSurf);
		beastSurf = setSurfGrayScale(beastSurf);
		// Lost life version
		deadBeast = SDL_CreateTextureFromSurface(renderer, beastSurf);
		SDL_DestroySurface(beastSurf);

		// Later difficulties remove one heart.
		if (difficulty >= lessHeart) {
			mrBeastPos = malloc(sizeof(SDL_FRect) * 2);
			heartCount = 2;
		}
		else {
			mrBeastPos = malloc(sizeof(SDL_FRect) * 3);
			heartCount = 3;
		}

		if (mrBeastPos == NULL) {
			fprintf(stderr, "%s\n", "Alloc for mrBeast postions have failed");
			quit(ytQueue);
			exit(1);
		}
	}

	if (gameAttr->state == normal && delX != 0) {
		delX = 0;
		deQueue(queue, queue->front->next);
	}

	data = handleTextChange(views, txtScore, mysteryText, subsTxts, gameAttr->score);

	if (nextTxt == NULL) {
		nextTxt = TTF_CreateText(textEngine, smallFont, "???", strlen("???") + 1);
		int w, h;
		TTF_GetTextSize(nextTxt, &w, &h);
	}


	float w = screen->w;
	float h = screen->h;



	// Create every relevant rect. delX will handle movement when user makes guess
	SDL_FRect viewRect = createRect(w / 4 + delX, h * 3 / 4, w / 4, h / 8, true);
	// WE want this rect to be relative to viewRect, so it ain't centered
	// Since it is anchored to viewRect, no offset needed
	SDL_FRect subRect = createRect(viewRect.x, h * 5 / 6, w / 8, h / 16, false);
	SDL_FRect mysteryRect = createRect(w * 3 / 4 + delX, h * 3 / 4, w / 4, h / 8, true);
	SDL_FRect subMysRect = createRect(mysteryRect.x, h * 5 / 6, w / 8, h / 16, false);
	// This is the img with mystery views. Too lazy to change name
	SDL_FRect lessImg = createRect(w * 3 / 4 + delX, h * 7 / 16, w / 4, h / 2, true);
	SDL_FRect less = createRect(w / 2 + delX, h * 10 / 16, w / 4, h / 8, true);
	// This is the img with shown views. Too lazy to change name
	SDL_FRect moreImg = createRect(w / 4 + delX, h * 7 / 16, w / 4, h / 2, true);
	SDL_FRect more = createRect(w / 2 + delX, h / 2, w / 4, h / 8, true);
	SDL_FRect hiddenMore = createRect(w * 5 / 4 + delX, h / 2, w / 4, h / 8, true);
	SDL_FRect hiddenLess = createRect(w * 5 / 4 + delX, h * 10 / 16, w / 4, h / 8, true);
	SDL_FRect hiddenRect = createRect(w * 3 / 2 + delX, h * 3 / 4, w / 4, h / 8, true);
	SDL_FRect hiddenImg = createRect(w * 3 / 2 + delX, h * 7 / 16, w / 4, h / 2, true);
	SDL_FRect subHideRect = createRect(hiddenRect.x, h * 5 / 6, w / 8, h / 16, false);

	SDL_FRect scoreRect = createRect(w / 24, h * 23 / 24, w / 12, h / 12, true);

	for (int a = 0; a < heartCount; a++) {
		float beastX = w / 2 + ((w / 12) * (a - 1));
		mrBeastPos[a] = createRect(beastX, h * 23 / 24, w / 12, h / 12, true);
	}

	int x = 0;
	int y = 0;

	get_text_center(viewRect, views->text, &x, &y);


	if (gameAttr->state != normal) {
		delX -= (center(w * 3 / 4, moreImg.w) - center(viewRect.x, viewRect.w)) / TRANSITION_GUESS_SPEED;
		//printf("%f\n", delX);
		/*These rectangles reach their destination before the animation
		is over. Importtant to remove their offfset.*/
		if (hiddenRect.x <= mysteryRect.x - delX) {
			hiddenLess = createRect(w / 2, h * 10 / 16, w / 4, h / 8, true);
			hiddenMore = createRect(w / 2, h / 2, w / 4, h / 8, true);
			hiddenImg = createRect(w * 3 / 4, h * 7 / 16, w / 4, h / 2, true);
			hiddenRect = createRect(w * 3 / 4, h * 3 / 4, w / 4, h / 8, true);
		}
		if (lessImg.x <= viewRect.x - delX) {
			lessImg = createRect(w / 4, h * 7 / 16, w / 4, h / 2, true);
			mysteryRect = createRect(w / 4, h * 3 / 4, w / 4, h / 8, true);
			subMysRect = createRect(mysteryRect.x, h * 5 / 6, w / 8, h / 16, false);
		}
	}

	// Sub Icons attributes
	SDL_Color subColor = { 39, 39, 39, 0 };
	float subRadius = 10.0f;

	// Rectangular galaore
	drawRectangle(&moreImg, 128, 128, 128, true);
	drawRectangle(&lessImg, 0, 0, 0, true);
	drawRectangle(&hiddenImg, 0, 0, 0, true);
	drawLogo(w / 2, h / 16, w / 8);
	// The Rectangle that says "More"
	drawRectangle(&more, 0, 83, 10, true);
	drawRectangle(&hiddenMore, 0, 83, 10, true);
	// The Rectangle that says "Less"
	drawRectangle(&less, 8, 39, 245, true);
	drawRectangle(&hiddenLess, 8, 39, 245, true);
	drawRectangle(&mysteryRect, 0, 0, 0, true);
	drawRectangle(&hiddenRect, 0, 0, 0, true);
	// The subs are hidden in any difficulty after standard
	if (difficulty == standard) {
		drawSmoothRectagle(subRect, subColor.r, subColor.g, subColor.b, subRadius);
		drawSmoothRectagle(subMysRect, subColor.r, subColor.g, subColor.b, subRadius);
		drawSmoothRectagle(subHideRect, subColor.r, subColor.g, subColor.b, subRadius);
	}

	SDL_RenderFillRect(renderer, &viewRect);
	SDL_SetRenderDrawColor(renderer, 197, 179, 88, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &scoreRect);


	moreLessTxt(moreTxt, more, lessTxt, less, gameAttr->state);
	if (gameAttr->state != normal) {
		TTF_SetTextColor(views->text, 255, 233, 0, SDL_ALPHA_OPAQUE);
		TTF_SetTextColor(mysteryText->text, 255, 233, 0, SDL_ALPHA_OPAQUE);
	}
	else {
		TTF_SetTextColor(views->text, 255, 255, 255, SDL_ALPHA_OPAQUE);
		TTF_SetTextColor(mysteryText->text, 255, 255, 255, SDL_ALPHA_OPAQUE);
	}

	displayText(viewRect, views->text, &x, &y);
	displayText(mysteryRect, mysteryText->text, &x, &y);
	displayText(hiddenRect, nextTxt, &x, &y);


	if (difficulty == standard) {
		displayText(subRect, subsTxts[0]->text, &x, &y); // Known
		displayText(subMysRect, subsTxts[1]->text, &x, &y); // Mystery
		displayText(subHideRect, subsTxts[2]->text, &x, &y); // Hidden
	}

	// Text Color section: Set txt color depending on gameState
	if (gameAttr->state == lessWrong || gameAttr->state == moreWrong) {
		// Set the score color to red
		TTF_SetTextColor(txtScore->text, 150, 0, 24, SDL_ALPHA_OPAQUE);
	}
	else if (gameAttr->state == lessRight || gameAttr->state == moreRight) {
		// Sets the score color to green
		TTF_SetTextColor(txtScore->text, 21, 71, 52, SDL_ALPHA_OPAQUE);
	}
	else {
		TTF_SetTextColor(txtScore->text, 255, 255, 255, SDL_ALPHA_OPAQUE);
	}

	displayText(scoreRect, txtScore->text, &x, &y);
	displayText(hiddenLess, lessTxt, &x, &y);
	displayText(hiddenMore, moreTxt, &x, &y);

	// Display the text color
	TTF_SetTextColor(txtScore->text, 255, 255, 255, SDL_ALPHA_OPAQUE);

	SDL_RenderTexture(renderer, queue->front->img, NULL, &moreImg);

	// Display the hearts
	for (int a = 0; a < heartCount; a++) {
		if (a >= gameAttr->health) {
			// This heart is depleted, show gray version
			SDL_RenderTexture(renderer, deadBeast, NULL, &(mrBeastPos[a]));
		}
		else {
			// This heart is still good
			SDL_RenderTexture(renderer, mrBeastHeart, NULL, &(mrBeastPos[a]));
		}
	}

	YTNode* nextThing = queue->front->next;
	if (nextThing != NULL) {
		SDL_RenderTexture(renderer, nextThing->img, NULL, &lessImg);
		YTNode* hiddenNext = nextThing->next;
		SDL_RenderTexture(renderer, hiddenNext->img, NULL, &hiddenImg);
	}

	/*if (gameAttr->state != normal) {
		printf("%d\n", (int)hiddenRect.x);
	}*/
	// The Rectangle with ??? is in the right spot. Once this animation,
	// The game is ready to move on to the next state
	if (gameAttr->state != normal && (int)hiddenRect.x == center(w * 3 / 4, hiddenRect.w)) {
		gameAttr->state = normal;
	}

	return gameAttr->state;
}