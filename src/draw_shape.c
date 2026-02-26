#include "draw_shape.h"

void drawCircle(int x, int y, float radius, SDL_Color color) {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

	SDL_FPoint* vertices = malloc(sizeof(SDL_FPoint) * 8000);
	if (vertices == NULL) {
		fprintf(stderr, "%s\n", "Error getting mem for vertices");
		quit(ytQueue);
		exit(1);
	}
}


/*Draws the logo for the game (Basically the YT symbol).
The size and location can be customized.*/
void drawLogo(int x, int y, float size) {
	SDL_FRect ytSymbol = createRect(x, y, size, size * 9 / 16, true);
	drawSmoothRectagle(ytSymbol, ytRed.r, ytRed.g, ytRed.b, size / 8);
	//displayText(ytSymbol, moreTxt, &x, &y);

	SDL_Color white = { 255, 255, 255, SDL_ALPHA_OPAQUE };
	float fontSize = size / 8;
	float widthOff = 0.52f * fontSize;
	float heightOff = 1.2f * fontSize;
	renderRotatedText("More", 25.0f, white, ytSymbol, fontSize, ytSymbol.x + ytSymbol.w / 2, ytSymbol.y + ytSymbol.h / 2 - heightOff * 5 / 8);
	renderRotatedText("Less", -25.0f, white, ytSymbol, fontSize, ytSymbol.x + ytSymbol.w / 2, ytSymbol.y + ytSymbol.h / 2 + heightOff * 5 / 8);
	renderRotatedText("Or", 90.0f, white, ytSymbol, fontSize, ytSymbol.x + ytSymbol.w / 2 - widthOff * 2, ytSymbol.y + ytSymbol.h / 2);
}

/*Takes a SDL_Surface, and makes all pixels beyond a radius = half of the shortest side
 transperent, making it look like a circle (SDL still treats it like a rect)
 NOTE: Will destroy the original surface*/
SDL_Surface* transformToCircle(SDL_Surface* surf) {
	// Every pixel MUST have 4 bytes allocated to it for this to work
	SDL_Surface* correctForm = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA8888);
	// We don't need the orignal
	SDL_DestroySurface(surf);
	int w = correctForm->w;
	int h = correctForm->h;
	int centerX = w / 2;
	int centerY = h / 2;
	int radius;
	if (w > h) radius = h / 2;
	else radius = w / 2;

	// Allows us to directly access pixels
	SDL_LockSurface(correctForm);
	uint32_t* surfPixels = (uint32_t*)correctForm->pixels;
	SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(correctForm->format);
	int index = 0;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			float xDistance = fabs(x - centerX);
			float yDistance = fabs(y - centerY);
			float distance = sqrtf(xDistance * xDistance + yDistance * yDistance);

	/*		printf("%d %f\n", radius, distance);
			quit(ytQueue);
			exit(0);*/
			// Makes eeverything outside the circle invisible
			if (distance > radius) {
	/*			printf("here");*/
				surfPixels[index] = SDL_MapRGBA(details, NULL, 0, 0, 0, 0);
			}
			index++;
		}
	}
	

	SDL_UnlockSurface(correctForm);
	return correctForm;
}

/*Given a surface, transform it into a grayscale version
of the same surface. Override the original version*/
SDL_Surface* setSurfGrayScale(SDL_Surface* surface) {

	SDL_Surface* correctForm = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA8888);
	SDL_DestroySurface(surface);

	SDL_LockSurface(correctForm);

	uint32_t* surfPixels = (uint32_t*)correctForm->pixels;
	SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(correctForm->format);
	SDL_Palette* surfPalette = SDL_GetSurfacePalette(correctForm);

	uint32_t red;
	uint32_t blue;
	uint32_t green;
	uint32_t alpha;

	int index = 0;
	for (int y = 0; y < correctForm->h; y++) {
		for (int x = 0; x < correctForm->w; x++) {
			SDL_GetRGBA(surfPixels[index], details, surfPalette, &red, &green, &blue, &alpha);
			
			int minInter = min(red, blue);
			int min = min(minInter, green);
			// Each pixel will be the minimum between each rgb value. 
			uint32_t grayScale = min;

			surfPixels[index] = SDL_MapRGBA(details, surfPalette, grayScale, grayScale, grayScale, alpha);

			index++;
		}
	}
	
	SDL_UnlockSurface(correctForm);
	return correctForm;
}

/*Draw a regtangle with the color parameters and a black border if border specified*/
void drawRectangle(SDL_FRect* rect, int r, int g, int b, bool border) {
	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, rect);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	if (border) {
		SDL_RenderRect(renderer, rect);
	}
}

