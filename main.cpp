#include <SDL.h>
#include <math.h>

int screenWidth;
int screenHeight;

bool isDebugging = true;
bool isDebuggingKeyDown = false;

const int TARGET_WIDTH = 320;
const int TARGET_HEIGHT = 180;
const float TARGET_ASPECT_RATIO = (float)TARGET_WIDTH / TARGET_HEIGHT;

const int LEVEL_ROWS = 10;
const int LEVEL_COLUMNS = 10;
int levelData[LEVEL_ROWS][LEVEL_COLUMNS] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 1, 1, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 1, 1, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
};
const int LEVEL_BLOCK_SIZE = 64;

const int PLAYER_BLOCK_SIZE = 15;
float playerX = LEVEL_BLOCK_SIZE * 3.5;
float playerY = LEVEL_BLOCK_SIZE * 7.5;

const float PLAYER_MOVE_SPEED = 64;
float playerForwardX;
float playerForwardY;
float playerRightX;
float playerRightY;

const int PLAYER_DIRECTION_LENGTH = 30;
const float PLAYER_TURN_SPEED = 180;
float playerAngle = 90;

Uint64 previousTimestampMs;
float deltaTimeSeconds;

int mapX = 10;
int mapY = 10;

void initTime() {
    previousTimestampMs = SDL_GetTicks64();
    deltaTimeSeconds = 0;
}

void updateTime() {
    Uint64 currentTimestampMs = SDL_GetTicks64();
    deltaTimeSeconds = (currentTimestampMs - previousTimestampMs) / 1000.0f;
    previousTimestampMs = currentTimestampMs;
}

float degToRad(float degrees) {
    return degrees * M_PI / 180;
}

void updatePlayerRotationData(float newPlayerAngle) {
    playerAngle = newPlayerAngle;
    while (playerAngle < 0) {
        playerAngle += 360;
    }
    while (playerAngle >= 360) {
        playerAngle -= 360;
    }

    playerForwardX = cos(degToRad(-playerAngle));
    playerForwardY = sin(degToRad(-playerAngle));
    playerRightX = cos(degToRad(-playerAngle + 90));
    playerRightY = sin(degToRad(-playerAngle + 90));
}

void drawWallLine(SDL_Renderer* renderer, int x, int height) {
    if (height <= 0) {
        return;
    }

    int topY = (TARGET_HEIGHT - height) / 2;
    int bottomY = topY + height - 1;
    SDL_RenderDrawLine(renderer, x, topY, x, bottomY);
}

void drawMap(SDL_Renderer* renderer) {
    SDL_Rect rectangle;
    for (int row = 0; row < LEVEL_ROWS; row++) {
        for (int column = 0; column < LEVEL_COLUMNS; column++) {
            if (levelData[row][column] > 0) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }

            rectangle.x = mapX + LEVEL_BLOCK_SIZE * column + 1;
            rectangle.y = mapY + LEVEL_BLOCK_SIZE * row + 1;
            rectangle.w = LEVEL_BLOCK_SIZE - 2;
            rectangle.h = LEVEL_BLOCK_SIZE - 2;
            SDL_RenderFillRect(renderer, &rectangle);
        }
    }
}

void drawPlayer(SDL_Renderer* renderer) {
    SDL_Rect player;
    player.x = mapX + playerX - PLAYER_BLOCK_SIZE / 2;
    player.y = mapY + playerY - PLAYER_BLOCK_SIZE / 2;
    player.w = PLAYER_BLOCK_SIZE;
    player.h = PLAYER_BLOCK_SIZE;
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(renderer, &player);

    SDL_RenderDrawLine(
        renderer,
        mapX + playerX,
        mapY + playerY,
        mapX + playerX + playerForwardX * PLAYER_DIRECTION_LENGTH,
        mapY + playerY + playerForwardY * PLAYER_DIRECTION_LENGTH
    );
}

