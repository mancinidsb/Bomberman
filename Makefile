# Makefile multiplataforma para Bomberman 3D
# Compatível com Linux, macOS e Windows

# Nome do executável
TARGET = bomberman
SRC = main.cpp

# Compilador
CXX = g++

# Flags de compilação padrão
CXXFLAGS = -Wall -O2 -std=c++11

# Detecção do sistema operacional
ifeq ($(OS),Windows_NT)
    # Windows
    LIBS = -lopengl32 -lglu32 -lfreeglut
    EXEC = $(TARGET).exe
    # Para Windows, pode ser necessário usar mingw32-make
    MAKE = mingw32-make
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        # macOS
        LIBS = -framework OpenGL -framework GLUT
        EXEC = $(TARGET)
        # macOS pode precisar de flags específicas
        CXXFLAGS += -I/opt/X11/include
        LIBS += -L/opt/X11/lib
    else
        # Linux
        LIBS = -lGL -lGLU -lglut
        EXEC = $(TARGET)
        # Verifica se está usando Ubuntu/Debian e instala dependências se necessário
        ifeq ($(shell which apt-get 2>/dev/null),/usr/bin/apt-get)
            DEPS_CMD = sudo apt-get install -y freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev
        else ifeq ($(shell which yum 2>/dev/null),/usr/bin/yum)
            DEPS_CMD = sudo yum install -y freeglut-devel mesa-libGL-devel mesa-libGLU-devel
        else ifeq ($(shell which pacman 2>/dev/null),/usr/bin/pacman)
            DEPS_CMD = sudo pacman -S --needed freeglut mesa
        endif
    endif
endif

# Regra padrão
all: check-deps $(EXEC)

# Verifica e instala dependências no Linux
check-deps:
ifdef DEPS_CMD
	@echo "Verificando dependências..."
	@if ! pkg-config --exists gl 2>/dev/null && ! ldconfig -p | grep -q libGL; then \
		echo "Instalando dependências OpenGL..."; \
		$(DEPS_CMD); \
	fi
endif

$(EXEC): $(SRC)
	@echo "Compilando para $(UNAME_S)..."
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXEC) $(LIBS)
	@echo "Compilação concluída: $(EXEC)"

# Regra para executar o jogo
run: $(EXEC)
	./$(EXEC)

# Limpeza
clean:
	rm -f $(TARGET) $(TARGET).exe
	@echo "Arquivos de build removidos"

# Instala dependências (Linux)
install-deps:
ifdef DEPS_CMD
	@echo "Instalando dependências..."
	$(DEPS_CMD)
else
	@echo "Não é necessário instalar dependências automaticamente nesta plataforma"
	@echo "Para Windows: Instale MinGW e freeglut"
	@echo "Para macOS: Instale Xcode Command Line Tools"
endif

# Mostra informações do sistema
info:
	@echo "Sistema operacional: $(UNAME_S)"
	@echo "Compilador: $(CXX)"
	@echo "Flags: $(CXXFLAGS)"
	@echo "Bibliotecas: $(LIBS)"
	@echo "Executável: $(EXEC)"

.PHONY: all clean run install-deps check-deps info