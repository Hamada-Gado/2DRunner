#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <glut.h>

// Game constants
// Running Config
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int FPS = 60;

// Game Config
const int INITIAL_LIVES = 5;
const int INITIAL_GAME_SPEED = 2.1f;
const int GAME_DURATION = 50;
const float GAME_SPEED_INCREASE = 0.003f;

// Player Config
const float PLAYER_SIZE = 40.0f;
const float PLAYER_HEAD_SIZE = 20.0f;
const float PLAYER_BASE_Y = 100.0f;
const float PLAYER_BASE_X = 50.0f;
const float JUMP_HEIGHT = 120.0f;
const float JUMP_SPEED_INIT = 10.0f;
const float DUCK_HEIGHT = 25.0f;

// Objects Config
const float OBSTACLE_SIZE = 40.0f;
const float COLLECTABLE_SIZE = 30.0f;
const float POWERUP_SIZE = 30.0f;
const int MAX_OBSTACLES = 10;
const int MAX_COLLECTABLES = 5;
const int MAX_POWERUPS = 2;
const float OBSTACLE_SPAWN_PROB = 5.0f;
const float COLLECTABLE_SPAWN_PROB = 3.0f;
const float POWERUP_SPAWN_PROB = 5.0f;
const float OBSTACLE_SPAWN_INTERVAL = 2.3f;
const float COLLECTABLE_SPAWN_INTERVAL = 0.6f;
const float POWERUP_SPAWN_INTERVAL = 20.0f;
const float POWERUP1_ACTIVE_TIME = 5;
const float POWERUP2_ACTIVE_TIME = 10;

// Game variables
int backgroundX = -WINDOW_WIDTH;
float gameSpeed;
float gameTime;
int gameState; // 0: Start, 1: Playing, 2: Game Over
int score;
int lives;
bool paused;
float playerY;
float jumpSpeed;
bool isJumping;
bool isDucking;
bool isInvincible;
bool isDoublePoints;
float powerup1ActiveTime;
float powerup2ActiveTime;
float obstacleSpawnTimer;
float collectableSpawnTimer;
float powerupSpawnTimer;
float obstacleSpawnInterval;
float collectableSpawnInterval;
float powerupSpawnInterval;
float collectableAngle;
float oscillatePowerupY;
float oscillatePowerupDY;

struct GameObject
{
    float x, y;
    bool active;
};

std::vector<GameObject> obstacles;
std::vector<GameObject> collectables;
std::vector<GameObject> powerups1;
std::vector<GameObject> powerups2;

// Function prototypes
void drawRect(float, float, float, float);
void drawCircle(int, int, float);
void drawShuriken(float, float, float);
void drawHeart(float, float);
void drawText(float, float, std::string);
void drawPlayer();
void drawObstacle(float, float);
void drawCollectable(float, float);
void drawPowerup(float, float, bool);
void drawHealth();
void drawScore();
void drawTime();
void drawPowerupsState();
void drawBackground();
void drawGameStart();
void drawGameOver();
void drawBoundaries();
void display();
void keyboard(unsigned char, int, int);
void keyboardUp(unsigned char, int, int);
void update(int);
void init();
void rollback();

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("just run :)");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(0, update, 0);

    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glutMainLoop();
    return 0;
}

void drawRect(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
    glVertex2d(x1, y1);
    glVertex2d(x2, y1);
    glVertex2d(x2, y2);
    glVertex2d(x1, y2);
    glEnd();
}

void drawCircle(int x, int y, float r)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    GLUquadric *quadObj = gluNewQuadric();
    gluDisk(quadObj, 0, r, 50, 50);
    glPopMatrix();
}

void drawShuriken(float x, float y, float r)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glBegin(GL_TRIANGLES);

    // Upper triangle
    glVertex2f(0, r);
    glVertex2f(-r / 2, 0);
    glVertex2f(r / 2, 0);

    // Lower triangle
    glVertex2f(0, -r);
    glVertex2f(-r / 2, 0);
    glVertex2f(r / 2, 0);

    // Right triangle
    glVertex2f(r, 0);
    glVertex2f(0, -r / 2);
    glVertex2f(0, r / 2);

    // Left triangle
    glVertex2f(-r, 0);
    glVertex2f(0, -r / 2);
    glVertex2f(0, r / 2);

    glEnd();
    glPopMatrix();
}

