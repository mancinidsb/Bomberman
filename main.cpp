/*
 * Bomberman 3D com visao isometrica (versao compativel com GCC 4.4.1)
 */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
using namespace std;

#define MAP_SIZE 13
#define ESC 27

struct Model {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<unsigned int> indices;
    std::vector<int> material_ids; // IDs dos materiais para cada vértice
    std::vector<tinyobj::material_t> materials; // Lista de materiais
};

// Declarações de funções
void updateCamera();
void display();
void drawMap();
void drawPlayer();
void drawEnemies();
void drawBombs();
void drawExplosions();
void drawGameOver();
void drawVictory();
void drawGroundTextured();
void drawCube(float r, float g, float b);
void drawCubeTextured(GLuint tex);
void drawModel(const Model& model);
void drawModelWithColor(const Model& model, float r, float g, float b);
bool loadModel(const char* filename, Model& model);
GLuint loadTexture(const char* filename);
void initMap();
void timer(int v);
void keyboard(unsigned char key, int, int);
void special(int key, int, int);
void reshape(int w, int h);
bool hasBomb(int x, int z);
bool playerInExplosion(int bomb_x, int bomb_z);
int enemyInExplosion(int bomb_x, int bomb_z);
void checkBombChainReaction(int bomb_x, int bomb_z);
void moveEnemies();

// Texturas
GLuint tex_grama;
GLuint tex_azulejo;
GLuint tex_tijolo;

int gameMap[MAP_SIZE][MAP_SIZE]; // 0: vazio, 1: parede, 2: bloco destruivel
int player_x = 1, player_z = 1;
bool player_alive = true;
bool player_won = false; // Nova variável para controlar vitória

Model playerModel;

struct Enemy {
    int x, z;
    bool alive;
};

vector<Enemy> enemies;
vector<int> fuga_inimigo;
const int NUM_ENEMIES = 3; // Total de inimigos (1 original + 2 novos)
bool timer_ativo = false;

float cam_angle_y = 45.0f;
float cam_angle_x = 30.0f;
float cam_dist = 20.0f; // Aumentado para acomodar o mapa maior

struct Bomba {
    int x, z;
    int timer;
    bool explodiu;
    int frame_explosao;
    bool jogador; // true se for bomba do jogador, false se for de inimigo
};

vector<Bomba> bombas;

GLuint loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        printf("Erro ao carregar imagem: %s\n", filename);
        exit(1);
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, 
                 channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return tex;
}

void initMap() {
    for (int x = 0; x < MAP_SIZE; x++) {
        for (int z = 0; z < MAP_SIZE; z++) {
            if (x == 0 || z == 0 || x == MAP_SIZE - 1 || z == MAP_SIZE - 1)
                gameMap[x][z] = 1; // parede
            else if ((x % 2 == 0 && z % 2 == 0))
                gameMap[x][z] = 1; // parede fixa
            else
                gameMap[x][z] = (rand() % 4 == 0 ? 2 : 0); // bloco aleatorio ou vazio
        }
    }
    
    // Garante uma área segura para o jogador iniciar
    gameMap[1][1] = 0; // posição inicial do jogador
    gameMap[1][2] = 0; // caminho para baixo
    gameMap[2][1] = 0; // caminho para direita
    
    // Garante que o jogador tenha pelo menos um caminho para explorar
    // Cria um caminho aleatório a partir da posição inicial
    int path_length = rand() % 5 + 3; // caminho de 3 a 7 blocos
    int current_x = 2;
    int current_z = 1;
    
    for (int i = 0; i < path_length; i++) {
        // Escolhe uma direção aleatória (direita ou para baixo)
        if (rand() % 2 == 0 && current_x < MAP_SIZE - 2) {
            current_x++;
            // Se for uma parede fixa, pula
            if (current_x % 2 == 0 && current_z % 2 == 0) {
                current_x++;
            }
            if (current_x < MAP_SIZE - 1)
                gameMap[current_x][current_z] = 0; // limpa o caminho
        } else if (current_z < MAP_SIZE - 2) {
            current_z++;
            // Se for uma parede fixa, pula
            if (current_x % 2 == 0 && current_z % 2 == 0) {
                current_z++;
            }
            if (current_z < MAP_SIZE - 1)
                gameMap[current_x][current_z] = 0; // limpa o caminho
        }
    }
    
    // Limpa o vetor de inimigos e inicializa com NUM_ENEMIES inimigos
    enemies.clear();
    enemies.resize(NUM_ENEMIES);
    
    // Inicializa cada inimigo em uma posição aleatória válida
    for (int i = 0; i < NUM_ENEMIES; i++) {
        bool valid_position = false;
        while (!valid_position) {
            int x = rand() % (MAP_SIZE - 2) + 1;
            int z = rand() % (MAP_SIZE - 2) + 1;
            
            // Verifica se a posição é válida (vazia e não muito perto do jogador)
            if (gameMap[x][z] == 0 && (abs(x - player_x) + abs(z - player_z) >= 4)) {
                // Verifica se não está na mesma posição que outro inimigo
                bool overlap = false;
                for (int j = 0; j < i; j++) {
                    if (x == enemies[j].x && z == enemies[j].z) {
                        overlap = true;
                        break;
                    }
                }
                
                if (!overlap) {
                    enemies[i].x = x;
                    enemies[i].z = z;
                    enemies[i].alive = true;
                    valid_position = true;
                }
            }
        }
    }
    fuga_inimigo.assign(NUM_ENEMIES, 0);

}

