#ifndef AULA_OPENGL_OBJECTS_BOARD_HPP
#define AULA_OPENGL_OBJECTS_BOARD_HPP

#include <glm/glm.hpp>

#include "gfx/material.hpp"
#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"

namespace objects {

// Quadro escolar (CG-61), 3,00 x 1,20 m: Y para cima, face de escrita para +Z.
// Origem no centro da face posterior de fixação (z = 0); a bandeja avança para +Z.
// Criar com contexto OpenGL ativo e manter vivo enquanto a árvore usar seus meshes.
class Board {
public:
    Board();

    Board(const Board&) = delete;
    Board& operator=(const Board&) = delete;

    // Raízes independentes, com todos os componentes relativos ao placement e meshes compartilhados.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement) const;

private:
    // Superfície branca fosca, suporte, alumínio, plástico e feltro separados.
    Mesh surface_;
    Mesh body_;
    Mesh frame_;
    Mesh corner_;
    Mesh tray_;
    Mesh eraser_;
    Mesh felt_;
    Mesh marker_;
    Mesh markerCap_;
    Mesh markerEnd_;
    Material writingSurface_;
    Material backing_;
    Material aluminum_;
    Material darkPlastic_;
    Material feltMaterial_;
    Material markerPlastic_;
    Material markerBlue_;
    Material markerRed_;
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_BOARD_HPP