void drawHeart(float x, float y)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glBegin(GL_POLYGON);
    for (int j = 0; j < 360; j++)
    {
        float theta = j * 3.14f / 180.0f;
        float x = 16 * pow(sin(theta), 3);
        float y = 13 * cos(theta) - 5 * cos(2 * theta) - 2 * cos(3 * theta) - cos(4 * theta);
        glVertex2f(x, y);
    }
    glEnd();
    glPopMatrix();
}

void drawText(float x, float y, std::string text)
{
    glRasterPos2f(x, y);
    for (char c : text)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

void drawPlayer()
{
    glPushMatrix();
    glTranslatef(PLAYER_BASE_X, playerY, 0);

    // Body (Hexagon)
    glColor3f(0.3f, 0.2f, 0.4f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 6; ++i)
    {
        float theta = 2.0f * 3.14f * float(i) / float(6);
        float x = (PLAYER_SIZE / 2) * cos(theta);
        float y = (PLAYER_SIZE / 2) * sin(theta);
        glVertex2f(x, y);
    }
    glEnd();

    // Head (Pentagon)
    glColor3f(1.0f, 0.5f, 0.6f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 5; ++i)
    {
        float theta = 2.0f * 3.14f * float(i) / float(5);
        float x = (PLAYER_HEAD_SIZE / 2) * cos(theta);
        float y = (PLAYER_HEAD_SIZE / 2) * sin(theta);
        glVertex2f(x, y + PLAYER_SIZE / 2);
    }
    glEnd();

    // Eyes (Triangles)
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-PLAYER_HEAD_SIZE / 4, PLAYER_SIZE / 2 + PLAYER_HEAD_SIZE / 4);
    glVertex2f(-PLAYER_HEAD_SIZE / 6, PLAYER_SIZE / 2 + PLAYER_HEAD_SIZE / 6);
    glVertex2f(-PLAYER_HEAD_SIZE / 4, PLAYER_SIZE / 2 + PLAYER_HEAD_SIZE / 6);

    glVertex2f(PLAYER_HEAD_SIZE / 4, PLAYER_SIZE / 2 + PLAYER_HEAD_SIZE / 4);
    glVertex2f(PLAYER_HEAD_SIZE / 6, PLAYER_SIZE / 2 + PLAYER_HEAD_SIZE / 6);
    glVertex2f(PLAYER_HEAD_SIZE / 4, PLAYER_SIZE / 2 + PLAYER_HEAD_SIZE / 6);
    glEnd();

    // Mouth (Arc)
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 180; ++i)
    {
        float theta = 3.14f * float(i) / float(180);
        float x = (PLAYER_HEAD_SIZE / 4) * cosf(theta);
        float y = (PLAYER_HEAD_SIZE / 8) * sinf(theta);
        glVertex2f(x, PLAYER_SIZE / 2 - PLAYER_HEAD_SIZE / 4 + y);
    }
    glEnd();

    glPopMatrix();
}

void drawObstacle(float x, float y)
{
    glPushMatrix();
    glTranslatef(x, y, 0);

    // Base (Rectangle)
    glColor3f(1.0f, 0.0f, 0.0f);
    drawRect(OBSTACLE_SIZE / 2, OBSTACLE_SIZE / 2, -OBSTACLE_SIZE / 2, -OBSTACLE_SIZE / 2);

    // Right (Triangle)
    glColor3f(0.8f, 0.2f, 0.2f);
    glBegin(GL_TRIANGLES);
    glVertex2f(OBSTACLE_SIZE / 2, -OBSTACLE_SIZE / 2);
    glVertex2f(OBSTACLE_SIZE / 2, OBSTACLE_SIZE / 2);
    glVertex2f(OBSTACLE_SIZE, 0);
    glEnd();

    glPopMatrix();
}

