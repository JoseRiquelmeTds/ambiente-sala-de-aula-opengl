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
| `1` a `9`, `0` | Inspeção: cena completa / carteira / objeto articulado / ventilador / lâmpada / interruptor / mesa do professor / tela retrátil / projetor / lixeira, cada um isolado |
| `[` / `]` | Alvo anterior / próximo, em ciclo pelos onze alvos: cadeira após a lixeira e quadro ao final |
| `L` | Liga / desliga as luzes do teto, como o interruptor da parede (a sala começa acesa) |
| `T` | Abre / recolhe a tela retrátil, com animação suave; apertar no meio do curso inverte o sentido |
| `V` | Liga / desliga o ventilador (começa desligado) |
| `Shift` + `V` | Troca a velocidade do ventilador: 2, 3 ou 4 voltas/s (começa na mais rápida) |
| `P` | Liga / desliga o projetor: o slide aparece na tela e a lente acende (começa desligado) |
| `←` / `→` | Slide anterior / próximo do projetor, em ciclo |
| `Esc` | Libera o cursor; com o cursor já livre, fecha |
| `Q` | Fecha |

Com o cursor livre não há movimento nem rotação pelo mouse; a inspeção (`1`–`9`, `0`, `[` e `]`), a luz (`L`), a
tela (`T`), o ventilador (`V`) e o projetor (`P`, `←` e `→`) funcionam mesmo assim.
As teclas numéricas mantêm os nove alvos anteriores; após uma seleção numérica, `[` e `]` continuam a partir dela.
Da cena completa (`1`), `]` seleciona a carteira e `[` seleciona o quadro. O ciclo retorna do último alvo ao primeiro
e vice-versa. Ao perder o foco, o cursor é liberado e é preciso clicar de novo.

<!-- CG-81 (Sprint 3): interações (tela retrátil, ventilador, projetor) e a inspeção de cada objeto. -->

A carteira (CG-62, `objects::StudentDesk`) tem tampo de MDF de 0,60 × 0,45 m a 0,75 m do chão, estrutura de tubo de
aço de 22 mm, porta-livros de arame sob o tampo e ponteiras de plástico. Pivô no chão, no centro entre os pés, com a
frente (+Z) para o lado do aluno. Tubo e espessura do tampo vêm de `objects/furniture_style.hpp`, compartilhado com a
cadeira. As seis instâncias da montagem compartilham os meshes e os materiais (CG-74): tubo de aço com pintura
grafite, tampo de laminado madeirado claro (`laminado_carteira.png`) e ponteiras de plástico preto. Os materiais do
tubo e do laminado também ficam em `objects/furniture_style.hpp`, usados pela cadeira.

O ventilador (CG-59) tem três pás de aproximadamente 0,50 m de diâmetro e grades de 0,556 m externos.
A origem local é a fixação na parede, com frente +Z. `Classroom` o fixa na parede esquerda, voltado para dentro;
`objects::Fan::setRotorAngle(graus)` gira só pás e tampa, sem reconstruir geometria. Com `V` (CG-76) ele liga e
desliga: o rotor acelera até a velocidade escolhida em até 1,5 s e, desligado, desacelera até parar em até 3 s, com o
giro calculado por `deltaTime` (igual em qualquer FPS). `Shift` + `V` alterna entre 2, 3 e 4 voltas/s, abaixo de um
ventilador real para as três pás não parecerem paradas ou girando para trás na tela (efeito roda de carroça). Só o
rotor gira, em torno do eixo do motor; grades, motor e suporte ficam parados. Suporte e eixo usam metal pintado
grafite acetinado; as grades são metálicas cinza, o motor é plástico escuro, as pás são plástico cinza-claro
e a tampa central é plástico grafite. Cor e reflexo diferenciam as peças, sem textura nas pás em movimento.

A cadeira escolar (CG-60, décimo alvo, acessível por `[` e `]`) tem assento de 0,40 × 0,40 m com topo a 0,45 m,
placas de 0,018 m e encosto inclinado 8° para trás (-Z), chegando a aproximadamente 0,85 m. Quatro pernas tubulares
de raio 0,011 m (16 segmentos), quadro sob o assento, travessas inferiores e dois montantes sustentam as placas;
sapatas e fixações discretas completam a estrutura. O pivô fica no chão entre as pernas, com frente +Z.
`objects::Chair::addTo` cria raízes independentes com meshes compartilhados; a CG-69 monta seis cadeiras atrás
das carteiras, olhando para -Z, com inspeção acompanhando o placement. A cadeira usa `objects::furniture::kTubeRadius`,
`kTubeSegments` e `kBoardThickness` do header compartilhado `include/objects/furniture_style.hpp` da CG-62,
sem duplicar constantes.
Tubos, travessas, montantes e fixações usam `furniture::tubeMaterial()`; assento e encosto usam
`furniture::boardMaterial()` com `laminado_carteira.png`, repetida a cada 0,5 m (`kBoardTextureSize`, `uvScale` = 1),
como na carteira. As ponteiras usam plástico preto com reflexo fraco. As seis instâncias compartilham os meshes,
os materiais e uma única textura pertencente a `Chair`, sem carregar imagens em `addTo`.