bool loadModel(const char* filename, Model& model) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Extrai o diretório do arquivo OBJ
    std::string obj_path = filename;
    size_t last_slash = obj_path.find_last_of("/\\");
    std::string base_path = (last_slash != std::string::npos) ? obj_path.substr(0, last_slash + 1) : "";

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, base_path.c_str())) {
        // printf("Erro ao carregar modelo: %s\n", err.c_str());
        return false;
    }


    // Armazena os materiais
    model.materials = materials;

    // Processa os dados do modelo
    for (const auto& shape : shapes) {
        
        // Processa cada face (triângulo)
        size_t index_offset = 0;
        for (size_t face = 0; face < shape.mesh.num_face_vertices.size(); face++) {
            int material_id = (face < shape.mesh.material_ids.size()) ? shape.mesh.material_ids[face] : -1;
            
            // Cada face tem 3 vértices (triângulo)
            for (size_t v = 0; v < 3; v++) {
                const auto& index = shape.mesh.indices[index_offset + v];
                
                model.vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
                model.vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
                model.vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);

                if (index.normal_index >= 0) {
                    model.normals.push_back(attrib.normals[3 * index.normal_index + 0]);
                    model.normals.push_back(attrib.normals[3 * index.normal_index + 1]);
                    model.normals.push_back(attrib.normals[3 * index.normal_index + 2]);
                }

                if (index.texcoord_index >= 0) {
                    model.texcoords.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
                    model.texcoords.push_back(attrib.texcoords[2 * index.texcoord_index + 1]);
                }

                // Cada vértice da face usa o mesmo material
                model.material_ids.push_back(material_id);
            }
            
            index_offset += 3; // Próxima face
        }
    }
    
    // printf("Vértices carregados: %zu\n", model.vertices.size() / 3);
    // printf("Materiais carregados: %zu\n", model.materials.size());
    
    return true;
}

void drawModelWithColor(const Model& model, float r, float g, float b) {
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < model.vertices.size(); i += 3) {
        // Aplica a cor personalizada
        glColor3f(r, g, b);
        
        // Aplica normal se disponível
        if (i < model.normals.size()) {
            glNormal3f(model.normals[i], model.normals[i+1], model.normals[i+2]);
        }
        
        glVertex3f(model.vertices[i], model.vertices[i+1], model.vertices[i+2]);
    }
    glEnd();
}

