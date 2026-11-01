<div align="center">

# Sala de Aula 3D em OpenGL

**Uma sala de aula navegável, modelada inteiramente em código, com iluminação, texturas e projetor funcionando.**

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)
![OpenGL 3.3](https://img.shields.io/badge/OpenGL-3.3%20core-5586A4?logo=opengl&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.21%2B-064F8C?logo=cmake&logoColor=white)
![Plataformas](https://img.shields.io/badge/Windows%20%7C%20Linux%20%7C%20WSL2-suportados-informational)

Projeto da AB2 da disciplina de **Computação Gráfica**

[Como compilar](#como-compilar) · [Comandos](#comandos) · [Roteiro de teste](#roteiro-de-teste) · [Técnicas](#técnicas-implementadas) · [Equipe](#equipe)

</div>

---

## Sobre o projeto

Construímos uma sala de aula em 3D que pode ser explorada em primeira pessoa com teclado e mouse. Não usamos motor
gráfico: a geometria, os shaders, a iluminação e as interações foram escritos do zero em **C++17** e
**OpenGL 3.3 core profile**.

- **9 objetos modelados no código**, sem nenhum modelo importado, além de paredes, piso e teto
- **Iluminação Blinn-Phong** com quatro luminárias, luz de preenchimento e materiais distintos por peça
- **Texturas** com mipmaps e repetição calculada pelo tamanho real de cada peça
- **4 interações:** tela retrátil, ventilador, projetor e luz da sala
- **Projetor com textura projetiva:** o slide é projetado na tela como uma luz que carrega uma imagem

As bibliotecas usadas são [GLFW](https://www.glfw.org) (janela e entrada), [GLAD](https://github.com/Dav1dde/glad)
(loader do OpenGL), [GLM](https://github.com/g-truc/glm) (matemática) e [stb_image](https://github.com/nothings/stb)
(leitura de imagens). Todas já vêm no repositório, em `lib/`.

## A cena

Uma sala de 8 × 3 × 7 m com seis carteiras e seis cadeiras em duas fileiras, mesa do professor, quadro, tela retrátil,
projetor de teto, ventilador de parede, quatro luminárias, interruptor e lixeira. A unidade é o metro, e os móveis
seguem medidas reais.

| Objeto | Destaques |
| --- | --- |
| Ventilador de parede | Hierarquia corpo → rotor; pás com perfil gerado por função; só o rotor gira |
| Cadeira | Tubo e metal compartilhados com a carteira; encosto inclinado 8° |
| Quadro | Moldura, bandeja, apagador e marcadores como peças separadas |
| Carteira | Tampo laminado, estrutura de tubo de 22 mm, porta-livros de arame |
| Luminária e interruptor | Refletor em "W", tubos que brilham acesos; tecla do interruptor bascula |
| Mesa do professor | Madeira com veio, gaveteiro de 3 gavetas com puxadores metálicos |
| Projetor | Suporte de teto; mira calculada para o centro da tela; lente acende ao ligar |
| Tela retrátil | Pivô no topo: a lona desce e sobe sem descolar da caixa |
| Lixeira | Tronco de cone oco montado vértice a vértice, com borda e fundo |

## Como compilar

O projeto usa **CMake + g++**. GLFW, GLAD, GLM e stb_image são compiladas junto com o projeto a partir de `lib/`, então
não é preciso instalar nenhuma delas. Passo a passo detalhado e problemas comuns em [docs/SETUP.md](docs/SETUP.md).

> [!IMPORTANT]
> No Windows, o compilador é o **g++ do MSYS2 (UCRT64)**. Visual Studio/MSVC e vcpkg não são suportados.

<details open>
<summary><b>Windows</b></summary>

1. Instale o Git e o MSYS2:

   ```powershell
   winget install Git.Git MSYS2.MSYS2
   ```

2. No terminal **MSYS2 UCRT64**, instale o compilador, o make e o CMake:

   ```bash
   pacman -Syu
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-cmake
   ```

3. Adicione `C:\msys64\ucrt64\bin` ao `Path` do Windows e reabra o terminal.
4. No PowerShell, clone, compile e rode:

   ```powershell
   git clone https://github.com/JoseRiquelmeTds/ambiente-sala-de-aula-opengl.git
   cd ambiente-sala-de-aula-opengl
   cmake --preset windows
   cmake --build --preset windows --target run
   ```

O `.exe` é linkado de forma estática e roda em outro PC sem o MSYS2.

</details>

<details open>
<summary><b>Linux e WSL2 (Ubuntu)</b></summary>

```bash
sudo apt update && sudo apt install -y build-essential cmake libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev mesa-utils
git clone https://github.com/JoseRiquelmeTds/ambiente-sala-de-aula-opengl.git
cd ambiente-sala-de-aula-opengl
cmake --preset linux
cmake --build --preset linux --target run
```

No WSL2 a janela abre pelo WSLg, sem configuração extra. Clone na home do Linux (`cd ~`): em `/mnt/c/...` o build
fica bem mais lento.

</details>

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
repositório, de onde os shaders e as texturas são lidos; se abrir o executável de outro jeito, use a raiz como pasta
atual. Para começar do zero, apague a pasta `build/`.

## Comandos

Clique na janela para capturar o cursor e começar a navegar. A sala começa com as luzes acesas, a tela recolhida e o
ventilador e o projetor desligados.

<table>
<tr>
<td valign="top">

**Navegação**

| Tecla | Ação |
| --- | --- |
| Clique esquerdo | Captura o cursor |
| Mouse | Olha ao redor |
| `W` / `S` | Avança / recua |
| `A` / `D` | Esquerda / direita |
| `Espaço` | Sobe |
| `Ctrl` esquerdo | Desce |

</td>
<td valign="top">

**Interações**

| Tecla | Ação |
| --- | --- |
| `T` | Abre / recolhe a tela retrátil |
| `V` | Liga / desliga o ventilador |
| `Shift` + `V` | Velocidade: 2, 3 ou 4 voltas/s |
| `P` | Liga / desliga o projetor |
| `←` / `→` | Slide anterior / próximo |
| `L` | Acende / apaga as luzes |

</td>
</tr>
<tr>
<td valign="top">

**Inspeção de objetos**

| Tecla | Ação |
| --- | --- |
| `1` | Sala completa |
| `2` a `9`, `0` | Um objeto isolado |
| `[` / `]` | Objeto anterior / próximo |

</td>
<td valign="top">

**Janela**

| Tecla | Ação |
| --- | --- |
| `Esc` | Libera o cursor; livre, fecha |
| `Q` | Fecha o programa |

</td>
</tr>
</table>

Ordem da inspeção: `2` carteira, `3` objeto articulado de demonstração, `4` ventilador, `5` lâmpada, `6` interruptor,
`7` mesa do professor, `8` tela retrátil, `9` projetor e `0` lixeira. A cadeira e o quadro são alcançados com `[` e `]`.
Com o cursor livre, a câmera fica parada, mas a inspeção e as interações continuam funcionando.

## Roteiro de teste

Um passeio de cerca de dois minutos que passa por todas as funcionalidades. A câmera começa no fundo da sala, olhando
para o quadro.

1. **Navegação (translação).** Clique na janela. Ande com `W`, `A`, `S` e `D`, olhe ao redor com o mouse e use `Espaço`
   e `Ctrl` para subir e descer. Aproxime-se da frente da sala.
2. **Tela retrátil (escala).** Aperte `T`. A lona desce em cerca de 2,5 s, acelerando no começo e freando no fim.
   Aperte `T` de novo no meio do curso: ela inverte o sentido sem saltar.
3. **Projetor.** Com a tela aberta, aperte `P`. A lente acende e o slide aparece na lona. Use `→` e `←` para trocar
   entre os três slides. Repare que a imagem é um pouco mais larga embaixo (efeito keystone), como num projetor real.
4. **Projeção sem a tela.** Com o projetor ligado, aperte `T` para recolher a tela. O slide passa a cair no quadro e na
   parede, porque a projeção funciona como uma luz e atinge o que estiver no caminho.
5. **Ventilador (rotação).** Vire para a parede esquerda e aperte `V`. O rotor acelera até a velocidade máxima. Use
   `Shift` + `V` para trocar entre 2, 3 e 4 voltas/s e `V` de novo para desligar: ele desacelera até parar.
6. **Luz da sala.** Aperte `L`. As luminárias apagam, os tubos param de brilhar e a tecla do interruptor, na parede
   direita, bascula. A sala escurece, mas continua visível pela luz de preenchimento. Aperte `L` para acender.
7. **Materiais.** Aproxime-se da carteira e da mesa do professor e mova a câmera: o tubo metálico tem um reflexo forte e
   concentrado, o plástico tem um brilho médio e a madeira quase não reflete.
8. **Inspeção.** Aperte `2` a `9` e `0`, ou percorra com `[` e `]`, para ver cada objeto isolado. Aperte `1` para voltar
   à sala completa.
9. **Saída.** `Esc` libera o cursor; `Esc` de novo, ou `Q`, fecha o programa.

| Transformação exigida | Onde ver | Tecla |
| --- | --- | --- |
| Translação | Movimento da câmera e posicionamento dos objetos | `W` `A` `S` `D` |
| Rotação | Hélices do ventilador girando em torno do próprio eixo | `V` |
| Escala | Lona da tela retrátil escalando em Y a partir do topo | `T` |

## Técnicas implementadas

| Técnica | Como foi feita |
| --- | --- |
| Modelagem procedural | Primitivas paramétricas (caixa, cilindro, plano) e meshes montados vértice a vértice, com posição, normal e UV |
| Mesh reutilizável | Uma estrutura única com VAO/VBO/EBO; carteiras, cadeiras e luminárias repetidas compartilham os dados na GPU |
| Grafo de cena | Cada peça guarda `T · R · S` relativo ao pai e herda a matriz dele |
| Matriz das normais | `transpose(inverse(mat3(model)))` por peça, para a luz ficar correta com escala não uniforme |
| Câmera em primeira pessoa | Yaw e pitch pelo mouse (pitch limitado a ±89°), `lookAt` e perspectiva de 60°, movimento por `deltaTime` |
| Iluminação | Blinn-Phong por fragmento: ambiente, luz direcional de preenchimento, 4 luzes pontuais com atenuação e emissiva |
| Materiais | Difusa, textura opcional com tiling, especular, brilho e emissiva; presets metal, plástico, madeira e fosco |
| Texturas | stb_image com flip vertical, mipmaps, filtragem trilinear e `GL_REPEAT`; textura multiplicada pela cor do material |
| Textura projetiva | O projetor é uma segunda câmera, com view e projeção próprias; o slide cai onde o feixe bater |
| Animações | Baseadas em `deltaTime`: smoothstep na tela, aceleração e frenagem no ventilador |

Fórmulas, valores e decisões estão em [docs/ILUMINACAO.md](docs/ILUMINACAO.md) e
[docs/ARQUITETURA.md](docs/ARQUITETURA.md). Os trechos de escala, rotação e projeção estão marcados com `AB2 ·` no
código; `git grep -n "AB2 ·"` lista todos.

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
espaço, com o pivô na origem, e só a `scene/` decide onde cada um fica e qual tecla aciona cada interação.

## Documentação

| Documento | Conteúdo |
| --- | --- |
| [docs/relatorio-tecnico.md](docs/relatorio-tecnico.md) ([PDF](docs/relatorio-tecnico.pdf)) | Relatório técnico da entrega: todas as técnicas implementadas |
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

## Créditos e licenças

- Todas as texturas e os três slides foram gerados pela equipe com scripts próprios. A origem de cada imagem está em
  [assets/textures/README.md](assets/textures/README.md).
- As bibliotecas em `lib/` mantêm as próprias licenças: GLFW (zlib), GLAD (código gerado em CC0/WTFPL e Apache-2.0),
  GLM (MIT) e stb_image (MIT ou domínio público). Versões em [lib/README.md](lib/README.md).
- Nenhum modelo 3D foi importado: toda a geometria da cena é gerada no código.