void drawCollectable(float x, float y)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glRotatef(collectableAngle, 0, 0, 1);

    // Circle
    glColor3f(1.0f, 1.0f, 0.0f);
    drawCircle(0, 0, COLLECTABLE_SIZE / 2);

    // Shuriken (Triangles)
    glColor3f(1.0f, 0.5f, 0.1f);
    drawShuriken(0, 0, COLLECTABLE_SIZE / 2);

    // Center (Point)
    glColor3f(1.0f, 0.0f, 0.0f);
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glVertex2f(0, 0);
    glEnd();

    glPopMatrix();
}

void drawPowerup(float x, float y, bool isTypeOne)
{
    glPushMatrix();
    glTranslatef(x, y, 0);

    if (isTypeOne)
    {
        // Type One:
        // Diamond shape
        glColor3f(0.9f, 0.1f, 0.3f);
        glPushMatrix();
        glRotatef(45, 0, 0, 1);
        drawRect(POWERUP_SIZE / 2, POWERUP_SIZE / 2, -POWERUP_SIZE / 2, -POWERUP_SIZE / 2);
        glPopMatrix();

        // Shuriken shape
        glColor3f(0.0f, 1.0f, 0.5f);
        drawShuriken(0, 0, POWERUP_SIZE / 2);

        // Inner lines
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex2f(-POWERUP_SIZE / 2, 0);
        glVertex2f(POWERUP_SIZE / 2, 0);
        glVertex2f(0, POWERUP_SIZE / 2);
        glVertex2f(0, -POWERUP_SIZE / 2);
        glEnd();
    }
    else
    {
        // Type Two:
        // Shuriken shape
        glColor3f(0.0f, 1.0f, 0.0f);
        drawShuriken(0, 0, POWERUP_SIZE / 2);
        glPushMatrix();
        glRotatef(45, 0, 0, 1);
        drawShuriken(0, 0, POWERUP_SIZE / 2);
        glPopMatrix();

        // Center circle
        glColor3f(1.0f, 1.0f, 0.0f);
        drawCircle(0, 0, POWERUP_SIZE / 6);
    }

    glPopMatrix();
}

void drawHealth()
{
    for (int i = 0; i < lives; i++)
    {
        // Heart shape
        glColor3f(1.0f, 0.0f, 0.0f);
        drawHeart(30 + i * 40, WINDOW_HEIGHT - 30);

        glPushMatrix();
        glTranslatef(30 + i * 40, WINDOW_HEIGHT - 45, 0);
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINE_STRIP);
        glVertex3f(-10, 0, 0);
        glVertex3f(10, 0, 0);
        glEnd();
        glPopMatrix();
    }
}

void drawScore()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    std::string scoreStr = "Score: " + std::to_string(score);
    drawText(WINDOW_WIDTH - 111, WINDOW_HEIGHT - 22, scoreStr);
}

void drawTime()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    std::string timeStr = "Time: " + std::to_string(int(gameTime));
    drawText(WINDOW_WIDTH / 2 - 55, WINDOW_HEIGHT - 22, timeStr);
}

void drawPowerupsState()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    std::string powerup1 = "Invincibility: ";
    if (isInvincible)
    {
        powerup1 += std::to_string(int(powerup1ActiveTime));
        ;
    }
    else
    {
        powerup1 += "NONE";
    }
    std::string powerup2 = "Double Points: ";
    if (isDoublePoints)
    {
        powerup2 += std::to_string(int(powerup2ActiveTime));
        ;
    }
    else
    {
        powerup2 += "NONE";
    }
    drawText(WINDOW_WIDTH / 2 + 111, WINDOW_HEIGHT - 22, powerup1);
    drawText(WINDOW_WIDTH / 2 + 111, WINDOW_HEIGHT - 44, powerup2);
}

