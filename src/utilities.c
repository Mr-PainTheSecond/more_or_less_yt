#include "utilities.h"

int convertToInt(char* sInt) {
	int sIntSize = strlen(sInt);
	int finalInt = 0;
	for (int a = 0; a < sIntSize; a++) {
		int digit = (int)(sInt[a]) - 48;
		int addedDigit = pow(10, sIntSize - a - 1) * digit;
		finalInt += addedDigit;
	}


	return finalInt;
}

/*Takes a string array, and returns a random element from that 
array*/
char* choiceStr(char** array, int size) {
	int randNum = rand() % size;
	return array[randNum];
}

/*Takes a double char pointer, and concanates with fileOne and fileTwo to become a valid file
location. fileOne is everything before each file and fileTwo is everything afterwards
fileOne and fileTwo can be NULL, but both being NULL would do nothing*/
void formatAsFileLocation(const char* fileOne, const char* fileTwo, char** files, int count) {
	// Will literally do nothing lol
	if (fileOne == NULL && fileTwo == NULL) {
		return;
	}
	char buffer[100];
	int extraSize = 0;
	if (fileOne != NULL) {
		extraSize += strlen(fileOne);
	}

	if (fileTwo != NULL) {
		extraSize += strlen(fileTwo);
	}
	for (int a = 0; a < count; a++) {
		// The original string needs to be saved since it is overrriden later
		strcpy(buffer, files[a]);
		char* temp = realloc(files[a], sizeof(char) * (strlen(files[a]) + extraSize + 1));
		if (temp == NULL) {
			fprintf(stderr, "%s\n", "Reallocation of the file location failed");
			quit(ytQueue);
			exit(1);
		}

		files[a] = temp;
		// If fileOne is NULL, doesn't need to do anything.
		if (fileOne != NULL) {
			strcpy(files[a], fileOne);
			strcat(files[a], buffer);
		}
		if (fileTwo != NULL) {
			strcat(files[a], fileTwo);
		}
	}
}


SDL_FRect get_rect_center(SDL_FRect dst, SDL_FRect src) {
	dst.x = src.x + (src.w / 2) - (dst.w / 2);
	dst.y = src.y + (src.h / 2) - (dst.h / 2);
	return dst;
}


/*Reads the specified file, and returns a double char pointer
which is split based upon the delimeter
Also has an int pointer so the length of the array can be stored*/
char** readAndSplit(const char* fileName, char delimeter, int* size) {
	FILE* readFile = fopen(fileName, "r");
	char buffer[100];
	if (readFile == NULL) {
		fprintf(stderr, "%s\n", "Something has gone wrong with the file/doesn't exist");
		quit(ytQueue);
		exit(1);
	}

	char** words = malloc(sizeof(char*) * 20000);
	if (words == NULL) {
		fprintf(stderr, "%s\n", "Something went wrong with the words");
		quit(ytQueue);
		exit(1);
	}
	int wordCount = 0;
	int charCount = 0;
	char nextChar = fgetc(readFile);
	while (nextChar != EOF) {
		if (nextChar != delimeter) {
			buffer[charCount] = nextChar;
		}
		else {
			// We don't put delimeter. Instead, we end string
			buffer[charCount] = '\0';
		}
		charCount++;
		if (nextChar == delimeter) {
			words[wordCount] = malloc(sizeof(char) * charCount);
			if (words[wordCount] == NULL) {
				fprintf(stderr, "Something went wrong with word %d\n", wordCount + 1);
				quit(ytQueue);
				exit(1);
			}
			
			// We put it into DMA so it lives outside of here
			strcpy(words[wordCount], buffer);
			wordCount++;
			charCount = 0;
		}

		nextChar = fgetc(readFile);
	}
	
	fclose(readFile);
	// Now that we know how much, we will reallocate the correct amount
	char** temp = realloc(words, sizeof(char*) * wordCount);

	if (temp == NULL) {
		fprintf(stderr, "%s\n", "Something went wrong with the words");
		quit(ytQueue);
		exit(1);
	}

	words = temp;
	*size = wordCount;
	return words;
}

int expandQueue(zsock_t* requester, Queue* queue, int counter) {
	counter++;
	if (queue->size <= 4) {
		// Let the server know we are ready for more
		zstr_send(requester, "Roger");
		if (!getYtData(requester, queue)) {
			return -1;
		}
		
	}

	return counter;
}


/*Initializes all the variables in the SDL_FRect struct
Not hard, but pretty repetitive task*/
SDL_FRect createRect(float x, float y, float w, float h, bool centered) {
	SDL_FRect rect;
	if (centered) {
		rect.x = center(x, w);
		rect.y = center(y, h);
	}
	else {
		rect.x = x;
		rect.y = y;
	}
	rect.w = w;
	rect.h = h;
	return rect;
}

float center(float pos, float size) {
	return pos - (size / 2);
}

void deQueue(Queue* queue, YTNode* next) {
	YTNode* oldFront = queue->front;
	SDL_DestroyTexture(oldFront->img);
	free(oldFront->filePath);
	free(oldFront->sViews);
	free(oldFront);
	queue->front = next;
	if (queue->front == NULL || queue->front->next == NULL) {
		zstr_send(requester, "STOP");
		zsock_destroy(&requester);
		fprintf(stderr, "%s\n", "FUCKKK");
		exit(1);
	}
	YTNode* buffer = queue->front->next;
	if (buffer->next == NULL) {
		zstr_send(requester, "STOP");
		zsock_destroy(&requester);
		fprintf(stderr, "%s\n", "FUCKKK");
		exit(1);

	}
	queue->size--;
}

Queue* createQueue() {
	Queue* ytData = malloc(sizeof(Queue));
	if (ytData == NULL) {
		exit(1);
	}
	ytData->front = NULL;
	ytData->back = NULL;
	ytData->size = 0;
	return ytData;
}

void deleteQueue(Queue* queue) {
	if (queue == NULL) return;
	while (queue->front != NULL) {
		YTNode* oldFront = queue->front;
		queue->front = queue->front->next;
		SDL_DestroyTexture(oldFront->img);
		free(oldFront->filePath);
		free(oldFront->sViews);
		free(oldFront->subs);
		free(oldFront);
	}

	free(queue);
}

void copyDymTxt(DynamicText* dstTxt, DynamicText* srcTxt) {
	free(dstTxt->str);

	dstTxt->str = malloc(sizeof(char) * strlen(srcTxt->str) + 1);
	if (dstTxt->str == NULL) {
		fprintf(stderr, "%s\n", "No mem for dstTxt string");
		quit(ytQueue);
		exit(1);
	}

	strcpy(dstTxt->str, srcTxt->str);

	TTF_DestroyText(dstTxt->text);

	// Able to copy the font from the srcTxt
	dstTxt->text = TTF_CreateText(textEngine, TTF_GetTextFont(srcTxt->text), dstTxt->str, strlen(dstTxt->str));
}

void quit(Queue* queue) {
	zstr_send(requester, "STOP");
	zsock_destroy(&requester);
	SDL_DestroySurface(screen->surface);
	free(screen);
	freeFontArray();
	deleteQueue(queue);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}
