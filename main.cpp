/*
 * Bomberman 3D com visao isometrica (versao compativel com GCC 4.4.1)
 */

#include <GL/glut.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
using namespace std;

#define MAP_SIZE 10
#define ESC 27

int map[MAP_SIZE][MAP_SIZE]; // 0: vazio, 1: parede, 2: bloco destruivel
int player_x = 1, player_z = 1;

float cam_angle_y = 45.0f;
float cam_angle_x = 45.0f;
float cam_dist = 15.0f;

struct Bomba {
    int x, z;
    int timer;
    bool explodiu;
    int frame_explosao;
};

vector<Bomba> bombas;

void initMap() {
    for (int x = 0; x < MAP_SIZE; x++) {
        for (int z = 0; z < MAP_SIZE; z++) {
            if (x == 0 || z == 0 || x == MAP_SIZE - 1 || z == MAP_SIZE - 1)
                map[x][z] = 1; // parede
            else if ((x % 2 == 0 && z % 2 == 0))
                map[x][z] = 1; // parede fixa
            else
                map[x][z] = (rand() % 4 == 0 ? 2 : 0); // bloco aleatorio ou vazio
        }
    }
    map[1][1] = 0; // espaco inicial do jogador
}

void drawCube(float r, float g, float b) {
    glColor3f(r, g, b);
    glutSolidCube(1.0);
}

void drawMap() {
    for (int x = 0; x < MAP_SIZE; x++) {
        for (int z = 0; z < MAP_SIZE; z++) {
            glPushMatrix();
            glTranslatef((float)x, -0.5f, (float)z);
            if (map[x][z] == 1)
                drawCube(0.3f, 0.3f, 0.3f); // parede
            else if (map[x][z] == 2)
                drawCube(0.6f, 0.4f, 0.2f); // bloco
            glPopMatrix();
        }
    }
}

void drawPlayer() {
    glPushMatrix();
    glTranslatef((float)player_x, 0.0f, (float)player_z);
    glColor3f(0.0f, 0.0f, 1.0f);
    glutSolidSphere(0.4, 16, 16);
    glPopMatrix();
}

void drawBombs() {
    for (size_t i = 0; i < bombas.size(); i++) {
        if (!bombas[i].explodiu && bombas[i].timer > 0) {
            glPushMatrix();
            glTranslatef((float)bombas[i].x, 0.0f, (float)bombas[i].z);
            glColor3f(0.0f, 0.0f, 0.0f);
            glutSolidSphere(0.3, 10, 10);
            glPopMatrix();
        }
    }
}

void drawExplosions() {
    for (size_t i = 0; i < bombas.size(); i++) {
        if (bombas[i].explodiu && bombas[i].frame_explosao > 0) {
            glColor3f(1.0f, 0.3f, 0.0f);
            for (int dx = -1; dx <= 1; dx++) {
                for (int dz = -1; dz <= 1; dz++) {
                    if (abs(dx) + abs(dz) == 1) {
                        glPushMatrix();
                        glTranslatef((float)(bombas[i].x + dx), 0.0f, (float)(bombas[i].z + dz));
                        glutSolidSphere(0.3, 10, 10);
                        glPopMatrix();
                    }
                }
            }
            bombas[i].frame_explosao--;
        }
    }
}

void updateCamera() {
    float rad_y = cam_angle_y * 3.141592f / 180.0f;
    float rad_x = cam_angle_x * 3.141592f / 180.0f;

    float eye_x = cam_dist * cos(rad_x) * sin(rad_y);
    float eye_y = cam_dist * sin(rad_x);
    float eye_z = cam_dist * cos(rad_x) * cos(rad_y);

    gluLookAt(eye_x, eye_y, eye_z, 5, 0, 5, 0, 1, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    updateCamera();

    drawMap();
    drawPlayer();
    drawBombs();
    drawExplosions();

    glutSwapBuffers();
}

void timer(int v) {
    vector<Bomba> novas;
    for (size_t i = 0; i < bombas.size(); i++) {
        if (bombas[i].timer > 0) {
            // Ainda esta contando para explodir
            bombas[i].timer--;
            novas.push_back(bombas[i]);
        } 
        else if (!bombas[i].explodiu) {
            // Explodiu agora!
            for (int dx = -1; dx <= 1; dx++) {
                for (int dz = -1; dz <= 1; dz++) {
                    if (abs(dx) + abs(dz) == 1) {
                        int nx = bombas[i].x + dx, nz = bombas[i].z + dz;
                        if (map[nx][nz] == 2) map[nx][nz] = 0;
                    }
                }
            }

            bombas[i].explodiu = true;
            bombas[i].frame_explosao = 4;  // ? tempo de duracao da explosao (4 ciclos = 2s se timerFunc=500ms)
            novas.push_back(bombas[i]);
        } 
        else if (bombas[i].frame_explosao > 0) {
            // Esta no tempo da explosao ainda
            bombas[i].frame_explosao--;
            novas.push_back(bombas[i]);
        }
        // ?? Quando frame_explosao chega a 0, a bomba e removida da lista (desaparece tudo)
    }

    bombas = novas;

    glutTimerFunc(150, timer, 0); // Diminua aqui se quiser animacaes mais rapidas (ex: 100)
    glutPostRedisplay();
}


void keyboard(unsigned char key, int, int) {
    if (key == ESC) exit(0);
    if (key == ' ') {
        Bomba nova;
        nova.x = player_x;
        nova.z = player_z;
        nova.timer = 4;
        nova.explodiu = false;
        nova.frame_explosao = 0;
        bombas.push_back(nova);
    } else if (key == 'q') cam_angle_y -= 5;
    else if (key == 'e') cam_angle_y += 5;
    else if (key == 'z') cam_angle_x -= 5;
    else if (key == 'x') cam_angle_x += 5;
    else if (key == '-') cam_dist += 1.0f;
    else if (key == '+') cam_dist -= 1.0f;

    glutPostRedisplay();
}

void special(int key, int, int) {
    int dx = 0, dz = 0;
    if (key == GLUT_KEY_UP) dz = -1;
    else if (key == GLUT_KEY_DOWN) dz = 1;
    else if (key == GLUT_KEY_LEFT) dx = -1;
    else if (key == GLUT_KEY_RIGHT) dx = 1;

    int nx = player_x + dx, nz = player_z + dz;
    if (map[nx][nz] == 0) {
        player_x = nx;
        player_z = nz;
    }
    glutPostRedisplay();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float)w / (float)h, 1, 100);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    srand((unsigned int)time(0));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Bomberman 3D Isometrico");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.8f, 0.9f, 1.0f, 1.0f);

    initMap();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutTimerFunc(100, timer, 0);

    glutMainLoop();
    return 0;
}


