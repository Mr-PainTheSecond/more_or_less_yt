#include "server.h"

zsock_t* establishConnection() {
	zsock_t* requester = zsock_new(ZMQ_REQ);
	if (zsock_connect(requester, "tcp://localhost:5555")) {
		fprintf(stderr, "%s\n", "Connection failed");
		SDL_DestroySurface(screen->surface);
		free(screen);
		freeFontArray();
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		exit(1);
	}

	zstr_send(requester, "Roger");
	return requester;
}

/*With the data grabbed from the backend/offline, will add every datapoint which partains to each
video. This includes: View Count (int and char*), file name, and sub counts */
void storeYTData(Queue* queue, char* sData, int data, char* file_name, char* subCount) {
	YTNode* dataNode = malloc(sizeof(YTNode));
	if (dataNode == NULL) {
		quit(queue);
		exit(1);
	}

	if (queue->front == NULL) {
		queue->front = dataNode;
		queue->back = dataNode;
	}
	else {
		queue->back->next = dataNode;
		queue->back = dataNode;
	}

	queue->size++;
	dataNode->views = data;
	dataNode->filePath = malloc(sizeof(char) * strlen(file_name) + 1);
	if (dataNode->filePath == NULL) {
		quit(queue);
		exit(1);
	}

	strcpy(dataNode->filePath, file_name);

	dataNode->sViews = malloc(sizeof(char) * strlen(sData) + 1);
	if (dataNode->sViews == NULL) {
		quit(queue);
		exit(1);
	}

	strcpy(dataNode->sViews, sData);

	dataNode->subs = malloc(sizeof(char) * strlen(subCount) + 1);
	if (dataNode->subs == NULL) {
		quit(queue);
		exit(1);
	}

	strcpy(dataNode->subs, subCount);

	printf("%s\n", dataNode->filePath);

	SDL_Surface* surf = IMG_Load(dataNode->filePath);
	if (surf == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());	
		quit(queue);
		exit(1);
	}

	dataNode->img = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_DestroySurface(surf);
	//printf("%s\n", dataNode->filePath);
	//printf("%s\n", dataNode->sViews);
	dataNode->next = NULL;
}

//void continueGame(zsock_t* connection) {
//	zstr_send(connection, "CONTINUE");
//	// Just for communication
//	char* buffer = zstr_recv(connection);
//}

/*When the server goes offline or is busy, this handles all the parts 
which are different when the server isn't involved*/
void storeYTDataOffline(Queue* queue, char* filePath, int count) {
	char** chosenVideos = malloc(sizeof(char*) * count);
	if (chosenVideos == NULL) {
		fprintf(stderr, "%s\n", "No storage for the videos :(");
		quit(queue);
		exit(1);
	}


	for (int a = 0; a < count; a++) {
		chosenVideos[a] = choiceStr(offlineVideos, offlineVideoCount);
	}


	/*From each video, we are gonna grab the path,
	views, and subs, grab them with split, and add to the
	queue*/
	for (int b = 0; b < count; b++) {
		int size = 4;
		char** dataPoint = malloc(sizeof(char*) * size);

		if (dataPoint == NULL) {
			fprintf(stderr, "%s\n", "No storage for the data point :(");
			quit(queue);
			exit(1);
		}

		dataPoint = split(chosenVideos[b], ':', &size);

		// Perform deep copies of the data points
		char* path = malloc(sizeof(char) * (strlen(dataPoint[0]) + 1));

		// First char is a space, so we need to ignore it
		char* sViews = malloc(sizeof(char) * (strlen(dataPoint[1])));

		// TODO: Fix duplicate view data
		char* subs = malloc(sizeof(char) * (strlen(dataPoint[3])));

		if (path == NULL || sViews == NULL || subs == NULL) {
			fprintf(stderr, "%s\n", "No storage for one of the data points");
			quit(queue);
			exit(1);
		}

		strcpy(path, dataPoint[0]);
		// Deletes an extra space at the beginning of the string
		strcpy(sViews, &(dataPoint[1][1]));

		strcpy(subs, &(dataPoint[3][1]));

		for (int a = 0; a < size; a++) {
			free(dataPoint[a]);
		}

		int views = convertToInt(sViews);

		/*printf("Subs: %s\n", subs);
		printf("Views: %s\n", sViews);
		printf("Views (int): %d\n", views);
		printf("Path: %s\n", path);*/

		storeYTData(queue, sViews, views, path, subs);

		free(sViews);
		free(path);
		free(subs);
	}
	
	// We leave the invidual points for the quit funciton
	free(chosenVideos);
}

/*When the game is running low on video, this function 
requests for more. If the server is busy or offline, we use
data that is readily avaliable. Otherwise, data from the
SQL db and stuff from the Google API is used*/
bool getYtData(zsock_t* connection, Queue* queue) {
	char* data = zstr_recv(connection);

	// Server is busy getting data, so we need to use offline data
	if (strcmp(data, "NOT_READY") == 0) {
		printf("%s\n", "System works?");
		storeYTDataOffline(queue, "..\\assets\\data\\offline_storage.txt", 20);
		return true;
	}

	// Server is offline, so we need to use offline data
	if (strcmp(data, "LOST") == 0) {
		fprintf(stderr, "%s\n", "CONNECTION LOST");
		storeYTDataOffline(queue, "..\\assets\\data\\offline_storage.txt", 20);
		return true;
	}

	while (strcmp(data, "-1") != 0) {
		int intData = convertToInt(data);
		zstr_send(connection, "Roger");
		char* fileName = zstr_recv(connection);
		zstr_send(connection, "Roger");
		char* subCount = zstr_recv(connection);
		storeYTData(queue, data, intData, fileName, subCount);
		zstr_send(connection, "Roger");
		zstr_free(&fileName);
		zstr_free(&subCount);
		data = zstr_recv(connection);
	}

	YTNode* current = queue->front;
	while (current != NULL) {
		current = current->next;
	}

	return true;
}

void startServer() {
	char path[] = "..\\server";

	if (access(path, 0) == 0) {
		// Starts the database
		system("start python.exe ..\\server\\main.py");
		system("cls");
	}
	else {
		printf("%s\n", "Start up Failed");
		exit(1);
	}
}
