#include "title_helper.h"

/*Handles everything for the xPos array, which keeps track of the positions
of the thumbnails which are all offset every frame. Will also change the width and
height if the screen size ever changes.*/
float handleXPos(float* realPOS, float* projectedPOS, float wrapPoint, float w, float* h, float rectW, float realW) {

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
				realPOS[a] = -(realW * 2) + (realW * 2 * (VIDEO_COUNT / LEVEL_COUNT - 1));
				projectedPOS[a] = -(rectW * 2) + (rectW * 2 * (VIDEO_COUNT / LEVEL_COUNT - 1));
			}
		}
	}

	return w;
}

void unprojectXPOS(float* realPOS, float* projectedPOS) {
	for (int a = 0; a < VIDEO_COUNT; a++) {
		projectedPOS[a] = realPOS[a];
	}
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

/*Given a project objects, set the projection equal
to the real attributes, undoing projection*/
ProjectedObject unprojectObject(ProjectedObject obj) {
	obj.projectedRect.x = obj.realRect.x;
	obj.projectedRect.y = obj.realRect.y;
	obj.projectedRect.w = obj.realRect.w;
	obj.projectedRect.h = obj.realRect.h;

	return obj;
}


void createExplanationTxt(MultiLineText* explanationTxt, TTF_Font* font, char*** jsonData, int entries, SDL_FRect ref) {

	// Where the line count is placed in the JSON file
	const int lineCountIndex = 2;
	float wrapPoint = ref.w * 0.9f;

	int textWidth = 0;
	int textHeight = 0;
	TTF_Font** newFonts = malloc(sizeof(TTF_Font*) * entries);

	if (newFonts == NULL) {
		errorExit("Allocation for the new fonts failed");
	}


	for (int a = 0; a < entries; a++) {
		explanationTxt[a].lineCount = convertToInt(jsonData[a][lineCountIndex]);

		// Need a unique font for each difficulty
		newFonts[a] = copyFont(font);
		int jsonLines = explanationTxt[a].lineCount;
		// Need for font size
		char* allText = join(jsonData[a], 3, 3 + jsonLines - 1, " ", &textWidth);
		int charCount = strlen(allText);
		int noAllConds = convertToInt(jsonData[a][lineCountIndex + 1 + jsonLines]);
		// Every difficulty past one will say "All previous conditions apply"
		if (noAllConds == 0) {
			explanationTxt[a].lineCount++;
			charCount += strlen(" All previous conditions apply");
		}

		free(allText);

		// New lines added by wrapping are non major
		int majorLineCount = explanationTxt[a].lineCount;
		// The more lines, the smaller the text
		float fontFactor;
		float logBase = 2.15f;

		// Would be 0 or negative otherwise
		if (charCount > 696) fontFactor = 0.005f;
		else fontFactor = 1.5f - log(1 + charCount * 0.005f) / log(logBase);
		TTF_SetFontSize(newFonts[a], TTF_GetFontSize(font) * fontFactor);

		// To determine the size of a single letter
		TTF_Text* refTxt = TTF_CreateText(textEngine, newFonts[a], "a", strlen("a"));
		TTF_GetTextSize(refTxt, &textWidth, &textHeight);
		TTF_DestroyText(refTxt);

		explanationTxt[a].lines = malloc(sizeof(TTF_Text*) * explanationTxt[a].lineCount);
		explanationTxt[a].lineRects = malloc(sizeof(ProjectedObject) * explanationTxt[a].lineCount);

		if (explanationTxt[a].lines == NULL || explanationTxt[a].lineRects == NULL) {
			fprintf(stderr, "%s\n", "Allocation for explanation text/pos failed");
			quit(ytQueue);
			exit(1);
		}

		int lineIndex = 0;
		for (int b = 0; b < jsonLines; b++) {
			int jsonIndex = 3 + b;

			char* newLine = jsonData[a][jsonIndex];
			TTF_Text* noWrapTxt = TTF_CreateText(textEngine, newFonts[a], newLine, strlen(newLine));

			int divisions = 0;
			TTF_Text** newLines = wrapText(noWrapTxt, wrapPoint, &divisions);
			if (divisions > 1) {
				explanationTxt[a].lineCount += (divisions - 1);
				TTF_Text** tempTxt = realloc(explanationTxt[a].lines, sizeof(TTF_Text*) * explanationTxt[a].lineCount);
				ProjectedObject* tempObj = realloc(explanationTxt[a].lineRects, sizeof(ProjectedObject) * explanationTxt[a].lineCount);

				if (tempTxt == NULL || tempObj == NULL) {
					fprintf(stderr, "%s\n", "Allocation for explanation text/pos failed");
					quit(ytQueue);
					exit(1);
				}

				explanationTxt[a].lines = tempTxt;
				explanationTxt[a].lineRects = tempObj;

				for (int c = lineIndex; c < lineIndex + divisions; c++) {
					explanationTxt[a].lines[c] = newLines[c - lineIndex];
					float coolY = (ref.y + (ref.h / majorLineCount) * b) + (textHeight * (5.0f / 4.0f) * (c - lineIndex));
					explanationTxt[a].lineRects[c] = createProjectedObject(ref.x, coolY, ref.w, ref.h / majorLineCount, false);
				}


				lineIndex += (divisions - 1);
			}
			else {
				explanationTxt[a].lines[lineIndex] = noWrapTxt;
				float coolY = ref.y + (ref.h / majorLineCount) * b;
				explanationTxt[a].lineRects[lineIndex] = createProjectedObject(ref.x, coolY, ref.w, ref.h / majorLineCount, false);
			}


			lineIndex++;

			free(newLines);
		}

		// All condition apply msg, except for initial and easiest difficulty
		if (noAllConds == 0) {
			float coolerY = ref.y + (ref.h / majorLineCount) * jsonLines;

			char condMsg[] = "All previous conditions apply";
			int divisions;
			// Gonna need to do wrapping for this as well
			TTF_Text* noWrapTxt = TTF_CreateText(textEngine, newFonts[a], condMsg, strlen(condMsg));
			TTF_Text** newLines = wrapText(noWrapTxt, wrapPoint, &divisions);

			if (divisions > 1) {
				explanationTxt[a].lineCount += (divisions - 1);
				TTF_Text** tempTxt = realloc(explanationTxt[a].lines, sizeof(TTF_Text*) * explanationTxt[a].lineCount);
				ProjectedObject* tempObj = realloc(explanationTxt[a].lineRects, sizeof(ProjectedObject) * explanationTxt[a].lineCount);

				if (tempTxt == NULL || tempObj == NULL) {
					fprintf(stderr, "%s\n", "Allocation for explanation text/pos failed");
					quit(ytQueue);
					exit(1);
				}

				explanationTxt[a].lines = tempTxt;
				explanationTxt[a].lineRects = tempObj;

				for (int c = lineIndex; c < lineIndex + divisions; c++) {
					explanationTxt[a].lines[c] = newLines[c - lineIndex];
					float coolY = (ref.y + (ref.h / majorLineCount) * jsonLines) + (textHeight * (5.0f / 4.0f) * (c - lineIndex));
					explanationTxt[a].lineRects[c] = createProjectedObject(ref.x, coolY, ref.w, ref.h / majorLineCount, false);
				}


				lineIndex += (divisions - 1);
			}
			else {
				explanationTxt[a].lines[lineIndex] = noWrapTxt;
				float coolY = ref.y + (ref.h / majorLineCount) * jsonLines;
				explanationTxt[a].lineRects[lineIndex] = createProjectedObject(ref.x, coolY, ref.w, ref.h / majorLineCount, false);
			}

			free(newLines);
		}
	}

	free(newFonts);
}
