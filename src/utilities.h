#ifndef UTILITIES_H_
#define UTILITIES_H_
#include "commons.h"
#include "server.h"
#include "globals.h"
#include "font.h"

void deleteQueue(Queue* queue);

void deQueue(Queue* queue, YTNode* next);

int convertToInt(char* sInt);

int expandQueue(zsock_t* requester, Queue* queue, int counter);

Queue* createQueue();

float center(float pos, float size);

char** readAndSplit(const char* fileName, char delimeter, int* size);

void formatAsFileLocation(const char* fileOne, const char* fileTwo, char** files, int count);

char* choiceStr(char** array, int size);

void copyDymTxt(DynamicText* dstTxt, DynamicText* srcTxt);

void quit(Queue* queue);

#endif // !UTILITIES_H_

