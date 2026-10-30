# Iluminação

Como a sala é iluminada (CG-71): o modelo de luz, as luzes da cena e a matriz das normais. Escrito para virar a
seção de iluminação do relatório técnico (CG-80). Onde cada coisa fica no código: [§ 6](#6-onde-está-no-código).

## Sumário

1. [Modelo de Blinn-Phong](#1-modelo-de-blinn-phong)
2. [Luzes pontuais e atenuação](#2-luzes-pontuais-e-atenuação)
3. [Luz ambiente e luz da janela](#3-luz-ambiente-e-luz-da-janela)
4. [Matriz das normais](#4-matriz-das-normais)
5. [Valores escolhidos](#5-valores-escolhidos)
6. [Onde está no código](#6-onde-está-no-código)

---

## 1. Modelo de Blinn-Phong

A cor de cada fragmento (pixel candidato) é calculada no fragment shader, por pixel e não por vértice, somando três
componentes. Todos os vetores são unitários, então o produto escalar `·` é o cosseno do ângulo entre eles:

| Vetor | O que é |
| --- | --- |
| **N** | normal da superfície no fragmento, no espaço do mundo |
| **L** | direção do fragmento **para** a luz |
| **V** | direção do fragmento para a câmera (`uViewPos - posição`) |
| **H** | *half-vector*: `normalize(L + V)`, o meio do caminho entre a luz e o olho |

```
cor = ambiente · albedo
    + Σ luzes  atenuação · corDaLuz · ( max(N·L, 0) · albedo  +  max(N·H, 0)^brilho · especular )
```

`albedo` é a cor difusa da superfície (hoje, a textura), `especular` é a cor do reflexo e `brilho` (*shininess*) o
expoente que controla o tamanho do reflexo.

- **Ambiente:** uma constante. Representa a luz que rebate nas paredes e chega de todo lado, que o modelo local não
  calcula. Sem ela, toda face de costas para as luzes ficaria preta.
- **Difusa (Lambert), `N·L`:** uma superfície fosca espalha a luz igualmente para todos os lados, então o que importa
  é quanta luz ela **recebe**: o máximo com a luz batendo de frente (`N·L = 1`), zero com a luz rasante ou por trás.
  O `max(…, 0)` descarta a luz que viria por trás da face.
- **Especular, `(N·H)^brilho`:** o reflexo brilhante, que muda quando a câmera se move. Quando H coincide com N, a luz
  refletida vai direto para o olho. O expoente estreita o reflexo: 8 dá uma mancha larga (plástico fosco), 128 um
  ponto pequeno e forte (metal polido). Se `N·L ≤ 0`, o especular é zerado, para o reflexo não aparecer no lado da
  face que não recebe luz.

**Por que Blinn-Phong e não Phong.** O Phong original usa `(R·V)^brilho`, com `R = reflect(-L, N)`, o raio de luz
espelhado na superfície. Blinn troca R·V por N·H:

- **Não corta o reflexo:** quando o ângulo entre R e V passa de 90°, `R·V` fica negativo e o reflexo some de repente,
  com uma borda visível em superfícies de brilho baixo. Com a luz e o olho na frente da face, H também fica na
  frente e N·H não chega a zero: o reflexo enfraquece suave.
- **É mais próximo do real** em ângulos rasantes (o reflexo se alonga, como num piso encerado visto de longe).
- **É mais barato:** `normalize(L + V)` em vez de `reflect`.

Para o reflexo ter o mesmo tamanho, o expoente do Blinn costuma ser de 2 a 4 vezes o do Phong.

## 2. Luzes pontuais e atenuação

Cada uma das quatro luminárias de teto tem uma luz pontual: sai de um ponto para todos os lados, então L muda em cada
fragmento (`L = normalize(posiçãoDaLuz - posição)`). O ponto fica 1,5 cm abaixo da abertura da calha, no meio dos
dois tubos (`Lamp::lightOrigin()`), e não dentro do tubo: o modelo não tem sombra nem emissão, e com a luz no
centro do tubo as faces do próprio tubo e do refletor vistas de baixo ficariam de costas para ela (escuras).

A luz enfraquece com a distância `d` (em metros):

```
atenuação = 1 / (kc + kl·d + kq·d²)
```

- **kc (constante) = 1:** em `d = 0` o fator é 1, a luz nunca é amplificada.
- **kq (quadrático):** a lei física é o inverso do quadrado da distância (a mesma luz se espalha numa esfera de área
  `4πd²`).
- **kl (linear):** só o termo quadrático cai rápido demais perto da lâmpada e devagar demais longe dela, porque o
  modelo não tem a luz rebatida que, no mundo real, clareia o resto da sala. O termo linear suaviza essa curva.

Valores da sala (`kc = 1`, `kl = 0,14`, `kq = 0,07`), com as luminárias a ~2,9 m do piso:

| Ponto | Distância à luminária mais próxima | Atenuação |
| --- | --- | --- |
| Tampo de uma carteira | ~2,2 m | ~0,60 |
| Piso logo abaixo | ~2,9 m | ~0,50 |
| Centro do piso (entre as quatro) | ~3,6 m | ~0,41 (de cada uma) |
| Canto oposto da sala | ~8 m | ~0,15 |

Os termos linear e quadrático se igualam em `d = 2 m`: antes disso a queda é suave, depois a luz morre antes de
atravessar a sala. As quatro luzes **somam**: o centro do piso recebe ~0,41 de cada uma. Por isso a cor de cada
lâmpada é (0,55; 0,53; 0,50), pouco mais de metade do branco; com 1,0 o piso e as paredes estouravam para branco.

Uma luz com `enabled = false` não é enviada ao shader: o renderer monta o array só com as ligadas e envia a quantidade
em `uPointLightCount`. É o que a CG-73 (ligar a luz) vai usar.

## 3. Luz ambiente e luz da janela

- **Ambiente (0,25; 0,25; 0,26):** garante que nenhuma face fique preta. Uma face de costas para todas as luzes
  mostra 25% da própria cor. Mais que isso, a sala fica "lavada" (todas as faces com a mesma claridade, sem volume).
- **Janela (direcional):** luz de muito longe (o sol), com todos os raios paralelos: L é a mesma em toda a cena e não
  há atenuação. As janelas ficam na parede esquerda (-X), à esquerda de quem olha para o quadro, como se recomenda em
  sala de aula (a mão de quem escreve com a direita não faz sombra no caderno). A luz viaja para +X e desce
  (direção (1; -0,6; 0,15)), fraca e levemente azulada (0,16; 0,17; 0,19). Ela só **preenche**: dá volume às faces
  voltadas para a esquerda, que as lâmpadas iluminam de lado.

## 4. Matriz das normais

A normal é um vetor perpendicular à superfície, guardado no espaço do objeto. Para levá-la ao mundo, a primeira
ideia seria multiplicá-la pela mesma `model` dos vértices. Isso só funciona com rotação e escala **uniforme**.

Com escala **não uniforme**, a normal entorta. Exemplo (a lona da tela retrátil é escalada só em Y): uma face
inclinada a 45°, com normal (0; 0,71; 0,71), achatada por S = (1; 0,5; 1), fica mais deitada (mais perto da
horizontal), então a normal deveria apontar **mais para cima**. Mas `S · n = (0; 0,35; 0,71)` aponta mais para o
lado e deixa de ser perpendicular à face: a luz seria calculada com a normal errada. A inversa transposta dá
`(0; 1,41; 0,71)`, que normalizada é (0; 0,89; 0,45): mais para cima e perpendicular à face achatada.

A matriz certa é a **inversa transposta** da parte 3×3 da model:

```
normalMatrix = transpose(inverse(mat3(model)))
```

Por quê: uma normal precisa continuar perpendicular a qualquer tangente t da superfície, ou seja, `n·t = 0` depois
da transformação. A tangente é transformada como os vértices (`t' = M·t`). Procuramos G tal que `(G·n)·(M·t) = 0`:

```
(G·n)ᵀ (M·t) = nᵀ (Gᵀ M) t = 0   para todo t com nᵀt = 0   ⇒   Gᵀ M = I   ⇒   G = (M⁻¹)ᵀ
```

- `mat3(model)` descarta a translação: normal é direção, não ponto, e não deve ser movida.
- Com só rotação, a inversa transposta é a própria rotação (`R⁻¹ = Rᵀ`); com escala, ela divide em vez de
  multiplicar, que é o que corrige a inclinação.
- A matriz é calculada **na CPU**, uma vez por peça (`SceneNode::draw`), e enviada como `uNormalMatrix`. No vertex
  shader seria calculada uma vez por vértice, o mesmo resultado repetido milhares de vezes.
- A normal resultante não tem comprimento 1, e a interpolação entre vértices a encurta ainda mais. Por isso o
  fragment shader faz `normalize(vNormal)` antes de usá-la.
- **Escala zero:** a matriz fica singular (sem inversa). A tela totalmente recolhida usa `visible = false`
  (abaixo de 0,001 de abertura) e não chega a ser desenhada; nenhuma outra peça da sala tem escala.

## 5. Valores escolhidos

| O quê | Valor | Onde |
| --- | --- | --- |
| Luzes pontuais | 4, uma por luminária, máximo `kMaxPointLights = 4` | `scene/classroom.cpp`, `gfx/light.hpp`, `basic.frag` |
| Posição | X = ±1,6; Y = 2,91; Z = ±1,5 m (1,5 cm abaixo da calha) | `Lamp::lightOrigin()` + placement |
| Cor da lâmpada | (0,55; 0,53; 0,50) | `kLampLightColor` |
| Atenuação | kc = 1; kl = 0,14; kq = 0,07 | `kLampConstant`, `kLampLinear`, `kLampQuadratic` |
| Ambiente | (0,25; 0,25; 0,26) | `kAmbientLight` |
| Janela | direção (1; -0,6; 0,15), cor (0,16; 0,17; 0,19) | `kWindowDirection`, `kWindowColor` |
| Especular / brilho | 0,3 branco / 32, iguais para todas as peças (provisório até a CG-72) | `basic.frag` |

## 6. Onde está no código

| Parte | Arquivo |
| --- | --- |
| Structs `PointLight`, `DirectionalLight`, `Lighting` | `include/gfx/light.hpp` |
| Luzes da sala (posição, cor, atenuação, ambiente, janela) | `src/scene/classroom.cpp` (`Classroom::lighting()`) |
| Ponto da luz na luminária | `Lamp::lightOrigin()` em `src/objects/lamp.cpp` |
| Envio dos uniforms (`uViewPos`, `uAmbient`, `uWindowLight`, `uPointLights[]`, `uPointLightCount`) | `src/gfx/renderer.cpp` |
| Normal matrix (`uNormalMatrix`) | `src/gfx/scene_node.cpp` (`SceneNode::draw`) |
| Normal para o mundo | `shaders/basic.vert` |
| Blinn-Phong, atenuação, modos de depuração | `shaders/basic.frag` (`blinnPhong`, `shade`) |

Modos do fragment shader (`kDebugMode` em `src/gfx/renderer.cpp`): **0 = iluminado** (padrão), 1 = normal como cor,
2 = UV como cor, 3 = textura sem luz. Os modos 1 a 3 servem para conferir normais e UVs de um objeto novo.
