#include <stdio.h>
#include <raylib.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#define WIDTH 800
#define HEIGHT 600
#define G 6.6743e-11f
#define DAMPING 0.02

typedef struct {
    float x,y;
    float vx,vy;
    float ax,ay;
    float m;
    float r;
}Object;

static float pressX, pressY;
static bool dragging = false;
Object* objects = NULL;
int ObjectCount = 0;
int ObjectCapacity = 0;

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
    ObjectCount++;
}

void UpdateObjects() {
    for(int i = 0; i < ObjectCount; i++) {
        objects[i].x += objects[i].vx;
        objects[i].y += objects[i].vy;

        if(objects[i].x + objects[i].r >= WIDTH) {
            objects[i].vx = -(DAMPING)*objects[i].vx;
            objects[i].x = WIDTH - objects[i].r;
        }
        if(objects[i].x - objects[i].r <= 0) {
            objects[i].vx = -(DAMPING)*objects[i].vx;
            objects[i].x = objects[i].r;
        }
        if(objects[i].y + objects[i].r >= HEIGHT) {
            objects[i].vy = -(DAMPING)*objects[i].vy;
            objects[i].y = HEIGHT - objects[i].r;
        }
        if(objects[i].y - objects[i].r <= 0) {
            objects[i].vy = -(DAMPING)*objects[i].vy;
            objects[i].y = objects[i].y;
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
                  objects[i].x += nx*overlap/2;
                  objects[i].y += ny*overlap/2;
                  objects[j].x += -nx*overlap/2;
                  objects[j].y += -ny*overlap/2;

                  float tx = -ny;
                  float ty = nx;
                  float v1t = objects[i].vx*tx + objects[i].vy*ty;
                  float v2t = objects[j].vx*tx + objects[j].vy*ty;
                  float v1n = objects[i].vx*nx + objects[i].vy*ny;
                  float v2n = objects[j].vx*nx + objects[j].vy*ny;
                  float relVelAlongNormal = (objects[i].vx - objects[j].vx)*nx + (objects[i].vy - objects[j].vy)*ny;
                  if(relVelAlongNormal) {
                      float vflip = v2n;
                      v2n = v1n;
                      v1n = vflip;
                  }
                  objects[i].vx = v1n*nx + v1t*tx;
                  objects[i].vy = v1n*ny + v1t*ty;
                  objects[j].vx = v2n*nx + v2t*tx;
                  objects[j].vy = v2n*ny + v2t*ty;

              }
        }
    }    
} 

void ComputeForces() {
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
void handleScrollResize() {
    float wheelMove = GetMouseWheelMove();
    if (wheelMove == 0) return;

    Vector2 mousePos = GetMousePosition();
    const float GROWTH_RATE = 2.0f;
    const float DENSITY = 1.0f;
    const float MIN_RADIUS = 2.0f;

    for (int i = 0; i < ObjectCount; i++) {
        Vector2 objPos = {objects[i].x, objects[i].y};
        if (CheckCollisionPointCircle(mousePos, objPos, objects[i].r)) {
            float newRadius = objects[i].r + wheelMove * GROWTH_RATE;
            if (newRadius < MIN_RADIUS) newRadius = MIN_RADIUS;

            objects[i].r = newRadius;
            objects[i].m = DENSITY * newRadius * newRadius;

            break;
        }
    }
}
int main(void) {
    InitWindow(WIDTH, HEIGHT, "N-body");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        SetRandomSeed(time(NULL));

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            pressX = (float)GetMouseX();
            pressY = (float)GetMouseY();
            dragging = true;
        }

        if (dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
             float releaseX = (float)GetMouseX();
             float releaseY = (float)GetMouseY();

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

        BeginDrawing();
        handleScrollResize();
        UpdateObjects();
        InitCollisions();
        ComputeForces();
        ClearBackground(BLACK);
                
        for (int i = 0; i < ObjectCount; i++) {
            printf("%f\n", objects[i].r);
            DrawCircle((int)objects[i].x, (int)objects[i].y, objects[i].r, BLUE);
        }

        EndDrawing();
    }

    free(objects);
    CloseWindow();
    return 0;
}
