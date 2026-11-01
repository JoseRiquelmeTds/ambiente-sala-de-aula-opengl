# Texturas

Imagens usadas como textura, carregadas com `Texture` (`include/gfx/texture.hpp`). Caminhos relativos à raiz do
repositório, ex.: `Texture madeira("assets/textures/madeira.png");`.

**Toda imagem nova entra na tabela abaixo no mesmo commit**, com a origem e a licença. Prefira imagens CC0
(domínio público, ex.: [ambientCG](https://ambientcg.com), [Poly Haven](https://polyhaven.com/textures)) ou
feitas pela equipe. Não use imagens sem licença clara (resultado de busca no Google, por exemplo).

Dicas:

- PNG ou JPG. Lados em potência de 2 (256, 512, 1024) não são obrigatórios no OpenGL 3.3, mas os mipmaps
  ficam melhores. 512 ou 1024 px bastam para a sala; imagens grandes deixam o repositório pesado.
- Texturas que se repetem (madeira, piso, metal) precisam ser *seamless* (sem emenda visível nas bordas).
- Nome em minúsculas, sem acento nem espaço: `madeira_carteira.png`.

| Arquivo | Usado em | Origem | Autor | Licença |
| --- | --- | --- | --- | --- |
| `uv_teste.png` | Depuração de UV e de `Material::diffuseMap` (sem uso fixo na cena desde a CG-72) | Gerada por script Python: R = u, G = v, xadrez 8×8 | Equipe | Própria (livre uso no projeto) |
| `lona_tela.png` | Lona da tela retrátil (CG-66; aplicada na CG-78) | Gerada por script Python: trama de 4 px e ruído periódico (seamless), 512×512 | Equipe | Própria (livre uso no projeto) |
| `plastico_projetor.png` | Gabinete do projetor (CG-65; aplicada na CG-78) | Gerada por script Python: plástico cinza fosco, ruído periódico (seamless), 512×512 | Equipe | Própria (livre uso no projeto) |
| `plastico_lixeira.png` | Lixeira (CG-67; aplicada na CG-78) | Gerada por script Python: polipropileno azul, ruído periódico (seamless), 512×512 | Equipe | Própria (livre uso no projeto) |
| `laminado_carteira.png` | Tampo da carteira (CG-74), assento e encosto da cadeira; `furniture::kBoardTexturePath`, repetição a cada 0,5 m nos dois móveis | Gerada por script Python: laminado madeirado claro, veio na horizontal (u) com ruído periódico (seamless), 512×512 | Equipe | Própria (livre uso no projeto) |
| `madeira_mesa_professor.png` | Mesa do professor: tampo, painéis e gavetas (CG-74) | Gerada por script Python: madeira escura com veio marcado na horizontal (u), ruído periódico (seamless), 512×512 | Equipe | Própria (livre uso no projeto) |
| `slide_cg_1.png` | Slide 1 do projetor (CG-77), projetado na tela | Gerada por script Python (biblioteca padrão): pipeline gráfico em cinco caixas coloridas ligadas por setas e um triângulo sobre pixels; faixa de título com bloco laranja no canto superior esquerdo para conferir a orientação, 1024×768 (4:3) | Equipe | Própria (livre uso no projeto) |
| `slide_cg_2.png` | Slide 2 do projetor (CG-77), projetado na tela | Gerada por script Python (biblioteca padrão): eixos de coordenadas (X vermelho para a direita, Y verde para cima, Z azul na diagonal) e cubo unitário; faixa de título com bloco laranja no canto superior esquerdo para conferir a orientação, 1024×768 (4:3) | Equipe | Própria (livre uso no projeto) |
| `slide_cg_3.png` | Slide 3 do projetor (CG-77), projetado na tela | Gerada por script Python (biblioteca padrão): triângulo com as cores RGB dos vértices interpoladas (coordenadas baricêntricas); faixa de título com bloco laranja no canto superior esquerdo para conferir a orientação, 1024×768 (4:3) | Equipe | Própria (livre uso no projeto) |
