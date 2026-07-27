#include <stdio.h>
#include <raylib.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#define WORLD_WIDTH 4000
#define WORLD_HEIGHT 3000
#define WIDTH 800
#define HEIGHT 600
#define G 6.6743e-11f
#define DAMPING 0.01
#define TRAIL_LENGTH 100

typedef struct {
    float x,y;
    float vx,vy;
    float ax,ay;
    float m;
    float r;
    Vector2 trail[TRAIL_LENGTH];
    int trailHead;
    int trailCount;
}Object;

static float pressX, pressY;
static bool dragging = false;
Object* objects = NULL;
int ObjectCount = 0;
int ObjectCapacity = 0;
Camera2D camera = { 0 };

void AddObject(float x, float y, float vx, float vy, float ax, float ay, float m, float r) {
    if(ObjectCount >= ObjectCapacity) {
        ObjectCapacity = (ObjectCapacity == 0) ? 8 : ObjectCapacity*2;
        objects = realloc(objects, ObjectCapacity*sizeof(Object));
        if(objects ==  NULL) {
            printf("ERROR! out of memory!");
            exit(1);
        }
    }
    objects[ObjectCount].x = x;
    objects[ObjectCount].y = y;
    objects[ObjectCount].vx = vx;
    objects[ObjectCount].vy = vy;
    objects[ObjectCount].ax = ax;
    objects[ObjectCount].ay = ay;
    objects[ObjectCount].m = m;
    objects[ObjectCount].r = r;
    objects[ObjectCount].trailHead = 0;
    objects[ObjectCount].trailCount = 0;
    ObjectCount++;
}
void UpdateObjects() {
    Vector2 topLeft     = GetScreenToWorld2D((Vector2){0, 0}, camera);
    Vector2 bottomRight = GetScreenToWorld2D((Vector2){WIDTH, HEIGHT}, camera);

    for(int i = 0; i < ObjectCount; i++) {
        objects[i].x += objects[i].vx;
        objects[i].y += objects[i].vy;

        if(objects[i].x + objects[i].r >= bottomRight.x) {
            objects[i].vx = -(DAMPING)*objects[i].vx;
            objects[i].x = bottomRight.x - objects[i].r;
        }
        if(objects[i].x - objects[i].r <= topLeft.x) {
            objects[i].vx = -(DAMPING)*objects[i].vx;
            objects[i].x = topLeft.x + objects[i].r;
        }
        if(objects[i].y + objects[i].r >= bottomRight.y) {
            objects[i].vy = -(DAMPING)*objects[i].vy;
            objects[i].y = bottomRight.y - objects[i].r;
        }
        if(objects[i].y - objects[i].r <= topLeft.y) {
            objects[i].vy = -(DAMPING)*objects[i].vy;
            objects[i].y = topLeft.y + objects[i].r;
        }
    }
}
void InitCollisions() {
    for(int i = 0; i < ObjectCount; i++) {
        for(int j = 0; j < ObjectCount; j++) {
              if (i == j) 
                  break;
              Vector2 c1 = {objects[i].x,objects[i].y};
              Vector2 c2 = {objects[j].x,objects[j].y};
              bool collision = CheckCollisionCircles(c1,objects[i].r,c2,objects[j].r);
              if(collision) {
                  float dx = objects[i].x - objects[j].x;
                  float dy = objects[i].y - objects[j].y;
                  float dis  = sqrt(dx*dx + dy*dy);
                  if (dis == 0) {dis = 0.001f; dx = dis; }

                  float nx = dx/dis;
                  float ny = dy/dis;
                  float overlap = objects[i].r + objects[j].r - dis;

                  float m1 = objects[i].m;
                  float m2 = objects[j].m;
                  float totalMass = m1 + m2;

                  objects[i].x += nx*overlap*(m2/totalMass);
                  objects[i].y += ny*overlap*(m2/totalMass);
                  objects[j].x += -nx*overlap*(m1/totalMass);
                  objects[j].y += -ny*overlap*(m1/totalMass);

                  float tx = -ny;
                  float ty = nx;
                  float v1t = objects[i].vx*tx + objects[i].vy*ty;
                  float v2t = objects[j].vx*tx + objects[j].vy*ty;
                  float v1n = objects[i].vx*nx + objects[i].vy*ny;
                  float v2n = objects[j].vx*nx + objects[j].vy*ny;

                  float relVelAlongNormal = (objects[i].vx - objects[j].vx)*nx + (objects[i].vy - objects[j].vy)*ny;
                  if(relVelAlongNormal < 0) { 
                      float v1nAfter = ((m1 - m2)*v1n + 2*m2*v2n) / totalMass;
                      float v2nAfter = ((m2 - m1)*v2n + 2*m1*v1n) / totalMass;
                      v1n = v1nAfter;
                      v2n = v2nAfter;
                  }

                  objects[i].vx = v1n*nx + v1t*tx;
                  objects[i].vy = v1n*ny + v1t*ty;
                  objects[j].vx = v2n*nx + v2t*tx;
                  objects[j].vy = v2n*ny + v2t*ty;
              }
        }
    }    
}void ComputeForces() {
    float dt = 0.0001;
    const float SOFTENING = 1.0f;
    for (int i = 0; i < ObjectCount; i++) {
        objects[i].ax = 0;
        objects[i].ay = 0;
    }
    for (int i = 0; i < ObjectCount; i++) {
        for (int j = i + 1; j < ObjectCount; j++) {
            float dx = objects[i].x - objects[j].x;
            float dy = objects[i].y - objects[j].y;
            float dis2 = dx*dx + dy*dy + SOFTENING;
            float dis = sqrt(dis2);
            float Force = ((G * objects[i].m * objects[j].m) / dis2)*10e13;
            objects[i].ax += -(Force / objects[i].m) * (dx / dis);
            objects[i].ay += -(Force / objects[i].m) * (dy / dis);
            objects[j].ax +=  (Force / objects[j].m) * (dx / dis);
            objects[j].ay +=  (Force / objects[j].m) * (dy / dis);
        }
    }
    for (int i = 0; i < ObjectCount; i++) {
        objects[i].vx += objects[i].ax * dt;
        objects[i].vy += objects[i].ay * dt;
        objects[i].x  += objects[i].vx * dt;
        objects[i].y  += objects[i].vy * dt;
    }
}
void handleScrollResize(Vector2 mouseWorld) {
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) return; 
    float wheelMove = GetMouseWheelMove();
    if (wheelMove == 0) return;

    const float GROWTH_RATE = 2.0f;
    const float DENSITY = 1.0f;
    const float MIN_RADIUS = 2.0f;

    for (int i = 0; i < ObjectCount; i++) {
        Vector2 objPos = {objects[i].x, objects[i].y};
        if (CheckCollisionPointCircle(mouseWorld, objPos, objects[i].r)) {
            float newRadius = objects[i].r + wheelMove * GROWTH_RATE;
            if (newRadius < MIN_RADIUS) newRadius = MIN_RADIUS;

            objects[i].r = newRadius;
            objects[i].m = DENSITY * newRadius * newRadius;

            break;
        }
    }
}

