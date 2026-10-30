#include "scene/classroom.hpp"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <cstddef>
#include <iterator>

#include "core/camera.hpp"
#include "core/input.hpp"
#include "gfx/primitives.hpp"

namespace scene {

namespace {

// Vista de dentro, pelo corredor traseiro; main também a seleciona na inicialização.
constexpr glm::vec3 kSceneEye(3.3f, 2.3f, 3.1f);
constexpr glm::vec3 kSceneCenter(-0.4f, 0.9f, -0.7f);

// Medidas da sala (docs/CONVENCOES.md § 7): origem no centro do piso, parede do quadro em -Z.
constexpr float kFrontWallZ = -objects::Room::kDepth / 2.0f;
constexpr float kCeilingHeight = objects::Room::kHeight;

// Caixa da tela retrátil no alto da parede do quadro: a lona aberta vai de 2,82 a 1,32 m do chão,
// na frente do quadro (0,9 a 2,1 m).
constexpr glm::vec3 kScreenMount(0.0f, 2.86f, kFrontWallZ);

// Projetor no teto, a ~3 m da tela: throw ratio de ~1,5 para a imagem de 2,0 m cobrir a lona.
constexpr glm::vec3 kProjectorMount(0.0f, kCeilingHeight, -0.5f);

// Mesmos eixos para carteira e cadeira: a cadeira fica atrás do tampo, do lado +Z da carteira.
constexpr float kStudentColumns[] = {-1.25f, 1.25f};
constexpr float kStudentRows[] = {-0.85f, 0.65f, 2.15f};
constexpr float kChairOffsetZ = 0.55f;
constexpr float kLampColumns[] = {-1.6f, 1.6f};
constexpr float kLampRows[] = {-1.5f, 1.5f};

// --- Iluminação (CG-71), técnica em docs/ILUMINACAO.md ---------------------
// Tubo fluorescente branco neutro (puxando de leve para o quente), com pouco mais de metade da
// força máxima: o centro da sala recebe as quatro luminárias somadas e, com 1,0, o piso e as
// paredes estouravam para branco.
constexpr glm::vec3 kLampLightColor(0.55f, 0.53f, 0.50f);

// Atenuação 1 / (1 + 0,14·d + 0,07·d²), d em metros. Na sala de 3 m de pé-direito, o tampo de uma
// carteira fica a ~2,2 m da luminária mais próxima (fator ~0,60) e o piso logo abaixo a ~2,9 m
// (~0,50); o canto mais distante, a ~8 m, recebe ~0,15. Assim o centro da sala fica claro e os
// cantos escurecem aos poucos, sem a queda brusca de uma atenuação só quadrática. Os termos se
// igualam em d = 2 m: antes pesa o linear (queda suave), depois o quadrático (a luz morre antes
// de atravessar a sala inteira).
constexpr float kLampConstant = 1.0f;
constexpr float kLampLinear = 0.14f;
constexpr float kLampQuadratic = 0.07f;

// Luz ambiente: piso de claridade para nenhuma face ficar preta (luz rebatida nas paredes).
// Baixa o bastante para as faces voltadas às lâmpadas continuarem mais claras que as outras.
constexpr glm::vec3 kAmbientLight(0.25f, 0.25f, 0.26f);

// Ambiente com as lâmpadas apagadas (CG-73). A luz ambiente aproxima a luz rebatida nas paredes, e
// parte dela vem das próprias lâmpadas: com elas apagadas, sobra só a claridade do dia rebatida. Por
// isso cai para cerca de 80% e puxa para o azul do dia. Não vai a zero: junto com o preenchimento,
// nenhuma face fica preta (a sala de dia, com a luz apagada, continua visível). Quem escurece de
// verdade a sala são as 4 luzes pontuais que saem da conta.
constexpr glm::vec3 kAmbientLightOff(0.20f, 0.21f, 0.23f);

// Luz de preenchimento: direcional, representa a claridade do dia (a sala não tem janela modelada).
// Vem da esquerda (-X) de quem olha para o quadro, lado onde as salas costumam ter janelas, viaja
// para +X e desce. Fraca e levemente azulada: só dá volume às faces que as lâmpadas iluminam de lado.
constexpr glm::vec3 kFillDirection(1.0f, -0.6f, 0.15f);
constexpr glm::vec3 kFillColor(0.16f, 0.17f, 0.19f);

// Ponto no espaço do objeto levado para o mundo: o enquadramento acompanha o placement.
glm::vec3 toWorld(const glm::mat4& placement, const glm::vec3& local) {
    return glm::vec3(placement * glm::vec4(local, 1.0f));
}

}  // namespace

Classroom::Classroom()
    : baseMesh_(primitives::cylinder(0.25f, 0.05f)),
      rodMesh_(primitives::cylinder(0.025f, 1.0f, 16)),
      armMesh_(primitives::box({0.5f, 0.04f, 0.04f})),
      demoWood_(materials::wood()),
      demoMetal_(materials::metal()),
      demoPlastic_(materials::plastic()) {
    room_.addTo(root_, glm::mat4(1.0f));

    lighting_.ambient = kAmbientLight;
    lighting_.fill.direction = kFillDirection;
    lighting_.fill.color = kFillColor;

    // Duas colunas, três fileiras: corredor central de 1,90 m e passo de 1,50 m entre fileiras.
    // No modelo real, +Z é o lado do ALUNO (porta-livros aberto), não a direção do olhar.
    // Sem giro, o aluno fica atrás da carteira em +Z e olha para a tela em -Z.
    // Cada instância é filha direta da raiz: a inspeção da primeira esconde as outras cinco.
    for (std::size_t i = 0; i < std::size(kStudentRows); ++i) {
        for (std::size_t j = 0; j < std::size(kStudentColumns); ++j) {
            const float z = kStudentRows[i];
            const float x = kStudentColumns[j];
            const glm::mat4 placement = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z));
            const SceneNode& desk = studentDesk_.addTo(root_, placement);
            if (i == 0 && j == 0) {
                targets_.push_back({"carteira", &desk, toWorld(placement, {0.7f, 1.1f, 1.1f}),
                                    toWorld(placement, {0.0f, 0.5f, 0.0f})});  // tecla 2
            }
        }
    }

    // Objeto composto: base -> haste -> articulação (sem mesh) -> braço.
    // Mover a base leva todo o resto junto; girar a articulação só mexe no braço.
    const glm::mat4 demoPlacement = glm::translate(glm::mat4(1.0f), glm::vec3(2.9f, 0.0f, 1.8f));
    SceneNode& base = root_.addChild(&baseMesh_, demoPlacement);
    SceneNode& rod = base.addChild(&rodMesh_, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.05f, 0.0f)));

    // Pivô no topo da haste (y = 1 no espaço da haste), girado 45° em Y e 20° para cima em Z.
    glm::mat4 jointTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    jointTransform = glm::rotate(jointTransform, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    jointTransform = glm::rotate(jointTransform, glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    SceneNode& joint = rod.addChild(nullptr, jointTransform);

    // O braço tem o pivô na base e centrado em x; deslocado 0,25 m para começar no pivô
    // em vez de atravessá-lo, e 0,02 m para baixo para o eixo do braço passar pelo pivô.
    joint.addChild(&armMesh_, glm::translate(glm::mat4(1.0f), glm::vec3(0.25f, -0.02f, 0.0f)));

    // Materiais (CG-72): base de madeira, haste de metal e braço de plástico. O plástico fica na
    // articulação (sem mesh) e o braço o herda; um filho novo da articulação também seria plástico.
    base.material = &demoWood_;
    rod.material = &demoMetal_;
    joint.material = &demoPlastic_;
    targets_.push_back({"objeto articulado", &base, toWorld(demoPlacement, {1.2f, 1.4f, 2.0f}),
                        toWorld(demoPlacement, {0.1f, 0.6f, 0.0f})});  // tecla 3

    // Fixação na parede esquerda: Ry(+90°) leva a frente +Z para +X, para dentro da sala.
    // O centro do rotor fica 0,16 m acima do pivô; a grade termina abaixo de Y = 2,79 m.
    glm::mat4 fanPlacement = glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 2.35f, -0.4f));
    fanPlacement = glm::rotate(fanPlacement, glm::radians(90.0f), glm::vec3(0, 1, 0));
    const SceneNode& fan = fan_.addTo(root_, fanPlacement);
    // Enquadramentos no espaço do objeto acompanham também uma futura mudança de orientação.
    const glm::vec3 fanEye = glm::vec3(fanPlacement * glm::vec4(0.48f, 0.40f, 1.10f, 1.0f));
    const glm::vec3 fanCenter = glm::vec3(fanPlacement * glm::vec4(0, 0.13f, 0.23f, 1.0f));
    targets_.push_back({"ventilador", &fan, fanEye, fanCenter});  // tecla 4

    // Face superior em Y = 3: o modelo desce em -Y. Quatro instâncias, longe do projetor central.
    for (std::size_t i = 0; i < std::size(kLampRows); ++i) {
        for (std::size_t j = 0; j < std::size(kLampColumns); ++j) {
            const float z = kLampRows[i];
            const float x = kLampColumns[j];
            const glm::mat4 placement = glm::translate(glm::mat4(1.0f), glm::vec3(x, kCeilingHeight, z));
            const SceneNode& lamp = lamp_.addTo(root_, placement);
            // Uma luz pontual por luminária, no ponto da luminária levado para o mundo.
            PointLight light;
            light.position = toWorld(placement, objects::Lamp::lightOrigin());
            light.color = kLampLightColor;
            light.constant = kLampConstant;
            light.linear = kLampLinear;
            light.quadratic = kLampQuadratic;
            lighting_.pointLights.push_back(light);
            if (i == 0 && j == 0) {
                targets_.push_back({"lâmpada", &lamp, toWorld(placement, {0.5f, -0.8f, 1.1f}),
                                    toWorld(placement, {0.0f, -0.04f, 0.0f})});  // tecla 5, de baixo
            }
        }
    }

    // Parede direita, região traseira: Ry(-90°) leva a frente +Z para -X, voltada ao interior.
    glm::mat4 switchPlacement = glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 1.1f, 2.8f));
    switchPlacement = glm::rotate(switchPlacement, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const SceneNode& lightSwitch = lightSwitch_.addTo(root_, switchPlacement);
    targets_.push_back({"interruptor", &lightSwitch, toWorld(switchPlacement, {0.1f, 0.06f, 0.26f}),
                        toWorld(switchPlacement, {0.0f, 0.0f, 0.01f})});  // tecla 6

    // Painel frontal +Z para a turma; 0,85 m entre tampo e parede para circulação do professor.
    const glm::mat4 deskPlacement = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.35f));
    const SceneNode& teacherDesk = teacherDesk_.addTo(root_, deskPlacement);
    // Enquadramento no espaço da mesa, do lado do professor (-Z) para mostrar as gavetas: acompanha a mesa se a
    // posição ou a orientação mudarem.
    const glm::vec3 deskEye = glm::vec3(deskPlacement * glm::vec4(-1.0f, 1.2f, -1.5f, 1.0f));
    const glm::vec3 deskCenter = glm::vec3(deskPlacement * glm::vec4(0.0f, 0.4f, 0.0f, 1.0f));
    targets_.push_back({"mesa do professor", &teacherDesk, deskEye, deskCenter});  // tecla 7
    // Tela retrátil (CG-66): frente (+Z do objeto) já aponta para dentro da sala, só translação.
    const glm::mat4 screenPlacement = glm::translate(glm::mat4(1.0f), kScreenMount);
    const SceneNode& screen = screen_.addTo(root_, screenPlacement);
    targets_.push_back({"tela retrátil", &screen, toWorld(screenPlacement, {0.9f, -0.5f, 2.6f}),
                        toWorld(screenPlacement, {0.0f, -0.8f, 0.0f})});  // tecla 8

    // Projetor (CG-65): girado 180° em Y para a frente (+Z do objeto) olhar para a tela, em -Z.
    glm::mat4 projectorPlacement = glm::translate(glm::mat4(1.0f), kProjectorMount);
    projectorPlacement = glm::rotate(projectorPlacement, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const SceneNode& projector = projector_.addTo(root_, projectorPlacement);
    // Alinhamento: centro da lona, do espaço da tela para o mundo e do mundo para o espaço do projetor.
    // Mover a tela ou o projetor realinha sem mudar nenhum ângulo à mão.
    const glm::vec3 screenCenter = toWorld(screenPlacement, screen_.canvasCenter());
    projector_.aimAt(toWorld(glm::inverse(projectorPlacement), screenCenter));
    targets_.push_back({"projetor", &projector, toWorld(projectorPlacement, {0.55f, -0.35f, 0.75f}),
                        toWorld(projectorPlacement, {0.0f, -0.22f, 0.0f})});  // tecla 9

    // Lixeira (CG-67): no chão, ao lado da região do quadro, fora da circulação da mesa.
    // Simétrica em Y, então basta a translação.
    const glm::mat4 trashCanPlacement = glm::translate(glm::mat4(1.0f), glm::vec3(2.6f, 0.0f, -3.1f));
    const SceneNode& trashCan = trashCan_.addTo(root_, trashCanPlacement);
    // Enquadramento de cima e de lado, para ver o interior oco e a espessura da borda.
    targets_.push_back({"lixeira", &trashCan, toWorld(trashCanPlacement, {-0.45f, 0.85f, 0.55f}),
                        toWorld(trashCanPlacement, {0.0f, 0.18f, 0.0f})});  // tecla 0

    // Cadeiras (CG-60): Ry(180°) leva a frente local +Z para -Z, olhando para o quadro.
    // O recuo de 0,55 m deixa 0,125 m entre a frente do assento e a borda do tampo.
    // Um só alvo representativo, depois dos nove anteriores; os outros cinco são irmãos independentes.
    for (std::size_t i = 0; i < std::size(kStudentRows); ++i) {
        for (std::size_t j = 0; j < std::size(kStudentColumns); ++j) {
            const float z = kStudentRows[i];
            const float x = kStudentColumns[j];
            glm::mat4 placement = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z + kChairOffsetZ));
            placement = glm::rotate(placement, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            const SceneNode& chair = chair_.addTo(root_, placement);
            if (i == 0 && j == 0) {
                targets_.push_back({"cadeira", &chair, toWorld(placement, {0.72f, 0.80f, 0.98f}),
                                    toWorld(placement, {0.0f, 0.42f, -0.03f})});  // décimo alvo, [ e ]
            }
        }
    }

    // Quadro (CG-61): centro da fixação na parede, bordas a 0,90 e 2,10 m do chão.
    // Corpo/moldura ficam atrás da lona; bandeja e acessórios abaixo da ponta da tela aberta.
    // Fixação definitiva: a moldura fica ao menos 14 mm atrás da lona, sem mover tela nem projetor.
    const glm::mat4 boardPlacement = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, kFrontWallZ));
    const SceneNode& board = board_.addTo(root_, boardPlacement);
    targets_.push_back({"quadro", &board, toWorld(boardPlacement, {0.85f, 0.65f, 3.10f}),
                        toWorld(boardPlacement, {0.0f, -0.03f, 0.04f})});  // 11º alvo, após a cadeira: [ e ]

    // Estado inicial: sala acesa, com a tecla do interruptor já na posição "ligado" e os tubos brilhando.
    // Depois das luminárias e do interruptor, que esta chamada altera.
    setLightOn(true);
}

