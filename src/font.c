#include "font.h"



/*Based on a rectangle and TTF text of a specific size, gives the values
to the x and y pointers on the respective x and y cordinates needed to make
the text be at the center of the rectangle*/
void get_text_center(SDL_FRect rect, TTF_Text* text, int* x, int* y) {
	if (!TTF_GetTextSize(text, x, y)) {
		fprintf(stderr, "Failed cause of %s\n", SDL_GetError());
		quit(ytQueue);
		exit(-1);
	}
	if (x != NULL) {
		*x = rect.x + (rect.w / 2) - (*x / 2);
	}
	if (y != NULL) {
		*y = rect.y + (rect.h / 2) - (*y / 2);
	}
}

/*Based on a rectangle and the width and height of a font,
writes what the x and y values would have to be such that the 
text would be centered on the rect*/
void get_text_center_size(SDL_FRect rect, int* x, int* y, float w, float h) {
	if (x != NULL) {
		*x = rect.x + (rect.w / 2) - (w / 2);
	}
	if (y != NULL) {
		*y = rect.y + (rect.h / 2) - (h / 2);
	}
}


/*Draw the text the given text into the center of the given
rectangle*/
void displayText(SDL_FRect rect, TTF_Text* txt, int* x, int* y) {
	get_text_center(rect, txt, x, y);

	if (!TTF_DrawRendererText(txt, (float)*x, (float)*y)) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(-1);
	}
}


void createFontArray() {
	fontArray = malloc(sizeof(Fonts));
	if (fontArray == NULL) {
		fprintf(stderr, "%s\n", "Could not allocate memory for the font array");
		quit(ytQueue);
		exit(1);
	}

	fontArray->fonts = malloc(sizeof(TTF_Font*) * 8);
	if (fontArray->fonts == NULL) {
		fprintf(stderr, "%s\n", "Could not allocate memory for a font");
		quit(ytQueue);
		exit(1);
	}

	fontArray->fontIndex = 0;
	fontArray->fontSize = 8;
}

void freeFontArray() {
	for (int a = 0; a < fontArray->fontIndex; a++) {
		free(fontArray->fonts[a]);
	}

	free(fontArray);
}

/*Given the string of an unformatted number,
returns a string representation with formatting.
Example: 100000 -> 100K.*/
char* format(char* original, const char* suffix) {
	int dataPoints = 0;
	int dotPlace = strlen(original) - 9;
	char place = ' ';
	if (strlen(original) > 9) {
		dotPlace = strlen(original) - 9;
		dataPoints = 4;
		place = 'B';
	}
	else if (strlen(original) > 6) {
		dotPlace = strlen(original) - 6;
		dataPoints = 4;
		place = 'M';
	}
	else if (strlen(original) > 3) {
		dotPlace = strlen(original) - 3;
		dataPoints = 4;
		place = 'K';
	}
	else {
		dataPoints = strlen(original);
	}
	if (strlen(original) % 3 == 0) {
		dataPoints = 3;
		
	}
	char* newChar = malloc(sizeof(char) * dataPoints + strlen(suffix) + 2);
	if (newChar == NULL) {
		quit(ytQueue);
		exit(1);
	}
	int a;
	int ogIndex = 0;
	for (a = 0; a < dataPoints; a++) {
		if (a == dotPlace && a + 1 < dataPoints && place != ' ') {
			newChar[a] = '.';
		}
		else {
			newChar[a] = original[ogIndex];
			ogIndex++;
		}
	}

	if (place != ' ') {
		newChar[a] = place;
		a++;
	}

	newChar[a] = '\0';
	strcat(newChar, suffix);
	return newChar;
}

/*Rotates and renders a text while rotating within the specified angle
parameters*/
void renderRotatedText(const char* txt, double angle, SDL_Color color, SDL_FRect dstRect, float size,
	int x, int y) {
	SDL_Texture* txtTexture = NULL;
	// Creates the font as a surface
	SDL_Surface* txtSurface = TTF_RenderText_Solid(smallFont, txt, strlen(txt),
		color);

	if (txtSurface == NULL) {
		fprintf(stderr, "%s\n", "Surface Creation failed");
		quit(ytQueue);
		exit(1);
	}

	txtTexture = SDL_CreateTextureFromSurface(renderer, txtSurface);
	SDL_DestroySurface(txtSurface);


	SDL_FRect realRect = createRect(x, y, size * 0.525f * strlen(txt),
		size * 1.2f, true);

	// Where the text will be rotated upon
	SDL_FPoint center;
	center.x = realRect.w / 2;
	center.y = realRect.h / 2;

	SDL_RenderTextureRotated(renderer, txtTexture, NULL, &realRect, angle, &center, SDL_FLIP_NONE);
	SDL_DestroyTexture(txtTexture);
}

/*Creates the structure for Dynamic Text. This struct is just for
organization purposes.
Font can be NULL, however it will set str and text to NULL as well*/
DynamicText* createDynamicText(const char* str, TTF_Font* font) {
	DynamicText* dText = malloc(sizeof(DynamicText));
	if (dText == NULL) {
		fprintf(stderr, "%s\n", "Could not allocate memory for DText");
		quit(ytQueue);
		exit(1);
	}

	if (font == NULL) {
		dText->str = NULL;
		dText->text = NULL;
		return dText;
	}

	dText->str = malloc(sizeof(char) * strlen(str) + 1);

	if (dText->str == NULL) {
		fprintf(stderr, "%s\n", "Could not allocate memory for str");
		quit(ytQueue);
		exit(1);
	}

	strcpy(dText->str, str);
	dText->text = TTF_CreateText(textEngine, font, str, strlen(str));
	return dText;
}

/*Frees all of the individual components of the
Dynamic Text structure*/
void destroyDynamicText(DynamicText* dmTxt) {
	TTF_DestroyText(dmTxt->text);
	free(dmTxt->str);
	free(dmTxt);
}

TTF_Font* createFont(char* file_name, float size) {
	TTF_Font* font;
	font = TTF_OpenFont(file_name, 100);
	if (font == NULL) {
		fprintf(stderr, "%s\n", "Error with oppening the font");
		quit(ytQueue);
		exit(-1);
	}

	if (!TTF_SetFontSize(font, size)) {
		fprintf(stderr, "%s\n", "Error with setting the font size");
		quit(ytQueue);
		exit(-1);
	}

	fontArray->fonts[fontArray->fontIndex] = font;
	(fontArray->fontSize)++;

	return font;
}