O quadro escolar (CG-61, `objects::Board`) tem moldura externa de 3,00 × 1,20 m, painel estrutural de 16 mm e
superfície lisa de escrita de 2,95 × 1,15 m, com face frontal em Z local = 0,018 m e UV 0–1 sem espelhamento.
Moldura nos quatro lados (25 mm de largura), quatro acabamentos de canto e bandeja inferior de 2,60 × 0,10 m
com bordas completam o conjunto. Na bandeja ficam um apagador de 0,10 × 0,05 × 0,03 m (largura, profundidade,
altura), com feltro separado, e dois marcadores de 0,13 m com tampas e extremidades separadas.
O pivô fica no centro da face posterior de fixação, frente +Z; a posição definitiva em `Classroom` é
`(0, 1,50, -3,50)` m, com bordas da moldura a 0,90 e 2,10 m do chão.
Moldura e cantos chegam a 0,022 e 0,024 m da parede: ficam pelo menos 14 mm atrás da traseira da lona
(0,038 m), sem mudar a tela ou o alinhamento do projetor. A bandeja avança de 0,012 a 0,112 m da parede;
com acessórios, fica entre 0,896 e 0,930 m do chão, abaixo da barra e do puxador da tela aberta.
O quadro é o 11º alvo, depois da cadeira: da cena completa, pressione `[`; da cadeira, `]`; da lixeira (`0`),
duas vezes `]`. A inspeção isola o conjunto, sem a tela, em perspectiva levemente acima e à direita.
Cada `addTo` cria uma raiz independente com meshes e materiais compartilhados e acessórios descendentes.
A superfície é branco fosco, sem textura para preservar a leitura da projeção; o suporte posterior é MDF,
moldura e bandeja são alumínio, cantos e corpo do apagador são plástico escuro e o feltro não tem especular.
Os marcadores têm corpo plástico claro e tampas/extremidades azuis ou vermelhas. Geometria, UVs e folga da lona
permanecem inalteradas.