void updateTrails() {
    for (int i = 0; i < ObjectCount; i++) {
        objects[i].trail[objects[i].trailHead] = (Vector2){objects[i].x, objects[i].y};
        objects[i].trailHead = (objects[i].trailHead + 1) % TRAIL_LENGTH;
        if (objects[i].trailCount < TRAIL_LENGTH) objects[i].trailCount++;
    }
}

void drawTrails() {
    for (int i = 0; i < ObjectCount; i++) {
        int count = objects[i].trailCount;
        for (int k = 0; k < count - 1; k++) {
            int idx0 = (objects[i].trailHead + k) % TRAIL_LENGTH;
            int idx1 = (objects[i].trailHead + k + 1) % TRAIL_LENGTH;
            float alpha = (float)k / (float)count;
            Color trailColor = Fade(SKYBLUE, alpha);
            DrawLineV(objects[i].trail[idx0], objects[i].trail[idx1], trailColor);
        }
    }
}

void drawDragArrow(float startX, float startY, float mouseX, float mouseY) {
    Vector2 start = {startX, startY};
    Vector2 dragVec = {startX - mouseX, startY - mouseY}; 
    Vector2 end = {start.x + dragVec.x, start.y + dragVec.y};

    DrawLineEx(start, end, 2.0f, RED);

    float angle = atan2f(dragVec.y, dragVec.x);
    float headLen = 10.0f;
    Vector2 left  = {end.x - headLen*cosf(angle - PI/6), end.y - headLen*sinf(angle - PI/6)};
    Vector2 right = {end.x - headLen*cosf(angle + PI/6), end.y - headLen*sinf(angle + PI/6)};
    DrawTriangle(end, left, right, RED);
}
void resetSimulation() {
     ObjectCount = 0; 
}

