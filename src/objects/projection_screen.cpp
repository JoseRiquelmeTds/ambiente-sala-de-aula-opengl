#include "objects/projection_screen.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

#include "gfx/primitives.hpp"

namespace objects {

namespace {

// Caixa de 2,10 m que guarda o rolo, centrada na altura da origem e encostada na parede (z de 0 a 0,08).
constexpr glm::vec3 kHousingSize(2.10f, 0.08f, 0.08f);
// Suportes laterais um pouco maiores que a caixa, presos à parede.
constexpr glm::vec3 kEndCapSize(0.03f, 0.10f, 0.10f);

// Fenda por onde a lona sai: embaixo do centro da caixa. É o pivô da escala.
constexpr glm::vec3 kSlot(0.0f, -0.04f, 0.04f);

constexpr float kCanvasWidth = 2.0f;
constexpr float kCanvasHeight = 1.5f;
constexpr float kCanvasThickness = 0.004f;

// A UV da lona vai de 0 a 1 na face inteira (textureSize = 0), reservada para o slide da CG-77. A trama,
// porém, tem de repetir com o mesmo tamanho nos dois eixos, senão esticaria 2 m e 1,5 m numa imagem
// quadrada: o tiling vai no uvScale do material, uma repetição a cada 25 cm (8 x 6 na lona aberta).
constexpr float kCanvasTexturePeriod = 0.25f;

constexpr float kBarRadius = 0.012f;
constexpr float kBarLength = 2.04f;
constexpr float kCordRadius = 0.003f;
constexpr float kCordLength = 0.25f;
constexpr float kHandleRadius = 0.012f;
constexpr float kHandleLength = 0.04f;

// Abaixo disso a lona some: com escala 0 a normal matrix fica singular.
constexpr float kMinVisibleAmount = 0.001f;

// Metal e plástico repetem a textura a cada 25 cm; a lona tem UV de 0 a 1 (tiling no material, acima).
constexpr float kMaterialTextureSize = 0.25f;

glm::mat4 translated(const glm::vec3& position) {
    return glm::translate(glm::mat4(1.0f), position);
}

}  // namespace

ProjectionScreen::ProjectionScreen()
    : housing_(primitives::box(kHousingSize, kMaterialTextureSize)),
      endCap_(primitives::box(kEndCapSize, kMaterialTextureSize)),
      canvas_(primitives::box({kCanvasWidth, kCanvasHeight, kCanvasThickness})),
      bottomBar_(primitives::cylinder(kBarRadius, kBarLength, 16, kMaterialTextureSize)),
      cord_(primitives::cylinder(kCordRadius, kCordLength, 8, kMaterialTextureSize)),
      handle_(primitives::cylinder(kHandleRadius, kHandleLength, 16, kMaterialTextureSize)),
      canvasTexture_("assets/textures/lona_tela.png"),
      // Caixa de aço com pintura branco-gelo: a tinta reflete como plástico, com brilho médio.
      housingPlastic_(materials::plastic(glm::vec3(0.92f, 0.92f, 0.90f))),
      // Lona vinílica fosca: só difusa. Um pouco abaixo de 1 porque a textura já é quase branca, e
      // somando as luzes um branco puro estouraria e a trama sumiria.
      canvasMaterial_(materials::matte(glm::vec3(0.90f))),
      // Alumínio: difusa clara e reflexo alto, como o refletor da luminária.
      barMetal_(materials::metal(glm::vec3(0.55f, 0.55f, 0.57f))),
      // Cordão de nylon trançado: fosco, cinza-escuro.
      cordMatte_(materials::matte(glm::vec3(0.15f))),
      // Puxador de plástico preto: reflexo fraco, só para dar volume.
      handlePlastic_(materials::plastic(glm::vec3(0.05f))) {
    // O material só guarda o ponteiro: a textura é membro deste objeto e vive o mesmo tempo.
    canvasMaterial_.diffuseMap = &canvasTexture_;
    canvasMaterial_.uvScale = glm::vec2(kCanvasWidth, kCanvasHeight) / kCanvasTexturePeriod;
    housingPlastic_.specular = glm::vec3(0.30f);
    housingPlastic_.shininess = 24.0f;
    barMetal_.specular = glm::vec3(0.85f);
    barMetal_.shininess = 64.0f;
    handlePlastic_.specular = glm::vec3(0.20f);
    handlePlastic_.shininess = 16.0f;
}

SceneNode& ProjectionScreen::addTo(SceneNode& parent, const glm::mat4& placement) {
    if (root_ != nullptr) {
        return *root_;
    }
    root_ = &parent.addChild(nullptr, placement);
    // O material da raiz (plástico da caixa) vale para a caixa e os suportes; lona e ponta sobrescrevem.
    root_->material = &housingPlastic_;

    // A box tem o pivô na base e é centrada em z: desce meia altura e avança meia profundidade.
    root_->addChild(&housing_, translated({0.0f, -kHousingSize.y * 0.5f, kHousingSize.z * 0.5f}));
    const float capX = (kHousingSize.x + kEndCapSize.x) * 0.5f;
    for (float side : {-1.0f, 1.0f}) {
        root_->addChild(&endCap_, translated({side * capX, -kEndCapSize.y * 0.5f, kEndCapSize.z * 0.5f}));
    }

    // A lona fica abaixo do pivô (topo em y = 0): a escala da articulação recolhe para cima.
    canvasPivot_ = &root_->addChild(nullptr);
    canvasPivot_->material = &canvasMaterial_;
    canvasPivot_->addChild(&canvas_, translated({0.0f, -kCanvasHeight, 0.0f}));

    // Barra deitada em X: Rz(-90°) leva o eixo do cilindro (+Y) para +X. O topo da barra
    // encosta na borda de baixo da lona.
    bottom_ = &root_->addChild(nullptr);
    bottom_->material = &barMetal_;
    glm::mat4 barTransform = translated({-kBarLength * 0.5f, -kBarRadius, 0.0f});
    barTransform = glm::rotate(barTransform, glm::radians(-90.0f), {0.0f, 0.0f, 1.0f});
    bottom_->addChild(&bottomBar_, barTransform);
    const float cordTop = -2.0f * kBarRadius;
    SceneNode& cord = bottom_->addChild(&cord_, translated({0.0f, cordTop - kCordLength, 0.0f}));
    cord.material = &cordMatte_;
    SceneNode& handle = bottom_->addChild(&handle_, translated({0.0f, cordTop - kCordLength - kHandleLength, 0.0f}));
    handle.material = &handlePlastic_;

    applyOpenAmount();
    return *root_;
}

void ProjectionScreen::setOpenAmount(float amount) {
    if (!std::isfinite(amount)) {
        return;
    }
    openAmount_ = std::clamp(amount, 0.0f, 1.0f);
    applyOpenAmount();
}

glm::vec3 ProjectionScreen::canvasCenter() const {
    return kSlot + glm::vec3(0.0f, -kCanvasHeight * 0.5f, kCanvasThickness * 0.5f);
}

void ProjectionScreen::applyOpenAmount() {
    if (root_ == nullptr) {
        return;  // addTo aplica o valor guardado
    }
    // T · S: lido da direita para a esquerda, escala em Y em volta do topo da lona e depois leva até a fenda.
    canvasPivot_->transform = glm::scale(translated(kSlot), {1.0f, openAmount_, 1.0f});
    canvasPivot_->visible = openAmount_ > kMinVisibleAmount;
    // A ponta acompanha a borda de baixo da lona, só com translação.
    bottom_->transform = translated(kSlot + glm::vec3(0.0f, -kCanvasHeight * openAmount_, 0.0f));
}

}  // namespace objects
