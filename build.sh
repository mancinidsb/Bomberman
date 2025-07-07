#!/bin/bash

# Script de build universal para Bomberman 3D
# Compatível com Linux, macOS e Windows (via WSL/Git Bash)

set -e  # Para em caso de erro

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Função para imprimir mensagens coloridas
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Detecta o sistema operacional
detect_os() {
    case "$(uname -s)" in
        Linux*)     echo "linux";;
        Darwin*)    echo "macos";;
        CYGWIN*|MINGW*|MSYS*) echo "windows";;
        *)          echo "unknown";;
    esac
}

# Mostra informações do sistema
show_system_info() {
    local os=$(detect_os)
    print_status "Informações do Sistema:"
    echo "  Sistema Operacional: $os"
    echo "  Arquitetura: $(uname -m)"
    echo "  Kernel: $(uname -r)"
    
    if command -v g++ &> /dev/null; then
        echo "  Compilador C++: $(g++ --version | head -n1)"
    else
        echo "  Compilador C++: Não encontrado"
    fi
    
    if command -v make &> /dev/null; then
        echo "  Make: $(make --version | head -n1)"
    else
        echo "  Make: Não encontrado"
    fi
    
    if command -v cmake &> /dev/null; then
        echo "  CMake: $(cmake --version | head -n1)"
    else
        echo "  CMake: Não encontrado"
    fi
    
    # Verifica bibliotecas OpenGL
    case $os in
        "linux")
            if ldconfig -p | grep -q libGL; then
                echo "  OpenGL: Disponível"
            else
                echo "  OpenGL: Não encontrado"
            fi
            ;;
        "macos")
            if [ -d "/System/Library/Frameworks/OpenGL.framework" ]; then
                echo "  OpenGL: Disponível"
            else
                echo "  OpenGL: Não encontrado"
            fi
            ;;
        "windows")
            echo "  OpenGL: Verificar manualmente"
            ;;
    esac
}

# Verifica dependências
check_dependencies() {
    local os=$(detect_os)
    
    print_status "Verificando dependências para $os..."
    
    case $os in
        "linux")
            # Verifica se as bibliotecas OpenGL estão instaladas
            if ! ldconfig -p | grep -q libGL; then
                print_warning "OpenGL não encontrado. Tentando instalar dependências..."
                if command -v apt-get &> /dev/null; then
                    sudo apt-get update
                    sudo apt-get install -y freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev
                elif command -v yum &> /dev/null; then
                    sudo yum install -y freeglut-devel mesa-libGL-devel mesa-libGLU-devel
                elif command -v pacman &> /dev/null; then
                    sudo pacman -S --needed freeglut mesa
                else
                    print_error "Gerenciador de pacotes não suportado. Instale manualmente:"
                    print_error "Ubuntu/Debian: sudo apt-get install freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev"
                    print_error "Fedora/RHEL: sudo yum install freeglut-devel mesa-libGL-devel mesa-libGLU-devel"
                    print_error "Arch: sudo pacman -S freeglut mesa"
                    exit 1
                fi
            fi
            ;;
        "macos")
            # Verifica se Xcode Command Line Tools está instalado
            if ! xcode-select -p &> /dev/null; then
                print_warning "Xcode Command Line Tools não encontrado."
                print_warning "Execute: xcode-select --install"
                exit 1
            fi
            ;;
        "windows")
            # Verifica se MinGW está instalado
            if ! command -v g++ &> /dev/null; then
                print_error "MinGW não encontrado. Instale MinGW-w64:"
                print_error "https://www.mingw-w64.org/downloads/"
                exit 1
            fi
            ;;
    esac
    
    print_success "Dependências verificadas!"
}

# Função principal de build
build_project() {
    local os=$(detect_os)
    local build_method=$1
    
    print_status "Iniciando build para $os..."
    
    case $build_method in
        "make")
            if command -v make &> /dev/null; then
                print_status "Usando Makefile..."
                make clean
                make all
                print_success "Build concluído com Makefile!"
            else
                print_error "Make não encontrado. Tente usar CMake."
                exit 1
            fi
            ;;
        "cmake")
            if command -v cmake &> /dev/null; then
                print_status "Usando CMake..."
                mkdir -p build
                cd build
                cmake ..
                make
                cd ..
                print_success "Build concluído com CMake!"
            else
                print_error "CMake não encontrado. Tente usar Make."
                exit 1
            fi
            ;;
        *)
            # Tenta CMake primeiro, depois Make
            if command -v cmake &> /dev/null; then
                print_status "Tentando CMake..."
                mkdir -p build
                cd build
                if cmake .. && make; then
                    cd ..
                    print_success "Build concluído com CMake!"
                else
                    cd ..
                    print_warning "CMake falhou, tentando Make..."
                    if command -v make &> /dev/null; then
                        make clean
                        make all
                        print_success "Build concluído com Makefile!"
                    else
                        print_error "Nenhum sistema de build disponível!"
                        exit 1
                    fi
                fi
            elif command -v make &> /dev/null; then
                print_status "Usando Makefile..."
                make clean
                make all
                print_success "Build concluído com Makefile!"
            else
                print_error "Nenhum sistema de build disponível!"
                exit 1
            fi
            ;;
    esac
}

# Função para executar o jogo
run_game() {
    local os=$(detect_os)
    
    print_status "Executando o jogo..."
    
    case $os in
        "linux"|"macos")
            if [ -f "./bomberman" ]; then
                ./bomberman
            elif [ -f "./build/Bomberman3D" ]; then
                ./build/Bomberman3D
            else
                print_error "Executável não encontrado!"
                exit 1
            fi
            ;;
        "windows")
            if [ -f "./bomberman.exe" ]; then
                ./bomberman.exe
            elif [ -f "./build/Bomberman3D.exe" ]; then
                ./build/Bomberman3D.exe
            else
                print_error "Executável não encontrado!"
                exit 1
            fi
            ;;
    esac
}

# Função para mostrar ajuda
show_help() {
    echo "Script de build universal para Bomberman 3D"
    echo ""
    echo "Uso: $0 [OPÇÃO]"
    echo ""
    echo "Opções:"
    echo "  build [make|cmake]  - Compila o projeto (padrão: auto)"
    echo "  run                 - Executa o jogo"
    echo "  clean               - Limpa arquivos de build"
    echo "  deps                - Verifica/instala dependências"
    echo "  info                - Mostra informações do sistema"
    echo "  help                - Mostra esta ajuda"
    echo ""
    echo "Exemplos:"
    echo "  $0 build make       - Compila usando Makefile"
    echo "  $0 build cmake      - Compila usando CMake"
    echo "  $0 build            - Compila automaticamente"
    echo "  $0 run              - Executa o jogo"
    echo "  $0 info             - Mostra informações do sistema"
}

# Função para limpar
clean_build() {
    print_status "Limpando arquivos de build..."
    rm -rf build/
    make clean 2>/dev/null || true
    print_success "Limpeza concluída!"
}

# Main
main() {
    case "${1:-build}" in
        "build")
            check_dependencies
            build_project "${2:-auto}"
            ;;
        "run")
            run_game
            ;;
        "clean")
            clean_build
            ;;
        "deps")
            check_dependencies
            ;;
        "info")
            show_system_info
            ;;
        "help"|"-h"|"--help")
            show_help
            ;;
        *)
            print_error "Opção inválida: $1"
            show_help
            exit 1
            ;;
    esac
}

# Executa o script
main "$@" 