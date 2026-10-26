#ifndef AULA_OPENGL_OBJECTS_FAN_HPP
#define AULA_OPENGL_OBJECTS_FAN_HPP

#include <glm/glm.hpp>

#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"

namespace objects {

// CG-59: metros, Y para cima, frente +Z; origem no centro da face encostada na parede.
// Criar com contexto OpenGL ativo e manter vivo enquanto a árvore usar seus meshes.
class Fan {
public:
    Fan();

    Fan(const Fan&) = delete;
    Fan& operator=(const Fan&) = delete;

    // Uma instância por Fan. Chamadas seguintes devolvem a raiz já anexada, sem duplicá-la.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement);
    // Graus, inclusive antes de addTo. Só altera a articulação das pás e da tampa.
    void setRotorAngle(float degrees);
    float rotorAngle() const { return rotorAngle_; }

private:
    // Peças separadas para receber metal escuro / plástico preto na CG-72.
    Mesh mount_;
    Mesh support_;
    Mesh motor_;
    Mesh shaft_;
    Mesh blade_;
    Mesh hub_;
    Mesh frontGuard_;
    Mesh rearGuard_;
    SceneNode* root_ = nullptr;   // não donos; a árvore deve sobreviver às chamadas de setRotorAngle
    SceneNode* rotor_ = nullptr;
    float rotorAngle_ = 0.0f;
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_FAN_HPP
