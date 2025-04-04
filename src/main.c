
#include "raylib.h"
#include "raymath.h"

const float PLAYER_SPEED            = 100.0f;
const float PLAYER_ROTATION_SPEED   = 100.0;
const float SHIP_HALF_HEIGHT        = 5.0 / 0.363970;
const float PHYSICS_TIME            = 0.02;
const int FONT_SIZE                 = 20;

typedef struct Player {
	Vector2 position;
	Vector2 topPoint;
	Vector2 leftPoint;
	Vector2 rightPoint;
	Vector2 speed;
	float rotation;
	float acceleration;
	bool isTurnLeft;
	bool isTurnRight;
	bool isAccelerating;
	bool isBreaking;
} Player;

typedef enum GameStateType {
    StateInGame = 1,
    StateStartMenu = 2,
    StateGameOver = 3,
} GameStateType;

typedef struct Game {
	int width;
	int height;
	float fwidth;
	float fheight;
	float halfWidth;
	float halfHeight;
	float frameTimeAccumulator;
	bool isPlayerRotationChange;
	float currentScore;
	float highestScore;
	GameStateType state;
	Player player;
} Game;

Game game = {0};
float menu_size_width = 200.0f;
float item_menu_size_height = 50.0f;
Rectangle exitMenuRec = {0};
Rectangle startMenuRec = {0};
Rectangle restartMenuRec = {0};

 
int MenuButtom(Rectangle buttom, const char *buttom_text) {
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), buttom))
    {
        return true;
    }
    DrawRectangleRec(buttom, GRAY);

    DrawText(buttom_text, buttom.x + 20, buttom.y + buttom.height / 2 - 10, 20, WHITE);
    return 0;
}

void PlaceUIButtons(){
    game.width = GetScreenWidth();
    game.height = GetScreenHeight();
	game.halfWidth = game.width / 2.0;
	game.halfHeight = game.height / 2.0;
    // Add start button
    startMenuRec.x = (game.width / 2) - menu_size_width / 2;
    startMenuRec.y = (game.height / 2) - item_menu_size_height / 1.5f;
    startMenuRec.width = menu_size_width;
    startMenuRec.height = item_menu_size_height;
    // Add restart button
    restartMenuRec = startMenuRec;
    // Add exit button
    exitMenuRec.x = (game.width / 2) - menu_size_width / 2;
    exitMenuRec.y = (game.height / 2) + item_menu_size_height / 1.5f;
    exitMenuRec.width = menu_size_width;
    exitMenuRec.height = item_menu_size_height;
}

void ResetPlayer() {
	game.player.position.x = game.halfWidth;
    game.player.position.y = game.halfHeight - (SHIP_HALF_HEIGHT / 2.0f);
    game.player.speed.x = 0.0f;
    game.player.speed.y = 0.0f;
	game.player.acceleration = 0.0f;
    game.player.isAccelerating = false;
    game.player.isBreaking = false;
    game.player.isTurnLeft = false;
    game.player.isTurnRight = false;
	game.frameTimeAccumulator = 0.0f;

	game.isPlayerRotationChange = false;
}

