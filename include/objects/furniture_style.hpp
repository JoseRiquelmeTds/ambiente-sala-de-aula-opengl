#ifndef AULA_OPENGL_OBJECTS_FURNITURE_STYLE_HPP
#define AULA_OPENGL_OBJECTS_FURNITURE_STYLE_HPP

#include "gfx/material.hpp"

// =============================================================================
//  Estilo comum da carteira (CG-62) e da cadeira (CG-60): as duas ficam lado a lado em todas
//  as fileiras e precisam parecer do mesmo conjunto (docs/CONVENCOES.md § 6).
// =============================================================================
//
//  Mude aqui, não no objeto: o valor vale para os dois móveis ao mesmo tempo.
//
//  Materiais (CG-74): o tubo de aço e as placas de laminado também são comuns. As funções
//  devolvem o Material por valor; cada móvel guarda o seu como membro, junto com a própria
//  Texture do laminado (o objeto é dono da textura, docs/ILUMINACAO.md § 8). Ambos usam esse padrão:
//
//      // chair.hpp, nesta ordem (a textura antes do material que aponta para ela):
//      Texture boardTexture_;   // no construtor: boardTexture_(furniture::kBoardTexturePath)
//      Material board_;         //                board_(furniture::boardMaterial())
//      Material tube_;          //                tube_(furniture::tubeMaterial())
//      // corpo do construtor:   board_.diffuseMap = &boardTexture_;
//      // addTo:                 raiz.material = &tube_;  assento.material = encosto.material = &board_;
//
//  e gerar assento e encosto com textureSize = kBoardTextureSize, para o veio ter a mesma
//  escala nos dois móveis.
namespace objects::furniture {

constexpr float kTubeRadius = 0.011f;  // tubo de aço de 22 mm
constexpr int kTubeSegments = 16;
constexpr float kBoardThickness = 0.018f;  // MDF do tampo, assento e encosto

// Laminado madeirado das placas: imagem seamless com o veio na horizontal (eixo u). No box()
// das primitivas, u corre ao longo de X nas faces de cima, de baixo, da frente e de trás; o
// tampo e o encosto são mais compridos em X, então o veio acompanha o comprimento da peça.
constexpr const char* kBoardTexturePath = "assets/textures/laminado_carteira.png";

// textureSize das placas (metros cobertos por UMA repetição do laminado): a UV sai em metros,
// então a textura repete em vez de esticar, e com o mesmo valor nos dois móveis o veio tem a
// mesma largura no tampo e no assento.
constexpr float kBoardTextureSize = 0.5f;

// Aço com pintura eletrostática grafite, sem textura (a tinta é lisa).
Material tubeMaterial();

// Laminado melamínico das placas. Vem sem textura: quem usa aponta diffuseMap para a própria
// Texture de kBoardTexturePath. diffuse branco = a cor vem toda da imagem.
Material boardMaterial();

}  // namespace objects::furniture

#endif  // AULA_OPENGL_OBJECTS_FURNITURE_STYLE_HPP
