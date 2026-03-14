#include "draw.h"
#include "font.h"

/*Sets the properties which are unique depending if the player won or lost.
Will run when the player first gets to end screen, and when their condition
changes*/
int finalConditionProperties(TTF_Text** videoTxt, TTF_Text** stateTxt, SDL_Texture** background, SDL_Texture** pfp) {
	SDL_Surface* winSurf;
	SDL_Surface* pfpSurf;
	if (gameAttr->state == justWon || gameAttr->state == gameWon) {
		char videoMsg[] = "Thumbs up crew CONGRAGULATES you";
		char stateMsg[] = "WINNER!!!";

		*videoTxt = TTF_CreateText(textEngine, ytFont, videoMsg, strlen(videoMsg));
		*stateTxt = TTF_CreateText(textEngine, moreLessFont, stateMsg, strlen(stateMsg));

		winSurf = IMG_Load("..\\assets\\images\\perm\\final_screen\\0\\winner\\winner_0.JPG");
		pfpSurf = IMG_Load("..\\assets\\images\\perm\\final_screen\\0\\winner\\winner_channel_0.PNG");
	}
	else {
		char videoMsg[] = "Markiplier is disappointed (ASMR)";
		char stateMsg[] = "GAME OVER";

		*videoTxt = TTF_CreateText(textEngine, ytFont, videoMsg, strlen(videoMsg));
		*stateTxt = TTF_CreateText(textEngine, moreLessFont, stateMsg, strlen(stateMsg));

		winSurf = IMG_Load("..\\assets\\images\\perm\\final_screen\\0\\loser\\loser_0.JPG");
		pfpSurf = IMG_Load("..\\assets\\images\\perm\\final_screen\\0\\loser\\loser_channel_0.PNG");
	}

	pfpSurf = transformToCircle(pfpSurf);

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
	// The game is ending. Free all the assets and return
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

	drawSmoothRectagle(announceRect, 21, 71, 52, SDL_ALPHA_OPAQUE, 20.0f);
	drawSmoothRectagle(videoTxtRect, 39, 39, 39, SDL_ALPHA_OPAQUE, 30.0f);
	drawSmoothRectagle(menuRect, 100, 27, 0, SDL_ALPHA_OPAQUE, menuRect.w / 6);
	drawSmoothRectagle(quitRect, 100, 27, 0, SDL_ALPHA_OPAQUE, menuRect.w / 6);

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

	if (gameAttr->state == shutDown) {
		SDL_HideWindow(window);
	}

	// There is a LOT of states within the regular game logic
	if (gameAttr->state >= normal && gameAttr->state <= justWon || gameAttr->state == shutDown) {
		gameAttr->state = drawMoreOrLess(more, less, queue);
	}
	if (gameAttr->state >= justLost && gameAttr->state <= gameWon || gameAttr->state == shutDown) {
		drawFinalScreen();
	}
	if (firstIter) {
		SDL_ShowWindow(window);
		firstIter = false;
	}


	SDL_RenderPresent(renderer);
	return gameAttr->state;
}