void UpdateFrame() {
    if(IsWindowResized()){
        PlaceUIButtons();
    }
    // Tick
    if(game.state == StateInGame){
		// Input
		if(IsKeyDown(KEY_LEFT)) {
			game.player.isTurnLeft = true;
			game.isPlayerRotationChange = true;
		} else {
			game.player.isTurnLeft = false;
		}
		if(IsKeyDown(KEY_RIGHT)) {
			game.player.isTurnRight = true;
			game.isPlayerRotationChange = true;
		} else {
			game.player.isTurnRight = false;
		}

		if(IsKeyDown(KEY_UP)) {
			game.player.isAccelerating = true;
		} else {
			game.player.isAccelerating = false;
		}

		if(IsKeyDown(KEY_DOWN)) {
			game.player.isBreaking = true;
		} else {
			game.player.isBreaking = false;
		}

		// Physics
        game.frameTimeAccumulator += GetFrameTime();
		if (game.frameTimeAccumulator > PHYSICS_TIME) {
			game.frameTimeAccumulator = 0.0f;

			float acceleration = PLAYER_SPEED * PHYSICS_TIME;

			if(game.player.isTurnLeft) {
				game.player.rotation -= PLAYER_ROTATION_SPEED * PHYSICS_TIME;
			} else if(game.player.isTurnRight) {
				game.player.rotation += PLAYER_ROTATION_SPEED * PHYSICS_TIME;
			}

			if(game.isPlayerRotationChange) {
				if(game.player.rotation > 180.0) {
					game.player.rotation -= 360.0;
				}
				if(game.player.rotation < -180.0) {
					game.player.rotation += 360.0;
				}
			}
			if(game.player.isAccelerating) {
				if(game.player.acceleration < PLAYER_SPEED) {
					game.player.acceleration += acceleration;
				}
			} else if(game.player.acceleration > 0.0f) {
				game.player.acceleration -= acceleration / 2.0;
			} else if(game.player.acceleration < 0.0f) {
				game.player.acceleration = 0.0f;
			}
			if(game.player.isBreaking) {
				if(game.player.acceleration > 0.0f) {
					game.player.acceleration -= acceleration;
				} else if(game.player.acceleration < 0.0f) {
					game.player.acceleration = 0.0f;
				}
			}

			Vector2 direction = {
				sin(game.player.rotation * DEG2RAD),
				-cos(game.player.rotation * DEG2RAD)
			};
			Vector2 norm_vector = Vector2Normalize(direction);
			game.player.speed = Vector2Scale(norm_vector, game.player.acceleration * PHYSICS_TIME);
			game.player.position = Vector2Add(game.player.position, game.player.speed);
			// Update Triangle Rotation
			if(Vector2Length(game.player.speed) > 0.0) {
				if(game.player.position.x > game.width+SHIP_HALF_HEIGHT) {
					game.player.position.x = -SHIP_HALF_HEIGHT;
				} else if(game.player.position.x < -SHIP_HALF_HEIGHT) {
					game.player.position.x = game.width + SHIP_HALF_HEIGHT;
				}

				if(game.player.position.y > game.height+SHIP_HALF_HEIGHT) {
					game.player.position.y = -SHIP_HALF_HEIGHT;
				} else if(game.player.position.y < -SHIP_HALF_HEIGHT) {
					game.player.position.y = game.height + SHIP_HALF_HEIGHT;
				}
			}
		}
    }

    // Draw
    BeginDrawing();
        ClearBackground(WHITE);
        DrawText(TextFormat("FPS: %d", GetFPS()), game.width-100, 12, FONT_SIZE, BLACK);
        switch (game.state)
        {
            case StateInGame:
                DrawText(TextFormat("Speed: %d", (int)game.player.acceleration), 20, 12, FONT_SIZE, BLACK);
                // Draw In Game UI
        
                float cosf = cos(game.player.rotation * DEG2RAD);
                float sinf = sin(game.player.rotation * DEG2RAD);
                
                game.player.topPoint.x = game.player.position.x + sinf*SHIP_HALF_HEIGHT;
                game.player.topPoint.y = game.player.position.y - cosf*SHIP_HALF_HEIGHT;
                // Temp vector to center the rotation
                Vector2 v1tmp = {
                    game.player.position.x - sinf*SHIP_HALF_HEIGHT,
                    game.player.position.y + cosf*SHIP_HALF_HEIGHT,
                };
                game.player.rightPoint.x = v1tmp.x - cosf*(SHIP_HALF_HEIGHT-2.0);
                game.player.rightPoint.y = v1tmp.y - sinf*(SHIP_HALF_HEIGHT-2.0);

                game.player.leftPoint.x = v1tmp.x + cosf*(SHIP_HALF_HEIGHT-2.0);
                game.player.leftPoint.y = v1tmp.y + sinf*(SHIP_HALF_HEIGHT-2.0);
                DrawTriangleLines(game.player.topPoint, game.player.rightPoint, game.player.leftPoint, GRAY);
                break;
            case StateStartMenu:
                if (MenuButtom(startMenuRec, "Start Game"))
                {
                    // Initialize game
                    game.state = StateInGame;
                }
                if (MenuButtom(exitMenuRec, "Exit Game")){
                    // Exit game
                    CloseWindow();
                }
                
                break;
            case StateGameOver:
                if (MenuButtom(restartMenuRec, "Restart Game"))
                {
                    // Initialize game
                    ResetPlayer();
                    game.state = StateInGame;
                }
                if (MenuButtom(exitMenuRec, "Exit Game"))
                {
                    // Exit game
                    CloseWindow();
                }
                break;
        }
    EndDrawing();

}

int main(void) {
    // Start game
    game.width = 640;
    game.height = 360;
    // Start Raylib Window
    InitWindow(game.width, game.height, "Space C");

    PlaceUIButtons();
    ResetPlayer();

    game.state = StateStartMenu;

    // Main game loop
    while (!WindowShouldClose()) {
        UpdateFrame();
    }

    CloseWindow();

    return 0;
}