A lâmpada (CG-63) é uma luminária de sobrepor de 1,27 × 0,25 m com dois tubos T8 de 1,20 m, refletor em "W" e aletas;
a origem local é o centro da face encostada no teto e ela desce para -Y. O interruptor tem espelho de 0,08 × 0,12 m,
origem no centro da face encostada na parede e frente +Z; `objects::LightSwitch::setOn(bool)` só inclina a tecla, sem
reconstruir geometria. A tecla `L` (CG-73) apaga e acende as quatro luminárias: a tecla do interruptor bascula, as luzes
pontuais desligam, os tubos param de brilhar e a luz ambiente cai, mas a luz de preenchimento continua iluminando a sala
([docs/ILUMINACAO.md § 9](docs/ILUMINACAO.md#9-interação-ligar-a-luz)). Materiais (CG-74), só cor, sem textura:
calha de chapa pintada de branco, refletor de alumínio, vidro leitoso do tubo e soquete de plástico; o interruptor é
de plástico branco brilhante. As luminárias encostam no teto e o interruptor fica na parede direita.

A mesa do professor (CG-64) tem tampo de 1,20 × 0,60 m a 0,75 m do chão, laterais e painel frontal de MDF e um
gaveteiro de 3 gavetas fixas com puxadores. O pivô fica no chão, no centro da mesa; a frente (+Z, painel frontal) é o
lado da turma e as gavetas abrem para o professor (-Z). A mesa fica na região frontal da sala. Materiais (CG-74):
madeira escura com veio (`madeira_mesa_professor.png`) no tampo, nos painéis e nas gavetas, puxadores de metal
escovado e sapatas de plástico preto.
A tela retrátil (CG-66) tem lona de 2,0 × 1,5 m e caixa de 2,10 m no alto da parede frontal.
A origem local é o centro da face da caixa encostada na parede, com frente +Z.
`objects::ProjectionScreen::setOpenAmount(s)` recolhe a lona para cima (0 = recolhida, 1 = aberta) escalando só a
articulação no topo da lona, então ela desce sem descolar da caixa. A tela começa recolhida, deixando o quadro à vista;
a tecla `T` (CG-75) abre ou recolhe em ~2,5 s, com aceleração e frenagem suaves, e para nos limites (só a barra na
fenda / lona inteira de 1,5 m). Materiais (CG-78): caixa e suportes de plástico branco-gelo, lona branca fosca com trama
(`lona_tela.png`, repetida a cada 25 cm pelo `uvScale` do material, porque a UV da lona fica de 0 a 1 para o slide),
barra inferior de alumínio, cordinha cinza-escura e puxador de plástico preto.

O projetor (CG-65) fica preso ao teto a cerca de 2,9 m da tela, com corpo de 0,30 × 0,10 × 0,25 m e a lente como peça
separada (a CG-77 dá brilho a ela). A origem local é o ponto de fixação no teto, com frente (lente) +Z.
`objects::Projector::aimAt(alvo)` inclina a cabeça para o eixo da lente passar pelo alvo; a `Classroom` usa o centro
da lona, então mover a tela ou o projetor realinha os dois sozinho. A tecla `P` (CG-77) liga o projetor: a lente
acende e o slide é projetado por textura projetiva, como uma luz que carrega a imagem, e cai onde o feixe bater
(na lona aberta ou, com ela recolhida, no quadro e na parede), num trapézio levemente mais largo embaixo
(keystone). `←` e `→` trocam entre três slides (`slide_cg_1.png` a `slide_cg_3.png`). Técnica em
[docs/ILUMINACAO.md § 10](docs/ILUMINACAO.md#10-projetor-textura-projetiva-cg-77). Materiais (CG-78): placa do teto
e haste de metal grafite, suporte e corpo de plástico cinza texturizado (`plastico_projetor.png`) e lente de vidro
escuro e brilhante, com material próprio que acende com o projetor ligado.

A lixeira (CG-67) é um tronco de cone oco de 0,30 m de diâmetro na boca e 0,40 m de altura, com parede de 6 mm e
borda com espessura, no chão ao lado da região do quadro. Pivô no centro do fundo.
O mesh é montado em `src/objects/trash_can.cpp` (paredes externa e interna, borda e dois fundos). Material (CG-78):
polipropileno azul acetinado com textura (`plastico_lixeira.png`, repetida a cada 25 cm), o mesmo na peça inteira.

## Montagem da sala (CG-69)

`objects::Room` forma um interior de **8 × 3 × 7 m**: X de -4 a +4, Y de 0 a 3 e Z de -3,5 a +3,5.
O piso de 12 cm termina em Y = 0; o teto de 10 cm começa em Y = 3. As paredes de 10 cm crescem para fora
do volume útil. Rodapés de 8 cm de altura dão acabamento. Piso, paredes, teto e rodapés têm meshes separados,
normais externas aos sólidos (voltadas para a sala nas faces internas) e UVs físicas: repetição a cada 1 m
nas superfícies principais e 0,5 m nos rodapés. Até receber os próprios materiais, usa o material padrão (cinza claro fosco).

| Elemento | Pivô no mundo (m) | Orientação / quantidade |
| --- | --- | --- |
| Carteiras | X = ±1,25; Y = 0; Z = -0,85 / 0,65 / 2,15 | 6, sem giro: +Z é o lado onde o aluno senta, olhando para -Z |
| Cadeiras | X = ±1,25; Y = 0; Z = -0,30 / 1,20 / 2,70 | 6, giro de 180° em Y: frente -Z para o quadro |
| Quadro | (0; 1,50; -3,50) | 1, frente +Z, atrás da tela; bandeja e acessórios preservados |
| Mesa do professor | (0; 0; -2,35) | 1, painel +Z para a turma; 0,85 m até a parede |
| Tela | (0; 2,86; -3,5) | 1, frente +Z, placement preservado |
| Projetor | (0; 3; -0,5) | 1, giro de 180° em Y e `aimAt(canvasCenter)` preservado |
| Ventilador | (-4; 2,35; -0,4) | 1, giro de +90° em Y: frente +X para dentro |
| Luminárias | X = ±1,6; Y = 3; Z = ±1,5 | 4, face superior no teto, corpo descendo |
| Interruptor | (4; 1,1; 2,8) | 1, giro de -90° em Y: frente -X para dentro |
| Lixeira | (2,6; 0; -3,1) | 1, pivô no chão |
| Demonstração articulada | (2,9; 0; 1,8) | 1, preservada na lateral |

O corredor central tem 1,90 m entre tampos e as fileiras têm passo de 1,50 m. Cada cadeira fica 0,55 m atrás
da sua carteira: a frente do assento tem 0,125 m de folga até o tampo, com espaço livre entre fileiras e junto
à parede traseira. O quadro de 3 × 1,2 m fica centrado em (0; 1,5; -3,5), com a moldura pelo menos 14 mm atrás
da lona. As geometrias originais de `Chair` (CG-60) e `Board` (CG-61) são reutilizadas sem alterações.

A vista inicial e `1` usam a câmera em (3,3; 2,3; 3,1), dentro do corredor traseiro, olhando para
(-0,4; 0,9; -0,7). Clique para navegar livremente com os controles acima; não há colisão ou gravidade.
As teclas `2`–`9` e `0` mantêm a ordem existente: cada uma mostra só um exemplar do objeto, escondendo
Room e todas as outras instâncias. `[` e `]` percorrem os onze alvos em ciclo, incluindo uma cadeira
representativa como décimo alvo e o quadro como décimo primeiro. A câmera de cada inspeção acompanha o placement.

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
