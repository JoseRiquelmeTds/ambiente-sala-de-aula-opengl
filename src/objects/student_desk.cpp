#include "objects/student_desk.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <initializer_list>

#include "gfx/primitives.hpp"
#include "objects/furniture_style.hpp"

namespace objects {

namespace {

// --- Medidas (metros) --------------------------------------------------------

// Tampo: medidas de referência de docs/CONVENCOES.md § 7.
constexpr float kTopWidth = 0.60f;  // em X
constexpr float kTopDepth = 0.45f;  // em Z
constexpr float kHeight = 0.75f;    // do chão até a face de cima do tampo
constexpr float kTopY = kHeight - furniture::kBoardThickness;  // face de baixo do tampo

// Eixo dos pés a 3 cm da borda do tampo: o tampo sobra um pouco em volta da estrutura.
constexpr float kLegX = kTopWidth / 2.0f - 0.03f;
constexpr float kLegZ = kTopDepth / 2.0f - 0.03f;

// Altura do eixo de cada travessa.
constexpr float kUpperRailY = kTopY - furniture::kTubeRadius;  // encostada embaixo do tampo, que se apoia nela
constexpr float kShelfY = 0.62f;      // fundo do porta-livros: 11 cm de vão até o tampo e o joelho passa embaixo
constexpr float kLowerRailY = 0.15f;  // travessa baixa, entre o pé da frente e o de trás

// O porta-livros vai do fundo até 10 cm à frente do centro (cerca de 0,30 m de fundura). Não chega
// à frente da carteira para não bater nas pernas do aluno.
constexpr float kShelfFrontZ = 0.10f;

// Gradil do porta-livros: arame de aço de 8 mm, mais fino que o tubo da estrutura.
constexpr float kWireRadius = 0.004f;
constexpr int kWireSegments = 8;
constexpr int kWireCount = 7;  // no fundo e na parede de trás, igualmente espaçados entre os pés

// Ponteira: copo de plástico que encaixa por fora do tubo (parede de 2 mm) e protege o piso.
constexpr float kFootRadius = furniture::kTubeRadius + 0.002f;
constexpr float kFootHeight = 0.025f;

// textureSize das primitivas: quantos metros uma repetição da textura cobre (tiling, não estica).
// O do tampo é furniture::kBoardTextureSize, o mesmo das placas da cadeira. Tubo e ponteira não
// têm textura (só cor), mas a UV continua em metros para o caso de ganharem uma.
constexpr float kMetalTextureSize = 0.2f;
constexpr float kPlasticTextureSize = 0.1f;

// --- Geometria ---------------------------------------------------------------

// Copia `piece` para dentro de `target`, com os vértices levados por `transform`.
// Assim a estrutura inteira vira um mesh só (um draw), em vez de um SceneNode por tubo.
// A transformação aqui só gira e translada: a normal gira com a mesma matriz e continua com
// comprimento 1. Com escala não uniforme seria preciso a inversa transposta (a normal matrix).
void appendTransformed(MeshData& target, const MeshData& piece, const glm::mat4& transform) {
    const GLuint offset = static_cast<GLuint>(target.vertices.size());
    const glm::mat3 rotation(transform);
    for (const Vertex& vertex : piece.vertices) {
        const glm::vec3 position = glm::vec3(transform * glm::vec4(vertex.position, 1.0f));
        target.vertices.push_back({position, rotation * vertex.normal, vertex.uv});
    }
    // Os índices da peça contam a partir de 0: somam o número de vértices que já estavam antes.
    for (GLuint index : piece.indices) {
        target.indices.push_back(offset + index);
    }
}

// Tubo reto de `start` a `end`. O cilindro das primitivas nasce em pé (eixo +Y, base na origem):
// primeiro gira o eixo +Y até a direção do tubo, depois leva a base até `start`, ou seja,
// transform = T(start) · R. Para um tubo vertical, `start` é a ponta de baixo.
void addTube(MeshData& data, const glm::vec3& start, const glm::vec3& end, float radius, int segments) {
    const glm::vec3 up(0.0f, 1.0f, 0.0f);
    const glm::vec3 direction = glm::normalize(end - start);
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), start);

