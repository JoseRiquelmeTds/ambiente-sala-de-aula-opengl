#include "gfx/renderer.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <cstdio>
#include <memory>
#include <string>

#include "core/camera.hpp"
#include "gfx/light.hpp"
#include "gfx/material.hpp"
#include "gfx/scene_node.hpp"
#include "gfx/shader.hpp"
#include "gfx/texture.hpp"

namespace renderer {

namespace {

// Modo do fragment shader básico: 0 = iluminado (Blinn-Phong), 1 = normal como cor,
// 2 = UV como cor, 3 = cor do material sem luz. Os modos 1 a 3 servem para depurar normais e UVs.
constexpr int kDebugMode = 0;

// Unidade de textura do slide do projetor (CG-77). A 0 é da cor difusa do material
// (kDiffuseTextureUnit), religada por Material::apply a cada peça; a 1 fica com o slide o frame todo.
constexpr int kProjectorTextureUnit = 1;

std::unique_ptr<Shader> basicShader;
float aspectRatio = 1.0f;
bool drawable = false;
bool warnedTooManyLights = false;

// Material das peças que não têm material nem herdam um: objetos ainda sem material aparecem
// em cinza claro fosco, iluminados, em vez de pretos ou com a cor da peça desenhada antes.
const Material kDefaultMaterial = materials::neutral();

// Envia as luzes para os uniforms do shader (nomes e structs em shaders/basic.frag).
void setLighting(const Shader& shader, const Lighting& lighting) {
    shader.setVec3("uAmbient", lighting.ambient);
    shader.setVec3("uWindowLight.direction", lighting.window.direction);
    shader.setVec3("uWindowLight.color", lighting.window.color);

    // Só as luzes ligadas entram no array, uma atrás da outra; uPointLightCount diz ao shader
    // quantas posições valem. Assim desligar uma luz não exige mudar o shader.
    int count = 0;
    for (const PointLight& light : lighting.pointLights) {
        if (!light.enabled) {
            continue;
        }
        if (count == kMaxPointLights) {
            if (!warnedTooManyLights) {
                std::fprintf(stderr, "[Renderer] Aviso: mais de %d luzes pontuais ligadas; as excedentes são "
                                     "ignoradas.\n",
                             kMaxPointLights);
                warnedTooManyLights = true;
            }
            break;
        }
        const std::string prefix = "uPointLights[" + std::to_string(count) + "].";
        shader.setVec3(prefix + "position", light.position);
        shader.setVec3(prefix + "color", light.color);
        shader.setFloat(prefix + "constant", light.constant);
        shader.setFloat(prefix + "linear", light.linear);
        shader.setFloat(prefix + "quadratic", light.quadratic);
        ++count;
    }
    shader.setInt("uPointLightCount", count);

    // Projetor (CG-77): sem slide válido não há o que projetar, mesmo ligado.
    const ProjectorLight& projector = lighting.projector;
    const bool projectorOn = projector.enabled && projector.image != nullptr && projector.image->isValid();
    shader.setInt("uProjector.enabled", projectorOn ? 1 : 0);
    if (projectorOn) {
        // bind troca a unidade ativa para a 1; Material::apply liga a textura difusa dizendo a
        // unidade 0 explicitamente, então a ordem não importa.
        projector.image->bind(kProjectorTextureUnit);
        shader.setMat4("uProjector.viewProjection", projector.viewProjection);
        shader.setVec3("uProjector.position", projector.position);
        shader.setVec3("uProjector.color", projector.color);
        shader.setFloat("uProjector.referenceDistance", projector.referenceDistance);
    }
    // Sempre, mesmo desligado: o sampler recebe o NÚMERO da unidade e, sem isso, ficaria na 0, a da
    // textura difusa.
    shader.setInt("uProjector.image", kProjectorTextureUnit);
}

}  // namespace

void init() {
    glClearColor(0.10f, 0.10f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    basicShader = std::make_unique<Shader>("shaders/basic.vert", "shaders/basic.frag");
}

void draw(const Camera& camera, const SceneNode& root, const Lighting& lighting) {
    if (!drawable) {
        return;  // framebuffer minimizado ou ainda sem tamanho válido
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (!basicShader->isValid()) {
        return;  // o erro de compilação/link já foi impresso no console
    }

    basicShader->use();
    basicShader->setMat4("uView", camera.view());
    basicShader->setMat4("uProjection", camera.projection(aspectRatio));
    basicShader->setInt("uDebugMode", kDebugMode);
    // O especular depende do ângulo entre a luz refletida e o olho: o shader precisa saber onde
    // a câmera está.
    basicShader->setVec3("uViewPos", camera.position());
    setLighting(*basicShader, lighting);

    // Cada peça define o próprio uModel, a normal matrix e o material antes de desenhar.
    root.draw(*basicShader, kDefaultMaterial);
}

void resize(int width, int height) {
    glViewport(0, 0, width, height);
    drawable = width > 0 && height > 0;
    if (drawable) {
        aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    }
}

void shutdown() {
    basicShader.reset();
}

}  // namespace renderer
