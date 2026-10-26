# Arquitetura

Como o código está organizado, quem depende de quem e **onde colocar cada coisa nova**. As regras de escrita
(nomes, formatação, Git) estão em [DEV-CONVENCOES.md](DEV-CONVENCOES.md) e as de modelagem (unidades, pivô,
vértice) em [CONVENCOES.md](CONVENCOES.md).

> [!NOTE]
> O `gfx/material` ainda não existe: nasce com a CG-72. Por enquanto `scene/classroom` monta a cena de exemplo da
> estrutura de mesh (piso, cubo e objeto articulado) e os objetos da Sprint 1 já modelados, em posição provisória até a
> sala (CG-69).

## Sumário

1. [Visão geral](#1-visão-geral)
2. [Camadas e regras de dependência](#2-camadas-e-regras-de-dependência)
3. [Pastas e módulos](#3-pastas-e-módulos)
4. [Onde adicionar uma feature](#4-onde-adicionar-uma-feature)
5. [Revisão e refactor](#5-revisão-e-refactor)
6. [Detalhes dos módulos](#6-detalhes-dos-módulos)

---

## 1. Visão geral

Um único executável, C++17 + OpenGL 3.3 core. A cena é uma árvore de `SceneNode` (peças com transformação relativa ao
pai) e cada objeto da sala é um módulo que cria os próprios meshes e pendura suas peças nessa árvore.

Um frame do loop principal (`src/main.cpp`):

```
input.beginFrame(); glfwPollEvents();        core/input     eventos do frame
deltaTime = ...                              main.cpp       segundos, limitado a [0, 0.1]
update(...)                                  main.cpp       atalhos globais (Q, Esc, captura do cursor)
  ├─ classroom.update(input, deltaTime)      scene/         teclas das interações -> métodos dos objetos
  │    └─ fan.update(deltaTime) ...          objects/       animação de cada objeto
  ├─ classroom.handleInspection(input, cam)  scene/         teclas 1-9: isola um alvo e reenquadra a câmera
  └─ camera.rotate(...) / camera.move(...)   core/camera    navegação, só com o cursor capturado
renderer::draw(camera, root, texture)        gfx/renderer   shader, matrizes view/projeção, desenha a árvore
glfwSwapBuffers(window);
```

## 2. Camadas e regras de dependência

```
main.cpp ──> scene/ ──> objects/ ──> gfx/ ──> core/
                 └──────────────────────────> core/  (Input, Camera)
```

Cada camada só inclui headers das camadas **à direita**. Na prática:

| Camada | Pode usar | **Não** pode |
| --- | --- | --- |
| `core/` | GLFW (só o `Input`), GLM, biblioteca padrão | OpenGL, meshes, objetos |
| `gfx/` | OpenGL (GLAD), GLM, stb_image, `core/` | conhecer objetos da sala, ler teclado |
| `objects/` | `gfx/` (Mesh, SceneNode, primitives, Texture, Material) | `Input`, GLFW, outros objetos, a posição na sala |
| `scene/` | tudo acima | chamar OpenGL direto |
| `main.cpp` | `core/`, `scene/`, `gfx/renderer` | lógica de objeto ou de interação |

Por que assim:

- **`objects/` não lê teclado.** O objeto expõe métodos (`toggle()`, `update(dt)`), quem decide a tecla é `scene/`.
  Assim a tabela de atalhos fica num lugar só e trocar uma tecla não mexe no objeto.
- **`objects/` não sabe onde está na sala.** O objeto é modelado no próprio espaço (pivô na origem) e quem posiciona é
  `scene/classroom`. A mesma cadeira serve para todas as fileiras.
- **`gfx/` não conhece a sala.** O renderer desenha qualquer árvore de `SceneNode`; não precisa ser editado para cada
  objeto novo.
- **Cada pessoa mexe nos próprios arquivos.** Na Sprint 1 cada membro cria os arquivos dos seus 3 objetos e adiciona
  uma ou duas linhas em `scene/classroom.cpp`. O conflito de merge fica restrito a essas linhas.

## 3. Pastas e módulos

```
include/                         src/
├── core/                        ├── core/
│   ├── input.hpp                │   ├── input.cpp          teclado, mouse, foco (GLFW)
│   └── camera.hpp               │   └── camera.cpp         câmera livre, view/projeção (sem GLFW)
├── gfx/                         ├── gfx/
│   ├── vertex.hpp               │   ├── vertex.cpp         formato de vértice (posição, normal, UV)
│   ├── mesh.hpp                 │   ├── mesh.cpp           VAO/VBO/EBO
│   ├── primitives.hpp           │   ├── primitives.cpp     box, cylinder, plane
│   ├── scene_node.hpp           │   ├── scene_node.cpp     hierarquia de peças (model = pai · transform)
│   ├── shader.hpp               │   ├── shader.cpp         programa GLSL, set* de uniforms
│   ├── texture.hpp              │   ├── texture.cpp        PNG/JPG -> textura com mipmaps
│   ├── material.hpp             │   ├── material.cpp       (a criar, CG-72) cor, textura, brilho por peça
│   └── renderer.hpp             │   └── renderer.cpp       estado do GL e desenho do frame
├── objects/                     ├── objects/               (a criar na Sprint 1)
│   ├── furniture_style.hpp      │   │                      medidas e materiais comuns de carteira e cadeira
│   ├── fan.hpp ...              │   ├── fan.cpp ...        um módulo por objeto (tabela abaixo)
│   └── room.hpp                 │   └── room.cpp           paredes, piso e teto
└── scene/                       ├── scene/
    └── classroom.hpp            │   └── classroom.cpp      monta a sala, inspeção, teclas das interações
                                 └── main.cpp               janela, contexto, loop, atalhos globais
shaders/                         basic.vert, basic.frag (e os da iluminação, CG-71)
assets/textures/                 imagens + tabela de licenças (README.md)
```

O include usa o caminho a partir de `include/`: `#include "gfx/mesh.hpp"`, `#include "objects/fan.hpp"`.
O `CMakeLists.txt` acha os `.cpp` de subpastas sozinho. Depois de criar um arquivo, rode `cmake --preset <preset>` de
novo se ele não entrar no build.

### Objetos da sala

Nomes de classe e arquivo seguem [DEV-CONVENCOES.md § Nomes](DEV-CONVENCOES.md#2-nomes-cases). Cada objeto fica em
`include/objects/<arquivo>.hpp` + `src/objects/<arquivo>.cpp`, dentro de `namespace objects`.

| Objeto | Classe | Arquivo | Task | Responsável | Animado |
| --- | --- | --- | --- | --- | --- |
| Ventilador de parede | `Fan` | `fan` | CG-59 | Victor | sim (hélices, CG-76) |
| Cadeira | `Chair` | `chair` | CG-60 | Victor | não |
| Quadro | `Board` | `board` | CG-61 | Victor | não |
| Carteira | `StudentDesk` | `student_desk` | CG-62 | Riquelme | não |
| Lâmpada + interruptor | `Lamp`, `LightSwitch` | `lamp` | CG-63 | Riquelme | sim (luz, CG-73) |
| Mesa do professor | `TeacherDesk` | `teacher_desk` | CG-64 | Riquelme | não |
| Projetor | `Projector` | `projector` | CG-65 | Erasmo | sim (liga/desliga, CG-77) |
| Tela retrátil | `ProjectionScreen` | `projection_screen` | CG-66 | Erasmo | sim (escala, CG-75) |
| Lixeira | `TrashCan` | `trash_can` | CG-67 | Erasmo | não |
| Sala (paredes, piso, teto) | `Room` | `room` | CG-69 | Victor | não |

## 4. Onde adicionar uma feature

| Quero... | Onde | Também atualizar |
| --- | --- | --- |
| Modelar um objeto | `objects/<nome>.hpp/.cpp`, no [padrão abaixo](#padrão-de-um-objeto) | tabela acima; `scene/classroom.cpp` (posição + alvo de inspeção) |
| Posicionar um objeto na sala | `scene/classroom.cpp` | — |
| Uma interação (tecla que muda um objeto) | método no objeto (`toggle()`, `setOpen()`) + tecla em `Classroom::update` | [tabela de atalhos](CONVENCOES.md#8-atalhos-de-teclado) e README § Comandos |
| Animação contínua (hélice girando) | `update(float deltaTime)` do objeto, chamado por `Classroom::update` | — |
| Uma primitiva nova (cone, esfera, tubo curvo) | `gfx/primitives.hpp/.cpp` | comentário no header com pivô e UV |
| Peça que nenhuma primitiva faz | `MeshData` montado no `.cpp` do objeto (vértices + índices) | — |
| Material / iluminação | `gfx/material.*`, `shaders/` e `gfx/renderer.cpp` (CG-71, CG-72) | `SceneNode` ganha `material` |
| Uma textura | `assets/textures/` | tabela de licenças em `assets/textures/README.md` |
| Um uniform novo | shader em `shaders/` + `set*` em quem desenha | comentário no shader dizendo quem define |
| Atalho global (sair, capturar cursor) | `update` em `src/main.cpp` | tabela de atalhos e README |
| Utilitário sem OpenGL (matemática, tempo) | `core/` | — |
| Utilitário de OpenGL | `gfx/` | — |
| Biblioteca de terceiros | `lib/` + `lib/CMakeLists.txt` | `lib/README.md` (versão, licença, SHA-256) |

### Padrão de um objeto

O construtor cria os meshes (uma vez). `addTo` pendura uma instância na árvore da cena e devolve a raiz dela.
O objeto é dono dos `Mesh`; as peças só apontam para eles, então o objeto precisa viver enquanto a cena for desenhada
(em `Classroom` ele é um membro, criado antes e destruído depois da árvore).

Objeto estático, várias instâncias (cadeira):

```cpp
// include/objects/chair.hpp
#ifndef AULA_OPENGL_OBJECTS_CHAIR_HPP
#define AULA_OPENGL_OBJECTS_CHAIR_HPP

#include <glm/glm.hpp>

#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"

namespace objects {

// Cadeira (CG-60). Pivô no chão, no centro das 4 pernas; frente para +Z.
class Chair {
public:
    Chair();

    // Pode ser chamada várias vezes (uma por fileira): todas as instâncias usam os mesmos meshes.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement) const;

private:
    Mesh seat_;
    Mesh backrest_;
    Mesh leg_;  // usado pelas 4 pernas
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_CHAIR_HPP
```

Objeto animado, uma instância (ventilador): guarda ponteiros para os nós que mexem e expõe métodos para `scene/`.

```cpp
namespace objects {

// Ventilador de parede (CG-59). Pivô no ponto de fixação na parede; frente (hélices) para +Z.
class Fan {
public:
    Fan();

    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement);  // uma instância só
    void toggle();                  // liga/desliga (CG-76)
    void update(float deltaTime);   // gira o rotor se estiver ligado

private:
    Mesh body_;
    Mesh blade_;
    SceneNode* rotor_ = nullptr;    // pivô no centro das hélices, criado em addTo
    float angle_ = 0.0f;            // radianos
    bool on_ = false;
};

}  // namespace objects
```

```cpp
// src/objects/fan.cpp (trecho; kRotorCenter e kSpeed são constantes do namespace anônimo do arquivo)
Fan::Fan() : body_(primitives::cylinder(0.08f, 0.25f)), blade_(primitives::box({0.06f, 0.01f, 0.20f})) {}

SceneNode& Fan::addTo(SceneNode& parent, const glm::mat4& placement) {
    SceneNode& root = parent.addChild(&body_, placement);
    rotor_ = &root.addChild(nullptr, glm::translate(glm::mat4(1.0f), kRotorCenter));
    // ... pás como filhas de rotor_
    return root;
}

void Fan::update(float deltaTime) {
    if (!on_ || rotor_ == nullptr) {
        return;
    }
    angle_ = std::remainder(angle_ + kSpeed * deltaTime, glm::two_pi<float>());
    rotor_->transform = glm::rotate(glm::translate(glm::mat4(1.0f), kRotorCenter), angle_, {0.0f, 0.0f, 1.0f});
}
```

E em `scene/classroom` só entram o membro, a posição, o alvo de inspeção e a tecla:

```cpp
// include/scene/classroom.hpp, na seção private (objetos antes de root_: a árvore aponta para os meshes deles)
objects::Fan fan_;
```

```cpp
// src/scene/classroom.cpp, no construtor
// Na parede x = +4 m, a 2,4 m do chão, com a frente (+Z do objeto) girada para -X (para dentro da sala).
glm::mat4 fanPlacement = glm::translate(glm::mat4(1.0f), {3.9f, 2.4f, -1.0f});
fanPlacement = glm::rotate(fanPlacement, glm::radians(-90.0f), {0.0f, 1.0f, 0.0f});
const SceneNode& fan = fan_.addTo(root_, fanPlacement);
targets_.push_back({"ventilador", &fan, {2.5f, 2.0f, 0.0f}, {3.9f, 2.4f, -1.0f}});  // tecla = posição na lista + 2

// src/scene/classroom.cpp, em Classroom::update
if (input.keyPressed(GLFW_KEY_V)) {
    fan_.toggle();
}
fan_.update(deltaTime);
```

O alvo de inspeção precisa ser um **filho direto de `root_`** (é o que `addTo(root_, ...)` devolve): ao isolar um alvo, a
inspeção esconde os outros filhos da raiz.

## 5. Revisão e refactor

Revisão feita na CG-53, sobre o `dev` depois do merge da CG-55 (PR #6). R1 a R3 foram feitos na **CG-87**; o R4 fica
para a CG-72.

### O que está bom e fica

- Wrappers RAII de OpenGL (`Shader`, `Mesh`, `Texture`) com cópia proibida e `isValid()` em vez de exceção.
- Primitivas no tamanho real com UV em metros (`textureSize`): textura não estica e as normais não dependem de escala.
- `SceneNode` com pivô por nó vazio: cobre o ventilador e a tela retrátil sem mudanças.
- `Camera` sem GLFW e `Input` sem atalhos: dá para usar os dois em qualquer camada.
- CMake com glob: arquivo novo não exige editar o build.

### Problemas

| # | Problema | Por que importa agora |
| --- | --- | --- |
| P1 | `src/renderer.cpp` mistura o pipeline (shader, viewport, uniforms) com o **conteúdo** da cena: meshes, nós, textura de teste e enquadramentos da inspeção, tudo em globais. | Na Sprint 1, 3 pessoas adicionariam 9 objetos no mesmo arquivo, e na Sprint 2 as 3 interações também. Conflito garantido. |
| P2 | `renderer::InspectionMode` é um enum fixo da cena de exemplo (`Scene`, `Cube`, `Articulated`) e as teclas `1`–`3` estão em `main.cpp`. | Cada objeto novo exigiria mexer no enum, no `switch` do renderer e no `main.cpp`. |
| P3 | Interações e animação não têm lugar: o único `update` é o de `main.cpp`. | CG-73, CG-75, CG-76 e CG-77 colocariam lógica de objeto no `main.cpp`. |
| P4 | Uma textura e um `uColor` para a cena inteira, definidos no `renderer::draw`. | Não há onde pendurar material por peça (CG-72) nem textura por objeto (CG-70, CG-74, CG-78). |
| P5 | `include/` e `src/` planos. | Com 10 objetos + sala, seriam mais de 20 headers misturados sem fronteira entre motor e conteúdo. |
| P6 | A normal matrix é calculada no vertex shader (`inverse` por vértice). | Irrelevante para o tamanho da sala; vale passar para a CPU (`setMat3`, já existe) quando a CG-71 mexer no shader. Opcional. |

### Plano

| Passo | O que muda | Resolve | Estado |
| --- | --- | --- | --- |
| R1 | `git mv` dos módulos para `core/` (input, camera) e `gfx/` (vertex, mesh, primitives, scene_node, shader, texture, renderer); includes viram `"gfx/mesh.hpp"` e os guards ganham a pasta (`AULA_OPENGL_GFX_MESH_HPP`). | P5 | feito (CG-87) |
| R2 | Cria `scene/classroom` com a cena de exemplo atual (dona dos meshes, da textura e da árvore). O renderer passa a só desenhar: `renderer::draw(const Camera&, const SceneNode& root, const Texture* texture)`, sem globais de conteúdo. A textura única é provisória até o R4. | P1 | feito (CG-87) |
| R3 | Inspeção vira uma lista de alvos `{nome, nó, enquadramento}` em `Classroom`; teclas `1` (sala inteira) e `2`–`9` (cada alvo), mantendo `1`/`2`/`3` da CG-55. `update` do `main.cpp` chama `classroom.update(input, deltaTime)` (interações e animação) e `classroom.handleInspection(input, camera)`. | P2, P3 | feito (CG-87) |
| R4 | Junto com a CG-72: `Material` (cor, textura, brilho) em `gfx/`, ponteiro `const Material* material` no `SceneNode`, aplicado no `SceneNode::draw`. Remove o parâmetro `texture` do `renderer::draw` e a textura de `Classroom`. | P4 | pendente |

## 6. Detalhes dos módulos

### Input e atualização por frame (CG-54)

`Input` (`include/core/input.hpp`, `src/core/input.cpp`) acompanha uma janela GLFW. O loop chama
`input.beginFrame()` antes de `glfwPollEvents()` e depois entrega o estado a `update` em `src/main.cpp`.
Todos os módulos podem consultar o mesmo frame, sem consumir os eventos:

| Consulta | Resultado |
| --- | --- |
| `keyDown(GLFW_KEY_...)` / `mouseDown(GLFW_MOUSE_BUTTON_...)` | Estado contínuo de tecla/botão |
| `keyPressed(...)` / `mousePressed(...)` | Pressionamento neste frame, sem repetição automática; preservado mesmo se houver soltura no mesmo poll |
| `mouseDelta()` | Movimento relativo acumulado no frame com cursor capturado; `x` para a direita, `y` para baixo |
| `setCursorCaptured(bool)` / `isCursorCaptured()` | Controle/consulta da captura (`GLFW_CURSOR_DISABLED`) |
| `isFocused()` / `focusChanged()` | Foco atual e ocorrência de transição neste frame |

As consultas de teclas/botões inválidos retornam `false`. Os eventos de pressionamento e o delta do mouse
duram até o próximo `beginFrame()`. O primeiro evento de posição após cada captura apenas estabelece a
referência, evitando saltos. A perda ou recuperação de foco limpa os estados e o movimento pendente;
o cursor volta livre e exige novo clique para capturar. Teclas mantidas antes da perda de foco exigem
novo pressionamento ao retornar.

Os atalhos ficam em `update`: clique esquerdo captura, `Esc` libera a captura ou fecha se o cursor já estiver
livre, e `Q` fecha sempre. `Input` não conhece esses atalhos. Ele usa o user pointer e os callbacks de teclado,
botões, posição e foco da janela; outros módulos devem usar sua interface sem substituir esses callbacks.
O callback de framebuffer continua independente. O objeto deve ser destruído antes da janela.

`update` recebe `deltaTime` em **segundos**, medido com `glfwGetTime()` após o poll e antes do desenho.
O primeiro frame, frames sem foco e frames com transição de foco recebem zero. Nos demais frames,
o intervalo real é limitado a `[0, 0.1]` segundo: o excesso de uma pausa longa é descartado, sem acumular
recuperação nos próximos frames. Não há passo fixo nem cálculo baseado em FPS.

### Câmera livre e inspeção (CG-55)

`Camera` (`include/core/camera.hpp`, `src/core/camera.cpp`) guarda posição e orientação e calcula view/projeção
sem depender do GLFW. `update` conecta o `Input` à câmera; `renderer::draw` usa suas matrizes
com a proporção atual do framebuffer e suspende o desenho quando a largura ou altura é zero.
`movementSpeed` (padrão **2 unidades/s**) e `mouseSensitivity` (**0,1 grau/pixel**) são configuráveis
na câmera. O movimento combinado é normalizado e usa `deltaTime`; o delta do mouse **não** usa
`deltaTime`. Não há colisões, gravidade ou limite de deslocamento.

A inspeção fica em `scene::Classroom` (CG-87): `1` mostra a sala inteira e `2` a `9` isolam cada alvo registrado, na
ordem da lista (hoje `2` = cubo, `3` = objeto articulado, `4` = ventilador, `5` = tela retrátil, `6` = projetor e `7` = lixeira). Funciona com a janela em foco, mesmo com o cursor livre, e
restaura um enquadramento para cada alvo; tecla sem alvo é ignorada. Depois a câmera continua livre. A perda de foco libera o cursor e exige
novo clique para retomar a navegação; com o cursor livre não há movimento nem rotação pelo mouse.

### Projetor e tela retrátil (CG-65, CG-66)

A projeção precisa cair na tela, mas nenhum dos dois objetos conhece o outro nem a própria posição na sala
(camada `objects/`). Quem alinha é `scene/classroom`, com mudanças de espaço:

```cpp
// centro da lona: espaço da tela -> mundo -> espaço do projetor
const glm::vec3 screenCenter = toWorld(screenPlacement, screen_.canvasCenter());
projector_.aimAt(toWorld(glm::inverse(projectorPlacement), screenCenter));
```

`Projector::aimAt` gira só a articulação da cabeça em X (inclinação), calculando o ângulo para o **eixo da lente**,
que fica abaixo do pivô, cruzar o alvo; o giro em Y é do placement. Com as posições atuais (provisórias até a CG-69) a
inclinação é de ~12,5° e a lente fica a ~2,9 m da lona: throw ratio de ~1,45 para a imagem de 2,0 m. Como o
alinhamento é calculado, mover a tela ou o projetor não exige ajustar ângulo nenhum. `lensCenter()` e
`lensDirection()` dão a origem e a direção da projeção para a CG-77. Com o projetor inclinado e a tela vertical, a
imagem real seria um trapézio (keystone): a CG-77 pode desenhar o slide direto na lona.

### Mesh, primitivas, SceneNode, Shader e Texture (CG-56 a CG-58)

Documentados nos próprios headers em `include/gfx/`, com exemplos de uso: `vertex.hpp` (formato de vértice), `mesh.hpp` (VAO/VBO/EBO),
`primitives.hpp` (pivô e UV de cada primitiva), `scene_node.hpp` (hierarquia, pivô, ventilador e tela retrátil),
`shader.hpp` e `texture.hpp` (wrap, filtro e unidades).