void drawBackground() {
    // Background Color
    glClearColor(0.0f, 0.1f, 0.9f, 1.0f);

    // Sun
    glColor3f(1.0f, 1.0f, 0.0f);
    drawCircle(WINDOW_WIDTH - 50, WINDOW_HEIGHT - 150, 25);

    glPushMatrix();
    glTranslatef(-backgroundX, -90, 0);

    // Clouds
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 4; i++)
        drawRect(i * 200, WINDOW_HEIGHT - 50 - 100 * i, i * 200 + 100, WINDOW_HEIGHT - 100 * i);
    glPopMatrix();
}

void drawGameStart()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    std::string startStr = "Press 'Space' to start, 'p' to pause, 'r' to restart, and 'Esc' to exit";
    drawText(WINDOW_WIDTH / 2 - 250, (float)WINDOW_HEIGHT / 2, startStr);
    std::string controlsStr = "Controls: 'j' to duck, 'k' to jump";
    drawText(WINDOW_WIDTH / 2 - 100, (float)WINDOW_HEIGHT / 2 - 30, controlsStr);
    std::string powerupsStr = "Powerups: Diamond - Invincibility, Shuriken - Double Points";
    drawText(WINDOW_WIDTH / 2 - 150, (float)WINDOW_HEIGHT / 2 - 60, powerupsStr);
}

void drawGameOver()
{
    if (gameTime <= 0)
    {
        glColor3f(0.0f, 1.0f, 0.0f);
        std::string timeUpStr = "Time's Up!";
        drawText(WINDOW_WIDTH / 2 - 50, (float)WINDOW_HEIGHT / 2, timeUpStr);
    }
    else
    {
        glColor3f(1.0f, 0.0f, 0.0f);
        std::string gameOverStr = "Game Over!";
        drawText(WINDOW_WIDTH / 2 - 50, (float)WINDOW_HEIGHT / 2, gameOverStr);
    }

    std::string livesStr = "Lives Remaining: " + std::to_string(lives);
    drawText(WINDOW_WIDTH / 2 - 70, WINDOW_HEIGHT / 2 - 30, livesStr);

    std::string timeStr = "Time Remaining: " + std::to_string(int(gameTime));
    drawText(WINDOW_WIDTH / 2 - 70, WINDOW_HEIGHT / 2 - 60, timeStr);

    std::string scoreStr = "Final Score: " + std::to_string(score);
    drawText(WINDOW_WIDTH / 2 - 70, WINDOW_HEIGHT / 2 - 90, scoreStr);
}

void drawBoundaries()
{
    glColor3f(0.5f, 0.5f, 0.5f);
    drawRect(0, WINDOW_HEIGHT - 55, WINDOW_WIDTH, WINDOW_HEIGHT);

    drawRect(0, 0, WINDOW_WIDTH, PLAYER_BASE_Y - PLAYER_SIZE / 2);

    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < WINDOW_WIDTH; i += 111)
    {
        glVertex2f(i, WINDOW_HEIGHT - 55);
        glVertex2f(i + 55, WINDOW_HEIGHT - 55);
        glVertex2f(i + 25, WINDOW_HEIGHT - 25);
    }

    glEnd();

    drawRect(0, PLAYER_BASE_Y - PLAYER_SIZE / 2, WINDOW_WIDTH, PLAYER_BASE_Y - PLAYER_SIZE / 2 - 20);
    drawRect(0, PLAYER_BASE_Y - PLAYER_SIZE / 2 - 40, WINDOW_WIDTH, PLAYER_BASE_Y - PLAYER_SIZE / 2 - 60); 
    drawRect(0, PLAYER_BASE_Y - PLAYER_SIZE / 2 - 80, WINDOW_WIDTH, PLAYER_BASE_Y - PLAYER_SIZE / 2 - 100);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    drawBackground();

    if (gameState == 0)
    {
        drawGameStart();
    }
    else if (gameState == 1)
    {
        drawPlayer();

        for (auto &obstacle : obstacles)
        {
            if (obstacle.active)
            {
                drawObstacle(obstacle.x, obstacle.y);
            }
        }

        for (auto &collectable : collectables)
        {
            if (collectable.active)
            {
                drawCollectable(collectable.x, collectable.y);
            }
        }

        for (auto &powerup : powerups1)
        {
            if (powerup.active)
            {
                drawPowerup(powerup.x, powerup.y, true);
            }
        }

        for (auto &powerup : powerups2)
        {
            if (powerup.active)
            {
                drawPowerup(powerup.x, powerup.y, false);
            }
        }

        drawBoundaries();
        drawHealth();
        drawScore();
        drawTime();
        drawPowerupsState();
    }
    else
    {
        drawGameOver();
    }

    glFlush();
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 'r')
    {
        init();
        gameState = 1;
    }
    else if (key == ' ' && gameState == 0)
    {
        gameState = 1;
    }
    else if (key == 'j' && playerY <= PLAYER_BASE_Y && !isJumping && !isDucking)
    {
        isDucking = true;
    }
    else if (key == 'k' && playerY <= PLAYER_BASE_Y && !isJumping && !isDucking)
    {
        isJumping = true;
    }
    else if (key == 'p')
    {
        paused = !paused;
    }
    else if (key == 27)
    {
        exit(0);
    }
}

