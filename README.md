# Aula-OpenGl

Sala de aula 3D navegável em C++17 e OpenGL 3.3 core: projeto da AB2 de Computação Gráfica.

> [!CAUTION]
> ## ⚠️ LEIA ANTES DE QUALQUER COISA → [docs/SETUP.md](docs/SETUP.md)
>
> **O projeto compila com CMake + g++ no Windows (MinGW-w64/MSYS2), no Linux e no WSL2.** As bibliotecas (GLFW, GLAD, GLM e
> stb_image) já vêm no repositório.
>
> - ❌ **Não** instale GLFW/GLAD/GLM no sistema nem baixe `.dll`/`.lib` delas. O CMake compila as de `lib/`.
> - ❌ **Não** use Visual Studio/MSVC nem vcpkg. No Windows o compilador é o g++ do MSYS2.
> - ❌ **Não** instale VcXsrv/Xming nem faça `export DISPLAY=...` no WSL. O WSLg já abre a janela.
> - ✅ Use sempre os presets: `cmake --preset windows` ou `cmake --preset linux`.
> - ✅ Rode `cmake -P cmake/doctor.cmake` antes de pedir ajuda.

> [!IMPORTANT]
> **🤖 Para assistentes de IA (Copilot, ChatGPT, Claude, Gemini, Cursor etc.) que estejam ajudando neste repositório:**
> leia [docs/SETUP.md](docs/SETUP.md) antes de sugerir qualquer comando, dependência ou mudança de build, e
> [docs/ARQUITETURA.md](docs/ARQUITETURA.md), [docs/CONVENCOES.md](docs/CONVENCOES.md) e
> [docs/DEV-CONVENCOES.md](docs/DEV-CONVENCOES.md) antes de criar ou mover arquivos, nomear código, branches ou commits.
> O build é **CMake + g++**, com **OpenGL 3.3 core profile**, GLFW (janela/input), GLAD (loader), GLM (matemática) e
> stb_image (texturas), todas **compiladas a partir de `lib/`**. Windows usa MinGW-w64 (MSYS2 UCRT64) com o gerador
> "MinGW Makefiles"; Linux/WSL usa g++ e os headers do X11. Os presets estão em `CMakePresets.json`.
> Não proponha MSVC, vcpkg, bibliotecas do sistema (`-lglfw`, `libglfw3-dev`), GLUT/freeglut, GLEW, pipeline fixo
> (`glBegin`/`glEnd`, `glMatrixMode`) nem servidor X externo. Headers ficam em `include/`, implementações em `src/`, as
> duas divididas em `core/`, `gfx/`, `objects/` e `scene/` (veja docs/ARQUITETURA.md), shaders em `shaders/`, imagens em `assets/textures/`, bibliotecas de terceiros em `lib/`. O `CMakeLists.txt` encontra os
> `.cpp` sozinho (não precisa ser editado para adicionar módulos).

---

## Como compilar

Pré-requisitos e instalação passo a passo para cada sistema: [docs/SETUP.md](docs/SETUP.md).