void drawModel(const Model& model) {
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < model.vertices.size(); i += 3) {
        // Aplica a cor do material
        int material_id = model.material_ids[i / 3];
        if (material_id >= 0 && material_id < model.materials.size()) {
            const auto& material = model.materials[material_id];
            glColor3f(material.diffuse[0], material.diffuse[1], material.diffuse[2]);
            // Debug: print dos primeiros materiais usados
            // if (i < 30) { // Primeiros 10 triângulos
            //     printf("Triângulo %zu: Material %d, Cor (%.3f, %.3f, %.3f)\n", 
            //            i/3, material_id, material.diffuse[0], material.diffuse[1], material.diffuse[2]);
            // }
        } else {
            // Cor padrão se não houver material
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        
        // Aplica normal se disponível
        if (i < model.normals.size()) {
            glNormal3f(model.normals[i], model.normals[i+1], model.normals[i+2]);
        }
        
        glVertex3f(model.vertices[i], model.vertices[i+1], model.vertices[i+2]);
    }
    glEnd();
}

void drawCube(float r, float g, float b) {
    glColor3f(r, g, b);
    glutSolidCube(1.0);
}

void drawCubeTextured(GLuint tex) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glColor3f(1,1,1);

    float s = 0.5f;
    float repeat = 2.0f; // Ajuste para mais repetições se quiser

    glBegin(GL_QUADS);
    // Frente
    glTexCoord2f(0,0); glVertex3f(-s,-s, s);
    glTexCoord2f(repeat,0); glVertex3f( s,-s, s);
    glTexCoord2f(repeat,repeat); glVertex3f( s, s, s);
    glTexCoord2f(0,repeat); glVertex3f(-s, s, s);
    // Trás
    glTexCoord2f(0,0); glVertex3f( s,-s,-s);
    glTexCoord2f(repeat,0); glVertex3f(-s,-s,-s);
    glTexCoord2f(repeat,repeat); glVertex3f(-s, s,-s);
    glTexCoord2f(0,repeat); glVertex3f( s, s,-s);
    // Direita
    glTexCoord2f(0,0); glVertex3f( s,-s, s);
    glTexCoord2f(repeat,0); glVertex3f( s,-s,-s);
    glTexCoord2f(repeat,repeat); glVertex3f( s, s,-s);
    glTexCoord2f(0,repeat); glVertex3f( s, s, s);
    // Esquerda
    glTexCoord2f(0,0); glVertex3f(-s,-s,-s);
    glTexCoord2f(repeat,0); glVertex3f(-s,-s, s);
    glTexCoord2f(repeat,repeat); glVertex3f(-s, s, s);
    glTexCoord2f(0,repeat); glVertex3f(-s, s,-s);
    // Topo
    glTexCoord2f(0,0); glVertex3f(-s, s, s);
    glTexCoord2f(repeat,0); glVertex3f( s, s, s);
    glTexCoord2f(repeat,repeat); glVertex3f( s, s,-s);
    glTexCoord2f(0,repeat); glVertex3f(-s, s,-s);
    // Base
    glTexCoord2f(0,0); glVertex3f(-s,-s,-s);
    glTexCoord2f(repeat,0); glVertex3f( s,-s,-s);
    glTexCoord2f(repeat,repeat); glVertex3f( s,-s, s);
    glTexCoord2f(0,repeat); glVertex3f(-s,-s, s);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}


void drawGameOver() {
    glDisable(GL_DEPTH_TEST); // Evita que o texto fique escondido
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600); // Tela 800x600
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(0, 0, 0); // Tela preta
    glBegin(GL_QUADS);
        glVertex2i(0, 0);
        glVertex2i(800, 0);
        glVertex2i(800, 600);
        glVertex2i(0, 600);
    glEnd();

    glColor3f(1.0, 0.0, 0.0); // Texto vermelho
    glRasterPos2i(330, 300);
    const char* msg = "GAME OVER";
    for (int i = 0; msg[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg[i]);
    }
    
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2i(310, 270);
    const char* msg2 = "Press R to restart";
    for (int i = 0; msg2[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg2[i]);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

void drawVictory() {
    glDisable(GL_DEPTH_TEST); // Evita que o texto fique escondido
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600); // Tela 800x600
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(0, 0.3f, 0); // Tela verde escuro
    glBegin(GL_QUADS);
        glVertex2i(0, 0);
        glVertex2i(800, 0);
        glVertex2i(800, 600);
        glVertex2i(0, 600);
    glEnd();

    glColor3f(1.0, 1.0, 0.0); // Texto amarelo
    glRasterPos2i(330, 300);
    const char* msg = "VICTORY!";
    for (int i = 0; msg[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg[i]);
    }
    
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2i(310, 270);
    const char* msg2 = "Press R to restart";
    for (int i = 0; msg2[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg2[i]);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

void drawGroundTextured() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_grama);
    glColor3f(1,1,1); // Para não alterar a cor da textura

    float y = -1.0f;
    float size = (float)MAP_SIZE;
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0, y, 0);
        glTexCoord2f(1, 0); glVertex3f(size, y, 0);
        glTexCoord2f(1, 1); glVertex3f(size, y, size);
        glTexCoord2f(0, 1); glVertex3f(0, y, size);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void drawMap() {
    // Desenha o chão branco
    drawGroundTextured();
    for (int x = 0; x < MAP_SIZE; x++) {
        for (int z = 0; z < MAP_SIZE; z++) {
            glPushMatrix();
            glTranslatef((float)x, -0.5f, (float)z);
            if (gameMap[x][z] == 1)
                drawCubeTextured(tex_azulejo);
            else if (gameMap[x][z] == 2)
                drawCubeTextured(tex_tijolo);
            glPopMatrix();
        }
    }
}