void keyboardUp(unsigned char key, int x, int y)
{
    if (key == 'j')
    {
        isDucking = false;
    }
}

void update(int value)
{
    backgroundX += 1;
    if (backgroundX >= WINDOW_WIDTH)
    {
        backgroundX = -WINDOW_WIDTH;
    }

    if (gameState == 1 && !paused)
    {
        // Update timings
        gameTime -= 1.0 / FPS;
        obstacleSpawnTimer -= 1.0 / FPS;
        collectableSpawnTimer -= 1.0 / FPS;
        powerupSpawnTimer -= 1.0 / FPS;

        collectableAngle += 5.0f;
        oscillatePowerupY += oscillatePowerupDY * 0.5;
        if (oscillatePowerupY > 2 || oscillatePowerupY < -2)
            oscillatePowerupDY *= -1;

        // Update powerups
        if (isInvincible)
            powerup1ActiveTime -= 1.0 / FPS;
        if (isDoublePoints)
            powerup2ActiveTime -= 1.0 / FPS;

        if (powerup1ActiveTime <= 0)
        {
            isInvincible = false;
            powerup1ActiveTime = POWERUP1_ACTIVE_TIME;
        }
        if (powerup2ActiveTime <= 0)
        {
            isDoublePoints = false;
            powerup2ActiveTime = POWERUP2_ACTIVE_TIME;
        }

        if (gameTime <= 0)
        {
            gameState = 2;
        }

        // Update game speed
        gameSpeed += GAME_SPEED_INCREASE;
        jumpSpeed += GAME_SPEED_INCREASE;
        obstacleSpawnInterval = OBSTACLE_SPAWN_INTERVAL / gameSpeed;
        collectableSpawnInterval = COLLECTABLE_SPAWN_INTERVAL / gameSpeed;
        powerupSpawnInterval = POWERUP_SPAWN_INTERVAL / gameSpeed;

        // Update player position
        if (isJumping)
        {
            playerY += jumpSpeed;
            if (playerY >= PLAYER_BASE_Y + JUMP_HEIGHT)
            {
                isJumping = false;
            }
        }
        else if (playerY > PLAYER_BASE_Y && !isDucking)
        {
            playerY -= jumpSpeed;
        }

        if (isDucking)
        {
            playerY = PLAYER_BASE_Y - DUCK_HEIGHT;
        }
        else if (!isJumping && playerY < PLAYER_BASE_Y)
        {
            playerY = PLAYER_BASE_Y;
        }

        // Update obstacles
        for (auto &obstacle : obstacles)
        {
            if (obstacle.active)
            {
                obstacle.x -= 2.7 * gameSpeed;

                if (!isInvincible && abs(obstacle.x - PLAYER_BASE_X) < PLAYER_SIZE / 2 + OBSTACLE_SIZE / 2 && abs(obstacle.y - playerY) < PLAYER_SIZE / 2 + OBSTACLE_SIZE / 2)
                {
                    lives--;
                    obstacle.active = false;
                    if (lives <= 0)
                    {
                        gameState = 2;
                    }
                    else
                    {
                        rollback();
                    }
                }


                if (obstacle.x < -OBSTACLE_SIZE)
                {
                    obstacle.active = false;
                }
            }
        }

        // Update collectables
        for (auto &collectable : collectables)
        {
            if (collectable.active)
            {
                collectable.x -= 4 * gameSpeed;

                if (abs(collectable.x - PLAYER_BASE_X) < PLAYER_SIZE / 2 + COLLECTABLE_SIZE / 2 &&
                    abs(collectable.y - playerY) < PLAYER_SIZE / 2 + COLLECTABLE_SIZE / 2)
                {
                    score += isDoublePoints ? 20 : 10;
                    collectable.active = false;
                }

                if (collectable.x < -COLLECTABLE_SIZE)
                {
                    collectable.active = false;
                }
            }
        }

        // Update powerups
        for (auto &powerup : powerups1)
        {
            if (powerup.active)
            {
                powerup.x -= 3.5 * gameSpeed;
                powerup.y += oscillatePowerupY;

                if (abs(powerup.x - PLAYER_BASE_X) < PLAYER_SIZE / 2 + POWERUP_SIZE / 2 &&
                    abs(powerup.y - playerY) < PLAYER_SIZE / 2 + POWERUP_SIZE / 2)
                {
                    isInvincible = true;
                    powerup1ActiveTime = POWERUP1_ACTIVE_TIME;
                    powerup.active = false;
                }

                if (powerup.x < -POWERUP_SIZE)
                {
                    powerup.active = false;
                }
            }
        }

        for (auto &powerup : powerups2)
        {
            if (powerup.active)
            {
                powerup.x -= 3.5 * gameSpeed;
                powerup.y += oscillatePowerupY;

                if (abs(powerup.x - PLAYER_BASE_X) < PLAYER_SIZE / 2 + POWERUP_SIZE / 2 &&
                    abs(powerup.y - playerY) < PLAYER_SIZE / 2 + POWERUP_SIZE / 2)
                {
                    isDoublePoints = true;
                    powerup2ActiveTime = POWERUP2_ACTIVE_TIME;
                    powerup.active = false;
                }

                if (powerup.x < -POWERUP_SIZE)
                {
                    powerup.active = false;
                }
            }
        }

        // Spawn new objects
        if (obstacleSpawnTimer <= 0 && obstacles.size() < MAX_OBSTACLES && rand() % 100 < OBSTACLE_SPAWN_PROB)
        {
            GameObject newObstacle;
            newObstacle.x = WINDOW_WIDTH;
            newObstacle.y = (PLAYER_BASE_Y + PLAYER_SIZE / 2 + PLAYER_HEAD_SIZE / 2) - (rand() % 2) * DUCK_HEIGHT;
            newObstacle.active = true;
            obstacles.push_back(newObstacle);
            obstacleSpawnTimer = obstacleSpawnInterval;
        }

        if (
            collectableSpawnTimer <= 0 &&
            collectables.size() < MAX_COLLECTABLES && rand() % 100 < COLLECTABLE_SPAWN_PROB)
        {
            GameObject newCollectable;
            newCollectable.x = WINDOW_WIDTH;
            newCollectable.y = PLAYER_BASE_Y + (rand() % int(JUMP_HEIGHT - PLAYER_HEAD_SIZE / 2));
            newCollectable.active = true;
            collectables.push_back(newCollectable);
            collectableSpawnTimer = collectableSpawnInterval;
        }

        bool isTypeOne = rand() % 2;
        if (
            isTypeOne &&
            powerupSpawnTimer <= 0 &&
            powerups1.size() + powerups2.size() < MAX_POWERUPS && rand() % 100 < POWERUP_SPAWN_PROB)
        {
            GameObject newPowerup;
            newPowerup.x = WINDOW_WIDTH;
            newPowerup.y = PLAYER_BASE_Y + (rand() % int(JUMP_HEIGHT - PLAYER_HEAD_SIZE / 2));
            newPowerup.active = true;
            powerups1.push_back(newPowerup);
            powerupSpawnTimer = powerupSpawnInterval;
        }

        if (
            !isTypeOne &&
            powerupSpawnTimer <= 0 &&
            powerups1.size() + powerups2.size() < MAX_POWERUPS && rand() % 100 < POWERUP_SPAWN_PROB)
        {
            GameObject newPowerup;
            newPowerup.x = WINDOW_WIDTH;
            newPowerup.y = PLAYER_BASE_Y + (rand() % int(JUMP_HEIGHT - PLAYER_HEAD_SIZE / 2));
            newPowerup.active = true;
            powerups2.push_back(newPowerup);
            powerupSpawnTimer = powerupSpawnInterval;
        }

        // remove inactive objects
        obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
                                       [](GameObject &o)
                                       { return !o.active; }),
                        obstacles.end());
        collectables.erase(std::remove_if(collectables.begin(), collectables.end(),
                                          [](GameObject &c)
                                          { return !c.active; }),
                           collectables.end());
        powerups1.erase(std::remove_if(powerups1.begin(), powerups1.end(),
                                       [](GameObject &p)
                                       { return !p.active; }),
                        powerups1.end());
        powerups2.erase(std::remove_if(powerups2.begin(), powerups2.end(),
                                       [](GameObject &p)
                                       { return !p.active; }),
                        powerups2.end());
    }

    glutPostRedisplay();
    glutTimerFunc(1000 / FPS, update, 0);
}

