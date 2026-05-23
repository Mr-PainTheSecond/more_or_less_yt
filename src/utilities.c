#include "utilities.h"

/*Takes a string representation of an integer,
returns its integer represenation*/
u_int64 convertToInt(char* sInt) {
	int sIntSize = strlen(sInt);
	u_int64 finalInt = 0;
	for (int a = 0; a < sIntSize; a++) {
		int digit = (int)(sInt[a]) - 48;
		u_int64 addedDigit = pow(10, sIntSize - a - 1) * digit;
		finalInt += addedDigit;
	}


	return finalInt;
}

/*Takes a string representation of a float,
returns its float representation 
NOTE: Can't handle the f (like 1.25f) format*/
float convertToFloat(char* sFloat) {
	int sFloatSize = strlen(sFloat);
	float finalFloat = 0;
	bool decimalPassed = false;
	int decimalPlace = 1;
	int decimalIndex = -1;
	for (int a = 0; a < sFloatSize; a++) {
		// First need to document where decimal is
		if (sFloat[a] == '.') {
			decimalIndex = a;
			break;
		}
	}

	// This isn't a decimal number
	if (decimalIndex == -1) {
		return (float)convertToInt(sFloat);
	}

	for (int a = 0; a < sFloatSize; a++) {
		if (sFloat[a] == '.') {
			decimalPassed = true;
			continue;
		}
		int digit = (int)(sFloat[a]) - 48;
		float addedDigit;
		if (!decimalPassed) {
			addedDigit = pow(10, decimalIndex - a - 1) * digit;
		}
		else {
			addedDigit = pow(10, -decimalPlace) * digit;
			decimalPlace++;
		}
		
		finalFloat += addedDigit;
	}
	return finalFloat;
}

/*Given the frame rate, returns how
many ms are in one frame*/
time_t frameRateinMs(int frameRate) {
	return 1000 / frameRate;
}

/*Takes an integer, and returns its string representation*/
char* converToStr(int num) {
	int numDigits;

	// Can't take log10 of 0 :(
	if (num == 0) {
		numDigits = 1;
	}
	else {
		numDigits = (int)log10(num) + 1;
	}
	char* sInt = malloc(sizeof(char) * (numDigits + 1));

	if (sInt == NULL) {
		fprintf(stderr, "%s\n", "Allocation for string int failed");
		quit(ytQueue);
		exit(1);
	}

	for (int a = numDigits - 1; a >= 0; a--) {
		int digit = num % 10;
		sInt[a] = (char)(digit + 48);
		num /= 10;
	}

	sInt[numDigits] = '\0';
	return sInt;
}

/*Takes a string array, and returns a random element from that 
array*/
char* choiceStr(char** array, int size) {
	int randNum = rand() % size;
	return array[randNum];
}

/*Takes two points and space, and return a float such that
the distance between the overall distance between the 
two points are the same, but the x and y components are equal*/
float normalize(float x1, float y1, float x2, float y2) {
	float distance = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
	return distance * sin(M_PI_4);
}