void drawPlayer() {
    if (player_alive) {
        glPushMatrix();
        glTranslatef((float)player_x, 0.0f, (float)player_z);
        glScalef(0.5f, 0.5f, 0.5f); // Ajuste o tamanho conforme necessário
        drawModel(playerModel);
        glPopMatrix();
    }
}

void drawEnemies() {
    for (int i = 0; i < enemies.size(); i++) {
        if (enemies[i].alive) {
            glPushMatrix();
            glTranslatef((float)enemies[i].x, 0.0f, (float)enemies[i].z);
            glScalef(0.5f, 0.5f, 0.5f); // Mesmo tamanho do jogador
            drawModelWithColor(playerModel, 1.0f, 0.0f, 0.0f);
            glPopMatrix();
        }
    }
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
            
            
            //  Centro da explosão
		    glPushMatrix();
		    glTranslatef((float)bombas[i].x, 0.0f, (float)bombas[i].z);
		    glutSolidSphere(0.3, 10, 10);
		    glPopMatrix();
            
            
            
            for (int dx = -1; dx <= 1; dx++) {
                for (int dz = -1; dz <= 1; dz++) {
                    if (abs(dx) + abs(dz) == 1) {
                        // glPushMatrix();
                        // glTranslatef((float)(bombas[i].x + dx), 0.0f, (float)(bombas[i].z + dz));
                        // glutSolidSphere(0.3, 10, 10);
                        // glPopMatrix();
                        
                        int nx = bombas[i].x + dx;
			            int nz = bombas[i].z + dz;
			
			            // Só desenha explosão se não for parede sólida
			            if (gameMap[nx][nz] != 1) {
			                glPushMatrix();
			                glTranslatef((float)nx, 0.0f, (float)nz);
			                glutSolidSphere(0.3, 10, 10);
			                glPopMatrix();
			            }
                    }
                }
            }
            bombas[i].frame_explosao--;
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    updateCamera();

    drawMap();
    drawPlayer();
    drawEnemies();
    drawBombs();
    drawExplosions();

    if (!player_alive) {
        drawGameOver();
    } else if (player_won) {
        drawVictory();
    }

    glutSwapBuffers();
}

void updateCamera() {
    float rad_y = cam_angle_y * 3.141592f / 180.0f;
    float rad_x = cam_angle_x * 3.141592f / 180.0f;

    float eye_x = cam_dist * cos(rad_x) * sin(rad_y);
    float eye_y = cam_dist * sin(rad_x);
    float eye_z = cam_dist * cos(rad_x) * cos(rad_y);

    // Ajustado para olhar para o centro do mapa 13x13
    gluLookAt(eye_x, eye_y, eye_z, 6, 0, 6, 0, 1, 0);
}

// Verifica se há uma bomba na posição (x,z)
bool hasBomb(int x, int z) {
    for (size_t i = 0; i < bombas.size(); i++) {
        // Verifica se há uma bomba ativa (não explodiu e timer > 0) OU se está explodindo (frame_explosao > 0)
        if (((!bombas[i].explodiu && bombas[i].timer > 0) || bombas[i].frame_explosao > 0) && 
            bombas[i].x == x && bombas[i].z == z) {
            return true;
        }
    }
    return false;
}