void Classroom::update(const Input& input, float /*deltaTime*/) {
    // keyPressed vale só no frame em que a tecla desce (borda): segurar L não fica piscando a luz.
    if (input.keyPressed(GLFW_KEY_L)) {
        setLightOn(!lightOn_);
    }
    // Interações (CG-75, CG-76, CG-77) e animações entram aqui.
}

void Classroom::setLightOn(bool on) {
    lightOn_ = on;

    // Única transformação da interação: a tecla bascula (rotação da articulação em torno do eixo X).
    lightSwitch_.setOn(on);

    // Luz desligada não é enviada ao shader (o renderer só manda as com enabled = true), então
    // uPointLightCount vai de 4 a 0 e o laço das luzes pontuais do fragment shader não roda.
    for (PointLight& light : lighting_.pointLights) {
        light.enabled = on;
    }

    // O tubo aceso brilha sozinho (emissive); apagado, volta a ser vidro iluminado pela sala.
    lamp_.setLit(on);
    lighting_.ambient = on ? kAmbientLight : kAmbientLightOff;
}

bool Classroom::handleInspection(const Input& input, Camera& camera) {
    // GLFW_KEY_0 a GLFW_KEY_9 são consecutivos. 1 = sala inteira, 2 = primeiro alvo, ..., 9 = oitavo e
    // 0 = nono (fica depois do 9 no teclado).
    for (int digit = 0; digit <= 9; ++digit) {
        if (input.keyPressed(GLFW_KEY_0 + digit)) {
            if (digit == 1) {
                return selectInspection(kWholeScene, camera);
            }
            return selectInspection(digit == 0 ? 8 : digit - 2, camera);
        }
    }
    // A cena inteira não faz parte do ciclo. Partindo dela, ] vai ao primeiro e [ ao último.
    const int targetCount = static_cast<int>(targets_.size());
    if (targetCount > 0) {
        if (input.keyPressed(GLFW_KEY_LEFT_BRACKET)) {
            const int previous = currentInspection_ <= 0 ? targetCount - 1 : currentInspection_ - 1;
            return selectInspection(previous, camera);
        }
        if (input.keyPressed(GLFW_KEY_RIGHT_BRACKET)) {
            const int next = (currentInspection_ + 1) % targetCount;
            return selectInspection(next, camera);
        }
    }
    return false;
}

bool Classroom::selectInspection(int index, Camera& camera) {
    if (index == kWholeScene) {
        for (std::size_t i = 0; i < root_.childCount(); ++i) {
            root_.child(i).visible = true;
        }
        camera.lookAt(kSceneEye, kSceneCenter);
        currentInspection_ = kWholeScene;
        return true;
    }
    if (index < 0 || index >= static_cast<int>(targets_.size())) {
        return false;  // tecla sem alvo: mantém a visão atual
    }
    const InspectionTarget& target = targets_[static_cast<std::size_t>(index)];
    for (std::size_t i = 0; i < root_.childCount(); ++i) {
        SceneNode& node = root_.child(i);
        node.visible = &node == target.node;
    }
    camera.lookAt(target.eye, target.center);
    currentInspection_ = index;
    return true;
}

}  // namespace scene
