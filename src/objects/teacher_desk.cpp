#include "objects/teacher_desk.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <initializer_list>

#include "gfx/primitives.hpp"

namespace objects {

namespace {

// --- Medidas gerais (docs/CONVENCOES.md § 7) -----------------------------------------------------------------
constexpr float kWidth = 1.20f;            // X
constexpr float kDepth = 0.60f;            // Z
constexpr float kHeight = 0.75f;           // do chão à superfície do tampo
constexpr float kTopThickness = 0.025f;    // tampo de MDF de 25 mm
constexpr float kPanelThickness = 0.018f;  // painéis de MDF de 18 mm
constexpr float kOverhang = 0.02f;         // quanto o tampo passa dos painéis em cada lado
constexpr float kFootHeight = 0.01f;       // sapatas: o MDF não encosta no piso (umidade)
constexpr float kFootLength = 0.05f;

// Derivadas: os painéis vão de cima das sapatas até embaixo do tampo.
constexpr float kTopY = kHeight - kTopThickness;
constexpr float kPanelHeight = kTopY - kFootHeight;
constexpr float kPanelDepth = kDepth - 2.0f * kOverhang;
constexpr float kSideX = kWidth / 2.0f - kOverhang - kPanelThickness / 2.0f;  // centro das laterais externas
constexpr float kBackZ = kPanelDepth / 2.0f - kPanelThickness / 2.0f;  // traseira alinhada à borda +Z das laterais

// --- Gaveteiro --------------------------------------------------------------------------------------------------
// Fica no lado -X, que é a direita de quem senta em -Z olhando para +Z.
constexpr float kPedestalWidth = 0.40f;                                            // de fora a fora
constexpr float kPedestalOpening = kPedestalWidth - 2.0f * kPanelThickness;        // vão entre as laterais
constexpr float kPedestalCenterX = -kWidth / 2.0f + kOverhang + kPedestalWidth / 2.0f;
constexpr float kPedestalInnerX = kPedestalCenterX + kPedestalWidth / 2.0f - kPanelThickness / 2.0f;
constexpr float kDrawerZoneBottom = 0.06f;  // embaixo da gaveta de baixo: fundo do gaveteiro + rodapé
constexpr float kPlinthRecess = 0.03f;      // rodapé recuado, como em móvel de escritório
constexpr int kDrawerCount = 3;
constexpr float kDrawerGap = 0.003f;        // folga entre as frentes, para elas não se tocarem
constexpr float kDrawerFrontHeight =
    (kTopY - kDrawerZoneBottom - (kDrawerCount + 1) * kDrawerGap) / static_cast<float>(kDrawerCount);

// --- Painel frontal (saia): esconde as pernas do professor de quem está na turma ------------------------------
constexpr float kModestyHeight = 0.40f;
constexpr float kModestyLeftX = kPedestalInnerX + kPanelThickness / 2.0f;  // encosta no gaveteiro
constexpr float kModestyRightX = kSideX - kPanelThickness / 2.0f;          // encosta na lateral +X

// --- Puxador de alça (furação padrão de 128 mm entre os parafusos) ----------------------------------------------
constexpr float kHandleSpacing = 0.128f;
constexpr float kHandleLength = 0.15f;
constexpr float kHandleRadius = 0.006f;
constexpr float kHandlePostRadius = 0.005f;
constexpr float kHandleStandoff = 0.025f;  // distância da barra até a frente da gaveta, para caber os dedos

// Tiling (docs/CONVENCOES.md § 3): metros cobertos por uma repetição da textura. Assim a textura não estica nas
// peças finas (borda do tampo, rodapé).
constexpr float kWoodTextureSize = 0.6f;
constexpr float kSmallPartTextureSize = 0.1f;  // puxador (metal) e sapatas (plástico)

glm::mat4 translated(const glm::vec3& position) {
    return glm::translate(glm::mat4(1.0f), position);
}

// Copia part para dentro de target, já transformado. Junta peças do mesmo material num mesh só (um material por mesh
// na CG-72, e menos nós na árvore).
// Só recebe translação e rotação: sem escala, mat3(transform) gira a normal sem mudar o comprimento dela, então não
// precisa da normal matrix transpose(inverse(...)); e o sentido anti-horário dos triângulos se mantém.
void append(MeshData& target, const MeshData& part, const glm::mat4& transform) {
    const GLuint offset = static_cast<GLuint>(target.vertices.size());
    const glm::mat3 rotation(transform);
    for (const Vertex& vertex : part.vertices) {
        target.vertices.push_back({glm::vec3(transform * glm::vec4(vertex.position, 1.0f)), rotation * vertex.normal,
                                   vertex.uv});
    }
    for (GLuint index : part.indices) {
        target.indices.push_back(offset + index);
    }
}

// Caixa de madeira com o pivô (centro da base) em base, como o box() já gera.
void addPanel(MeshData& data, const glm::vec3& size, const glm::vec3& base) {
    append(data, primitives::box(size, kWoodTextureSize), translated(base));
}

MeshData makeBody() {
    MeshData data;
    const glm::vec3 side(kPanelThickness, kPanelHeight, kPanelDepth);
    addPanel(data, side, {-kSideX, kFootHeight, 0.0f});           // lateral externa do gaveteiro
    addPanel(data, side, {kPedestalInnerX, kFootHeight, 0.0f});   // lateral interna do gaveteiro
    addPanel(data, side, {kSideX, kFootHeight, 0.0f});            // lateral do lado livre

    // Lado da turma (+Z): traseira do gaveteiro até o chão e, ao lado, o painel frontal só na metade de cima.
    addPanel(data, {kPedestalOpening, kPanelHeight, kPanelThickness}, {kPedestalCenterX, kFootHeight, kBackZ});
    addPanel(data, {kModestyRightX - kModestyLeftX, kModestyHeight, kPanelThickness},
             {(kModestyLeftX + kModestyRightX) / 2.0f, kTopY - kModestyHeight, kBackZ});

    // Fundo do gaveteiro (entre as frentes das gavetas e a traseira) e rodapé recuado embaixo dele.
    const float bottomY = kDrawerZoneBottom - kPanelThickness;
    addPanel(data, {kPedestalOpening, kPanelThickness, kPanelDepth - 2.0f * kPanelThickness},
             {kPedestalCenterX, bottomY, 0.0f});
    addPanel(data, {kPedestalOpening, bottomY - kFootHeight, kPanelThickness},
             {kPedestalCenterX, kFootHeight, -kPanelDepth / 2.0f + kPlinthRecess + kPanelThickness / 2.0f});
    return data;
}

// Puxador no espaço da gaveta: origem no centro da frente, barra em X e hastes saindo para +Z (para fora).
// cylinder() nasce em pé (eixo +Y): girar -90° em Z leva +Y para +X (barra) e +90° em X leva +Y para +Z (hastes).
MeshData makeHandle() {
    MeshData data;
    glm::mat4 bar = translated({-kHandleLength / 2.0f, 0.0f, kHandleStandoff});
    bar = glm::rotate(bar, glm::radians(-90.0f), {0.0f, 0.0f, 1.0f});
    append(data, primitives::cylinder(kHandleRadius, kHandleLength, 16, kSmallPartTextureSize), bar);

    for (float x : {-kHandleSpacing / 2.0f, kHandleSpacing / 2.0f}) {
        const glm::mat4 post = glm::rotate(translated({x, 0.0f, 0.0f}), glm::radians(90.0f), {1.0f, 0.0f, 0.0f});
        append(data, primitives::cylinder(kHandlePostRadius, kHandleStandoff, 12, kSmallPartTextureSize), post);
    }
    return data;
}

}  // namespace

TeacherDesk::TeacherDesk()
    : top_(primitives::box({kWidth, kTopThickness, kDepth}, kWoodTextureSize)),
      body_(makeBody()),
      drawerFront_(primitives::box({kPedestalOpening - 2.0f * kDrawerGap, kDrawerFrontHeight, kPanelThickness},
                                   kWoodTextureSize)),
      handle_(makeHandle()),
      foot_(primitives::box({kPanelThickness, kFootHeight, kFootLength}, kSmallPartTextureSize)) {}

SceneNode& TeacherDesk::addTo(SceneNode& parent, const glm::mat4& placement) const {
    // Raiz sem mesh: só carrega o placement, e as peças ficam relativas ao pivô da mesa.
    SceneNode& root = parent.addChild(nullptr, placement);
    root.addChild(&top_, translated({0.0f, kTopY, 0.0f}));
    root.addChild(&body_);

    // Uma sapata em cada ponta das 3 laterais. O mesmo mesh, só transladado.
    const float footZ = kPanelDepth / 2.0f - kFootLength / 2.0f;
    for (float x : {-kSideX, kPedestalInnerX, kSideX}) {
        root.addChild(&foot_, translated({x, 0.0f, -footZ}));
        root.addChild(&foot_, translated({x, 0.0f, footZ}));
    }

    // Gavetas embutidas entre as laterais do gaveteiro, rente à borda -Z. Model = T · R: a gaveta é modelada com a
    // frente para +Z, como todo objeto, e girada 180° em Y para abrir para o lado do professor (-Z).
    // O puxador é filho da frente: herda a posição e o giro dela, e fica no centro da face externa.
    for (int i = 0; i < kDrawerCount; ++i) {
        const float y = kDrawerZoneBottom + kDrawerGap + static_cast<float>(i) * (kDrawerFrontHeight + kDrawerGap);
        glm::mat4 drawer = translated({kPedestalCenterX, y, -kPanelDepth / 2.0f + kPanelThickness / 2.0f});
        drawer = glm::rotate(drawer, glm::radians(180.0f), {0.0f, 1.0f, 0.0f});
        SceneNode& front = root.addChild(&drawerFront_, drawer);
        front.addChild(&handle_, translated({0.0f, kDrawerFrontHeight / 2.0f, kPanelThickness / 2.0f}));
    }
    return root;
}

}  // namespace objects