// Verifica se o jogador está na explosão
bool playerInExplosion(int bomb_x, int bomb_z) {
    // Verifica se o jogador está no centro da explosão
    if (player_x == bomb_x && player_z == bomb_z)
        return true;
    
    // Verifica se o jogador está nos braços da explosão
    for (int dx = -1; dx <= 1; dx++) {
        for (int dz = -1; dz <= 1; dz++) {
            if (abs(dx) + abs(dz) == 1) {
                int nx = bomb_x + dx, nz = bomb_z + dz;
                // Só verifica se não há parede bloqueando
                if (gameMap[nx][nz] != 1 && player_x == nx && player_z == nz)
                    return true;
            }
        }
    }
    return false;
}

// Verifica se algum inimigo está na explosão e retorna o índice do inimigo atingido
// Retorna -1 se nenhum inimigo foi atingido
int enemyInExplosion(int bomb_x, int bomb_z) {
    for (int i = 0; i < enemies.size(); i++) {
        if (!enemies[i].alive) continue;
        
        // Verifica se o inimigo está no centro da explosão
        if (enemies[i].x == bomb_x && enemies[i].z == bomb_z)
            return i;
        
        // Verifica se o inimigo está nos braços da explosão
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                if (abs(dx) + abs(dz) == 1) {
                    int nx = bomb_x + dx, nz = bomb_z + dz;
                    // Só verifica se não há parede bloqueando
                    if (gameMap[nx][nz] != 1 && enemies[i].x == nx && enemies[i].z == nz)
                        return i;
                }
            }
        }
    }
    return -1; // Nenhum inimigo atingido
}

// Verifica se há outra bomba na explosão
void checkBombChainReaction(int bomb_x, int bomb_z) {
    for (size_t j = 0; j < bombas.size(); j++) {
        if (bombas[j].timer > 0 && !bombas[j].explodiu) {
            // Verifica se a bomba está no centro da explosão
            if (bombas[j].x == bomb_x && bombas[j].z == bomb_z)
                bombas[j].timer = 0;
            
            // Verifica se a bomba está nos braços da explosão
            for (int dx = -1; dx <= 1; dx++) {
                for (int dz = -1; dz <= 1; dz++) {
                    if (abs(dx) + abs(dz) == 1) {
                        int nx = bomb_x + dx, nz = bomb_z + dz;
                        // Só verifica se não há parede bloqueando
                        if (gameMap[nx][nz] != 1 && 
                            bombas[j].x == nx && bombas[j].z == nz)
                            bombas[j].timer = 0;
                    }
                }
            }
        }
    }
}