/*Takes a JSON file and an array of objects, and returns an array
formatted as [object][entry]. Only takes string entries*/
char*** readJSONArray(const char* fileName, const char* array, int* objCount, int** entries) {
	FILE* jsonFile = fopen(fileName, "r");
	printf("%s\n", "HELLO");
	if (jsonFile == NULL) {
		fprintf(stderr, "%s\n", "Something has gone wrong with the JSON file/doesn't exist");
		quit(ytQueue);
		exit(1);
	}

	char buffer[100];
	char newItem = ',';
	char presentItem = ':';

	char*** data = malloc(sizeof(char**) * 20000);
	if (data == NULL) {
		fprintf(stderr, "%s\n", "Something went wrong with first array");
		quit(ytQueue);
		exit(1);
	}

	*entries = malloc(sizeof(int) * 20000);
	if (*entries == NULL) {
		fprintf(stderr, "%s\n", "Entry array failed to initialize");
		quit(ytQueue);
		exit(1);
	}

	char newChar = fgetc(jsonFile);
	bool foundArray = false;
	bool potentialArray = false;
	printf("%s\n", "Setup was good");

	while (!foundArray){
		if (newChar == EOF) {
			fprintf(stderr, "%s\n", "Array not found in JSON");
			quit(ytQueue);
			exit(1);
		}

		if (newChar == '"') {
			printf("%s\n", "Found potential array");
			char tempBuffer[100];
			int charCount = 0;
			newChar = fgetc(jsonFile);
			while (newChar != '"' && newChar != EOF) {
				tempBuffer[charCount] = newChar;
				charCount++;
				newChar = fgetc(jsonFile);
			}
			tempBuffer[charCount] = '\0';
			if (strcmp(tempBuffer, array) == 0) {
				foundArray = true;
			}
		}

		newChar = fgetc(jsonFile);
	}

	printf("%s\n", "Found array in JSON");
	// We are gonna find our first entry
	while (newChar != '{' && newChar != EOF) {
		newChar = fgetc(jsonFile);
	}

	if (newChar == EOF) {
		fprintf(stderr, "%s\n", "Array not found in JSON");
		quit(ytQueue);
		exit(1);
	}

	int entryCount = 0;
	bool moreEntries = true;

	while (moreEntries) {
		data[entryCount] = malloc(sizeof(char*) * 20000);

		if (data[entryCount] == NULL) {
			fprintf(stderr, "%s\n", "Something went wrong with second array");
			quit(ytQueue);
			exit(1);
		}

		int itemCount = 0;

		// Reads one object in the array
		while (newChar != '}' && newChar != EOF) {
			newChar = fgetc(jsonFile);

			while(newChar != ':' && newChar != '}') {
				newChar = fgetc(jsonFile);
				if (newChar == EOF) {
					fprintf(stderr, "%s\n", "Array not found in JSON");
					quit(ytQueue);
					exit(1);
				}
			}

			// End of object
			if (newChar == '}') {
				break;
			}

			while (newChar != '"') {
				newChar = fgetc(jsonFile);
				if (newChar == EOF) {
					fprintf(stderr, "%s\n", "Array not found in JSON");
					quit(ytQueue);
					exit(1);
				}
			}

			char tempBuffer[100];
			newChar = fgetc(jsonFile);
			int charCount = 0;

			// We are gonna read string only
			while (newChar != '"' && newChar != EOF) {
				tempBuffer[charCount] = newChar;
				charCount++;
				newChar = fgetc(jsonFile);
			}

			tempBuffer[charCount] = '\0';

			data[entryCount][itemCount] = malloc(sizeof(char) * (charCount + 1));
			if (data[entryCount][itemCount] == NULL) {
				fprintf(stderr, "%s\n", "Something went wrong with third array");
				quit(ytQueue);
				exit(1);
			}

			strcpy(data[entryCount][itemCount], tempBuffer);
			itemCount++;
		}

		printf("%s\n", "Finished reading entry");

		char** temp = realloc(data[entryCount], sizeof(char*) * itemCount);
		if (temp == NULL) {
			fprintf(stderr, "%s\n", "Something went wrong with third array");
			quit(ytQueue);
			exit(1);
		}

		printf("The item count for this %d\n", itemCount);
		data[entryCount] = temp;
		(*entries)[entryCount] = itemCount;
		printf("%d\n", (*entries)[entryCount]);
		entryCount++;

		newChar = fgetc(jsonFile);
		// No commas? No more entries!
		if (newChar != ',') {
			moreEntries = false;
		} else {
			// We need to find the next object
			while (newChar != '{' && newChar != EOF) {
				newChar = fgetc(jsonFile);
			}
		}

		if (newChar == EOF) {
			fprintf(stderr, "%s\n", "Array ended prematurely in JSON");
			quit(ytQueue);
			exit(1);
		}
	}

	fclose(jsonFile);
	char*** temp = realloc(data, sizeof(char**) * entryCount);
	printf("Entries in the jsoNnFile %d\n", entryCount);
	if (temp == NULL) {
		fprintf(stderr, "%s\n", "Something went wrong with first array");
		quit(ytQueue);
		exit(1);
	}

	int* intTemp = realloc(*entries, sizeof(int) * entryCount);
	if (intTemp == NULL) {
		fprintf(stderr, "%s\n", "Something went wrong with realloc of int entries");
		quit(ytQueue);
		exit(1);
	}

	*entries = intTemp;

	data = temp;
	*objCount = entryCount;
	return data;
}

