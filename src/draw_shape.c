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

/*Given a rect, returns true if on screen,
returns false otherwise. Important cause we shouldn't
draw shit we can't see.*/
bool inBounds(SDL_FRect rect) {
	return rect.x + rect.w > 0 && rect.x < screen->w
		&& rect.y + rect.h > 0 && rect.y < screen->h;
}

/*Draws the logo for the game (Basically the YT symbol).
The size and location can be customized.*/
void drawLogo(int x, int y, float size, bool centered) {
	SDL_FRect ytSymbol = createRect(x, y, size, size * 9 / 16, centered);
	if (!inBounds(ytSymbol)) return;
	drawSmoothRectagle(ytSymbol, ytRed.r, ytRed.g, ytRed.b, SDL_ALPHA_OPAQUE, size / 8);
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


SDL_FRect zoom(float oldX, float oldY, float newX, float newY, float w, float h) {
	float distanceX = newX - oldX;
	float distanceY = oldY - newY;

	float ratio = distanceX / screen->w;

	float spawnZ = BASE_Z + FOCAL;
	float currentZ = FOCAL + BASE_Z + (ratio) * THIS_IS_A_CERTIFIED_CLASSIC * ZOOM_EFFECT;

	float scale = spawnZ / currentZ;

	float newW = w * scale;
	float newH = h * scale;

	SDL_FRect object = createRect(newX, oldY * scale, newW, newH, false);

	return object;
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
void drawRectangle(SDL_FRect* rect, int r, int g, int b, int a, bool border) {
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderFillRect(renderer, rect);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, a);
	if (border) {
		SDL_RenderRect(renderer, rect);
	}
}

void drawSmoothEdges(SDL_FPoint* points, int firstIndex, int segments, SDL_FColor color) {
	SDL_Vertex* coolVertex = malloc(sizeof(SDL_Vertex) * (segments + 2));
	int* indicies = malloc(sizeof(int) * segments * 3);

	if (coolVertex == NULL || indicies == NULL) {
		fprintf(stderr, "%s\n", "Error getting mem for the cooler vertices");
		quit(ytQueue);
		exit(1);
	}

	coolVertex[0].color = (SDL_FColor){ color.r, color.g, color.b, color.a};
	coolVertex[0].position = (SDL_FPoint){ points[firstIndex].x, points[firstIndex].y};
	coolVertex[0].tex_coord = (SDL_FPoint){ 0, 0 };
	// Draw the corners :) (i think i forgor lol)
	// Tragically forced to use i cause of colors lol
	for (int i = firstIndex + 1; i <= segments + 1; i++) {
		coolVertex[i].position = (SDL_FPoint){ points[i].x, points[i].y };
		coolVertex[i].tex_coord = (SDL_FPoint){ 0, 0 };
		coolVertex[i].color = (SDL_FColor){ color.r, color.g, color.b, color.a};
	}

	for (int a = 0; a < segments; a++) {
		indicies[a * 3] = 0;
		indicies[a * 3 + 1] = a + 1;
		indicies[a * 3 + 2] = a + 2;
	}

	SDL_RenderGeometry(renderer, NULL, coolVertex, segments + 2, indicies, segments * 3);

	free(coolVertex);
	free(indicies);
}

/*Draws a rectangle with smooth edges. This isn't included within SDL, so
it is a little bit of a more involved process.*/
void drawSmoothRectagle(SDL_FRect rect, int r, int g, int b, int a, float radius) {
	if (!inBounds(rect)) return;
	int segments = 24;
	int vertexes = (segments * 4 + 1) * 2;
	// Makes sure the edges don't go overboard
	radius = min(radius, min(rect.w, rect.h) / 2);

	SDL_FPoint* vertices = malloc(sizeof(SDL_Point) * vertexes);

	if (vertices == NULL) {
		fprintf(stderr, "%s\n", "Error getting mem for vertices");
		quit(ytQueue);
		exit(1);
	}

	vertices[0].x = rect.x + rect.w / 2;
	vertices[0].y = rect.y + rect.h / 2;

	int index = 1;

	float top = rect.y + radius;
	float left = rect.x + radius;
	float right = rect.x + rect.w - radius;
	float bottom = rect.y + rect.h - radius;

	// Top Left
	float angleStep = (M_PI / 2.0f) / segments;
	for (int a = 0; a <= segments; a++) {
		float angles = M_PI + a * angleStep;
		vertices[index].x = left + cosf(angles) * radius;
		vertices[index].y = top + sinf(angles) * radius;
	/*	printf("Top Left: %f, %f\n", vertices[index].x, vertices[index].y);*/
		index++;
	}

	// Top Right
	for (int a = 0; a <= segments; a++) {
		float angles = -M_PI / 2 + a * angleStep;
		vertices[index].x = right + cosf(angles) * radius;
		vertices[index].y = top + sinf(angles) * radius;
		/*printf("Top Right: %f, %f\n", vertices[index].x, vertices[index].y);*/
		index++;
	}

	// Bottom Right
	for (int a = 0; a <= segments; a++) {
		float angles = a * angleStep;
		vertices[index].x = right + cosf(angles) * radius;
		vertices[index].y = bottom + sinf(angles) * radius;
		//printf("%f, %f\n", vertices[index].x, vertices[index].y);
		index++;
	}

	// Bottom Left
	for (int a = 0; a <= segments; a++) {
		float angles = M_PI / 2 + a * angleStep;
		vertices[index].x = left + cosf(angles) * radius;
		vertices[index].y = bottom + sinf(angles) * radius;
		//printf("%f, %f\n", vertices[index].x, vertices[index].y);
		index++;
	}

	vertices[index].x = left + cosf(M_PI) * radius;
	vertices[index].y = top + sinf(M_PI) * radius;

	// We used the diameter a lot
	float diameter = radius * 2;

	rect.w -= radius * 4;
	rect.h -= radius * 4;

	rect.x += diameter;
	rect.y += diameter;
	// Draws the center Rectangles
	//drawRectangle(&rect, r, g, b, a, false);

	//// Draws four rectangles on the side
	//SDL_FRect topRect = createRect(vertices[segments].x + radius, vertices[1].y + radius,
	//	rect.w, vertices[segments].y - rect.y, false);

	//drawRectangle(&topRect, r, g, b, a, false);

	//SDL_FRect bottomRect = createRect(vertices[segments].x + radius, vertices[segments * 3].y,
	//	rect.w, vertices[segments].y - rect.y, false);

	//drawRectangle(&bottomRect, r, g, b, a, false);

	//SDL_FRect leftRect = createRect(vertices[1].x + diameter, vertices[1].y,
	//	vertices[1].x - rect.x, rect.h + diameter, false);

	//drawRectangle(&leftRect, r, g, b, a, false);

	//SDL_FRect rightRect = createRect(vertices[segments * 2].x, vertices[1].y,
	//	vertices[1].x - rect.x, rect.h + diameter, false);

	//drawRectangle(&rightRect, r, g, b, a, false);

	//// Draw messes up the renderer color
	//SDL_SetRenderDrawColor(renderer, r, g, b, a);
	//index = 1;

	SDL_FColor color = (SDL_FColor){ r / 255.f, g / 255.f, b / 255.f, a / 255.f };
	drawSmoothEdges(vertices, 0, (vertexes / 2) + 3, color);
	free(vertices);
	
}