// Movimento aleatório dos inimigos
void moveEnemies() {
    for (int i = 0; i < enemies.size(); i++) {
        if (!enemies[i].alive) continue;
        
        int dx = 0, dz = 0;
		if (fuga_inimigo[i] > 0) {
		    // Tenta fugir da posição da bomba
		    int max_dist = -1;
		    int best_dx = 0, best_dz = 0;
		
		    for (int dir = 0; dir < 4; dir++) {
		        int test_dx = (dir == 0) ? -1 : (dir == 1) ? 1 : 0;
		        int test_dz = (dir == 2) ? -1 : (dir == 3) ? 1 : 0;
		
		        int nx = enemies[i].x + test_dx;
		        int nz = enemies[i].z + test_dz;
		
		        if (gameMap[nx][nz] == 0 && !hasBomb(nx, nz)) {
		            // Calcula distância até a bomba mais próxima
		            int min_dist = 1000;
		            for (size_t b = 0; b < bombas.size(); b++) {
		                int dist = abs(nx - bombas[b].x) + abs(nz - bombas[b].z);
		                if (dist < min_dist) min_dist = dist;
		            }
		            if (min_dist > max_dist) {
		                max_dist = min_dist;
		                best_dx = test_dx;
		                best_dz = test_dz;
		            }
		        }
		    }
		
		    dx = best_dx;
		    dz = best_dz;
		    fuga_inimigo[i]--;
		} else {
		    // Movimento aleatório normal
		    int dir = rand() % 4;
		    dx = (dir == 0) ? -1 : (dir == 1) ? 1 : 0;
		    dz = (dir == 2) ? -1 : (dir == 3) ? 1 : 0;
		}
        
        
        int nx = enemies[i].x + dx;
        int nz = enemies[i].z + dz;
        
        // Verifica se o movimento é válido (não colide com paredes, blocos ou bombas)
        if (gameMap[nx][nz] == 0 && !hasBomb(nx, nz)) {
            // Verifica se não colide com outro inimigo
            bool collision = false;
            for (int j = 0; j < enemies.size(); j++) {
                if (j != i && enemies[j].alive && nx == enemies[j].x && nz == enemies[j].z) {
                    collision = true;
                    break;
                }
            }
            
            if (!collision) {
                enemies[i].x = nx;
                enemies[i].z = nz;
            }
        }
        
        
        
        bool perto_de_bloco = false;
		bool perto_do_jogador = false;
		
		// Verifica vizinhança
		for (int dx = -1; dx <= 1; dx++) {
		    for (int dz = -1; dz <= 1; dz++) {
		        if (abs(dx) + abs(dz) == 1) {
		            int nx = enemies[i].x + dx;
		            int nz = enemies[i].z + dz;
		
		            if (gameMap[nx][nz] == 2)
		                perto_de_bloco = true;
		
		            if (player_alive && player_x == nx && player_z == nz)
		                perto_do_jogador = true;
		        }
		    }
		}
		
		// Define a chance: maior se perto do jogador, média se perto de bloco, pequena caso contrário
		int chance = 30; // padrão: chance baixa
		if (perto_de_bloco) chance = 10; // médio (10%)
		if (perto_do_jogador) chance = 3; // alto (33%)
		
		if (rand() % chance == 0 && !hasBomb(enemies[i].x, enemies[i].z)) {
		    Bomba nova;
		    nova.x = enemies[i].x;
		    nova.z = enemies[i].z;
		    nova.timer = 4;
		    nova.explodiu = false;
		    nova.frame_explosao = 0;
		    nova.jogador = false;
		    bombas.push_back(nova);
		    fuga_inimigo[i] = 4; // inimigo entra em fuga imediatamente
		}
        
        
        
 
    }
}

void timer(int v) {
    vector<Bomba> novas;
    bool player_hit = false;
    if (!player_alive) return;
    
    // Movimento dos inimigos a cada 2 ciclos (para não ficar muito rápido)
    static int enemy_move_counter = 0;
    if (++enemy_move_counter >= 2) {
        moveEnemies();
        enemy_move_counter = 0;
    }
    
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
                        if (gameMap[nx][nz] == 2) gameMap[nx][nz] = 0;
                    }
                }
            }
            
            // Verifica colisão da explosão com o jogador
            if (playerInExplosion(bombas[i].x, bombas[i].z)) {
                player_hit = true;
            }
            
            // Verifica colisão da explosão com os inimigos
            int hit_enemy_index = enemyInExplosion(bombas[i].x, bombas[i].z);
            if (hit_enemy_index >= 0) {
                enemies[hit_enemy_index].alive = false;
            }
            
            // Verifica colisão da explosão com outras bombas (reação em cadeia)
            checkBombChainReaction(bombas[i].x, bombas[i].z);

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
    
    // Jogador morre se for atingido por uma explosão
    if (player_hit) {
        player_alive = false;
    }
    
    // Verifica se o jogo acabou
    if (!player_alive) {
        // Jogador perdeu
        // Aqui você pode adicionar código para reiniciar o jogo ou mostrar uma mensagem
    } else {
        // Verifica se todos os inimigos estão mortos
        bool all_enemies_dead = true;
        for (int i = 0; i < enemies.size(); i++) {
            if (enemies[i].alive) {
                all_enemies_dead = false;
                break;
            }
        }
        
        if (all_enemies_dead) {
            // Jogador venceu
            player_won = true;
        }
    }

    if (player_alive || !timer_ativo) {
	    glutTimerFunc(400, timer, 0);
	}
	
	glutPostRedisplay();
}