/*Creates a new JSON file or overwrites one contains a
JSON array nammed after array. Only takes in strings.
Covert to string first if writing ints, floats, etc.*/
void writeJSONArray(const char* fileName, const char* array, char** entryNames, char*** data, int objCount, int* entries) {
	FILE* jsonFile = fopen(fileName, "w");
	if (jsonFile == NULL) {
		fprintf(stderr, "%s\n", "Something has gone wrong with the JSON file/doesn't exist");
		quit(ytQueue);
		exit(1);
	}
	fprintf(jsonFile, "{\n\t\"%s\": [\n", array);


	// Goes through each obj in arrray
	for (int a = 0; a < objCount; a++) {
		fprintf(jsonFile, "\t\t{\n");
		for (int b = 0; b < entries[a]; b++) {
			fprintf(jsonFile, "\t\t\t\"%s\": \"%s\"", entryNames[b], data[a][b]);
			if (b != entries[a] - 1) {
				fprintf(jsonFile, ",");
			}
			fprintf(jsonFile, "\n");
		}
		fprintf(jsonFile, "\t\t}");
		if (a != objCount - 1) {
			fprintf(jsonFile, ",");
		}
		fprintf(jsonFile, "\n");
	}
	fprintf(jsonFile, "\t]\n}");
	fclose(jsonFile);
}	

/*Frees an Array read from I/O after it has been used up*/
void freeJSONArray(char*** data, int objCount, int* entries) {
	for (int a = 0; a < objCount; a++) {
		for (int b = 0; b < entries[a]; b++) {
			free(data[a][b]);
		}
		free(data[a]);
	}

	free(data);
	free(entries);
}