void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    srand(time(nullptr));

    // Initialize game variables
    gameSpeed = INITIAL_GAME_SPEED;
    gameTime = GAME_DURATION;
    lives = INITIAL_LIVES;
    score = 0;

    gameState = 0;
    paused = false;

    playerY = PLAYER_BASE_Y;
    jumpSpeed = JUMP_SPEED_INIT;

    isJumping = false;
    isDucking = false;
    isInvincible = false;
    isDoublePoints = false;

    powerup1ActiveTime = POWERUP1_ACTIVE_TIME;
    powerup2ActiveTime = POWERUP2_ACTIVE_TIME;

    obstacleSpawnTimer = 0;
    collectableSpawnTimer = 0;
    powerupSpawnTimer = 0;
    obstacleSpawnInterval = OBSTACLE_SPAWN_INTERVAL;
    collectableSpawnInterval = COLLECTABLE_SPAWN_INTERVAL;
    powerupSpawnInterval = POWERUP_SPAWN_INTERVAL;

    collectableAngle = 0;
    oscillatePowerupY = 0;
    oscillatePowerupDY = 1;

    obstacles.clear();
    collectables.clear();
    powerups1.clear();
    powerups2.clear();
}

void rollback()
{
    // Rollback game variables when player hits an obstacle
    gameSpeed = INITIAL_GAME_SPEED;
    gameState = 1;
    paused = false;

    playerY = PLAYER_BASE_Y;
    jumpSpeed = JUMP_SPEED_INIT;

    isJumping = false;
    isDucking = false;
    isInvincible = false;
    isDoublePoints = false;

    powerup1ActiveTime = POWERUP1_ACTIVE_TIME;
    powerup2ActiveTime = POWERUP2_ACTIVE_TIME;

    obstacleSpawnTimer = 0;
    collectableSpawnTimer = 0;
    powerupSpawnTimer = 0;
    obstacleSpawnInterval = OBSTACLE_SPAWN_INTERVAL;
    collectableSpawnInterval = COLLECTABLE_SPAWN_INTERVAL;
    powerupSpawnInterval = POWERUP_SPAWN_INTERVAL;

    collectableAngle = 0;

    oscillatePowerupY = 0;
    oscillatePowerupDY = 1;

    obstacles.clear();
    collectables.clear();
    powerups1.clear();
    powerups2.clear();
}