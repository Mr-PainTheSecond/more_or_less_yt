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

/*With the data grabbed from the backend, will add every datapoint which partains to each
video. This includes: View Count (int and char*), file name, and sub counts */
void storeYTData(Queue* queue,char* sData, int data, char* file_name, char* subCount) {
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

bool getYtData(zsock_t* connection, Queue* queue) {
	char* data = zstr_recv(connection);
	if (strcmp(data, "LOST") == 0) {
		fprintf(stderr, "%s\n", "CONNECTION LOST");
		return false;
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
