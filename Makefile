# Nome do executável
TARGET = bomberman
SRC = main.cpp

# Compilador
CXX = g++

# Flags de compilação
CXXFLAGS = -Wall -O2 -std=c++11

# Detecção do sistema operacional
ifeq ($(OS),Windows_NT)
    # Para Windows
    LIBS = -lopengl32 -lglu32 -lfreeglut
    EXEC = $(TARGET).exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        # Para macOS
        LIBS = -framework OpenGL -framework GLUT
        EXEC = $(TARGET)
    else
        # Para Linux
        LIBS = -lGL -lGLU -lglut
        EXEC = $(TARGET)
    endif
endif

# Regra padrão
all: $(EXEC)

$(EXEC): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXEC) $(LIBS)

# Limpeza
clean:
	rm -f $(TARGET) $(TARGET).exe