void keyboard(unsigned char key, int, int) {
    if (key == ESC) exit(0);
    if (key == ' ') {
        // Debug: mostra informações sobre bombas existentes
        printf("Tentando plantar bomba na posição (%d, %d)\n", player_x, player_z);
        printf("Total de bombas: %zu\n", bombas.size());
        
        for (size_t i = 0; i < bombas.size(); i++) {
            printf("Bomba %zu: pos(%d,%d) timer=%d explodiu=%s frame_explosao=%d\n", 
                   i, bombas[i].x, bombas[i].z, bombas[i].timer, 
                   bombas[i].explodiu ? "true" : "false", bombas[i].frame_explosao);
        }
        
        // Verifica se existe qualquer bomba ativa do jogador
        bool tem_bomba_ativa = false;
        for (size_t i = 0; i < bombas.size(); i++) {
            if (bombas[i].jogador && ((!bombas[i].explodiu && bombas[i].timer > 0) || bombas[i].frame_explosao > 0)) {
                tem_bomba_ativa = true;
                break;
            }
        }
        
        printf("Existe bomba ativa do jogador: %s\n", tem_bomba_ativa ? "true" : "false");
        
        // Só planta nova bomba se não houver nenhuma bomba ativa do jogador
        if (!tem_bomba_ativa) {
            printf("Plantando nova bomba!\n");
            Bomba nova;
            nova.x = player_x;
            nova.z = player_z;
            nova.timer = 4;
            nova.explodiu = false;
            nova.frame_explosao = 0;
            nova.jogador = true;
            bombas.push_back(nova);
        } else {
            printf("Já existe bomba ativa, não pode plantar nova!\n");
        }
    } else if (key == 'q') cam_angle_y -= 5;
    else if (key == 'e') cam_angle_y += 5;
    else if (key == 'z') cam_angle_x -= 5;
    else if (key == 'x') cam_angle_x += 5;
    else if (key == '-') cam_dist += 1.0f;
    else if (key == '+') cam_dist -= 1.0f;
    else if (key == 'r' || key == 'R') {
        player_alive = true;
        player_won = false; // Reset do estado de vitória
        player_x = 1;
        player_z = 1;
        bombas.clear();
        initMap(); // reinicia o jogo
        timer_ativo = true;
        glutTimerFunc(100, timer, 0);
    }

    glutPostRedisplay();
}


void special(int key, int, int) {
    int dx = 0, dz = 0;
    if (key == GLUT_KEY_UP) dz = -1;
    else if (key == GLUT_KEY_DOWN) dz = 1;
    else if (key == GLUT_KEY_LEFT) dx = -1;
    else if (key == GLUT_KEY_RIGHT) dx = 1;

	int nx = player_x + dx, nz = player_z + dz;

	// Verifica se há inimigo no destino
	bool tem_inimigo = false;
	for (int i = 0; i < enemies.size(); i++) {
	    if (enemies[i].alive && enemies[i].x == nx && enemies[i].z == nz) {
	        tem_inimigo = true;
	        break;
	    }
	}
	
	// Só anda se o destino for livre, sem bomba nem inimigo
	if (gameMap[nx][nz] == 0 && !hasBomb(nx, nz) && !tem_inimigo) {
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
	glutIgnoreKeyRepeat(1); // Ignora repetição automática de tecla
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    
    // Configuração de iluminação básica
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    
    // Posição da luz
    GLfloat light_position[] = { 10.0f, 10.0f, 10.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    // Cor da luz ambiente
    GLfloat light_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    
    // Cor da luz difusa
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    
    // Carrega as texturas
    tex_grama = loadTexture("assets/grass.jpg");
    tex_azulejo = loadTexture("assets/tiles.jpg"); 
    tex_tijolo = loadTexture("assets/brick.jpg");
    
    // Carrega o modelo do jogador
    if (!loadModel("assets/bomberman.obj", playerModel)) {
        printf("Falha ao carregar modelo do jogador\n");
        exit(1);
    }
    
    glClearColor(0.8f, 0.9f, 1.0f, 1.0f);

    initMap();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    timer_ativo = true;
    glutTimerFunc(100, timer, 0);

    glutMainLoop();
    return 0;
}