/*Takes a double char pointer, and concanates with fileOne and fileTwo to become a valid file
location. fileOne is everything before each file and fileTwo is everything afterwards
fileOne and fileTwo can be NULL, but both being NULL would do nothing*/
void formatAsFileLocation(const char* fileOne, const char* fileTwo, char** files, int count) {
	// Will literally do nothing with the input lol
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

__declspec(noreturn) void errorExit(const char* msg) {
	fprintf(stderr, "%s\n", msg);
	quit(ytQueue);
	exit(1);
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

/*It takes a string, and splits it every time it sees
the delimeter. 
The great joy of C is making a function that is built-in
on every other programming language.*/
char** split(const char* str, char delimeter, int* size) {
	char buffer[100];
	char** words = malloc(sizeof(char*) * 20000);
	if (words == NULL) {
		fprintf(stderr, "%s\n", "Something went wrong with the words");
		quit(ytQueue);
		exit(1);
	}

	// Initialize all so compiler don't whine
	for (int a  = 0; a < 100; a++) {
		buffer[a] = '\0';
	}

	int wordCount = 0;
	int charCount = 0;
	int totalCharCount = 0;
	char nextChar = str[charCount];
	while (nextChar != '\0') {
		if (nextChar != delimeter) {
			buffer[charCount] = nextChar;
		}
		else {
			// We don't put delimeter. Instead, we end string
			buffer[charCount] = '\0';
		}
		charCount++;
		totalCharCount++;
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

		nextChar = str[totalCharCount];
	}
	
	buffer[charCount] = '\0';
	charCount++;

	words[wordCount] = malloc(sizeof(char) * charCount);

	if (words[wordCount] == NULL) {
		fprintf(stderr, "Something went wrong with word %d\n", wordCount + 1);
		quit(ytQueue);
		exit(1);
	}

	// Add the last word in.
	strcpy(words[wordCount], buffer);

	wordCount++;

	int finalSize = wordCount;
	char** temp = realloc(words, sizeof(char*) * finalSize);
	if (temp == NULL) {
		fprintf(stderr, "%s\n", "Something went wrong with the words");
		quit(ytQueue);
		exit(1);
	}

	words = temp;
	*size = finalSize;
	return words;
}

/*Does a string concatination where
A) The size is guaranteed to be enough
B) It creates a new copy as oppose to overiding original*/
char* properConcat(const char* str1, const char* str2) {
	char* newStr = malloc(sizeof(char) * (strlen(str1) + strlen(str2) + 1));
	if (newStr == NULL) {
		fprintf(stderr, "%s\n", "Allocation of new string failed");
		quit(ytQueue);
		exit(1);
	}
	strcpy(newStr, str1);
	strcat(newStr, str2);
	return newStr;
}	

/*Given an array of string, joins them to become one string which are each seperated
by the string specified in newChar*/
char* join(char** arr, int lower, int upper, const char* newChar, int* newLen) {
	*newLen = 0;
	for (int a = lower; a <= upper; a++) {
		*newLen += strlen(arr[a]);
	}

	// Adding the frequency of the delimeter
	*newLen += strlen(newChar) * (upper - lower + 1);

	char* newStr = malloc(sizeof(char) * (*newLen + 1));
	if (newStr == NULL) {
		errorExit("The new string for join failed");
	}

	strcpy(newStr, arr[lower]);
	strcat(newStr, newChar);

	for (int a = lower + 1; a <= upper; a++) {
		strcat(newStr, arr[a]);
		strcat(newStr, newChar);
	}

	newStr[*newLen] = '\0';

	printf("%s %d\n", newStr, *newLen);
	
	return newStr;
}


/*Checks if the queue is running low, 
and if it is, it will ask the server for more data.*/
int expandQueue(zsock_t* requester, Queue* queue, int counter) {
	counter++;
	if (queue->size <= 4) {
		zsock_set_rcvtimeo(requester, 17);
		if (!connected) {
			if (zstr_recv(requester) != NULL) connected = true;
		}
		// Let the server know we are ready for more
		if (connected) zstr_send(requester, "Roger");
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

void deleteNode(YTNode* node) {
	SDL_DestroyTexture(node->img);
	free(node->filePath);
	free(node->sViews);
	free(node->subs);
	free(node);
}

void deQueue(Queue* queue, YTNode* next) {
	YTNode* oldFront = queue->front;
	deleteNode(oldFront);
	queue->front = next;
	if (queue->front == NULL || queue->front->next == NULL) {
		zstr_send(requester, "STOP");
		zsock_destroy(&requester);
		fprintf(stderr, "%s\n", "FUCKKK our first/second queue elements are messed up lol");
		exit(1);
	}
	YTNode* buffer = queue->front->next;
	if (buffer->next == NULL) {
		zstr_send(requester, "STOP");
		zsock_destroy(&requester);
		fprintf(stderr, "%s\n", "FUCKKK our third queue element are messed up lol");
		exit(1);
	}

	//continueGame(requester);
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

/*Creates a deep copy of the srcTxt DynamicTxt
into the dstTxt*/
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

/*Quits/Frees all global variables, stops the server*/
void quit(Queue* queue) {
	zstr_send(requester, "STOP");
	SDL_DestroySurface(screen->surface);
	free(screen);
	freeFontArray();
	deleteQueue(queue);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	TTF_DestroyGPUTextEngine(textEngine);
	TTF_Quit();
	SDL_Quit();

	for (int a = 0; a < offlineVideoCount; a++) {
		free(offlineVideos[a]);
	}

	free(offlineVideos);

	// We need to shutdown backup too.
	if (!offline && strcmp(zstr_recv(requester), "ONE_MORE") == 0) {
		zstr_send(requester, "STOP");
		zstr_recv(requester);
	}

	CloseHandle(hMutex);
	zsock_destroy(&requester);
}

bool difficultyUnlocked(int difficultyIndex) {
	// This difficulty is beaten or hardest accessible
	if (saveData[stars] >= difficultyIndex) return true;
	// Timer was beaten, so every difficulty is unlocked
	if (saveData[stars] >= SECOND_UNLOCK) return true;
	// Standard was beaten, not timer
	if (saveData[stars] >= FIRST_UNLOCK && difficultyIndex < SECOND_UNLOCK) return true;
	return false;
}