#ifndef UTILITIES_H_
#define UTILITIES_H_
#include "commons.h"
#include "server.h"
#include "globals.h"
#include "font.h"

void deleteQueue(Queue* queue);

void deQueue(Queue* queue, YTNode* next);

int convertToInt(char* sInt);

char* converToStr(int num);

int expandQueue(zsock_t* requester, Queue* queue, int counter);

Queue* createQueue();

float center(float pos, float size);

char** readAndSplit(const char* fileName, char delimeter, int* size);

char** split(const char* str, char delimeter, int* size);

char* join(char** arr, int lower, int upper, const char* newChar, int* newLen);

void formatAsFileLocation(const char* fileOne, const char* fileTwo, char** files, int count);

char* choiceStr(char** array, int size);

void copyDymTxt(DynamicText* dstTxt, DynamicText* srcTxt);

time_t frameRateinMs(int frameRate);

float normalize(float x1, float y1, float x2, float y2);

char*** readJSONArray(const char* fileName, const char* array, int* objCount, int** entries);

__declspec(noreturn) void errorExit(const char* msg);


void quit(Queue* queue);

#endif // !UTILITIES_H_

