#ifndef TITLE_HELPER_H_
#define TITLE_HELPER_H_

#include "commons.h"
#include "config.h"
#include "utilities.h"
#include "globals.h"

void createExplanationTxt(MultiLineText* explanationTxt, TTF_Font* font, char*** jsonData, int entries, SDL_FRect ref);

ProjectedObject unprojectObject(ProjectedObject obj);

ProjectedObject createProjectedObject(float x, float y, float w, float h, bool centered);

ProjectedObject projectRect(ProjectedObject obj, float xDifference, float yDifference);

float handleXPos(float* realPOS, float* projectedPOS, float wrapPoint, float w, float* h, float rectW);

#endif // !TITLE_HELPER_H_
