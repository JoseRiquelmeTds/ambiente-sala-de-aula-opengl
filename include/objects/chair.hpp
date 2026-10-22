#ifndef AULA_OPENGL_OBJECTS_CHAIR_HPP
#define AULA_OPENGL_OBJECTS_CHAIR_HPP

#include <glm/glm.hpp>

#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"

namespace objects {

// CG-60: metros, Y para cima, frente +Z; origem no chão, no centro das quatro pernas.
// Criar com contexto OpenGL ativo e manter vivo enquanto a árvore usar seus meshes.
class Chair {
public:
    Chair();

    Chair(const Chair&) = delete;
    Chair& operator=(const Chair&) = delete;

    // Cada chamada cria uma raiz independente; todas as instâncias compartilham os meshes deste objeto.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement) const;

private:
    // Placas e estrutura separadas para receber madeira / metal cinza-escuro na CG-72.
    Mesh seat_;
    Mesh backrest_;
    Mesh leg_;
    Mesh rail_;
    Mesh seatSide_;
    Mesh backSupport_;
    Mesh foot_;
    Mesh fastener_;
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_CHAIR_HPP