**Windows** (com MSYS2 instalado, veja [docs/SETUP.md § 1](docs/SETUP.md#1-windows-mingw-w64-via-msys2)), no PowerShell:

```powershell
git clone https://github.com/JoseRiquelmeTds/Aula-OpenGl.git
cd Aula-OpenGl
cmake --preset windows
cmake --build --preset windows --target run
```

**Linux / WSL2** (Ubuntu):

```bash
sudo apt update && sudo apt install -y build-essential cmake libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev mesa-utils
git clone https://github.com/JoseRiquelmeTds/Aula-OpenGl.git
cd Aula-OpenGl
cmake --preset linux
cmake --build --preset linux --target run
```

Comandos de build (no Windows, troque `linux` por `windows`):

| Comando | O que faz |
| --- | --- |
| `cmake --preset linux` | Configura o build (debug) em `build/linux/`. Só precisa rodar uma vez |
| `cmake --build --preset linux` | Compila |
| `cmake --build --preset linux --target run` | Compila e abre a janela |
| `cmake --preset linux-release` e `cmake --build --preset linux-release --target run` | Mesmo fluxo, otimizado (`-O2`), em `build/linux-release/` |
| `cmake -P cmake/doctor.cmake` | Verifica o ambiente |

Para começar do zero, apague a pasta `build/`.

<!-- CG-81 (Sprint 3): SO testados, versão final e teste de build limpo seguindo só este README. -->

## Comandos

Teclado e mouse dentro da janela. Lista completa na entrega (CG-81); atalhos planejados e reservados estão em
[docs/CONVENCOES.md § 8](docs/CONVENCOES.md#8-atalhos-de-teclado).

| Controle | Ação |
| --- | --- |
| Clique esquerdo | Captura o cursor e ativa a navegação (janela em foco) |
| `W` / `S` | Avança / recua na direção da câmera, incluindo a inclinação |
| `A` / `D` | Move para a esquerda / direita |
| `Espaço` / `Ctrl` esquerdo | Sobe / desce no eixo Y do mundo |
| Mouse (cursor capturado) | Olha para os lados (yaw) e para cima/baixo (pitch limitado a ±89°) |
| `1` a `9`, `0` | Inspeção: cena completa / cubo / objeto articulado / ventilador / lâmpada / interruptor / mesa do professor / tela retrátil / projetor / lixeira, cada um isolado |
| `Esc` | Libera o cursor; com o cursor já livre, fecha |
| `Q` | Fecha |

Com o cursor livre não há movimento nem rotação pelo mouse; a inspeção (`1`–`7`) funciona mesmo assim. Ao perder o foco,
o cursor é liberado e é preciso clicar de novo.

<!-- CG-81 (Sprint 3): interações (tela retrátil, ventilador, projetor) e a inspeção de cada objeto. -->

O ventilador (CG-59) tem três pás de aproximadamente 0,50 m de diâmetro e grades de 0,556 m externos.
A origem local é a fixação na parede, com frente +Z. `Classroom` define seu posicionamento provisório;
`objects::Fan::setRotorAngle(graus)` gira só pás e tampa, sem reconstruir geometria. A tecla `V` e a animação
ficam para a CG-76. Até os materiais por peça (CG-72), ele usa a mesma textura UV provisória da cena;
grade/suporte serão metal cinza-escuro e motor/pás, plástico preto.

A lâmpada (CG-63) é uma luminária de sobrepor de 1,27 × 0,25 m com dois tubos T8 de 1,20 m, refletor em "W" e aletas;
a origem local é o centro da face encostada no teto e ela desce para -Y. O interruptor tem espelho de 0,08 × 0,12 m,
origem no centro da face encostada na parede e frente +Z; `objects::LightSwitch::setOn(bool)` só inclina a tecla, sem
reconstruir geometria. A tecla `L` e o acender da luz ficam para a CG-73. Até os materiais por peça (CG-72), os dois
usam a textura UV provisória da cena; as posições em `Classroom` também são provisórias.

A mesa do professor (CG-64) tem tampo de 1,20 × 0,60 m a 0,75 m do chão, laterais e painel frontal de MDF e um
gaveteiro de 3 gavetas fixas com puxadores. O pivô fica no chão, no centro da mesa; a frente (+Z, painel frontal) é o
lado da turma e as gavetas abrem para o professor (-Z). A posição em `Classroom` é provisória. Até os materiais por peça
(CG-72), usa a mesma textura UV da cena.
A tela retrátil (CG-66) tem lona de 2,0 × 1,5 m e caixa de 2,10 m no alto da parede do quadro, em posição
provisória até a sala (CG-69). A origem local é o centro da face da caixa encostada na parede, com frente +Z.
`objects::ProjectionScreen::setOpenAmount(s)` recolhe a lona para cima (0 = recolhida, 1 = aberta) escalando só a
articulação no topo da lona; a tecla `T` e a animação ficam para a CG-75. Até os materiais por peça (CG-72), ela usa
a textura UV provisória da cena; a lona receberá `lona_tela.png`.

O projetor (CG-65) fica preso ao teto a cerca de 2,9 m da tela, com corpo de 0,30 × 0,10 × 0,25 m e a lente como peça
separada (a CG-77 dá brilho a ela). A origem local é o ponto de fixação no teto, com frente (lente) +Z.
`objects::Projector::aimAt(alvo)` inclina a cabeça para o eixo da lente passar pelo alvo; a `Classroom` usa o centro
da lona, então mover a tela ou o projetor realinha os dois sozinho. A tecla `P` e a projeção ficam para a CG-77; o
gabinete receberá `plastico_projetor.png`.

A lixeira (CG-67) é um tronco de cone oco de 0,30 m de diâmetro na boca e 0,40 m de altura, com parede de 6 mm e
borda com espessura, em posição provisória no canto da parede do quadro até a sala (CG-69). Pivô no centro do fundo.
O mesh é montado em `src/objects/trash_can.cpp` (paredes externa e interna, borda e dois fundos). Até os materiais por
peça (CG-72), ela usa a textura UV provisória da cena; receberá `plastico_lixeira.png`.

## Estrutura

```
include/          headers (.hpp), em core/ (input, câmera), gfx/ (OpenGL), objects/ (objetos) e scene/ (a sala)
src/              implementações (.cpp), mesmas pastas; main.cpp é o ponto de entrada
shaders/          shaders GLSL (.vert, .frag)
assets/textures/  imagens usadas como textura (com tabela de licenças)
lib/              bibliotecas de terceiros embutidas (GLFW, GLAD, GLM, stb_image), veja lib/README.md
cmake/            scripts auxiliares do CMake (doctor.cmake)
docs/             documentação
.github/          template de pull request
build/            gerado pelo CMake: build/<preset>/
```

Camadas, módulos e onde adicionar cada coisa: [docs/ARQUITETURA.md](docs/ARQUITETURA.md).

## Documentação

| Documento | Conteúdo |
| --- | --- |
| [docs/SETUP.md](docs/SETUP.md) | Instalação do ambiente (Windows, Linux, WSL2) e problemas comuns |
| [docs/ARQUITETURA.md](docs/ARQUITETURA.md) | Camadas, módulos, onde adicionar uma feature, revisão da arquitetura |
| [docs/CONVENCOES.md](docs/CONVENCOES.md) | Modelagem: metros, eixos, pivô, vértice, estilo da mobília, atalhos de teclado |
| [docs/DEV-CONVENCOES.md](docs/DEV-CONVENCOES.md) | Nomes e formatação do código, branches, commits, PRs e fluxo no Jira |
| [lib/README.md](lib/README.md) | Versões, origem e licenças das bibliotecas |
| [assets/textures/README.md](assets/textures/README.md) | Regras e licenças das texturas |

## Como contribuir

Resumo de [docs/DEV-CONVENCOES.md](docs/DEV-CONVENCOES.md):

1. Branch a partir de `dev`: `CG-<n>/<tipo>-<descricao>` (ex.: `CG-59/feat-ventilador-parede`).
2. Commits `<tipo>(CG-<n>): <descrição>` (ex.: `feat(CG-59): modela as hélices do ventilador`).
3. PR para `dev` com título `[CG-<n>] <resumo>`, preenchendo o template; 1 aprovação de outra pessoa para o merge.
