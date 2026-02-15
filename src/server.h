#ifndef SERVER_H_
#define SERVER_H_
#include "commons.h"
#include "font.h"
#include "globals.h"
#include "utilities.h"
zsock_t* establishConnection();
bool getYtData(zsock_t* connection, Queue* queue);
SDL_FRect createRect(float x, float y, float w, float h, bool centered);
void startServer();
#endif
