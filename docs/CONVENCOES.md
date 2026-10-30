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
- As medidas compartilhadas ficam num header só, `include/objects/furniture_style.hpp`, integrado pela CG-62 e
  usado por `StudentDesk` e `Chair`, sem duplicar constantes:

```cpp
namespace objects::furniture {

constexpr float kTubeRadius = 0.011f;  // tubo de aço de 22 mm
constexpr int kTubeSegments = 16;
constexpr float kBoardThickness = 0.018f;  // MDF do tampo, assento e encosto

}  // namespace objects::furniture
```

Os materiais comuns também estão nesse header (CG-74): `furniture::tubeMaterial()` (aço pintado grafite),
`furniture::boardMaterial()` (laminado), o caminho da textura do laminado (`kBoardTexturePath`) e o tiling das placas
(`kBoardTextureSize`). Cada móvel guarda o próprio `Material` e a própria `Texture` como membros; o comentário do header
mostra como aplicar na cadeira.

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

### Montagem da sala (CG-69)

- `Room` delimita X = ±4, Z = ±3,5, piso superior Y = 0 e teto inferior Y = 3. Espessuras para fora:
  paredes/teto de 0,10 m e piso de 0,12 m. Rodapés de 0,08 m avançam 0,016 m para dentro.
- Parede frontal em -Z: tela em (0; 2,86; -3,5); quadro 3 × 1,2 m centrado em (0; 1,5; -3,5),
  moldura pelo menos 14 mm atrás da lona, com bandeja e acessórios abaixo dela.
- `StudentDesk` usa +Z para o **lado do aluno**, com porta-livros aberto. Isso difere da direção do olhar:
  na montagem, carteiras sem giro recebem o aluno atrás, em +Z, olhando para -Z. Cada `Chair` olha para
  -Z (giro de 180° em Y), em (X da carteira; 0; Z da carteira + 0,55). São seis conjuntos, com fileiras
  de carteiras em Z = -0,85 / 0,65 / 2,15 e de cadeiras em Z = -0,30 / 1,20 / 2,70;
  colunas em X = ±1,25. Não girar a carteira apenas para fazer +Z apontar para o quadro.
- Fixações usam o pivô real: ventilador na parede esquerda gira +90° em Y (+Z local vira +X);
  interruptor na direita gira -90° (+Z vira -X); luminárias e projetor encostam em Y = 3 e descem em -Y.
- Room e cada instância são filhos diretos da raiz. Um único alvo por tipo; esconder os demais filhos também
  esconde paredes, teto, piso e móveis repetidos. Enquadramentos são pontos locais transformados pelo placement.
  Os nove alvos numéricos são preservados; cadeira e quadro completam os onze alvos do ciclo `[`/`]`.
- UVs de Room repetem a cada 1 m nas superfícies principais e 0,5 m nos rodapés, sem escala nos placements.

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
| `1`, `2`–`9`, `0` | Inspeção: sala inteira / os nove primeiros alvos, preservados. `2` = carteira, `3` = objeto articulado, `4` = ventilador, `5` = lâmpada, `6` = interruptor, `7` = mesa do professor, `8` = tela retrátil, `9` = projetor, `0` = lixeira | `scene/classroom` | CG-55, CG-87, CG-62, CG-59, CG-63, CG-64, CG-66, CG-65, CG-67 | em uso |
| `[` / `]` | Inspeção: alvo anterior / próximo, em ciclo por todos os alvos (hoje onze; cadeira após a lixeira e quadro ao final), sem limite de teclas numéricas | `scene/classroom` | CG-60, CG-61 | em uso |
| `T` | Tela retrátil: abre / recolhe (a tela começa recolhida) | `scene/classroom` | CG-75 | em uso |
| `V` | Ventilador: liga / desliga (acelera ao ligar, desacelera até parar ao desligar) | `scene/classroom` | CG-76 | em uso |
| `Shift` + `V` | Ventilador: troca a velocidade (2, 3, 4 voltas/s), também com ele desligado | `scene/classroom` | CG-76 | em uso |
| `P` | Projetor: liga / desliga | `scene/classroom` | CG-77 | reservada |
| `←` / `→` | Slide anterior / próximo | `scene/classroom` | CG-77 (opcional) | reservada |
| `L` | Luz: liga / desliga (a sala começa acesa) | `scene/classroom` | CG-73 | em uso |

As seleções funcionam com o cursor livre e reenquadram a câmera. Após uma tecla numérica, `[` e `]` continuam
do alvo selecionado. Da sala inteira, `]` seleciona o primeiro alvo (carteira) e `[` o último (quadro, 11º alvo);
o ciclo retorna do último ao primeiro e vice-versa. A tecla `1` restaura a cena completa.

## 9. Texturas

Regras em [assets/textures/README.md](../assets/textures/README.md): toda imagem entra na tabela de licenças no mesmo
commit, nome em `snake_case` sem acento, PNG/JPG de 512 ou 1024 px, *seamless* quando repete.
