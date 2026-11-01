# Sala de Aula 3D em OpenGL

Cena 3D de uma sala de aula, explorável com teclado e mouse, feita em **C++17** e **OpenGL 3.3 core profile** sem motor
gráfico. Toda a geometria é gerada no código: são nove objetos modelados pela equipe, além de paredes, piso e teto. A
cena tem iluminação Blinn-Phong, materiais por peça, texturas e quatro interações, entre elas um projetor que projeta
slides na tela retrátil por textura projetiva.

Projeto da AB2 da disciplina de **Computação Gráfica**.

| | |
| --- | --- |
| **Linguagem** | C++17 com CMake (3.21+) |
| **Gráficos** | OpenGL 3.3 core profile, shaders GLSL próprios (sem pipeline fixo) |
| **Bibliotecas** | [GLFW](https://www.glfw.org) (janela e entrada), [GLAD](https://github.com/Dav1dde/glad) (loader OpenGL), [GLM](https://github.com/g-truc/glm) (matemática), [stb_image](https://github.com/nothings/stb) (texturas). Todas já vêm em `lib/` |
| **Sistemas suportados** | Windows (MinGW-w64 via MSYS2), Linux (Ubuntu) e WSL2 |

## Sumário

- [O que tem na cena](#o-que-tem-na-cena)
- [Técnicas implementadas](#técnicas-implementadas)
- [Como compilar](#como-compilar)
- [Comandos](#comandos)
- [Estrutura do projeto](#estrutura-do-projeto)
- [Documentação](#documentação)
- [Equipe](#equipe)
- [Créditos e licenças](#créditos-e-licenças)

## O que tem na cena

Uma sala de 8 × 3 × 7 m com seis carteiras e seis cadeiras em duas fileiras, mesa do professor, quadro, tela retrátil
diante do quadro, projetor de teto, ventilador de parede, quatro luminárias, interruptor e lixeira. A unidade é o metro,
e os móveis seguem medidas reais (tampo da carteira a 0,75 m do chão, quadro de 3,00 × 1,20 m, lona de 2,0 × 1,5 m).

| Objeto | Destaques |
| --- | --- |
| Ventilador de parede | Hierarquia corpo → rotor; pás com perfil gerado por função; só o rotor gira |
| Cadeira | Tubo e material metálico compartilhados com a carteira; encosto inclinado 8° |
| Quadro | Moldura, bandeja, apagador e marcadores como peças separadas |
| Carteira | Tampo laminado, estrutura de tubo de 22 mm, porta-livros de arame |
| Luminária e interruptor | Refletor em "W", tubos que brilham quando acesos; tecla do interruptor bascula |
| Mesa do professor | Madeira com veio, gaveteiro de 3 gavetas com puxadores metálicos |
| Projetor | Suporte de teto; mira calculada para o centro da tela; lente acende ao ligar |
| Tela retrátil | Pivô no topo: a lona desce e sobe sem descolar da caixa |
| Lixeira | Tronco de cone oco montado vértice a vértice, com borda e fundo |

## Técnicas implementadas

- **Modelagem procedural:** primitivas paramétricas (caixa, cilindro, plano) e meshes montados vértice a vértice, todos
  com posição, normal e UV. As UVs são calculadas em metros, então a textura não estica em peças finas.
- **Mesh reutilizável (VAO/VBO/EBO):** uma única estrutura de mesh para todos os objetos. Instâncias repetidas
  (carteiras, cadeiras, luminárias) compartilham os mesmos dados na GPU.
- **Grafo de cena e transformações:** cada peça guarda `T · R · S` relativo ao pai, e o filho herda a matriz do pai.
  Translação no posicionamento e na câmera, rotação nas hélices (`T(centro) · Rz(θ)`), escala na lona
  (`T(fenda) · S(1, s, 1)`).
- **Matriz das normais:** `transpose(inverse(mat3(model)))` por peça, para a luz continuar correta com escala não
  uniforme.
- **Câmera em primeira pessoa:** yaw/pitch pelo mouse (pitch limitado a ±89°), `lookAt` e projeção perspectiva de 60°,
  movimento normalizado e baseado em `deltaTime`.
- **Iluminação Blinn-Phong por fragmento:** luz ambiente, uma luz direcional de preenchimento e quatro luzes pontuais
  com atenuação, uma por luminária, além de emissiva para os tubos e a lente.
- **Materiais por peça:** cor difusa, textura opcional com tiling, especular, brilho e emissiva. Os presets metal,
  plástico, madeira e fosco são herdados pela árvore da cena.
- **Texturas:** stb_image com flip vertical, mipmaps, filtragem trilinear e `GL_REPEAT`. A cor da textura é multiplicada
  pela cor do material.
- **Textura projetiva:** o projetor funciona como uma segunda câmera (view + projeção próprias). O slide cai onde o
  feixe bater, com keystone e queda de intensidade com a distância.
- **Animações por `deltaTime`:** a tela usa smoothstep e o ventilador acelera e desacelera, com a mesma duração em
  qualquer FPS.

Fórmulas, valores e decisões estão em [docs/ILUMINACAO.md](docs/ILUMINACAO.md) e
[docs/ARQUITETURA.md](docs/ARQUITETURA.md). Os trechos de escala, rotação e projeção estão marcados com `AB2 ·` no
código (`git grep -n "AB2 ·"`).

## Como compilar

O projeto compila com **CMake + g++**. GLFW, GLAD, GLM e stb_image são compiladas a partir de `lib/`, então não é
preciso instalar nenhuma delas no sistema. Passo a passo completo e problemas comuns: [docs/SETUP.md](docs/SETUP.md).

> [!IMPORTANT]
> - Use sempre os presets do CMake (`windows` ou `linux`).
> - No Windows o compilador é o **g++ do MSYS2 (UCRT64)**. Visual Studio/MSVC e vcpkg não são suportados.
> - Não instale GLFW/GLAD/GLM no sistema nem baixe `.dll`/`.lib` delas.
> - No WSL2 a janela abre pelo WSLg: não é preciso VcXsrv/Xming nem `export DISPLAY`.

### Windows

1. Instale o Git e o MSYS2 (`winget install Git.Git MSYS2.MSYS2`).
2. No terminal **MSYS2 UCRT64**, instale o compilador, o make e o CMake:

   ```bash
   pacman -Syu
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-cmake
   ```

3. Adicione `C:\msys64\ucrt64\bin` ao `Path` do Windows e reabra o terminal.
4. No PowerShell:

   ```powershell
   git clone https://github.com/JoseRiquelmeTds/ambiente-sala-de-aula-opengl.git
   cd ambiente-sala-de-aula-opengl
   cmake --preset windows
   cmake --build --preset windows --target run
   ```

O `.exe` é linkado de forma estática e roda em outro PC sem o MSYS2.

### Linux e WSL2 (Ubuntu)

```bash
sudo apt update && sudo apt install -y build-essential cmake libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev mesa-utils
git clone https://github.com/JoseRiquelmeTds/ambiente-sala-de-aula-opengl.git
cd ambiente-sala-de-aula-opengl
cmake --preset linux
cmake --build --preset linux --target run
```

No WSL2, clone na home do Linux (`cd ~`): em `/mnt/c/...` o build fica bem mais lento.

### Comandos de build

No Windows, troque `linux` por `windows`.

| Comando | O que faz |
| --- | --- |
| `cmake --preset linux` | Configura o build de debug em `build/linux/` (só na primeira vez) |
| `cmake --build --preset linux` | Compila |
| `cmake --build --preset linux --target run` | Compila e abre a janela |
| `cmake --preset linux-release` | Configura o build otimizado (`-O2`) em `build/linux-release/` |
| `cmake --build --preset linux-release --target run` | Compila otimizado e abre a janela |
| `cmake -P cmake/doctor.cmake` | Verifica o ambiente (compilador, CMake, OpenGL) |

O executável fica em `build/<preset>/bin/aula-opengl` (`.exe` no Windows). O alvo `run` executa a partir da raiz do
repositório, de onde os shaders e as texturas são lidos. Se abrir o executável de outro jeito, rode-o com a raiz do
repositório como pasta atual. Para começar do zero, apague a pasta `build/`.

## Comandos

Clique na janela para capturar o cursor e começar a navegar. A sala começa com as luzes acesas, a tela recolhida e o
ventilador e o projetor desligados.

### Navegação

| Tecla | Ação |
| --- | --- |
| Clique esquerdo | Captura o cursor e ativa a navegação |
| Mouse | Olha para os lados e para cima/baixo |
| `W` / `S` | Avança / recua na direção da câmera |
| `A` / `D` | Move para a esquerda / direita |
| `Espaço` / `Ctrl` esquerdo | Sobe / desce |

### Interações

| Tecla | Ação |
| --- | --- |
| `T` | Abre / recolhe a tela retrátil (apertar no meio do curso inverte o sentido) |
| `V` | Liga / desliga o ventilador |
| `Shift` + `V` | Troca a velocidade do ventilador: 2, 3 ou 4 voltas/s |
| `P` | Liga / desliga o projetor: o slide aparece na tela e a lente acende |
| `←` / `→` | Slide anterior / próximo |
| `L` | Acende / apaga as luzes da sala (o interruptor da parede acompanha) |

### Inspeção de objetos

Mostra um objeto isolado, com a câmera enquadrada nele. Depois a câmera continua livre.

| Tecla | Ação |
| --- | --- |
| `1` | Sala completa |
| `2` a `9`, `0` | Carteira, objeto articulado de demonstração, ventilador, lâmpada, interruptor, mesa do professor, tela retrátil, projetor, lixeira |
| `[` / `]` | Alvo anterior / próximo, em ciclo, incluindo a cadeira e o quadro |

### Janela

| Tecla | Ação |
| --- | --- |
| `Esc` | Libera o cursor; com o cursor já livre, fecha o programa |
| `Q` | Fecha o programa |

Com o cursor livre, a câmera não se move, mas a inspeção e as interações continuam funcionando. Ao perder o foco da
janela, o cursor é liberado e é preciso clicar de novo.

## Estrutura do projeto

```
include/          headers (.hpp)
  core/           entrada (teclado/mouse) e câmera
  gfx/            OpenGL: mesh, primitivas, shader, textura, material, luzes, renderer, grafo de cena
  objects/        um módulo por objeto da sala
  scene/          montagem da sala, luzes e teclas das interações
src/              implementações (.cpp), nas mesmas pastas; main.cpp é o ponto de entrada
shaders/          vertex e fragment shader (GLSL 330)
assets/textures/  texturas e slides do projetor
lib/              GLFW, GLAD, GLM e stb_image
cmake/            doctor.cmake (diagnóstico do ambiente)
docs/             documentação técnica
```

Cada camada só depende das que estão abaixo dela (`scene → objects → gfx → core`). Os objetos são modelados no próprio
espaço, com pivô na origem, e só a `scene/` decide onde cada um fica e qual tecla aciona cada interação.

## Documentação

| Documento | Conteúdo |
| --- | --- |
| [docs/SETUP.md](docs/SETUP.md) | Instalação do ambiente (Windows, Linux, WSL2) e problemas comuns |
| [docs/ARQUITETURA.md](docs/ARQUITETURA.md) | Camadas, módulos, loop principal, montagem da sala e interações |
| [docs/ILUMINACAO.md](docs/ILUMINACAO.md) | Blinn-Phong, luzes, materiais e textura projetiva do projetor |
| [docs/CONVENCOES.md](docs/CONVENCOES.md) | Convenções de modelagem: metros, eixos, pivô, vértice, posições na sala |
| [docs/DEV-CONVENCOES.md](docs/DEV-CONVENCOES.md) | Código, branches, commits e pull requests |
| [lib/README.md](lib/README.md) | Versões e licenças das bibliotecas |
| [assets/textures/README.md](assets/textures/README.md) | Origem e licença de cada textura |

## Equipe

| Membro | Objetos | Outras partes |
| --- | --- | --- |
| Victor André Lopes Brasileiro | Ventilador, cadeira, quadro | Janela e entrada, câmera, montagem da sala |
| José Riquelme Teixeira da Silva | Carteira, luminária e interruptor, mesa do professor | Shader, mesh, texturas, iluminação, materiais, interação da luz |
| Erasmo da Silva Sá Junior | Projetor, tela retrátil, lixeira | Repositório e build, interações da tela, do ventilador e do projetor |

### Contribuindo

Crie a branch a partir de `dev` (`CG-<n>/<tipo>-<descricao>`), faça commits `<tipo>(CG-<n>): <descrição>` e abra o PR
para `dev` com uma aprovação de outro membro. Detalhes em [docs/DEV-CONVENCOES.md](docs/DEV-CONVENCOES.md).

Assistentes de IA que estiverem ajudando neste repositório devem ler [docs/SETUP.md](docs/SETUP.md) e
[docs/ARQUITETURA.md](docs/ARQUITETURA.md) antes de sugerir comandos ou mudanças de build. Não proponha MSVC, vcpkg,
bibliotecas do sistema, GLUT/GLEW, pipeline fixo nem servidor X externo.

## Créditos e licenças

- Todas as texturas e os três slides foram gerados pela equipe com scripts próprios. A origem de cada imagem está em
  [assets/textures/README.md](assets/textures/README.md).
- Bibliotecas de terceiros em `lib/`, cada uma com a própria licença: GLFW (zlib), GLAD (código gerado em CC0/WTFPL e Apache-2.0),
  GLM (MIT) e stb_image (MIT ou domínio público). Versões em [lib/README.md](lib/README.md).
- Nenhum modelo 3D foi importado: toda a geometria da cena é gerada no código.