    // Girar em torno de up x direction (perpendicular aos dois) pelo ângulo entre eles leva up
    // até direction, pela regra da mão direita.
    const glm::vec3 axis = glm::cross(up, direction);
    if (glm::length(axis) > 1e-6f) {
        const float angle = std::acos(glm::clamp(glm::dot(up, direction), -1.0f, 1.0f));
        transform = glm::rotate(transform, angle, glm::normalize(axis));
    } else if (direction.y < 0.0f) {
        // Tubo para baixo: up e direction são opostos e o produto vetorial também é nulo. Qualquer
        // eixo perpendicular a up serve; 180° em X leva +Y para -Y. Tubo para cima: não gira.
        transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    const float length = glm::length(end - start);
    appendTransformed(data, primitives::cylinder(radius, length, segments, kMetalTextureSize), transform);
}

// Estrutura metálica: 4 pés, travessas e o gradil do porta-livros, num mesh só.
// Cada travessa termina no eixo do tubo em que encosta: a ponta fica escondida dentro dele, como
// numa solda, e não aparece nenhuma tampa de cilindro solta.
MeshData makeFrame() {
    const float tube = furniture::kTubeRadius;
    const int segments = furniture::kTubeSegments;
    MeshData data;

    for (float x : {-kLegX, kLegX}) {
        for (float z : {-kLegZ, kLegZ}) {
            // De 1 cm dentro da ponteira até 1 mm abaixo do tampo. Se a tampa do tubo ficasse no
            // mesmo plano da face de baixo do tampo, as duas brigariam no depth buffer (z-fighting).
            addTube(data, {x, kFootHeight - 0.01f, z}, {x, kTopY - 0.001f, z}, tube, segments);
        }
        // Laterais (ao longo de Z), ligando o pé de trás ao da frente: apoio do tampo, do
        // porta-livros e travessa baixa.
        for (float y : {kUpperRailY, kShelfY, kLowerRailY}) {
            addTube(data, {x, y, -kLegZ}, {x, y, kLegZ}, tube, segments);
        }
    }

    // Travessas ao longo de X: atrás, sob o tampo e no fundo do porta-livros; na frente, só a
    // borda do porta-livros. A frente fica aberta embaixo para as pernas do aluno.
    addTube(data, {-kLegX, kUpperRailY, -kLegZ}, {kLegX, kUpperRailY, -kLegZ}, tube, segments);
    addTube(data, {-kLegX, kShelfY, -kLegZ}, {kLegX, kShelfY, -kLegZ}, tube, segments);
    addTube(data, {-kLegX, kShelfY, kShelfFrontZ}, {kLegX, kShelfY, kShelfFrontZ}, tube, segments);

    // Gradil: arames no fundo (de trás para a frente) e na parede de trás (de baixo para cima).
    for (int i = 1; i <= kWireCount; ++i) {
        const float x = -kLegX + 2.0f * kLegX * static_cast<float>(i) / static_cast<float>(kWireCount + 1);
        addTube(data, {x, kShelfY, -kLegZ}, {x, kShelfY, kShelfFrontZ}, kWireRadius, kWireSegments);
        addTube(data, {x, kShelfY, -kLegZ}, {x, kUpperRailY, -kLegZ}, kWireRadius, kWireSegments);
    }
    return data;
}

glm::mat4 translated(const glm::vec3& position) {
    return glm::translate(glm::mat4(1.0f), position);
}

}  // namespace

StudentDesk::StudentDesk()
    : frame_(makeFrame()),
      top_(primitives::box({kTopWidth, furniture::kBoardThickness, kTopDepth}, furniture::kBoardTextureSize)),
      foot_(primitives::cylinder(kFootRadius, kFootHeight, furniture::kTubeSegments, kPlasticTextureSize)),
      boardTexture_(furniture::kBoardTexturePath),
      tube_(furniture::tubeMaterial()),
      board_(furniture::boardMaterial()),
      // Borracha/plástico preto fosco: reflexo fraco, só para a ponteira não parecer um furo.
      footPlastic_(materials::plastic(glm::vec3(0.05f))) {
    // O material só guarda o ponteiro: a textura é membro deste objeto e vive o mesmo tempo.
    board_.diffuseMap = &boardTexture_;
    footPlastic_.specular = glm::vec3(0.20f);
    footPlastic_.shininess = 16.0f;
}

SceneNode& StudentDesk::addTo(SceneNode& parent, const glm::mat4& placement) const {
    // A raiz é a estrutura: o tampo e as ponteiras são filhos dela, então a model de cada peça é
    // placement · transform e mover a carteira leva tudo junto.
    // O material da raiz (o tubo) é herdado por quem não tem o seu; tampo e ponteiras sobrescrevem.
    SceneNode& root = parent.addChild(&frame_, placement);
    root.material = &tube_;

    // A box tem o pivô na base: basta subir até a face de baixo do tampo.
    SceneNode& top = root.addChild(&top_, translated({0.0f, kTopY, 0.0f}));
    top.material = &board_;

    // O mesmo mesh de ponteira nos 4 pés: cada nó só muda a posição.
    for (float x : {-kLegX, kLegX}) {
        for (float z : {-kLegZ, kLegZ}) {
            root.addChild(&foot_, translated({x, 0.0f, z})).material = &footPlastic_;
        }
    }
    return root;
}

}  // namespace objects