void drawResetButton(Rectangle btn) {
    Vector2 mousePos = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mousePos, btn);

    DrawRectangleRec(btn, hovered ? RED : MAROON);
    DrawRectangleLinesEx(btn, 2, WHITE);
    DrawText("Reset", (int)(btn.x + 10), (int)(btn.y + 8), 18, WHITE);
}
void initCamera() {
    camera.target = (Vector2){ WIDTH/2.0f, HEIGHT/2.0f };
    camera.offset = (Vector2){ WIDTH/2.0f, HEIGHT/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void handleCameraZoom() {
    float wheelMove = GetMouseWheelMove();
    if (wheelMove == 0) return;
    if (!IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL)) return; 

    Vector2 mouseWorldBefore = GetScreenToWorld2D(GetMousePosition(), camera);

    const float ZOOM_SPEED = 0.1f;
    camera.zoom += wheelMove * ZOOM_SPEED * camera.zoom; 
    if (camera.zoom < 0.1f) camera.zoom = 0.1f;
    if (camera.zoom > 5.0f) camera.zoom = 5.0f;

    Vector2 mouseWorldAfter = GetScreenToWorld2D(GetMousePosition(), camera);

    camera.target.x += (mouseWorldBefore.x - mouseWorldAfter.x);
    camera.target.y += (mouseWorldBefore.y - mouseWorldAfter.y);
}

void handleCameraPan() {
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta = GetMouseDelta();
        camera.target.x -= delta.x / camera.zoom;
        camera.target.y -= delta.y / camera.zoom;
    }
}
int main(void) {
    InitWindow(WIDTH, HEIGHT, "N-body");
    SetTargetFPS(60);
    SetRandomSeed(time(NULL));
    initCamera();

    Rectangle resetBtn = {WIDTH - 90, 10, 80, 32};

    while (!WindowShouldClose()) {

        handleCameraZoom();
        handleCameraPan();
        Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
        Vector2 mouseScreen = GetMousePosition(); 

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mouseScreen, resetBtn)) {
                resetSimulation();
            } else {
                pressX = mouseWorld.x;
                pressY = mouseWorld.y;
                dragging = true;
            }
        }
        if (dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            float releaseX = mouseWorld.x;
            float releaseY = mouseWorld.y;

            float dx = releaseX - pressX;
            float dy = releaseY - pressY;
            float scale = 0.04f;
            float vx = -dx * scale;
            float vy = -dy * scale;

            float r = (float)GetRandomValue(10, 20);
            float m = r * 10.0f;
            AddObject(pressX, pressY, vx, vy, 0.0f, 0.0f, m, r);

            dragging = false;
        }
        handleScrollResize(mouseWorld); 
        UpdateObjects();
        InitCollisions();
        ComputeForces();
        updateTrails();

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);

            drawTrails();

            for (int i = 0; i < ObjectCount; i++) {
                DrawCircle((int)objects[i].x, (int)objects[i].y, objects[i].r, BLUE);
            }

            if (dragging) {
                drawDragArrow(pressX, pressY, mouseWorld.x, mouseWorld.y);
            }

        EndMode2D();

        drawResetButton(resetBtn); 

        EndDrawing();
    }

    free(objects);
    CloseWindow();
    return 0;
}
