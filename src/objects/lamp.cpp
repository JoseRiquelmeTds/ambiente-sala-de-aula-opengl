#include "objects/lamp.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cstddef>
#include <initializer_list>
#include <iterator>

#include "gfx/primitives.hpp"

namespace objects {

namespace {

// --- Luminária --------------------------------------------------------------
// Calha de chapa: aberta embaixo, com a tampa encostada no teto (y = 0).
constexpr float kBodyLength = 1.27f;  // X: tubo de 1,20 m + um soquete em cada ponta
constexpr float kBodyWidth = 0.25f;   // Z: cabe 2 tubos
constexpr float kBodyHeight = 0.075f;
constexpr float kSheet = 0.005f;  // espessura aparente da chapa (a real tem ~0,6 mm, invisível de longe)

// Tubo T8: 26 mm de diâmetro (8/8 de polegada) e 1,20 m de comprimento.
constexpr float kTubeLength = 1.20f;
constexpr float kTubeRadius = 0.013f;
constexpr float kTubeY = -0.040f;  // centro do tubo, abaixo do refletor
constexpr float kTubeZ = 0.060f;   // tubos em z = -kTubeZ e +kTubeZ

// Soquete encostado na tampa lateral da calha, segurando a ponta do tubo.
constexpr glm::vec3 kSocketSize(0.030f, 0.034f, 0.032f);
constexpr float kSocketBottom = kTubeY - kSocketSize.y / 2.0f;  // centrado na altura do tubo

// Aletas: lâminas transversais embaixo dos tubos, que cortam o ofuscamento visto de lado.
constexpr int kFinCount = 13;
constexpr float kFinSpacing = 0.10f;
constexpr float kFinThickness = 0.003f;
constexpr float kFinHeight = 0.017f;

// --- Interruptor ------------------------------------------------------------
constexpr glm::vec3 kPlateSize(0.08f, 0.12f, 0.008f);  // espelho padrão 4x2
constexpr float kPlateBevel = 0.003f;
constexpr glm::vec3 kKeySize(0.032f, 0.058f, 0.009f);
constexpr float kKeyBevel = 0.0015f;
constexpr float kKeyTiltDegrees = 7.0f;
// Eixo de basculamento: no centro da tecla, na face da frente do espelho.
constexpr glm::vec3 kKeyPivot(0.0f, 0.0f, kPlateSize.z);

glm::mat4 translated(const glm::vec3& offset) {
    return glm::translate(glm::mat4(1.0f), offset);
}

// Copia `source` para dentro de `target`, já movido por `transform`. Junta várias peças do mesmo
// material num mesh só, sem um SceneNode por peça. Só para translação e rotação: sem escala,
// mat3(transform) gira a normal do mesmo jeito que a posição e ela continua com comprimento 1.
void append(MeshData& target, const MeshData& source, const glm::mat4& transform) {
    const GLuint offset = static_cast<GLuint>(target.vertices.size());
    const glm::mat3 rotation(transform);
    for (const Vertex& vertex : source.vertices) {
        const glm::vec3 position(transform * glm::vec4(vertex.position, 1.0f));
        target.vertices.push_back({position, glm::normalize(rotation * vertex.normal), vertex.uv});
    }
    for (GLuint index : source.indices) {
        target.indices.push_back(offset + index);
    }
}

// Quadrilátero plano com os cantos em ordem anti-horária vistos de fora (inferior esquerdo,
// inferior direito, superior direito, superior esquerdo), como o das primitivas. A normal é
// (direita) x (cima), que aponta para fora pela regra da mão direita. UV em metros / textureSize.
void addQuad(MeshData& data, const glm::vec3& bottomLeft, const glm::vec3& bottomRight, const glm::vec3& topRight,
             const glm::vec3& topLeft, float textureSize) {
    const glm::vec3 right = bottomRight - bottomLeft;
    const glm::vec3 up = topLeft - bottomLeft;
    const glm::vec3 normal = glm::normalize(glm::cross(right, up));
    const float u = glm::length(right) / textureSize;
    const float v = glm::length(up) / textureSize;

    const GLuint base = static_cast<GLuint>(data.vertices.size());
    data.vertices.push_back({bottomLeft, normal, {0.0f, 0.0f}});
    data.vertices.push_back({bottomRight, normal, {u, 0.0f}});
    data.vertices.push_back({topRight, normal, {u, v}});
    data.vertices.push_back({topLeft, normal, {0.0f, v}});
    data.indices.insert(data.indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});
}

// Calha: tampa + 2 laterais + 2 cabeceiras, caixas no tamanho final. As laterais começam abaixo
// da tampa e as cabeceiras entre as laterais, para nenhuma face externa ficar sobreposta a outra
// no mesmo plano (o que daria z-fighting).
MeshData makeBody() {
    const float wallHeight = kBodyHeight - kSheet;
    const float halfLength = kBodyLength / 2.0f;
    const float halfWidth = kBodyWidth / 2.0f;
    MeshData data;
    append(data, primitives::box({kBodyLength, kSheet, kBodyWidth}, 0.5f), translated({0.0f, -kSheet, 0.0f}));
    for (float side : {-1.0f, 1.0f}) {
        append(data, primitives::box({kBodyLength, wallHeight, kSheet}, 0.5f),
               translated({0.0f, -kBodyHeight, side * (halfWidth - kSheet / 2.0f)}));
        append(data, primitives::box({kSheet, wallHeight, kBodyWidth - 2.0f * kSheet}, 0.5f),
               translated({side * (halfLength - kSheet / 2.0f), -kBodyHeight, 0.0f}));
    }
    return data;
}

// Refletor: perfil em "W" (corte em Z-Y) estendido ao longo de X. Cada tubo fica sob um "vale" e
// a crista do meio separa os dois, jogando a luz de cima para baixo. A face espelhada é a de
// baixo: com z crescendo no perfil, (+X) x (próximo ponto - ponto) aponta para -Y.
// As aletas entram no mesmo mesh porque são do mesmo alumínio.
MeshData makeReflector() {
    const float innerHalfLength = kBodyLength / 2.0f - kSheet;
    const float innerHalfWidth = kBodyWidth / 2.0f - kSheet;
    const glm::vec2 profile[] = {
        {-innerHalfWidth, -0.050f},  // (z, y): encosta na lateral
        {-kTubeZ, -0.012f},          // vale sobre o primeiro tubo
        {0.0f, -0.045f},             // crista entre os tubos
        {kTubeZ, -0.012f},           // vale sobre o segundo tubo
        {innerHalfWidth, -0.050f},
    };

    MeshData data;
    for (std::size_t i = 0; i + 1 < std::size(profile); ++i) {
        const glm::vec2& a = profile[i];
        const glm::vec2& b = profile[i + 1];
        addQuad(data, {-innerHalfLength, a.y, a.x}, {innerHalfLength, a.y, a.x}, {innerHalfLength, b.y, b.x},
                {-innerHalfLength, b.y, b.x}, 0.25f);
    }
    for (int i = 0; i < kFinCount; ++i) {
        const float x = (static_cast<float>(i) - (kFinCount - 1) / 2.0f) * kFinSpacing;
        append(data, primitives::box({kFinThickness, kFinHeight, 2.0f * innerHalfWidth}, 0.1f),
               translated({x, -kBodyHeight, 0.0f}));
    }
    return data;
}

// Tubo deitado ao longo de X e centrado na origem. O cilindro das primitivas nasce em pé, de
// y = 0 a y = kTubeLength: R gira -90° em Z (+Y vira +X) e T centra o tubo (T · R, R primeiro).
MeshData makeTube() {
    glm::mat4 transform = translated({-kTubeLength / 2.0f, 0.0f, 0.0f});
    transform = glm::rotate(transform, glm::radians(-90.0f), {0.0f, 0.0f, 1.0f});
    MeshData data;
    append(data, primitives::cylinder(kTubeRadius, kTubeLength, 24), transform);
    return data;
}

// Posição de cada peça repetida dentro da luminária. side = -1 ou +1.
glm::mat4 tubePlacement(float side) {
    return translated({0.0f, kTubeY, side * kTubeZ});
}

glm::mat4 socketPlacement(float sideX, float sideZ) {
    const float x = sideX * (kTubeLength + kSocketSize.x) / 2.0f;  // logo depois da ponta do tubo
    return translated({x, kSocketBottom, sideZ * kTubeZ});
}

// Placa com a borda chanfrada: costas de width x height em z = 0 e frente menor (recuada `bevel` em
// cada lado) em z = depth. Serve para o espelho e para a tecla. As 4 laterais inclinadas têm
// normais para fora e para a frente, por isso pegam luz diferente da frente e marcam a borda.
MeshData makeBeveledPlate(const glm::vec3& size, float bevel, float textureSize) {
    const float w = size.x / 2.0f;
    const float h = size.y / 2.0f;
    const float fw = w - bevel;
    const float fh = h - bevel;
    const float d = size.z;
    MeshData data;
    addQuad(data, {-fw, -fh, d}, {fw, -fh, d}, {fw, fh, d}, {-fw, fh, d}, textureSize);  // frente (+Z)
    addQuad(data, {w, -h, 0}, {-w, -h, 0}, {-w, h, 0}, {w, h, 0}, textureSize);          // costas (-Z)
    addQuad(data, {-fw, fh, d}, {fw, fh, d}, {w, h, 0}, {-w, h, 0}, textureSize);        // cima
    addQuad(data, {-w, -h, 0}, {w, -h, 0}, {fw, -fh, d}, {-fw, -fh, d}, textureSize);    // baixo
    addQuad(data, {fw, -fh, d}, {w, -h, 0}, {w, h, 0}, {fw, fh, d}, textureSize);        // direita
    addQuad(data, {-w, -h, 0}, {-fw, -fh, d}, {-fw, fh, d}, {-w, h, 0}, textureSize);    // esquerda
    return data;
}

// T · R: gira em torno do eixo X que passa pelo pivô, não em torno da origem do interruptor.
// Ângulo negativo afunda a metade de cima da tecla (ligado); positivo, a de baixo (desligado).
glm::mat4 keyJointTransform(bool on) {
    const float degrees = on ? -kKeyTiltDegrees : kKeyTiltDegrees;
    return glm::rotate(translated(kKeyPivot), glm::radians(degrees), {1.0f, 0.0f, 0.0f});
}

}  // namespace

