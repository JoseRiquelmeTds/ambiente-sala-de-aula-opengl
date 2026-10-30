#ifndef AULA_OPENGL_SCENE_CLASSROOM_HPP
#define AULA_OPENGL_SCENE_CLASSROOM_HPP

#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "gfx/light.hpp"
#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"
#include "gfx/texture.hpp"
#include "objects/board.hpp"
#include "objects/chair.hpp"
#include "objects/student_desk.hpp"
#include "objects/fan.hpp"
#include "objects/lamp.hpp"
#include "objects/teacher_desk.hpp"
#include "objects/projection_screen.hpp"
#include "objects/projector.hpp"
#include "objects/trash_can.hpp"
#include "objects/room.hpp"

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
//  Sala de 8 x 3 x 7 m (CG-69), com seis conjuntos de carteira/cadeira e quadro na parede frontal.
//  Room e cada móvel são filhos diretos da raiz para o isolamento, compartilhando meshes por tipo.
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

    // Tecla 1 mostra a sala inteira; 2 a 9 e 0 preservam os nove primeiros alvos.
    // [ e ] percorrem todos os alvos em ciclo (hoje onze, com o quadro no fim), mesmo com cursor livre.
    // Reenquadra a câmera, que depois continua livre. Retorna true se reenquadrou neste frame
    // (o chamador não aplica movimento pendente).
    bool handleInspection(const Input& input, Camera& camera);

    // index = kWholeScene ou a posição do alvo na lista. Retorna false (sem mudar nada) se não
    // houver alvo nessa posição.
    bool selectInspection(int index, Camera& camera);
    static constexpr int kWholeScene = -1;

    const SceneNode& root() const { return root_; }
    // Luzes da sala (CG-71): uma pontual por luminária, o preenchimento e a luz ambiente.
    const Lighting& lighting() const { return lighting_; }
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
    Mesh baseMesh_;
    Mesh rodMesh_;
    Mesh armMesh_;
    // Vermelho cresce com u (para a direita) e verde com v (para cima). Verde embaixo = imagem invertida.
    Texture testTexture_;
    objects::StudentDesk studentDesk_;
    objects::Fan fan_;
    objects::Lamp lamp_;
    objects::LightSwitch lightSwitch_;
    objects::TeacherDesk teacherDesk_;
    objects::ProjectionScreen screen_;
    objects::Projector projector_;
    objects::TrashCan trashCan_;
    objects::Chair chair_;
    objects::Board board_;
    objects::Room room_;

    SceneNode root_;
    Lighting lighting_;
    std::vector<InspectionTarget> targets_;
    int currentInspection_ = kWholeScene;
};

}  // namespace scene

#endif  // AULA_OPENGL_SCENE_CLASSROOM_HPP
