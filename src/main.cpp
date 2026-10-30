#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "core/camera.hpp"
#include "core/input.hpp"
#include "gfx/renderer.hpp"
#include "scene/classroom.hpp"

namespace {

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;

// Versão do OpenGL do projeto: 3.3 core (a mesma do loader gerado em lib/glad)
constexpr int kGlMajor = 3;
constexpr int kGlMinor = 3;

void onGlfwError(int code, const char* description) {
    std::fprintf(stderr, "GLFW (%d): %s\n", code, description);
}

void update(GLFWwindow* window, Input& input, Camera& camera, scene::Classroom& classroom, float deltaTime) {
    if (input.keyPressed(GLFW_KEY_Q)) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (input.keyPressed(GLFW_KEY_ESCAPE)) {
        if (input.isCursorCaptured()) {
            input.setCursorCaptured(false);
        } else {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    } else if (input.mousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
        input.setCursorCaptured(true);
    }

    if (glfwWindowShouldClose(window) || !input.isFocused()) {
        return;
    }

    // Teclas da cena (interações, animações e inspeção) também funcionam com o cursor livre.
    // A inspeção restaura o enquadramento; não aplica movimento pendente nesse frame.
    classroom.update(input, deltaTime);
    if (classroom.handleInspection(input, camera)) {
        return;
    }

    if (!input.isCursorCaptured() || input.focusChanged()) {
        return;
    }
    const auto mouse = input.mouseDelta();
    camera.rotate(static_cast<float>(mouse.x), static_cast<float>(mouse.y));
    const glm::vec3 direction(
        static_cast<float>(input.keyDown(GLFW_KEY_D)) - static_cast<float>(input.keyDown(GLFW_KEY_A)),
        static_cast<float>(input.keyDown(GLFW_KEY_SPACE)) - static_cast<float>(input.keyDown(GLFW_KEY_LEFT_CONTROL)),
        static_cast<float>(input.keyDown(GLFW_KEY_W)) - static_cast<float>(input.keyDown(GLFW_KEY_S)));
    camera.move(direction, deltaTime);
}

void onFramebufferSize(GLFWwindow* /*window*/, int width, int height) {
    renderer::resize(width, height);
}

}  // namespace

int main() {
    glfwSetErrorCallback(onGlfwError);
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, kGlMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, kGlMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, "Aula OpenGL", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::fprintf(stderr, "GLAD: não foi possível carregar o OpenGL %d.%d\n", kGlMajor, kGlMinor);
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }
    std::printf("OpenGL %s | %s\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)),
                reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    glfwSetFramebufferSizeCallback(window, onFramebufferSize);

    renderer::init();
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    renderer::resize(width, height);

    {
        Input input(window);
        Camera camera;
        scene::Classroom classroom;  // cria meshes e texturas: precisa do contexto, sai antes da janela
        classroom.selectInspection(scene::Classroom::kWholeScene, camera);  // mesma vista da tecla 1
        constexpr double kMaxDeltaTime = 0.1;  // descarta o excesso de pausas longas
        double previousTime = glfwGetTime();
        bool firstFrame = true;

        while (!glfwWindowShouldClose(window)) {
            input.beginFrame();
            glfwPollEvents();

            // Medido após o poll: inclui também esperas do sistema (ex.: resize).
            const double currentTime = glfwGetTime();
            const float deltaTime = firstFrame || !input.isFocused() || input.focusChanged()
                                        ? 0.0f
                                        : static_cast<float>(std::clamp(currentTime - previousTime,
                                                                        0.0, kMaxDeltaTime));
            previousTime = currentTime;
            firstFrame = false;

            update(window, input, camera, classroom, deltaTime);
            if (glfwWindowShouldClose(window)) {
                break;
            }
            renderer::draw(camera, classroom.root(), &classroom.texture(), classroom.lighting());
            glfwSwapBuffers(window);
        }
    }

    renderer::shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
