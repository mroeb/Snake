#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <string>

// ANSI escape codes for colors and styles
const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string BOLD = "\033[1m";

// Log functions
void logInfo(const std::string& message) {
    std::cout << GREEN << "[INFO] " << RESET << message << std::endl;
}

void logWarning(const std::string& message) {
    std::cout << YELLOW << "[WARNING] " << RESET << message << std::endl;
}

void logError(const std::string& message) {
    std::cout << RED << "[ERROR] " << RESET << message << std::endl;
}

SDL_Window* window;
SDL_Renderer* renderer;
TTF_Font* font;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int SNAKE_SIZE = 20;

enum Direction { UP, DOWN, LEFT, RIGHT };

struct Point {
    int x, y;
};


class SnakeGame {
public:
    SnakeGame() : direction(RIGHT), running(true), score(0) {
        snake.push_back({ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 });
        placeFood();
        logInfo("Game initialized.");
    }

    void run() {
        logInfo("Game started.");
        while (running) {
            handleEvents();
            update();
            render();
            SDL_Delay(100); // Control the speed of the game
        }
        renderGameOver();
    }

private:
    std::vector<Point> snake;
    Point food;
    Direction direction;
    bool running;
    int score;

    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                logInfo("Quit event received.");
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    if (direction != DOWN) direction = UP;
                    logInfo("Direction changed to UP.");
                    break;
                case SDLK_DOWN:
                    if (direction != UP) direction = DOWN;
                    logInfo("Direction changed to DOWN.");
                    break;
                case SDLK_LEFT:
                    if (direction != RIGHT) direction = LEFT;
                    logInfo("Direction changed to LEFT.");
                    break;
                case SDLK_RIGHT:
                    if (direction != LEFT) direction = RIGHT;
                    logInfo("Direction changed to RIGHT.");
                    break;
                }
            }
        }
    }

    void update() {
        Point newHead = snake.front();
        switch (direction) {
        case UP: newHead.y -= SNAKE_SIZE; break;
        case DOWN: newHead.y += SNAKE_SIZE; break;
        case LEFT: newHead.x -= SNAKE_SIZE; break;
        case RIGHT: newHead.x += SNAKE_SIZE; break;
        }

        // Check for collision with walls
        if (newHead.x < 0 || newHead.x >= SCREEN_WIDTH || newHead.y < 0 || newHead.y >= SCREEN_HEIGHT) {
            running = false;
            logError("Collision with wall detected.");
        }

        // Check for collision with itself
        for (const auto& segment : snake) {
            if (newHead.x == segment.x && newHead.y == segment.y) {
                running = false;
                logError("Collision with self detected.");
            }
        }

        // Check for food consumption
        if (newHead.x == food.x && newHead.y == food.y) {
            snake.insert(snake.begin(), newHead);
            placeFood();
            score++;
            logInfo("Food consumed. Score: " + std::to_string(score));
        }
        else {
            snake.insert(snake.begin(), newHead);
            snake.pop_back();
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw snake
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (const auto& segment : snake) {
            SDL_Rect rect = { segment.x, segment.y, SNAKE_SIZE, SNAKE_SIZE };
            SDL_RenderFillRect(renderer, &rect);
        }

        // Draw food
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect rect = { food.x, food.y, SNAKE_SIZE, SNAKE_SIZE };
        SDL_RenderFillRect(renderer, &rect);

        // Draw score
        renderText("Score: " + std::to_string(score), 10, 10, { 255, 255, 255, 255 });

        SDL_RenderPresent(renderer);
    }

    void renderGameOver() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderText("Game Over", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, { 255, 0, 0, 255 });
        renderText("Score: " + std::to_string(score), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, { 255, 255, 255, 255 });
        renderButton("Restart", SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 + 50, { 0, 255, 0, 255 });
        renderButton("Quit", SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 + 110, { 255, 0, 0, 255 });

        SDL_RenderPresent(renderer);

        // Wait for user to click the restart or quit button
        bool waiting = true;
        while (waiting) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    waiting = false;
                    running = false;
                    logInfo("Quit event received.");
                }
                else if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    if (x >= SCREEN_WIDTH / 2 - 50 && x <= SCREEN_WIDTH / 2 + 50) {
                        if (y >= SCREEN_HEIGHT / 2 + 50 && y <= SCREEN_HEIGHT / 2 + 100) {
                            waiting = false;
                            restartGame();
                            logInfo("Game restarted.");
                        }
                        else if (y >= SCREEN_HEIGHT / 2 + 110 && y <= SCREEN_HEIGHT / 2 + 160) {
                            waiting = false;
                            running = false;
                            logInfo("Game quit.");
                        }
                    }
                }
            }
        }
    }

    void renderText(const std::string& message, int x, int y, SDL_Color color) {
        SDL_Surface* surface = TTF_RenderText_Solid(font, message.c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect rect = { x, y, surface->w, surface->h };
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }

    void renderButton(const std::string& message, int x, int y, SDL_Color color) {
        SDL_Rect buttonRect = { x, y, 100, 50 };
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &buttonRect);
        renderText(message, x + 10, y + 10, { 0, 0, 0, 255 });
    }

    void placeFood() {
        std::srand(static_cast<unsigned int>(std::time(0)));
        food.x = (std::rand() % (SCREEN_WIDTH / SNAKE_SIZE)) * SNAKE_SIZE;
        food.y = (std::rand() % (SCREEN_HEIGHT / SNAKE_SIZE)) * SNAKE_SIZE;
        logInfo("Food placed at (" + std::to_string(food.x) + ", " + std::to_string(food.y) + ").");
    }

    void restartGame() {
        snake.clear();
        snake.push_back({ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 });
        direction = RIGHT;
        score = 0;
        running = true;
        placeFood();
        run();
    }
};

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        logError("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
        return 1;
    }

    if (TTF_Init() == -1) {
        logError("TTF could not initialize! TTF_Error: " + std::string(TTF_GetError()));
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        logError("IMG could not initialize! IMG_Error: " + std::string(IMG_GetError()));
        return 1;
    }

    window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        logError("Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        logError("Renderer could not be created! SDL_Error: " + std::string(SDL_GetError()));
        return 1;
    }

    font = TTF_OpenFont("resources/font/Silvertones.ttf", 24);
    if (font == NULL) {
        logError("Failed to load font! TTF_Error: " + std::string(TTF_GetError()));
        return 1;
    }

    // Load and set the window icon
    SDL_Surface* iconSurface = IMG_Load("resources/icon.png");
    if (iconSurface == NULL) {
        logError("Failed to load icon! IMG_Error: " + std::string(IMG_GetError()));
        return 1;
    }
    SDL_SetWindowIcon(window, iconSurface);
    SDL_FreeSurface(iconSurface);
    logInfo("Window icon set.");


    SnakeGame game;
    game.run();

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    logInfo("Game terminated.");
    return 0;
}