/*Draws a rectangle with smooth edges. This isn't included within SDL, so
it is a little bit of a more involved process.*/
void drawSmoothRectagle(SDL_FRect rect, int r, int g, int b, float radius) {
	int segments = 2000;
	int vertexes = (segments * 4 + 1) * 2;

	SDL_FPoint* vertices = malloc(sizeof(SDL_Point) * vertexes);

	if (vertices == NULL) {
		fprintf(stderr, "%s\n", "Error getting mem for vertices");
		quit(ytQueue);
		exit(1);
	}

	vertices[0].x = rect.x + rect.w / 2;
	vertices[0].y = rect.y + rect.h / 2;

	int index = 1;

	// Top Left
	for (int a = 0; a <= segments; a++) {
		float angles = M_PI + a * (M_PI / 2) / segments;
		vertices[index].x = rect.x + radius + cos(angles) * radius;
		vertices[index].y = rect.y + radius + sin(angles) * radius;
		//printf("%f, %f\n", vertices[index].x, vertices[index].y);
		index++;
	}

	// Top Right
	for (int a = 0; a <= segments; a++) {
		float angles = -M_PI / 2 + a * (M_PI / 2) / segments;
		vertices[index].x = rect.x + +rect.w - radius + cos(angles) * radius;
		vertices[index].y = rect.y + radius + sin(angles) * radius;
		//printf("%f, %f\n", vertices[index].x, vertices[index].y);
		index++;
	}

	// Bottom Right
	for (int a = 0; a <= segments; a++) {
		float angles = a * (M_PI / 2) / segments;
		vertices[index].x = rect.x + rect.w - radius + cos(angles) * radius;
		vertices[index].y = rect.y + rect.h - radius + sin(angles) * radius;
		//printf("%f, %f\n", vertices[index].x, vertices[index].y);
		index++;
	}

	// Bottom Left
	for (int a = 0; a <= segments; a++) {
		float angles = M_PI / 2 + a * (M_PI / 2) / segments;
		vertices[index].x = rect.x + radius + cos(angles) * radius;
		vertices[index].y = rect.y + rect.h - radius + sin(angles) * radius;
		//printf("%f, %f\n", vertices[index].x, vertices[index].y);
		index++;
	}

	// Coloring everything
	/*SDL_FColor color = { r, g, b, SDL_ALPHA_OPAQUE };
	for (int a = 0; a < vertexes; a++) {
		vertices[a].color = color;
	}*/
	// We used the diameter a lot
	float diameter = radius * 2;

	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	SDL_RenderPoints(renderer, vertices, index);
	// Rendering the lines for the rectangle
	// Top Left to Top Right
	SDL_RenderLine(renderer, vertices[segments].x, vertices[segments].y,
		vertices[segments].x + rect.w - diameter, vertices[segments].y);
	// Top to Bottom (Left)
	SDL_RenderLine(renderer, vertices[1].x, vertices[1].y,
		vertices[1].x, vertices[1].y + rect.h - diameter);

	// Top to Bottom (Right)
	SDL_RenderLine(renderer, vertices[segments * 2].x, vertices[segments * 2].y,
		vertices[segments * 2].x, vertices[segments * 2].y + rect.h - diameter);
	// Bottom Right to Bottom Left
	SDL_RenderLine(renderer, vertices[segments * 3].x, vertices[segments * 3].y,
		vertices[segments * 3].x - rect.w + diameter, vertices[segments * 3].y);

	rect.w -= radius * 4;
	rect.h -= radius * 4;

	rect.x += diameter;
	rect.y += diameter;
	// Draws the center Rectangles
	drawRectangle(&rect, r, g, b, false);

	// Draws four rectangles on the side
	SDL_FRect topRect = createRect(vertices[segments].x + radius, vertices[1].y + radius,
		rect.w, vertices[segments].y - rect.y, false);

	drawRectangle(&topRect, r, g, b, false);

	SDL_FRect bottomRect = createRect(vertices[segments].x + radius, vertices[segments * 3].y,
		rect.w, vertices[segments].y - rect.y, false);

	drawRectangle(&bottomRect, r, g, b, false);

	SDL_FRect leftRect = createRect(vertices[1].x + diameter, vertices[1].y,
		vertices[1].x - rect.x, rect.h + diameter, false);

	drawRectangle(&leftRect, r, g, b, false);

	SDL_FRect rightRect = createRect(vertices[segments * 2].x, vertices[1].y,
		vertices[1].x - rect.x, rect.h + diameter, false);

	drawRectangle(&rightRect, r, g, b, false);

	// Draw messes up the renderer color
	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	index = 1;
	for (int a = 0; a <= segments * 2; a++) {
		SDL_RenderLine(renderer, vertices[index].x, vertices[index].y,
			topRect.x, vertices[index].y);
		index++;
	}

	for (int a = 0; a <= segments * 2; a++) {
		SDL_RenderLine(renderer, vertices[index].x, vertices[index].y,
			bottomRect.x, vertices[index].y);
		index++;
	}

	free(vertices);
	/*quit(ytQueue);
	exit(0);*/
}
