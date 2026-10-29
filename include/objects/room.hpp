#ifndef AULA_OPENGL_OBJECTS_ROOM_HPP
#define AULA_OPENGL_OBJECTS_ROOM_HPP

#include <glm/glm.hpp>

#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"

namespace objects {

// Sala (CG-69): volume interno de 8 x 3 x 7 m, origem no centro da face superior do piso.
// As espessuras crescem para fora do volume útil. Parede frontal em -Z, Y para cima.
// Criar com contexto OpenGL ativo; manter viva enquanto a árvore apontar para seus meshes.
class Room {
public:
    Room();

    Room(const Room&) = delete;
    Room& operator=(const Room&) = delete;

    // Instâncias independentes compartilham os meshes; placement é T · R, sem escala.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement) const;

    static constexpr float kWidth = 8.0f;
    static constexpr float kHeight = 3.0f;
    static constexpr float kDepth = 7.0f;

private:
    // Separação por material futuro (CG-72), sem modificar renderer ou textura da cena.
    Mesh floor_;
    Mesh endWall_;   // compartilhado pelas paredes frontal e traseira
    Mesh sideWall_;  // compartilhado pelas paredes laterais
    Mesh ceiling_;
    Mesh endSkirting_;
    Mesh sideSkirting_;
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_ROOM_HPP
