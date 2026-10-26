# Convenções de modelagem e cena

Regras que **todos** seguem ao modelar e posicionar objetos, para as peças de pessoas diferentes se encaixarem na mesma
sala sem retrabalho na Sprint 2. Onde cada arquivo fica: [ARQUITETURA.md](ARQUITETURA.md). Nomes, formatação e Git:
[DEV-CONVENCOES.md](DEV-CONVENCOES.md).

## Sumário

1. [Unidades e eixos](#1-unidades-e-eixos)
2. [Pivô e frente do objeto](#2-pivô-e-frente-do-objeto)
3. [Vértices, normais e UV](#3-vértices-normais-e-uv)
4. [Modelagem no código](#4-modelagem-no-código)
5. [Hierarquia e transformações](#5-hierarquia-e-transformações)
6. [Estilo comum de carteira e cadeira](#6-estilo-comum-de-carteira-e-cadeira)
7. [Medidas de referência](#7-medidas-de-referência)
8. [Atalhos de teclado](#8-atalhos-de-teclado)
9. [Texturas](#9-texturas)

---

## 1. Unidades e eixos

- **1 unidade = 1 metro.** Modele no tamanho real: uma carteira tem 0,75 de altura, não 75 nem 1.
- **Eixo Y para cima**, mão direita: +X à direita, +Z na direção de quem olha para a tela.
- **Mundo (a sala):** origem no **centro do piso**. A parede do quadro é a de **-Z**: os alunos sentados olham para -Z,
  que é a direção "para frente" padrão do OpenGL.
- Ângulos em **graus** na interface (`camera`, parâmetros de objeto) e convertidos com `glm::radians` na hora de montar
  a matriz.

## 2. Pivô e frente do objeto

Cada objeto é modelado no **próprio espaço**, com o pivô na origem:

- **Padrão: pivô na base**, no chão, centrado em X e Z (`y = 0` é onde o objeto toca o piso). É o que as primitivas
  já fazem, e posicionar fica trivial: `translate(x, 0, z)`.
- **Objetos fixos na parede ou no teto** (quadro, ventilador, interruptor, lâmpada, projetor): pivô no **ponto de
  fixação** (centro da face encostada na parede, ou topo encostado no teto).
- **Peças animadas: pivô no ponto da transformação.** Centro das hélices do ventilador (rotação), **topo** da tela
  retrátil (escala em Y que recolhe para cima). Use um `SceneNode` sem mesh como articulação (exemplos em
  `include/gfx/scene_node.hpp`).
- **Frente do objeto para +Z** no espaço local (o lado que o usuário vê: assento da cadeira, face do quadro, lente do
  projetor). Assim, posicionar na sala é só translação + rotação em Y.

## 3. Vértices, normais e UV

O formato é único e está em `include/gfx/vertex.hpp`: **todo vértice tem posição, normal e UV**, nessa ordem
(locations 0, 1 e 2 no shader).

- **Normal com comprimento 1, apontando para fora.** Em faces planas, cada canto tem o próprio vértice com a normal da
  face (cubo = 24 vértices). Em superfícies curvas (lateral do cilindro), vértices vizinhos compartilham a normal.
- **Triângulos no sentido anti-horário vistos de fora** (regra da mão direita: o polegar aponta para o lado da normal).
- **UV com (0, 0) no canto inferior esquerdo** da textura, u para a direita e v para cima.
- **Materiais contínuos repetem, imagens únicas esticam:** use `textureSize > 0` nas primitivas para madeira, metal e piso
  (a textura não estica em peças finas) e `textureSize = 0` para o quadro, o slide e etiquetas.
- Nada de vértice sem normal ou sem UV, mesmo que a peça seja só cor: a iluminação (CG-71) e os materiais (CG-72)
  contam com os três atributos.

## 4. Modelagem no código

- Os 9 objetos são **modelados no código**, com as primitivas de `include/gfx/primitives.hpp` (`box`, `cylinder`, `plane`)
  ou com um `MeshData` montado à mão no `.cpp` do objeto.
- **Modelo feito no Blender (ou baixado, OBJ/FBX) conta como importado**, mesmo que a equipe tenha modelado. O relatório
  lista os importados e eles têm limite de **30%** (CG-80). Hoje o projeto não tem carregador de modelo; incluir um
  é decisão da equipe e entra como biblioteca em `lib/`.
- Gere as peças **no tamanho final** em vez de escalar um cubo de 1 m com `glm::scale`. A UV acompanha a medida e as
  normais saem certas. `glm::scale` fica para o que é **animado** (a tela retrátil).
- Primitiva que serviria para mais de um objeto (cone, esfera, tubo curvo) vai para `primitives`, não para o objeto.

## 5. Hierarquia e transformações

- Objeto de várias peças = árvore de `SceneNode`. A `transform` de cada peça é **relativa ao pai**:
  `model_peça = model_pai · transform`.
- Monte cada transformação na ordem **T · R · S** (translação, depois rotação, depois escala no código GLM:
  `translate`, `rotate`, `scale` aplicados nessa sequência sobre a mesma matriz). Lido da direita para a esquerda no
  vértice: escala primeiro, depois gira, depois move.
- Animação muda **só a `transform` do nó de articulação** (o pivô), nunca os vértices.
- Escala zero torna a normal matrix singular: para esconder uma peça (tela totalmente recolhida) use `visible = false`.
- Movimento e animação usam `deltaTime` (segundos), nunca "um passo por frame".

## 6. Estilo comum de carteira e cadeira

Carteira (Riquelme, CG-62) e cadeira (Victor, CG-60) ficam lado a lado em todas as fileiras e precisam parecer do mesmo
conjunto:

- **Estrutura em tubo metálico:** cilindros de **raio 0,011 m** (tubo de 22 mm), **16 segmentos**, mesma cor/material
  de metal. Travessas são o mesmo cilindro girado 90°.
- **Tampo e assento/encosto em placa:** caixas finas (**0,018 m**, MDF) com o mesmo material de madeira ou laminado.
- As medidas e materiais compartilhados ficam num header só, `include/objects/furniture_style.hpp`, criado por quem
  chegar primeiro e usado pelos dois objetos:

```cpp
namespace objects::furniture {

constexpr float kTubeRadius = 0.011f;  // tubo de aço de 22 mm
constexpr int kTubeSegments = 16;
constexpr float kBoardThickness = 0.018f;  // MDF do tampo, assento e encosto

}  // namespace objects::furniture
```

Quando o sistema de materiais (CG-72) existir, os materiais `metal` e `madeira` de mobília entram no mesmo header.

## 7. Medidas de referência

Ordem de grandeza para os objetos combinarem entre si. Não são obrigatórias; o que é obrigatório é estar em metros
reais.

| Objeto | Medidas aproximadas |
| --- | --- |
| Sala | 8 m (X) × 3 m (Y) × 7 m (Z), origem no centro do piso |
| Carteira | tampo 0,60 × 0,45 m, altura 0,75 m |
| Cadeira | assento 0,40 × 0,40 m a 0,45 m do chão, encosto até 0,85 m |
| Mesa do professor | 1,20 × 0,60 m, altura 0,75 m |
| Quadro | 3,0 × 1,2 m, borda inferior a 0,9 m do chão, na parede -Z |
| Tela retrátil | 2,0 × 1,5 m aberta, caixa no alto da parede -Z |
| Projetor | 0,30 × 0,10 × 0,25 m, preso ao teto |
| Ventilador de parede | hélice com 0,50 m de diâmetro, a ~2,4 m do chão |
| Lâmpada | tubular de 1,20 m no teto |
| Interruptor | espelho 0,08 × 0,12 m, a 1,1 m do chão |
| Lixeira | 0,30 m de diâmetro, 0,40 m de altura |

## 8. Atalhos de teclado

Para duas pessoas não usarem a mesma tecla. **Quem cria ou muda um atalho atualiza esta tabela e o README (§ Comandos)
no mesmo PR.** "Reservada" é a sugestão para a task; o responsável pode trocar, desde que atualize aqui.

| Tecla | Ação | Onde é tratada | Task | Estado |
| --- | --- | --- | --- | --- |
| Clique esquerdo | Captura o cursor e ativa a navegação | `main.cpp` | CG-54 | em uso |
| `Esc` | Libera o cursor; com o cursor livre, fecha | `main.cpp` | CG-54 | em uso |
| `Q` | Fecha | `main.cpp` | CG-54 | em uso |
| `W` `A` `S` `D` + mouse | Anda e olha | `main.cpp` | CG-55, CG-68 | em uso |
| `Espaço` / `Ctrl` esquerdo | Sobe / desce | `main.cpp` | CG-55 | em uso |
| `1`, `2`–`9`, `0` | Inspeção: sala inteira / cada alvo, na ordem de registro em `Classroom` (`0` é o nono alvo). Hoje `2` = cubo, `3` = objeto articulado (cena de exemplo), `4` = ventilador, `5` = lâmpada, `6` = interruptor, `7` = mesa do professor, `8` = tela retrátil, `9` = projetor e `0` = lixeira; com os objetos reais, a ordem da [tabela de objetos](ARQUITETURA.md#objetos-da-sala). Cabem 9 alvos | `scene/classroom` | CG-55, CG-87, CG-59, CG-63, CG-64, CG-66, CG-65, CG-67 | em uso |
| `T` | Tela retrátil: abre / recolhe | `scene/classroom` | CG-75 | reservada |
| `V` | Ventilador: liga / desliga | `scene/classroom` | CG-76 | reservada |
| `P` | Projetor: liga / desliga | `scene/classroom` | CG-77 | reservada |
| `←` / `→` | Slide anterior / próximo | `scene/classroom` | CG-77 (opcional) | reservada |
| `L` | Luz: liga / desliga | `scene/classroom` | CG-73 | reservada |

## 9. Texturas

Regras em [assets/textures/README.md](../assets/textures/README.md): toda imagem entra na tabela de licenças no mesmo
commit, nome em `snake_case` sem acento, PNG/JPG de 512 ou 1024 px, *seamless* quando repete.