Lamp::Lamp()
    : body_(makeBody()),
      reflector_(makeReflector()),
      tube_(makeTube()),
      socket_(primitives::box(kSocketSize, 0.05f)) {}

SceneNode& Lamp::addTo(SceneNode& parent, const glm::mat4& placement) const {
    SceneNode& root = parent.addChild(&body_, placement);
    root.addChild(&reflector_);
    for (float sideZ : {-1.0f, 1.0f}) {
        root.addChild(&tube_, tubePlacement(sideZ));
        for (float sideX : {-1.0f, 1.0f}) {
            root.addChild(&socket_, socketPlacement(sideX, sideZ));
        }
    }
    return root;
}

LightSwitch::LightSwitch()
    : plate_(makeBeveledPlate(kPlateSize, kPlateBevel, 0.08f)), key_(makeBeveledPlate(kKeySize, kKeyBevel, 0.08f)) {}

SceneNode& LightSwitch::addTo(SceneNode& parent, const glm::mat4& placement) {
    if (root_ != nullptr) {
        return *root_;
    }
    root_ = &parent.addChild(&plate_, placement);
    // A tecla nasce com as costas em z = 0 e centrada em X e Y, então o eixo do pivô passa pelo
    // meio dela: basta pendurá-la na articulação, sem deslocamento.
    keyJoint_ = &root_->addChild(nullptr, keyJointTransform(on_));
    keyJoint_->addChild(&key_);
    return *root_;
}

void LightSwitch::setOn(bool on) {
    on_ = on;
    if (keyJoint_ != nullptr) {
        keyJoint_->transform = keyJointTransform(on_);
    }
}

}  // namespace objects
