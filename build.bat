@echo off
setlocal enabledelayedexpansion

REM Script de build para Windows
REM Bomberman 3D - Build Script

set "PROJECT_NAME=Bomberman3D"
set "EXECUTABLE=bomberman.exe"

REM Cores para output (Windows 10+)
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "NC=[0m"

REM Função para imprimir mensagens coloridas
:print_status
echo %BLUE%[INFO]%NC% %~1
goto :eof

:print_success
echo %GREEN%[SUCCESS]%NC% %~1
goto :eof

:print_warning
echo %YELLOW%[WARNING]%NC% %~1
goto :eof

:print_error
echo %RED%[ERROR]%NC% %~1
goto :eof

REM Verifica se o MinGW está instalado
:check_mingw
call :print_status "Verificando MinGW..."
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    call :print_error "MinGW não encontrado!"
    call :print_error "Instale MinGW-w64: https://www.mingw-w64.org/downloads/"
    call :print_error "Ou use MSYS2: https://www.msys2.org/"
    exit /b 1
)
call :print_success "MinGW encontrado!"
goto :eof

REM Verifica se o Make está disponível
:check_make
where make >nul 2>&1
if %errorlevel% equ 0 (
    set "MAKE_AVAILABLE=1"
) else (
    where mingw32-make >nul 2>&1
    if %errorlevel% equ 0 (
        set "MAKE_AVAILABLE=1"
        set "MAKE_CMD=mingw32-make"
    ) else (
        set "MAKE_AVAILABLE=0"
    )
)
goto :eof

REM Verifica se o CMake está disponível
:check_cmake
where cmake >nul 2>&1
if %errorlevel% equ 0 (
    set "CMAKE_AVAILABLE=1"
) else (
    set "CMAKE_AVAILABLE=0"
)
goto :eof

REM Build usando Makefile
:build_make
call :print_status "Compilando com Makefile..."
if "%MAKE_CMD%"=="" set "MAKE_CMD=make"
%MAKE_CMD% clean
%MAKE_CMD% all
if %errorlevel% equ 0 (
    call :print_success "Build concluído com Makefile!"
) else (
    call :print_error "Erro na compilação com Makefile!"
    exit /b 1
)
goto :eof

REM Build usando CMake
:build_cmake
call :print_status "Compilando com CMake..."
if not exist "build" mkdir build
cd build
cmake ..
if %errorlevel% equ 0 (
    cmake --build .
    if %errorlevel% equ 0 (
        cd ..
        call :print_success "Build concluído com CMake!"
    ) else (
        cd ..
        call :print_error "Erro na compilação com CMake!"
        exit /b 1
    )
) else (
    cd ..
    call :print_error "Erro na configuração do CMake!"
    exit /b 1
)
goto :eof

REM Executa o jogo
:run_game
call :print_status "Executando o jogo..."
if exist "%EXECUTABLE%" (
    %EXECUTABLE%
) else if exist "build\%PROJECT_NAME%.exe" (
    build\%PROJECT_NAME%.exe
) else (
    call :print_error "Executável não encontrado!"
    call :print_error "Execute 'build.bat build' primeiro."
    exit /b 1
)
goto :eof

REM Limpa arquivos de build
:clean_build
call :print_status "Limpando arquivos de build..."
if exist "build" rmdir /s /q build
if exist "%EXECUTABLE%" del "%EXECUTABLE%"
if exist "*.o" del *.o
call :print_success "Limpeza concluída!"
goto :eof

REM Mostra ajuda
:show_help
echo Script de build para Windows - Bomberman 3D
echo.
echo Uso: build.bat [OPÇÃO]
echo.
echo Opções:
echo   build [make^|cmake]  - Compila o projeto ^(padrão: auto^)
echo   run                 - Executa o jogo
echo   clean               - Limpa arquivos de build
echo   deps                - Verifica dependências
echo   help                - Mostra esta ajuda
echo.
echo Exemplos:
echo   build.bat build make       - Compila usando Makefile
echo   build.bat build cmake      - Compila usando CMake
echo   build.bat build            - Compila automaticamente
echo   build.bat run              - Executa o jogo
echo.
echo Requisitos:
echo   - MinGW-w64 ou MSYS2
echo   - CMake ^(opcional^)
echo   - Make ^(opcional^)
goto :eof

REM Função principal
:main
if "%1"=="" (
    set "ACTION=build"
    set "METHOD=auto"
) else if "%1"=="build" (
    set "ACTION=build"
    if "%2"=="" (
        set "METHOD=auto"
    ) else (
        set "METHOD=%2"
    )
) else (
    set "ACTION=%1"
)

REM Executa a ação
if "%ACTION%"=="build" (
    call :check_mingw
    call :check_make
    call :check_cmake
    
    if "%METHOD%"=="make" (
        if "%MAKE_AVAILABLE%"=="1" (
            call :build_make
        ) else (
            call :print_error "Make não disponível!"
            exit /b 1
        )
    ) else if "%METHOD%"=="cmake" (
        if "%CMAKE_AVAILABLE%"=="1" (
            call :build_cmake
        ) else (
            call :print_error "CMake não disponível!"
            exit /b 1
        )
    ) else (
        REM Tenta CMake primeiro, depois Make
        if "%CMAKE_AVAILABLE%"=="1" (
            call :print_status "Tentando CMake..."
            call :build_cmake
        ) else if "%MAKE_AVAILABLE%"=="1" (
            call :print_status "Tentando Make..."
            call :build_make
        ) else (
            call :print_error "Nenhum sistema de build disponível!"
            call :print_error "Instale CMake ou Make."
            exit /b 1
        )
    )
) else if "%ACTION%"=="run" (
    call :run_game
) else if "%ACTION%"=="clean" (
    call :clean_build
) else if "%ACTION%"=="deps" (
    call :check_mingw
    call :check_make
    call :check_cmake
) else if "%ACTION%"=="help" (
    call :show_help
) else (
    call :print_error "Opção inválida: %ACTION%"
    call :show_help
    exit /b 1
)

goto :eof

REM Executa o script
call :main %* 