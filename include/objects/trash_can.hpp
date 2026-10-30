#ifndef AULA_OPENGL_OBJECTS_TRASH_CAN_HPP
#define AULA_OPENGL_OBJECTS_TRASH_CAN_HPP

#include <glm/glm.hpp>

#include "gfx/material.hpp"
#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"
#include "gfx/texture.hpp"

namespace objects {

// =============================================================================
//  TrashCan: lixeira de plástico em tronco de cone, oca (CG-67).
// =============================================================================
//
//  Metros, Y para cima. Pivô na base: origem no centro do fundo, no chão. Simétrica em
//  volta de Y, então qualquer lado serve de frente.
//
//  0,30 m de diâmetro na boca, 0,26 m no fundo e 0,40 m de altura. Nenhuma primitiva faz um
//  sólido oco, então o mesh é montado no .cpp com cinco superfícies fechando a casca:
//
//      parede externa   normal para fora, perpendicular à geratriz (um pouco para baixo)
//      parede interna   a mesma normal negada e triângulos em ordem invertida
//      borda            anel com a espessura da parede (6 mm), normal +Y
//      fundo externo    disco em y = 0, normal -Y
//      piso interno     disco em y = 8 mm, normal +Y
//
//  Material (CG-78): polipropileno azul com textura (plastico_lixeira.png) na peça inteira, um
//  material só, na raiz. A UV do mesh já está em metros / 0,25, então o uvScale fica 1.
//
//  Criar com contexto OpenGL ativo e manter vivo enquanto a árvore usar seus meshes, a textura e
//  o material.
class TrashCan {
public:
    TrashCan();

    TrashCan(const TrashCan&) = delete;
    TrashCan& operator=(const TrashCan&) = delete;

    // Pode ser chamada várias vezes: todas as instâncias usam o mesmo mesh e o mesmo material.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement) const;

private:
    Mesh body_;
    // A textura vem antes do material que aponta para ela: os membros são construídos nessa ordem.
    Texture plasticTexture_;
    Material plastic_;
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_TRASH_CAN_HPP
