# Setup do ambiente: Windows, Linux ou WSL2

O projeto compila com **CMake + g++** em três ambientes, todos com o mesmo `CMakeLists.txt` e os presets de `CMakePresets.json`:

| Ambiente | Compilador | Preset | Janela |
| --- | --- | --- | --- |
| **Windows** (nativo) | MinGW-w64 do MSYS2 (UCRT64) | `windows` | Win32 |
| **Linux** (Ubuntu/Debian) | g++ do sistema | `linux` | X11 |
| **WSL2** (Ubuntu) | g++ do sistema | `linux` | WSLg |

O projeto usa **OpenGL 3.3 core profile**. As bibliotecas **já vêm no repositório** em `lib/` (GLFW 3.5.1, GLAD, GLM 1.0.3 e
stb_image, veja [lib/README.md](../lib/README.md)) e o CMake compila tudo junto com o projeto.
**Não instale nenhuma delas no sistema**, nem baixe DLL/`.lib`. Do sistema vêm só o compilador, o CMake, o driver de vídeo
e, no Linux, os headers do X11.

> [!WARNING]
> **Não use** Visual Studio/MSVC, vcpkg, GLFW/GLEW/GLUT do sistema, VcXsrv/Xming nem `export DISPLAY=<ip>:0`.
> Esses caminhos aparecem em tutoriais e não são o build deste repo. Se uma IA sugerir algum deles, ela está errada para este projeto.

---

## Sumário

