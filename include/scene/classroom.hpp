#ifndef AULA_OPENGL_SCENE_CLASSROOM_HPP
#define AULA_OPENGL_SCENE_CLASSROOM_HPP

#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"
#include "gfx/texture.hpp"
#include "objects/fan.hpp"
#include "objects/lamp.hpp"
#include "objects/teacher_desk.hpp"
#include "objects/projection_screen.hpp"
#include "objects/projector.hpp"
#include "objects/trash_can.hpp"

class Camera;
class Input;

namespace scene {

// =============================================================================
//  Classroom: a sala montada. Dona dos objetos, da árvore de SceneNode e da inspeção.
// =============================================================================
//
//  É o único lugar que liga o Input aos objetos: as teclas das interações e da inspeção
//  ficam aqui (tabela em docs/CONVENCOES.md § 8), e os objetos só expõem métodos.
//  Objeto novo: crie objects/<nome>.hpp/.cpp, guarde-o como membro, pendure-o na raiz com
//  addTo no construtor e registre um alvo de inspeção (docs/ARQUITETURA.md § 4).
//  Todo alvo é um filho direto da raiz: a inspeção esconde os outros filhos.
//
//  Cena de testes: o exemplo da estrutura de mesh (piso, cubo e objeto articulado) e os objetos já
//  modelados, em posições provisórias até a montagem da sala (CG-69).
//
//  Cria meshes e texturas: construir com o contexto OpenGL ativo e destruir antes da janela.
class Classroom {
public:
    Classroom();

    // Os nós da árvore apontam para os meshes dos membros: uma cópia apontaria para os do original.
    Classroom(const Classroom&) = delete;
    Classroom& operator=(const Classroom&) = delete;

    // Teclas das interações e animações. Uma vez por frame, com a janela em foco.
    void update(const Input& input, float deltaTime);

    // Tecla 1 mostra a sala inteira; 2 a 9, cada alvo isolado, na ordem em que foram
    // registrados (cabem 8 alvos). Reenquadra a câmera, que depois continua livre. Retorna true se reenquadrou
    // neste frame (o chamador não aplica movimento pendente).
    bool handleInspection(const Input& input, Camera& camera);

    // index = kWholeScene ou a posição do alvo na lista. Retorna false (sem mudar nada) se não
    // houver alvo nessa posição.
    bool selectInspection(int index, Camera& camera);
    static constexpr int kWholeScene = -1;

    const SceneNode& root() const { return root_; }
    // Textura da cena inteira, até existir material por peça (CG-72).
    const Texture& texture() const { return testTexture_; }

private:
    struct InspectionTarget {
        std::string name;
        const SceneNode* node;  // filho direto de root_
        glm::vec3 eye;     // de onde a câmera olha
        glm::vec3 center;  // para onde olha
    };

    // Membros na ordem de construção: meshes e textura antes da árvore, que aponta para eles
    // e é destruída primeiro.
    Mesh floorMesh_;
    Mesh cubeMesh_;
    Mesh baseMesh_;
    Mesh rodMesh_;
    Mesh armMesh_;
    // Vermelho cresce com u (para a direita) e verde com v (para cima). Verde embaixo = imagem invertida.
    Texture testTexture_;
    objects::Fan fan_;
    objects::Lamp lamp_;
    objects::LightSwitch lightSwitch_;
    objects::TeacherDesk teacherDesk_;
    objects::ProjectionScreen screen_;
    objects::Projector projector_;
    objects::TrashCan trashCan_;

    SceneNode root_;
    std::vector<InspectionTarget> targets_;
};

}  // namespace scene

#endif  // AULA_OPENGL_SCENE_CLASSROOM_HPP
