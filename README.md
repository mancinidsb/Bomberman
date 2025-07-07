# Bomberman 3D - Jogo Multiplataforma

Um jogo Bomberman 3D com visão isométrica desenvolvido em C++ usando OpenGL e GLUT.

## 🎮 Características

- **Visão 3D isométrica** com câmera rotativa
- **Texturas** para grama, azulejos e tijolos
- **Modelo 3D** do personagem (formato OBJ)
- **Inimigos inteligentes** com IA básica
- **Sistema de bombas** com explosões em cruz
- **Física de jogo** com colisões
- **Interface de usuário** com controles intuitivos

## 🖥️ Compatibilidade

- ✅ **Linux** (Ubuntu, Debian, Fedora, Arch, etc.)
- ✅ **macOS** (10.14+)
- ✅ **Windows** (7, 8, 10, 11)

## 📋 Pré-requisitos

### Linux
```bash
# Ubuntu/Debian
sudo apt-get install freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev g++ make

# Fedora/RHEL
sudo yum install freeglut-devel mesa-libGL-devel mesa-libGLU-devel gcc-c++ make

# Arch Linux
sudo pacman -S freeglut mesa gcc make
```

### macOS
```bash
# Instale Xcode Command Line Tools
xcode-select --install

# Ou via Homebrew
brew install freeglut
```

### Windows
1. **MinGW-w64**: [Download](https://www.mingw-w64.org/downloads/)
2. **MSYS2** (recomendado): [Download](https://www.msys2.org/)
3. **CMake** (opcional): [Download](https://cmake.org/download/)

## 🚀 Instalação e Execução

### Método 1: Script Automático (Recomendado)

#### Linux/macOS
```bash
# Clone o repositório
git clone <url-do-repositorio>
cd Bomberman

# Execute o script de build
./build.sh build

# Execute o jogo
./build.sh run
```

#### Windows
```cmd
# Clone o repositório
git clone <url-do-repositorio>
cd Bomberman

# Execute o script de build
build.bat build

# Execute o jogo
build.bat run
```

### Método 2: Makefile

```bash
# Compilar
make

# Executar
./bomberman

# Limpar
make clean
```

### Método 3: CMake

```bash
# Criar diretório de build
mkdir build
cd build

# Configurar e compilar
cmake ..
make

# Executar
./Bomberman3D
```

## 🎯 Controles

| Tecla | Ação |
|-------|------|
| **Setas** | Mover o jogador |
| **Espaço** | Plantar bomba |
| **Q/E** | Rotacionar câmera horizontalmente |
| **Z/X** | Rotacionar câmera verticalmente |
| **+/-** | Zoom in/out |
| **R** | Reiniciar jogo |
| **ESC** | Sair |

## 🎮 Como Jogar

1. **Objetivo**: Elimine todos os inimigos vermelhos
2. **Movimento**: Use as setas para navegar pelo mapa
3. **Bombas**: Pressione espaço para plantar bombas
4. **Estratégia**: Use as bombas para destruir blocos e eliminar inimigos
5. **Cuidado**: Evite as explosões das bombas!

## 📁 Estrutura do Projeto

```
Bomberman/
├── main.cpp              # Código principal do jogo
├── Makefile              # Sistema de build para Make
├── CMakeLists.txt        # Sistema de build para CMake
├── build.sh              # Script de build para Linux/macOS
├── build.bat             # Script de build para Windows
├── stb_image.h           # Biblioteca para carregar imagens
├── tiny_obj_loader.h     # Biblioteca para carregar modelos OBJ
├── assets/               # Recursos do jogo
│   ├── bomberman.obj     # Modelo 3D do personagem
│   ├── bomberman.mtl     # Materiais do modelo
│   ├── grass.jpg         # Textura do chão
│   ├── tiles.jpg         # Textura das paredes
│   └── brick.jpg         # Textura dos blocos
└── README.md             # Este arquivo
```

## 🔧 Solução de Problemas

### Linux

**Erro: "GL/glut.h: No such file or directory"**
```bash
sudo apt-get install freeglut3-dev
```

**Erro: "libGL.so not found"**
```bash
sudo apt-get install libgl1-mesa-dev
```

### macOS

**Erro: "framework not found"**
```bash
xcode-select --install
```

**Erro: "GLUT not found"**
```bash
brew install freeglut
```

### Windows

**Erro: "g++ not found"**
- Instale MinGW-w64 ou MSYS2
- Adicione ao PATH do sistema

**Erro: "freeglut.dll not found"**
- Copie `freeglut.dll` para o diretório do executável
- Ou instale via MSYS2: `pacman -S mingw-w64-x86_64-freeglut`

## 🛠️ Desenvolvimento

### Compilando com Debug
```bash
# Com Makefile
make CXXFLAGS="-Wall -g -O0"

# Com CMake
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Adicionando Novas Texturas
1. Adicione o arquivo de imagem em `assets/`
2. Carregue a textura em `main.cpp`:
```cpp
GLuint tex_nova = loadTexture("assets/nova_textura.jpg");
```

### Modificando o Mapa
Edite a função `initMap()` em `main.cpp` para alterar:
- Tamanho do mapa (`MAP_SIZE`)
- Layout das paredes
- Distribuição dos blocos

## 📝 Licença

Este projeto é de código aberto. Sinta-se livre para modificar e distribuir.

## 🤝 Contribuições

Contribuições são bem-vindas! Por favor:
1. Faça um fork do projeto
2. Crie uma branch para sua feature
3. Commit suas mudanças
4. Push para a branch
5. Abra um Pull Request

## 📞 Suporte

Se encontrar problemas:
1. Verifique se todas as dependências estão instaladas
2. Consulte a seção "Solução de Problemas"
3. Abra uma issue no repositório

---

**Divirta-se jogando Bomberman 3D!** 🎮💥 