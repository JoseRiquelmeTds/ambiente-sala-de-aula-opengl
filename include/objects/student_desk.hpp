#ifndef AULA_OPENGL_OBJECTS_STUDENT_DESK_HPP
#define AULA_OPENGL_OBJECTS_STUDENT_DESK_HPP

#include <glm/glm.hpp>

#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"

namespace objects {

// =============================================================================
//  Carteira do aluno (CG-62).
// =============================================================================
//
//  Tampo de MDF de 0,60 x 0,45 m com a face de cima a 0,75 m do chão, sobre uma estrutura de
//  tubo de aço com porta-livros de arame embaixo do tampo. Tubo e espessura do tampo vêm de
//  objects/furniture_style.hpp, os mesmos da cadeira.
//
//  Metros, Y para cima. Pivô no chão, no centro entre os 4 pés. Frente (+Z) é o lado onde o
//  aluno senta: é por ali que a cadeira entra e que o porta-livros abre.
//
//  Cria meshes: construir com o contexto OpenGL ativo e manter vivo enquanto a árvore da cena
//  apontar para eles.
class StudentDesk {
public:
    StudentDesk();

    // Os nós da árvore apontam para os meshes deste objeto: uma cópia apontaria para os do original.
    StudentDesk(const StudentDesk&) = delete;
    StudentDesk& operator=(const StudentDesk&) = delete;

    // Pendura uma carteira em parent, na posição placement (T · R, sem escala), e devolve a raiz dela.
    // Pode ser chamada várias vezes (uma por lugar na sala): todas as instâncias usam os mesmos meshes.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement) const;

private:
    // Um mesh por material futuro (CG-72): cada um vai receber o seu.
    Mesh frame_;  // estrutura soldada + porta-livros: metal
    Mesh top_;    // tampo: MDF com laminado
    Mesh foot_;   // ponteira de plástico, usada pelos 4 pés
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_STUDENT_DESK_HPP
