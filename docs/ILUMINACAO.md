# Iluminação

Como a sala é iluminada (CG-71): o modelo de luz, as luzes da cena e a matriz das normais; e como cada peça responde
à luz (materiais, CG-72). Escrito para virar a seção de iluminação e materiais do relatório técnico (CG-80). Onde
cada coisa fica no código: [§ 6](#6-onde-está-no-código). Para aplicar material num objeto:
[§ 8](#8-como-aplicar-material-no-seu-objeto).

## Sumário

1. [Modelo de Blinn-Phong](#1-modelo-de-blinn-phong)
2. [Luzes pontuais e atenuação](#2-luzes-pontuais-e-atenuação)
3. [Luz ambiente e luz de preenchimento](#3-luz-ambiente-e-luz-de-preenchimento)
4. [Matriz das normais](#4-matriz-das-normais)
5. [Valores escolhidos](#5-valores-escolhidos)
6. [Onde está no código](#6-onde-está-no-código)
7. [Materiais](#7-materiais)
8. [Como aplicar material no seu objeto](#8-como-aplicar-material-no-seu-objeto)
9. [Interação: ligar a luz](#9-interação-ligar-a-luz)
10. [Projetor: textura projetiva (CG-77)](#10-projetor-textura-projetiva-cg-77)

---

## 1. Modelo de Blinn-Phong

A cor de cada fragmento (pixel candidato) é calculada no fragment shader, por pixel e não por vértice, somando as
componentes. Todos os vetores são unitários, então o produto escalar `·` é o cosseno do ângulo entre eles:

| Vetor | O que é |
| --- | --- |
| **N** | normal da superfície no fragmento, no espaço do mundo |
| **L** | direção do fragmento **para** a luz |
| **V** | direção do fragmento para a câmera (`uViewPos - posição`) |
| **H** | *half-vector*: `normalize(L + V)`, o meio do caminho entre a luz e o olho |

```
cor = emissiva + ambiente · albedo
    + Σ luzes  atenuação · corDaLuz · ( max(N·L, 0) · albedo  +  max(N·H, 0)^brilho · especular )
```

`albedo` é a cor difusa da superfície, `especular` é a cor do reflexo, `brilho` (*shininess*) o expoente que controla
o tamanho do reflexo e `emissiva` a luz própria da peça. Os quatro vêm do material da peça ([§ 7](#7-materiais)).

- **Ambiente:** uma constante. Representa a luz que rebate nas paredes e chega de todo lado, que o modelo local não
  calcula. Sem ela, toda face de costas para as luzes ficaria preta.
- **Difusa (Lambert), `N·L`:** uma superfície fosca espalha a luz igualmente para todos os lados, então o que importa
  é quanta luz ela **recebe**: o máximo com a luz batendo de frente (`N·L = 1`), zero com a luz rasante ou por trás.
  O `max(…, 0)` descarta a luz que viria por trás da face.
- **Especular, `(N·H)^brilho`:** o reflexo brilhante, que muda quando a câmera se move. Quando H coincide com N, a luz
  refletida vai direto para o olho. O expoente estreita o reflexo: 8 dá uma mancha larga (plástico fosco), 128 um
  ponto pequeno e forte (metal polido). Se `N·L ≤ 0`, o especular é zerado, para o reflexo não aparecer no lado da
  face que não recebe luz.
- **Emissiva:** luz que a própria peça emite, somada sem depender de luz nenhuma. Zero em quase tudo; é o que faz o
  tubo da lâmpada acesa parecer aceso (CG-73, [§ 9](#9-interação-ligar-a-luz)).

**Por que Blinn-Phong e não Phong.** O Phong original usa `(R·V)^brilho`, com `R = reflect(-L, N)`, o raio de luz
espelhado na superfície. Blinn troca R·V por N·H:

- **Não corta o reflexo:** quando o ângulo entre R e V passa de 90°, `R·V` fica negativo e o reflexo some de repente,
  com uma borda visível em superfícies de brilho baixo. Com a luz e o olho na frente da face, H também fica na
  frente e N·H não chega a zero: o reflexo enfraquece suave.
- **É mais próximo do real** em ângulos rasantes (o reflexo se alonga, como num piso encerado visto de longe).
- **É mais barato:** `normalize(L + V)` em vez de `reflect`.

Para o reflexo ter o mesmo tamanho, o expoente do Blinn costuma ser de 2 a 4 vezes o do Phong.

**Divisões protegidas.** Normalizar é dividir o vetor pelo próprio comprimento, então um vetor nulo daria divisão por
zero (resultado `NaN`, pixel com cor indefinida). Dois casos são tratados com um epsilon (`kEpsilon = 1e-4`):

- **Distância até a luz pontual:** `d = max(length(toLight), kEpsilon)` antes de `toLight / d`, para o caso de um
  fragmento coincidir com a posição da luz.
- **Half-vector:** se `L + V` for nulo (luz exatamente atrás da superfície e câmera de frente, em sentidos opostos),
  não há reflexo para a câmera: o especular fica 0 em vez de normalizar o vetor nulo.

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
em `uPointLightCount`. É o que a tecla `L` (CG-73) usa para apagar as lâmpadas ([§ 9](#9-interação-ligar-a-luz)).

## 3. Luz ambiente e luz de preenchimento

- **Ambiente (0,25; 0,25; 0,26):** garante que nenhuma face fique preta. Uma face de costas para todas as luzes
  mostra 25% da própria cor. Mais que isso, a sala fica "lavada" (todas as faces com a mesma claridade, sem volume).
- **Preenchimento (direcional):** representa a claridade do dia, uma luz de muito longe com todos os raios
  paralelos: L é a mesma em toda a cena e não há atenuação. A sala não tem janela modelada, então essa luz não sai de
  nenhum objeto da cena; ela só vem do lado esquerdo (-X) de quem olha para o quadro, onde as salas costumam ter
  janelas. A luz viaja para +X e desce (direção (1; -0,6; 0,15)), fraca e levemente azulada (0,16; 0,17; 0,19). Ela só
  **preenche**: dá volume às faces voltadas para a esquerda, que as lâmpadas iluminam de lado.

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
| Ambiente | (0,25; 0,25; 0,26); com a luz apagada, (0,20; 0,21; 0,23) | `kAmbientLight`, `kAmbientLightOff` |
| Brilho do tubo aceso | emissiva (0,80; 0,82; 0,86) | `kTubeLitEmissive` (`objects/lamp.cpp`) |
| Preenchimento | direção (1; -0,6; 0,15), cor (0,16; 0,17; 0,19) | `kFillDirection`, `kFillColor` |
| Especular / brilho | por peça, no material ([§ 7](#7-materiais)) | `gfx/material.cpp` |

## 6. Onde está no código

| Parte | Arquivo |
| --- | --- |
| Structs `PointLight`, `DirectionalLight`, `Lighting` | `include/gfx/light.hpp` |
| Luzes da sala (posição, cor, atenuação, ambiente, preenchimento) | `src/scene/classroom.cpp` (`Classroom::lighting()`) |
| Ponto da luz na luminária | `Lamp::lightOrigin()` em `src/objects/lamp.cpp` |
| Envio dos uniforms (`uViewPos`, `uAmbient`, `uFillLight`, `uPointLights[]`, `uPointLightCount`) | `src/gfx/renderer.cpp` |
| Normal matrix (`uNormalMatrix`) | `src/gfx/scene_node.cpp` (`SceneNode::draw`) |
| Normal para o mundo | `shaders/basic.vert` |
| Blinn-Phong, atenuação, modos de depuração | `shaders/basic.frag` (`blinnPhong`, `shade`, `albedo`) |
| Ligar/desligar a luz (tecla `L`) | `src/scene/classroom.cpp` (`Classroom::update`, `Classroom::setLightOn`), `Lamp::setLit` |
| Projetor: matrizes, tecla `P` e setas (CG-77) | `src/scene/classroom.cpp` (construtor, `Classroom::setProjectorOn`), `Projector::setOn`, `Projector::nextSlide` |
| Projetor: struct, envio e termo de luz (CG-77) | `ProjectorLight` em `include/gfx/light.hpp`, `src/gfx/renderer.cpp`, `projectorLight` em `shaders/basic.frag` |
| Struct `Material`, presets e envio (`uMaterial.*`) | `include/gfx/material.hpp`, `src/gfx/material.cpp` |
| Herança do material pela árvore | `src/gfx/scene_node.cpp` (`SceneNode::draw`) |
| Material padrão (`materials::neutral()`) | `src/gfx/renderer.cpp` (`kDefaultMaterial`) |
| Demonstração dos presets (objeto articulado, tecla `3`) | `src/scene/classroom.cpp` |

Modos do fragment shader (`kDebugMode` em `src/gfx/renderer.cpp`): **0 = iluminado** (padrão), 1 = normal como cor,
2 = UV como cor, 3 = cor difusa do material sem luz. Os modos 1 a 3 servem para conferir normais, UVs e texturas de um
objeto novo.

## 7. Materiais

O material diz **como uma superfície responde à luz**. As luzes são as mesmas para a sala inteira; o que faz o metal
parecer metal e a madeira parecer madeira é o material de cada peça. A struct `Material` (`include/gfx/material.hpp`)
vai para o shader como `uniform Material uMaterial`, enviada por `Material::apply` antes de cada peça ser desenhada.

| Campo | Termo da equação | O que faz |
| --- | --- | --- |
| `diffuse` | albedo (ambiente e difusa) | Cor da superfície: a fração de cada canal RGB que ela espalha |
| `diffuseMap` | albedo | Textura opcional (ponteiro não dono). Com textura, `albedo = diffuse × textura(uv · uvScale)`: `diffuse` branco deixa a textura pura, outra cor a tinge |
| `uvScale` | — | Repetições da textura (tiling). Peça esticada: aumente na direção que cresceu para a textura não esticar |
| `specular` | especular | Cor e força do reflexo. Branco/cinza em não metais (o reflexo tem a cor da luz); alto em metal; ~0 em fosco |
| `shininess` | expoente de `N·H` | Tamanho do reflexo: 4–16 largo e fraco (superfície áspera), 64–128 pequeno e nítido (polida). Mínimo 1 |
| `emissive` | emissiva | Luz própria, somada sem depender das luzes nem da normal (o tubo da lâmpada acesa, CG-73) |

A textura é ligada na unidade **0** (`kDiffuseTextureUnit`) e o sampler `uMaterial.diffuseMap` lê dela. Sem textura,
`uMaterial.hasDiffuseMap` é falso e o shader nem amostra: a unidade ainda guardaria a textura da peça anterior.

### Presets

Funções em `namespace materials` (`include/gfx/material.hpp`), sem textura obrigatória. Retornam por valor: guarde
num membro e ajuste cor, textura ou `uvScale`.

| Preset | `diffuse` (padrão) | `specular` | `shininess` | Por quê |
| --- | --- | --- | --- | --- |
| `metal()` | (0,30; 0,30; 0,32) | (0,80; 0,80; 0,82) | 96 | Metal devolve quase toda a luz como reflexo: difusa escura, reflexo forte e concentrado |
| `wood()` | (0,55; 0,37; 0,22) | 0,06 | 12 | Madeira/MDF é áspera: quase só difusa, reflexo mínimo e espalhado |
| `plastic()` | (0,70; 0,12; 0,10) | 0,45 | 32 | Plástico liso: a cor fica na difusa e o reflexo é branco, de tamanho médio |
| `matte()` | (0,80; 0,80; 0,78) | 0,01 | 4 | Quadro, parede, lona: espalha a luz por igual, sem reflexo perceptível |
| `neutral()` | 0,72 | 0,04 | 8 | Material padrão das peças sem material: cinza claro fosco |

Todos aceitam a cor como parâmetro (`materials::plastic(glm::vec3(0.1f, 0.3f, 0.7f))` para plástico azul), exceto
`neutral()`.

O objeto articulado de demonstração (tecla `3`) usa três presets lado a lado: base de madeira, haste de metal e
braço de plástico; as paredes e os objetos ainda sem material mostram o comportamento fosco do `neutral()`. A
diferença aparece sobretudo no reflexo: a haste de metal fica escura, com uma faixa clara quando vista no ângulo do
reflexo da lâmpada; o braço de plástico ganha um brilho claro no topo; a base de madeira não brilha.

### Materiais aplicados (CG-74: carteira, luminária, interruptor e mesa do professor)

Cada peça parte de um preset e ajusta o que a superfície real pede. Só as madeiras têm textura: tubo pintado, chapa
pintada, alumínio, vidro e plástico são lisos na vida real, e a cor + o reflexo bastam. Com textura, `diffuse` é
branco (a cor vem da imagem). O tiling vem do `textureSize` das primitivas (UV em metros), então `uvScale` fica 1.

| Peça | Preset | `diffuse` | `specular` | `shininess` | Textura (metros por repetição) |
| --- | --- | --- | --- | --- | --- |
| Carteira: tubos e porta-livros | `furniture::tubeMaterial()` (metal) | (0,20; 0,20; 0,22) grafite | 0,55 | 48 | — |
| Carteira: tampo | `furniture::boardMaterial()` (wood) | 1 | 0,12 | 24 | `laminado_carteira.png` (0,5 m) |
| Carteira: ponteiras | plastic | 0,05 | 0,20 | 16 | — |
| Mesa: tampo, painéis, gavetas | wood | 1 | 0,10 | 20 | `madeira_mesa_professor.png` (0,6 m) |
| Mesa: puxadores | metal | (0,45; 0,45; 0,47) | 0,80 | 96 | — |
| Mesa: sapatas | plastic | 0,05 | 0,20 | 16 | — |
| Luminária: calha (chapa pintada) | plastic | (0,86; 0,86; 0,84) | 0,30 | 24 | — |
| Luminária: refletor e aletas | metal | (0,55; 0,55; 0,57) alumínio | 0,85 | 64 | — |
| Luminária: vidro do tubo | plastic | (0,90; 0,92; 0,95) | 0,50 | 64 | — |
| Luminária: soquetes | plastic | (0,82; 0,82; 0,80) | 0,25 | 16 | — |
| Interruptor: espelho e tecla | plastic | (0,88; 0,88; 0,86) | 0,50 | 48 | — |

- **Tinta e plástico usam o preset de plástico**, mesmo sobre chapa de aço: o que reflete é a camada de tinta, um
  não metal (reflexo branco, cor na difusa). O tubo da carteira é a exceção pedida pelo estilo comum (metal), com o
  reflexo mais fraco e largo que o do aço nu por causa da pintura.
- **Veio na direção do comprimento:** as imagens têm o veio na horizontal (eixo u), e no `box()` u segue X nas faces
  de cima e da frente, que é o comprimento do tampo, do painel frontal e das gavetas.
- **Nada estourando:** os brancos (calha, soquete, vidro, interruptor) ficam entre 0,82 e 0,90, não 1,0: somando
  ambiente, preenchimento e luminárias, um branco puro passaria de 1 nas faces de frente para a luz, e o OpenGL cortaria em
  branco chapado, sem volume.

### Materiais aplicados (CG-78: tela retrátil, projetor e lixeira)

Aqui os plásticos e a lona têm textura (`plastico_projetor.png`, `plastico_lixeira.png`, `lona_tela.png`), com
`diffuse` branco ou quase: a cor vem da imagem. Metais, vidro, cordinha e puxador são só cor.

| Peça | Preset | `diffuse` | `specular` | `shininess` | Textura (metros por repetição) |
| --- | --- | --- | --- | --- | --- |
| Projetor: placa do teto e haste | metal | (0,20; 0,20; 0,22) grafite | 0,55 | 48 | — |
| Projetor: suporte e corpo | plastic | 1 | 0,25 | 24 | `plastico_projetor.png` (0,25 m) |
| Projetor: lente (`lensGlass_`) | plastic | 0,05 | 0,90 | 128 | — (`emissive` 0; a CG-77 acende) |
| Tela: caixa e suportes laterais | plastic | (0,92; 0,92; 0,90) branco-gelo | 0,30 | 24 | — |
| Tela: lona (`canvasMaterial_`) | matte | 0,90 | 0,01 | 4 | `lona_tela.png` (0,25 m, `uvScale` = {8, 6}) |
| Tela: barra inferior | metal | (0,55; 0,55; 0,57) alumínio | 0,85 | 64 | — |
| Tela: cordinha | matte | 0,15 | 0,01 | 4 | — |
| Tela: puxador | plastic | 0,05 | 0,20 | 16 | — |
| Lixeira (peça inteira) | plastic | 1 | 0,30 | 24 | `plastico_lixeira.png` (0,25 m) |

- **Material na articulação:** o projetor põe o metal na raiz (placa e haste herdam), o plástico na cabeça (suporte
  e corpo) e o vidro só na lente. Na tela, a lona recebe o material no nó da articulação (`canvasPivot_`) e a barra
  no nó da ponta (`bottom_`); cordinha e puxador sobrescrevem dentro da ponta.
- **Tiling da lona pelo `uvScale`:** a UV da lona vai de 0 a 1 na face inteira, reservada para o slide da CG-77. Para a
  trama não esticar 2,0 × 1,5 m numa imagem quadrada, `uvScale = (largura, altura) / 0,25 m` = {8, 6}
  (`kCanvasTexturePeriod` em `src/objects/projection_screen.cpp`). Com a lona recolhida a escala em Y achata a trama;
  a CG-75 faz o `uvScale.y` acompanhar a abertura. Projetor e lixeira já têm UV em metros / 0,25, então `uvScale` = 1.
- **Lona abaixo de 1:** a textura da lona já é quase branca; com `diffuse` 0,90 ela fica clara sem cortar em branco
  chapado sob as luminárias, e a trama continua visível.
- **Sem espelhar:** a parede interna da lixeira tem u invertido em relação à externa, porque vista de dentro a volta
  anda para a esquerda; assim a textura aparece na mesma orientação nas duas faces.

### Materiais aplicados: cadeira, ventilador e quadro (Victor)

Os materiais são membros de `Chair`, `Fan` e `Board`; os nós guardam ponteiros não donos, como nos demais objetos.
`Chair` carrega uma única `Texture` antes dos materiais; suas seis instâncias compartilham esses recursos.
Nenhuma geometria, articulação ou interação foi alterada para aplicar os acabamentos.

| Peça | Material / cor difusa | Especular | Brilho | Textura |
| --- | --- | --- | --- | --- |
| Cadeira: tubos, travessas, montantes e fixações | `furniture::tubeMaterial()`, grafite | 0,55 | 48 | — |
| Cadeira: assento e encosto | `furniture::boardMaterial()`, branco | 0,12 | 24 | `laminado_carteira.png`, 0,5 m |
| Cadeira: ponteiras | plastic, 0,05 | 0,20 | 16 | — |
| Ventilador: fixação, suporte e eixo | metal pintado, (0,16; 0,17; 0,19) | 0,30 | 32 | — |
| Ventilador: grades | metal, (0,30; 0,32; 0,34) | 0,50 | 48 | — |
| Ventilador: motor | plastic, (0,09; 0,10; 0,12) | 0,20 | 24 | — |
| Ventilador: pás | plastic, (0,58; 0,61; 0,64) | 0,25 | 24 | — |
| Ventilador: tampa | plastic, (0,20; 0,22; 0,25) | 0,30 | 32 | — |
| Quadro: escrita | matte, (0,88; 0,89; 0,87) | 0,01 | 4 | — |
| Quadro: suporte posterior | wood, (0,28; 0,24; 0,18) | 0,06 | 12 | — |
| Quadro: moldura e bandeja | metal/alumínio, (0,55; 0,55; 0,57) | 0,60 | 48 | — |
| Quadro: cantos e corpo do apagador | plastic, (0,08; 0,09; 0,10) | 0,20 | 16 | — |
| Quadro: feltro | matte, (0,12; 0,14; 0,15) | 0 | 4 | — |
| Quadro: corpos dos marcadores | plastic, (0,82; 0,82; 0,80) | 0,45 | 32 | — |
| Quadro: tampas e extremidades | plastic, azul (0,05; 0,12; 0,55) / vermelho (0,65; 0,05; 0,04) | 0,45 | 32 | — |

O laminado conserva o veio horizontal em X no topo do assento e na frente do encosto. As UVs são calculadas
nas dimensões reais das placas com `furniture::kBoardTextureSize`, como na carteira, e `uvScale` permanece 1.
A inclinação de 8° do encosto transforma a peça inteira, preservando a escala e a orientação locais da textura.
Não há textura nova: cor e especular bastam nos acabamentos lisos do ventilador, evitando padrões nas pás
em movimento. O quadro permanece branco fosco e sem textura para receber a projeção com a tela recolhida;
suas UVs 0–1 e a folga de 14 mm até a lona permanecem intactas.

### Herança pela árvore

`SceneNode` tem `const Material* material = nullptr` (ponteiro **não dono**, como o `mesh`). No `SceneNode::draw`:

```cpp
const Material& currentMaterial = material != nullptr ? *material : inheritedMaterial;
// desenha a peça com currentMaterial e o passa como "herdado" para os filhos
```

Uma peça sem material usa o do pai, que pode ter herdado do avô. Basta definir o material na **raiz** do objeto e
sobrescrever só nas peças diferentes. Um nó sem mesh (articulação) também pode ter material, que vale para os filhos:
no demo, o plástico está na articulação e o braço o herda. A raiz da cena recebe o **material padrão** do renderer
(`materials::neutral()`): objeto que ainda não tem material aparece cinza claro fosco e iluminado, nunca preto nem com
o material da peça desenhada antes.

## 8. Como aplicar material no seu objeto

1. **Material e textura são membros do objeto**, vivos enquanto a árvore existir (a peça só guarda o ponteiro, como
   faz com o `Mesh`). A textura vem **antes** do material na declaração da classe, porque o material aponta para ela.
2. **Comece de um preset** e ajuste cor, textura e `uvScale`.
3. **Aponte a raiz** para o material principal e sobrescreva só as peças diferentes.

```cpp
// include/objects/trash_can.hpp (exemplo)
#include "gfx/material.hpp"
#include "gfx/texture.hpp"

class TrashCan {
    // ...
private:
    Mesh body_;
    Texture plasticTexture_;  // antes do material que aponta para ela
    Material plastic_;
};
```

```cpp
// src/objects/trash_can.cpp
TrashCan::TrashCan()
    : body_(makeBody()),
      plasticTexture_("assets/textures/plastico_lixeira.png"),
      plastic_(materials::plastic(glm::vec3(1.0f))) {  // branco: a cor vem da textura
    plastic_.diffuseMap = &plasticTexture_;
}

SceneNode& TrashCan::addTo(SceneNode& parent, const glm::mat4& placement) const {
    SceneNode& root = parent.addChild(&body_, placement);
    root.material = &plastic_;  // a lixeira inteira e tudo o que for pendurado nela
    return root;
}
```

- **Peça diferente dentro do objeto:** `perna.material = &metal_;` muda só a perna e os filhos dela.
- **Sem textura:** não defina `diffuseMap`; a cor é `diffuse`. Ex.: `metal_(materials::metal())`.
- **Textura esticando numa peça comprida:** as primitivas já geram UV em metros com `textureSize`, então em geral
  basta escolher o `textureSize`. Se a UV do mesh vai de 0 a 1 numa peça de 2 × 0,5 m, use `uvScale = {4, 1}`: a
  textura repete 4 vezes no comprimento e mantém a proporção.
- **`addTo` é `const`:** sem problema, `root.material = &plastic_` guarda um `const Material*`.
- **Mesma instância em vários lugares:** todas as cópias (6 carteiras) apontam para o mesmo material; mudar o membro
  muda todas.
- **Material compartilhado entre objetos** (carteira e cadeira, que precisam parecer do mesmo conjunto): a função
  que monta o material fica num header comum (`furniture::tubeMaterial()` e `furniture::boardMaterial()` em
  `objects/furniture_style.hpp`) e cada objeto guarda a sua cópia. Mudar a função muda os dois.
- **Conferir:** `kDebugMode = 3` em `src/gfx/renderer.cpp` mostra a cor do material sem luz; volte para 0.
- Textura nova vai para `assets/textures/` com a linha na tabela de licenças, no mesmo commit.

## 9. Interação: ligar a luz

A tecla `L` (CG-73) apaga e acende as quatro luminárias, como o interruptor da parede. A sala começa acesa. A troca
é instantânea, como num fluorescente real, e é feita por `Classroom::setLightOn(bool)`, chamada uma vez no
construtor (estado inicial) e a cada vez que `L` é pressionada. `Input::keyPressed` só vale no frame em que a tecla
desce: segurar `L` não fica piscando a luz.

O que muda em cada chamada:

| O quê | Ligada | Apagada | Efeito no shader |
| --- | --- | --- | --- |
| `PointLight::enabled` das 4 luzes | `true` | `false` | O renderer só envia as ligadas: `uPointLightCount` vai de 4 a 0 e o laço das luzes pontuais não roda |
| `emissive` do vidro do tubo (`Lamp::setLit`) | (0,80; 0,82; 0,86) | 0 | `uMaterial.emissive` do tubo: aceso, ele aparece branco mesmo visto de lado ou de cima |
| `Lighting::ambient` | (0,25; 0,25; 0,26) | (0,20; 0,21; 0,23) | `uAmbient` |
| Tecla do interruptor (`LightSwitch::setOn`) | metade de cima afundada | metade de baixo afundada | `uModel` da tecla (rotação de ±7° em X) |

- **Só uniforms mudam, não a geometria.** Ligar a luz não move nem deforma nada: a cena é a mesma, o que muda é a
  quantidade de luz que entra na equação de Blinn-Phong. A única transformação é a da tecla, uma rotação da
  articulação em torno do eixo do basculamento (`T(pivô) · R(±7°)`), como uma tecla real.
- **Os tubos acendem juntos** porque as quatro luminárias apontam para o **mesmo** `Material` do vidro (membro da
  `Lamp`): mudar o `emissive` uma vez muda as quatro. O mesmo vale para a luz de cada uma, que é da `Classroom`.
- **A sala não fica preta.** É uma sala de dia: o preenchimento continua aceso, e a luz ambiente não vai a zero. Parte
  do ambiente é a luz das lâmpadas rebatida nas paredes; apagadas, sobra só a claridade do dia rebatida. Por isso o
  ambiente cai para cerca de 80% e puxa para o azul do dia, em vez de sumir. Na vista inicial, a claridade média da
  imagem cai de ~140 para ~45 (de 255): bem mais escura, mas sem nenhuma face preta, e as faces voltadas para a esquerda (-X)
  continuam mais claras que as outras.
- **Por que o tubo precisa de emissiva:** a luz pontual fica abaixo dos tubos, então ela ilumina a parte de baixo do
  vidro, mas o modelo não tem nada que faça uma superfície parecer uma fonte de luz. A emissiva soma uma cor fixa,
  independente de luzes e normais: o tubo aparece aceso de qualquer ângulo. Apagado, ele volta a ser só vidro.

## 10. Projetor: textura projetiva (CG-77)

A tecla `P` liga o projetor e o slide aparece na tela retrátil; `←` e `→` trocam o slide. O slide **não** é uma
textura colada na lona: o projetor é uma **luz que carrega uma imagem** (*projective texture mapping*). Cada
fragmento da cena pergunta "em que ponto da imagem do projetor eu estou?" e recebe a luz daquele texel. A imagem
cai onde o feixe bater: na lona aberta, no quadro e na parede com a tela recolhida, em quem passar na frente.

### O projetor como câmera

A pergunta é a mesma que a câmera responde para desenhar a cena: onde um ponto do mundo cai na imagem. Então o
projetor ganha uma view e uma projeção próprias, como uma câmera posta na lente:

```
V_proj = lookAt(lente, lente + eixo, cima)          mundo -> espaço do projetor
P_proj = perspective(fovY, 4/3, 0,1 m, 10 m)        espaço do projetor -> clip
```

`lente`, `eixo` e `cima` são `Projector::lensCenter()`, `lensDirection()` e `lensUp()` levados ao mundo pelo
placement (as direções só pela parte 3×3, sem translação), já com a inclinação calculada por `aimAt`
([ARQUITETURA.md § 6](ARQUITETURA.md#projetor-e-tela-retrátil-cg-65-cg-66-cg-77)). O produto `P_proj · V_proj` é
calculado uma vez no construtor da `Classroom` e enviado como `uProjector.viewProjection`. No fragment shader, cada
fragmento percorre a cadeia:

```
mundo --V_proj--> projetor --P_proj--> clip (x, y, z, w) --÷w--> NDC [-1, 1] --×0,5 + 0,5--> UV [0, 1]
```

- **÷w:** é a divisão de perspectiva que a GPU faz sozinha com `gl_Position`; aqui ela é feita à mão. É o que faz
  um ponto mais longe ocupar menos da imagem: o mesmo texel cobre uma área maior quanto mais longe da lente.
- **×0,5 + 0,5:** leva o quadrado do NDC (-1 a 1) para o da textura (0 a 1). O centro da imagem (NDC 0) vira a UV
  (0,5; 0,5), o centro do slide.
- **w ≤ 0:** o ponto está atrás da lente (w é a profundidade na frente do projetor). A divisão inverteria o sinal
  de x e y e projetaria o slide espelhado nas costas do projetor; esses fragmentos não recebem nada.
- **NDC fora de [-1, 1]:** fora do frustum do projetor. Em x e y é fora da imagem (fora do cone de luz); em z é
  antes do near (0,1 m) ou depois do far (10 m). Não recebem nada. O slide usa `ClampToEdge`, para a borda não
  "vazar" pelo lado oposto no filtro linear.

### Ângulo de abertura pelo throw ratio

O *throw ratio* de um projetor real é distância ÷ largura da imagem. A lente fica a `d` = 2,89 m do centro da lona
(inclinação de 12,5° na montagem da CG-69) e a imagem ocupa 92% dos 2,0 m de largura no centro (`kImageFill`,
motivo no [keystone](#keystone)): `w` = 1,84 m, throw ratio de ~1,57. Com o eixo como cateto de dois triângulos
retângulos:

```
fovX = 2 · atan( (w/2) / d )              = 2 · atan(0,92 / 2,89)    ≈ 35,3°
fovY = 2 · atan( tan(fovX/2) / aspecto )  = 2 · atan(0,318 / 1,333)  ≈ 26,9°
```

`glm::perspective` recebe o fov **vertical** e o aspecto 4:3 dos slides (1024 × 768). A largura vem de
`ProjectionScreen::canvasSize()` e a distância da posição real da lente: mover a tela ou o projetor recalcula tudo.

### Orientação: em pé e sem espelhar

O `lookAt` monta os eixos da "câmera" do projetor: frente = eixo da lente, cima = `lensUp` e direita =
frente × cima. O projetor está girado 180° em Y (olha para -Z, como a turma), então no mundo a frente é
(0; -0,22; -0,98), o cima (0; 0,98; -0,22) e a direita sai **+X**, a direita de quem olha para a tela de dentro da
sala. Logo, +X do projetor → NDC +x → u crescendo para a direita da plateia, e cima → v crescendo para cima. A
`Texture` inverte a imagem na carga (v = 0 é a linha de baixo do arquivo), então v = 1 é o topo do slide e ele
aparece em pé. É projeção frontal, com projetor e plateia do mesmo lado da tela: sem espelho. O bloco laranja no
canto superior esquerdo de cada slide serve para conferir.

### Termo de luz

```
projetor = albedo · slide(u, v) · cor · max(N·L, 0) · (d_ref / d)²
```

- **slide(u, v):** a cor do texel é a cor da luz que chega ali. Branco ilumina; preto não ilumina. Um projetor
  não projeta preto: as partes escuras do slide só não recebem luz e ficam com a cor que a sala já dá à superfície.
- **Lambert, `N·L`:** a mesma difusa das outras luzes, com L do fragmento para a lente. Uma face de costas para o
  projetor (o verso da lona) não recebe imagem.
- **(d_ref / d)²:** a mesma luz se espalha por uma área que cresce com o quadrado da distância. Com `d_ref` = a
  distância até o centro da lona, o fator é 1 ali e `cor` (0,90; 0,90; 0,85) é a força do feixe na tela. Não há
  termo constante nem linear como nas lâmpadas: o feixe é concentrado e não há luz rebatida a imitar. `d` tem um
  mínimo de `kEpsilon` (1e-4), como nas lâmpadas: com o fragmento em cima da lente, `L = toLens / d` e `1 / d²`
  dividiriam por zero. O near já descarta esses pontos; o mínimo protege se ele mudar.
- **Sem especular:** a lona é fosca, e o reflexo de uma lente pequena seria um ponto. O termo é somado em `shade()`
  sem mudar nenhum dos outros.

Com as lâmpadas acesas, a lona já recebe de 0,7 a 0,8 de luz: as partes brancas do slide somam mais ~0,6 e
saturam em 1, e as escuras ficam no cinza claro da sala. A imagem aparece clara e com pouco contraste, como num
projetor real com a luz da sala acesa. Apagando as lâmpadas (`L`), a lona cai para ~0,15 e o slide se destaca.

### Keystone

O projetor está acima do centro da lona e inclinado para baixo, e a lona é vertical: a imagem não chega
perpendicular. A borda de baixo da imagem fica mais longe da lente (~3,05 m) que a de cima (~2,73 m) e, como a
largura cresce com a distância, sai mais larga: um **trapézio mais largo embaixo** (keystone). Na lona, ~1,75 m de
largura em cima (a 2,74 m do chão) e ~1,94 m embaixo (a 1,32 m). Por isso a imagem ocupa 92% da largura no centro:
com 95%, a base passaria da lona. Projetores reais corrigem isso deformando a imagem ao contrário (correção de
keystone); aqui o trapézio fica, como num projetor sem correção.

### Limitações

- **Sem shadow map:** o shader não sabe se há algo entre a lente e o fragmento. A imagem também cai no quadro e na
  parede **atrás** da lona aberta; para a turma, a própria lona esconde essa cópia. Pela mesma razão, alguém na
  frente do projetor recebe a imagem no corpo, mas não faz sombra na tela. Resolver exigiria renderizar a
  profundidade da cena do ponto de vista do projetor e comparar (*shadow mapping*).
- **Tela recolhida:** a imagem cai no quadro e na parede, sem regra especial; é o resultado natural da técnica
  (decisão da CG-77).

### Interação e texturas

| O quê | Ligado | Desligado | Efeito no shader |
| --- | --- | --- | --- |
| `Lighting::projector.enabled` | `true` | `false` | `uProjector.enabled`: desligado, `projectorLight` devolve 0 sem calcular nada |
| `emissive` do vidro da lente (`Projector::setOn`) | (0,85; 0,90; 1,0) | 0 | `uMaterial.emissive` da lente: acesa de qualquer ângulo, como o tubo da lâmpada |

A tecla `P` chama `Classroom::setProjectorOn`, no mesmo desenho do `setLightOn` ([§ 9](#9-interação-ligar-a-luz)); o
projetor começa desligado. `←` e `→` trocam o slide em ciclo (`Projector::nextSlide`, `previousSlide`), mesmo com o
projetor desligado, e a `Classroom` atualiza `Lighting::projector.image`.

O slide fica na **unidade de textura 1** (`kProjectorTextureUnit`, `src/gfx/renderer.cpp`): a 0 é a da cor difusa,
religada pelo `Material::apply` de cada peça. O renderer liga o slide uma vez por frame, e o sampler
`uProjector.image` aponta sempre para a 1, mesmo com o projetor desligado.
