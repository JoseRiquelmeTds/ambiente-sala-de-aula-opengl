#ifndef AULA_OPENGL_OBJECTS_FURNITURE_STYLE_HPP
#define AULA_OPENGL_OBJECTS_FURNITURE_STYLE_HPP

// =============================================================================
//  Estilo comum da carteira (CG-62) e da cadeira (CG-60): as duas ficam lado a lado em todas
//  as fileiras e precisam parecer do mesmo conjunto (docs/CONVENCOES.md § 6).
// =============================================================================
//
//  Mude aqui, não no objeto: o valor vale para os dois móveis ao mesmo tempo.
//  Quando o sistema de materiais (CG-72) existir, os materiais de metal e madeira da mobília
//  também entram neste header.
namespace objects::furniture {

constexpr float kTubeRadius = 0.011f;  // tubo de aço de 22 mm
constexpr int kTubeSegments = 16;
constexpr float kBoardThickness = 0.018f;  // MDF do tampo, assento e encosto

}  // namespace objects::furniture

#endif  // AULA_OPENGL_OBJECTS_FURNITURE_STYLE_HPP