void processInput() {
    const Uint8* keyboardState = SDL_GetKeyboardState(NULL);

    if (!isDebuggingKeyDown && keyboardState[SDL_SCANCODE_TAB]) {
        isDebuggingKeyDown = true;
        isDebugging = !isDebugging;
    } else if (!keyboardState[SDL_SCANCODE_TAB]) {
        isDebuggingKeyDown = false;
    }

    int turnInput = keyboardState[SDL_SCANCODE_J] - keyboardState[SDL_SCANCODE_L];
    if (turnInput != 0) {
        updatePlayerRotationData(playerAngle + turnInput * PLAYER_TURN_SPEED * deltaTimeSeconds);
    }

    int strafeInput = keyboardState[SDL_SCANCODE_D] - keyboardState[SDL_SCANCODE_A];
    int moveForwardInput = keyboardState[SDL_SCANCODE_W] - keyboardState[SDL_SCANCODE_S];
    if (strafeInput != 0) {
        playerX += playerRightX * strafeInput * PLAYER_MOVE_SPEED * deltaTimeSeconds;
        playerY += playerRightY * strafeInput * PLAYER_MOVE_SPEED * deltaTimeSeconds;
    }
    if (moveForwardInput != 0) {
        playerX += playerForwardX * moveForwardInput * PLAYER_MOVE_SPEED * deltaTimeSeconds;
        playerY += playerForwardY * moveForwardInput * PLAYER_MOVE_SPEED * deltaTimeSeconds;
    }
}

void renderView(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 0);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int x = 0; x < TARGET_WIDTH; x++) {
        drawWallLine(renderer, x, (float)(x + 1) / TARGET_WIDTH * TARGET_HEIGHT);
    }
}

SDL_Rect determineViewDisplayRectangle() {
    SDL_Rect viewDisplayRectangle;

    if (isDebugging) {
        int displayHeight = (screenWidth / 2) / TARGET_ASPECT_RATIO;
        viewDisplayRectangle.x = screenWidth / 2;
        viewDisplayRectangle.y = (screenHeight - displayHeight) / 2;
        viewDisplayRectangle.w = screenWidth / 2;
        viewDisplayRectangle.h = displayHeight;
    } else {
        int proposedScreenHeight = screenWidth / TARGET_ASPECT_RATIO;
        if (proposedScreenHeight <= screenHeight) {
            viewDisplayRectangle.x = 0;
            viewDisplayRectangle.y = (screenHeight - proposedScreenHeight) / 2;
            viewDisplayRectangle.w = screenWidth;
            viewDisplayRectangle.h = proposedScreenHeight;
        } else {
            int proposedScreenWidth = screenHeight * TARGET_ASPECT_RATIO;
            viewDisplayRectangle.x = (screenWidth - proposedScreenWidth) / 2;
            viewDisplayRectangle.y = 0;
            viewDisplayRectangle.w = proposedScreenWidth;
            viewDisplayRectangle.h = screenHeight;
        }
    }

    return viewDisplayRectangle;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("Raycast Demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, TARGET_WIDTH, TARGET_HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, TARGET_WIDTH, TARGET_HEIGHT);

    SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

    initTime();
    updatePlayerRotationData(playerAngle);

    bool gameIsRunning = true;
    while (gameIsRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    gameIsRunning = false;
                    break;
            }
        }

        updateTime();

        processInput();

        SDL_SetRenderTarget(renderer, target);
        renderView(renderer);
        SDL_SetRenderTarget(renderer, NULL);


        if (isDebugging) {
            SDL_SetRenderDrawColor(renderer, 64, 64, 64, 0);
            SDL_RenderClear(renderer);

            drawMap(renderer);
            drawPlayer(renderer);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);
        }

        SDL_Rect viewDisplayRectangle = determineViewDisplayRectangle();
        SDL_RenderCopy(renderer, target, NULL, &viewDisplayRectangle);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(target);
    SDL_DestroyWindow(window);
    SDL_Quit();
}