#ifndef AULA_OPENGL_OBJECTS_TEACHER_DESK_HPP
#define AULA_OPENGL_OBJECTS_TEACHER_DESK_HPP

#include <glm/glm.hpp>

#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"

namespace objects {

// Mesa do professor (CG-64): tampo de 1,20 x 0,60 m com a superfície a 0,75 m do chão, painéis laterais, painel
// frontal e gaveteiro de 3 gavetas (geometria fixa, sem interação).
// Metros, Y para cima. Pivô no chão, no centro da mesa vista de cima. Frente (+Z) = lado da turma, com o painel
// frontal; o professor senta do lado -Z, para onde as gavetas abrem.
// Criar com o contexto OpenGL ativo e manter vivo enquanto a árvore usar seus meshes.
class TeacherDesk {
public:
    TeacherDesk();

    // Os nós da árvore apontam para os meshes deste objeto: uma cópia apontaria para os do original.
    TeacherDesk(const TeacherDesk&) = delete;
    TeacherDesk& operator=(const TeacherDesk&) = delete;

    // Pendura uma mesa em parent e devolve a raiz dela. Pode ser chamada mais de uma vez: todas as instâncias
    // usam os mesmos meshes.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement) const;

private:
    // Um mesh por material previsto para a CG-72.
    Mesh top_;          // tampo: laminado madeirado
    Mesh body_;         // painéis laterais, painel frontal e caixa do gaveteiro: MDF
    Mesh drawerFront_;  // frente de gaveta, usada pelas 3 gavetas
    Mesh handle_;       // puxador metálico, um por gaveta
    Mesh foot_;         // sapata de plástico sob os painéis, usada 6 vezes
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_TEACHER_DESK_HPP