1. [Windows (MinGW-w64 via MSYS2)](#1-windows-mingw-w64-via-msys2)
2. [Linux (Ubuntu/Debian)](#2-linux-ubuntudebian)
3. [WSL2](#3-wsl2)
4. [Verificar o ambiente](#4-verificar-o-ambiente)
5. [Compilar e rodar](#5-compilar-e-rodar)
6. [Estrutura do projeto](#6-estrutura-do-projeto)
7. [Problemas comuns](#7-problemas-comuns)

---

## 1. Windows (MinGW-w64 via MSYS2)

1. Instale o **Git for Windows** (<https://git-scm.com/download/win>) e o **MSYS2** (<https://www.msys2.org>), ou pelo PowerShell:

   ```powershell
   winget install Git.Git MSYS2.MSYS2
   ```

2. Abra o terminal **"MSYS2 UCRT64"** (menu Iniciar) e instale o compilador, o make e o CMake:

   ```bash
   pacman -Syu
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-cmake
   ```

   Se o `pacman -Syu` fechar o terminal, abra de novo e rode a segunda linha.

3. Adicione **`C:\msys64\ucrt64\bin`** ao `Path` do Windows (Iniciar → "variáveis de ambiente" → `Path` do usuário → Novo).
   Feche e abra o terminal depois disso.

4. No **PowerShell** (ou cmd, Git Bash, terminal do VS Code), confira e clone:

   ```powershell
   g++ --version          # deve mostrar "Built by MSYS2 project"
   cmake --version        # 3.21 ou mais novo
   git clone https://github.com/JoseRiquelmeTds/ambiente-sala-de-aula-opengl.git
   cd ambiente-sala-de-aula-opengl
   cmake --preset windows
   cmake --build --preset windows --target run
   ```

O preset `windows` usa o gerador **"MinGW Makefiles"** e o `g++` do `Path`. O `.exe` é linkado com `-static`: só depende de DLLs
do próprio Windows, então abre com dois cliques e roda em outro PC sem MSYS2.

## 2. Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y build-essential cmake libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev mesa-utils
git clone https://github.com/JoseRiquelmeTds/ambiente-sala-de-aula-opengl.git
cd ambiente-sala-de-aula-opengl
cmake --preset linux
cmake --build --preset linux --target run
```

| Pacote | Para quê |
| --- | --- |
| `build-essential` | `g++`, `gcc`, `make` |
| `cmake` | o build (3.21 ou mais novo; o Ubuntu 22.04 já tem) |
| `libx11-dev`, `libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`, `libxi-dev` | headers do X11 que o GLFW usa para abrir a janela e ler teclado/mouse |
| `mesa-utils` | `glxinfo` (usado pelo `doctor.cmake`) e o driver OpenGL do Mesa |

Não precisa de `libgl1-mesa-dev` nem `libglu1-mesa-dev`: o GLAD carrega as funções do OpenGL direto do driver.

Outras distros: instale os equivalentes (Fedora: `gcc-c++ make cmake libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel glx-utils`).

## 3. WSL2

Igual ao Linux (seção 2), rodando dentro do Ubuntu do WSL2. A janela abre pelo **WSLg**, sem servidor X extra.

Requisitos: Windows 11, ou Windows 10 21H2+ (build 19044+), com WSL **versão 2**. No **PowerShell como administrador**:

```powershell
wsl --install -d Ubuntu     # primeira instalação (reinicie quando pedir)
wsl --update                # se já tinha WSL
wsl --shutdown
wsl -l -v                   # a coluna VERSION precisa ser 2
```

Dicas:

- Clone na **home do Linux** (`cd ~`). Em `/mnt/c/...` funciona, mas o build fica bem mais lento.
- Pode usar **o mesmo clone** no Windows e no WSL: cada preset compila numa pasta própria (`build/windows/`, `build/linux/`).
- Para editar no VS Code do Windows, instale a extensão **WSL** (Microsoft) e rode `code .` dentro da pasta.

## 4. Verificar o ambiente

Na raiz do repositório, em qualquer sistema:

```bash
cmake -P cmake/doctor.cmake
```

Tudo precisa aparecer como `[ok]`. Exemplo no WSL:

```
== Sistema
  plataforma: Linux (WSL)
  [ok] repo no filesystem do Linux
== Ferramentas
  [ok] cmake 3.28.3
  [ok] g++ 13.3.0 (x86_64-linux-gnu)
  [ok] make encontrado
== Bibliotecas em lib/
  [ok] lib/glfw/src/init.c
  [ok] lib/glad/src/gl.c
  [ok] lib/glm/glm/glm.hpp
  [ok] lib/stb/stb_image.h
== Headers do X11 (usados pelo GLFW)
  [ok] X11/Xlib.h
  [ok] X11/extensions/Xrandr.h
  [ok] X11/extensions/Xinerama.h
  [ok] X11/Xcursor/Xcursor.h
  [ok] X11/extensions/XInput2.h
== Display e OpenGL
  [ok] DISPLAY=:0
  OpenGL renderer string: D3D12 (NVIDIA GeForce ...)
  [ok] OpenGL core 4.6 (o projeto precisa de 3.3)

Tudo certo. ...
```

No WSL, `D3D12 (...)` no renderer = usando a GPU. `llvmpipe` = renderização por software (funciona, mas veja a [seção 7](#7-problemas-comuns)).
No Windows o script confere só as ferramentas e o `lib/`. O OpenGL é testado ao abrir a janela.

## 5. Compilar e rodar

No Windows, troque `linux` por `windows`:

```bash
cmake --preset linux                          # configura (só na primeira vez)
cmake --build --preset linux --target run     # compila e abre a janela
```

Deve abrir uma janela **"Aula OpenGL"** com fundo azul-escuro, e o terminal mostra a versão do OpenGL e a placa de vídeo usada.
Clique esquerdo na janela captura o cursor. `Esc` libera o cursor capturado; com o cursor livre, `Esc` fecha.
`Q` fecha em ambos os modos. Ao perder foco, a janela libera o cursor; ao voltar, clique novamente para capturar.
A primeira compilação demora mais porque compila o GLFW e o GLAD. As próximas reaproveitam.

| Comando | O que faz |
| --- | --- |
| `cmake --preset linux` | Configura o build debug (`-g`) em `build/linux/` |
| `cmake --build --preset linux` | Compila |
| `cmake --build --preset linux --target run` | Compila e executa (a partir da raiz do repositório) |
| `cmake --preset linux-release` | Configura o build otimizado (`-O2`) em `build/linux-release/` |
| `cmake --build --preset linux-release --target run` | Compila otimizado e executa |
| `cmake -P cmake/doctor.cmake` | Diagnóstico do ambiente |

O executável fica em `build/<preset>/bin/aula-opengl` (`.exe` no Windows). O alvo `run` executa o programa **a partir da raiz
do repositório**, então o código pode abrir arquivos com caminho relativo, como `shaders/basic.vert` ou `assets/textures/uv_teste.png`.
Se rodar o executável de outro lugar (dois cliques, por exemplo), abra o terminal na raiz antes.

Para começar do zero (depois de trocar de compilador, por exemplo), apague a pasta `build/` e configure de novo.

## 6. Estrutura do projeto

```
ambiente-sala-de-aula-opengl/
├── CMakeLists.txt        # build do projeto
├── CMakePresets.json     # presets linux, linux-release, windows, windows-release
├── include/              # headers (.hpp): só declarações (módulos em docs/ARQUITETURA.md)
│   ├── core/             # input e câmera (sem OpenGL)
│   ├── gfx/              # mesh, shader, textura, primitivas, SceneNode, renderer
│   ├── objects/          # um módulo por objeto da sala (a partir da Sprint 1)
│   └── scene/            # classroom: monta a sala, inspeção, teclas das interações
├── src/                  # implementações (.cpp), nas mesmas pastas de include/
│   └── main.cpp          # ponto de entrada (janela GLFW, GLAD, loop principal)
├── shaders/              # shaders GLSL (#version 330 core)
├── assets/
│   └── textures/         # imagens de textura (PNG/JPG, lidas com stb_image)
├── lib/                  # bibliotecas de TERCEIROS, embutidas (veja lib/README.md)
│   ├── CMakeLists.txt    # alvos glfw, glad, glm e stb_image
│   ├── glfw/             # GLFW 3.5.1
│   ├── glad/             # loader do OpenGL 3.3 core (gerado)
│   ├── glm/              # GLM 1.0.3 (só headers)
│   └── stb/              # stb_image
├── cmake/
│   └── doctor.cmake      # diagnóstico do ambiente
├── docs/
│   ├── SETUP.md          # este arquivo
│   ├── ARQUITETURA.md    # camadas, módulos e onde adicionar cada coisa
│   ├── CONVENCOES.md     # modelagem: metros, eixos, pivô, vértice, atalhos
│   └── DEV-CONVENCOES.md # código, branches, commits e PRs
├── .github/
│   └── pull_request_template.md
└── build/                # gerado pelo CMake, ignorado pelo git
    └── <preset>/
        ├── lib/          # libglfw3.a, libglad.a, libstb_image.a, libaula-opengl.a
        └── bin/          # aula-opengl(.exe)
```

Como o build funciona:

1. `lib/CMakeLists.txt` compila o GLFW, o GLAD e o stb_image como bibliotecas estáticas. O GLM é só header.
2. Todo `src/**/*.cpp`, **exceto o `main.cpp`**, vira a biblioteca `libaula-opengl.a`, compilada com `-Wall -Wextra -Wpedantic`
   e com acesso a `include/` e aos headers de todas as bibliotecas de `lib/`.
3. O executável é `main.cpp` + `libaula-opengl.a` + as bibliotecas de `lib/`. No Windows ele é linkado com `-static`.

**Para adicionar um módulo** (ex.: o ventilador): crie `include/objects/fan.hpp` e `src/objects/fan.cpp`, depois
`#include "objects/fan.hpp"` onde precisar. Em qual pasta cada coisa fica: [ARQUITETURA.md § 4](ARQUITETURA.md#4-onde-adicionar-uma-feature).
**Não precisa editar o `CMakeLists.txt`.** Ele encontra os `.cpp` sozinho, inclusive em subpastas.
Se o arquivo novo não entrar no build, rode `cmake --preset ...` de novo.

Nos `.cpp`, inclua `<glad/gl.h>` antes de qualquer outro header de OpenGL. Use só OpenGL **3.3 core**: nada de `glBegin`/`glEnd`,
`glMatrixMode` ou `gluPerspective`. As matrizes vêm do GLM e o desenho é feito com VAO/VBO e shaders.

## 7. Problemas comuns

<details>
<summary>Windows: <code>cmake</code>, <code>g++</code> ou <code>mingw32-make</code> não é reconhecido como comando</summary>

O `C:\msys64\ucrt64\bin` não está no `Path`, ou o terminal foi aberto antes de você mudar o `Path`. Refaça os passos 2 e 3 da
[seção 1](#1-windows-mingw-w64-via-msys2) e abra um terminal novo.
</details>

<details>
<summary>Windows: <code>MSVC não é suportado</code> ou o CMake escolheu o Visual Studio</summary>

O CMake foi chamado sem o preset (ou pelo Visual Studio). Apague `build/` e rode `cmake --preset windows` no terminal.
Se o erro disser que o `g++` "is not a full path and was not found in the PATH", o `C:\msys64\ucrt64\bin` não está no `Path`.
</details>

<details>
<summary>Linux/WSL: <code>RandR headers not found</code>, <code>Xinerama headers not found</code> ou parecido</summary>

Faltam os headers do X11. Rode o `apt install` da [seção 2](#2-linux-ubuntudebian), apague `build/` e configure de novo.
</details>

<details>
<summary><code>No such preset</code> ou <code>CMakePresets.json</code> com versão não suportada</summary>

O CMake é mais antigo que o 3.21. No Windows, atualize com `pacman -Syu` no terminal MSYS2. No Ubuntu 20.04 ou mais antigo,
instale um CMake novo pelo `pip install cmake` ou pelo <https://apt.kitware.com>.
</details>

<details>
<summary>WSL: <code>GLFW (65550): X11: Failed to open display</code>, <code>The DISPLAY environment variable is missing</code> ou a janela não aparece</summary>

O WSLg não está ativo. No PowerShell:

```powershell
wsl --update
wsl --shutdown
```

Abra o Ubuntu de novo e rode `echo $DISPLAY`. Tem que aparecer `:0`.
Se você colocou `export DISPLAY=...` no `~/.bashrc` por causa de um tutorial antigo (VcXsrv/Xming), **apague essa linha**.
</details>

<details>
<summary><code>GLFW (65543): ... requested version 3.3 ...</code> ou <code>GLAD: não foi possível carregar o OpenGL 3.3</code></summary>

O driver de vídeo não oferece OpenGL 3.3 core. Atualize o driver da placa de vídeo **no Windows** (também vale para o WSL;
não instale driver dentro do Ubuntu). No Linux, rode `cmake -P cmake/doctor.cmake` e confira a linha `OpenGL core`.
</details>

<details>
<summary><code>$'\r': command not found</code> ou erros estranhos de sintaxe depois de editar no Windows</summary>

Os arquivos estão com quebra de linha do Windows (CRLF). O `.gitattributes` do repo força LF em todo clone, então isso só acontece
se algum editor salvou o arquivo com CRLF. No VS Code, clique em `CRLF` na barra inferior, troque para `LF` e salve.
</details>

<details>
<summary>WSL: janela preta, lenta, ou o <code>doctor.cmake</code> mostra <code>llvmpipe</code></summary>

O OpenGL está rodando na CPU. Atualize o driver de vídeo **no Windows** (não instale driver dentro do Ubuntu) e rode `wsl --shutdown`.
Se a GPU der problema, force software para testar:

```bash
LIBGL_ALWAYS_SOFTWARE=1 ./build/linux/bin/aula-opengl
```
</details>
