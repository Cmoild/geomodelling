#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_programs.h"
#include <cmath>
#include "object.h"

const GLuint WIDTH = 800, HEIGHT = 600;

// Функции для работы с матрицами
void setPerspective(float* matrix, float fov, float aspect, float near, float far) {
    float tanHalfFOV = tan(fov / 2.0f);
    
    matrix[0] = 1.0f / (aspect * tanHalfFOV);
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = 1.0f / tanHalfFOV;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = -(far + near) / (far - near);
    matrix[11] = -1.0f;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = -(2.0f * far * near) / (far - near);
    matrix[15] = 0.0f;

}

void setView(glm::mat4& matrix, float cameraX, float cameraY, float cameraZ) {
    matrix[3][0] = -cameraX; matrix[3][1] = -cameraY; matrix[3][2] = -cameraZ;
}

// Обработчик изменения размера окна
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Обработка ввода
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Глобальные переменные для отслеживания состояния мыши
bool mousePressed = false;
bool middlePressed = false;
bool lcontrolPressed = false;
double lastMouseX = 0.0, lastMouseY = 0.0;
float angleX = 0.0f, angleY = 0.0f; // Углы вращения по осям X и Y

float viewCameraZ = 3.0f; // Удаление по оси Z
float viewCameraX = 0.0f; // Позиция камеры по оси X
float viewCameraY = 0.0f; // Позиция камеры по оси Y

// Обработчик нажатия мыши
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mousePressed = (action == GLFW_PRESS);
        if (mousePressed) {
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        middlePressed = (action == GLFW_PRESS);
        if (middlePressed) {
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        }
    }
}

// Обработчик движения мыши
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    //if (!lcontrolPressed) return;
    if (mousePressed) {
        double deltaX = xpos - lastMouseX;
        double deltaY = ypos - lastMouseY;

        // Обновление углов вращения
        angleX += deltaY * 0.5f; // Чувствительность по оси Y
        angleY += deltaX * 0.5f; // Чувствительность по оси X

        lastMouseX = xpos;
        lastMouseY = ypos;
    }
    if (middlePressed) {
        double deltaX = xpos - lastMouseX;
        double deltaY = ypos - lastMouseY;

        // Обновление углов вращения
        viewCameraX -= deltaX * 0.01f; // Чувствительность по оси X
        viewCameraY += deltaY * 0.01f; // Чувствительность по оси Y

        lastMouseX = xpos;
        lastMouseY = ypos;
    }
}

void lconrol_button_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_LEFT_CONTROL) {
        lcontrolPressed = (action == GLFW_PRESS);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    viewCameraZ -= yoffset * 0.1f;
    if (viewCameraZ < 1.0f)
        viewCameraZ = 1.0f;
    if (viewCameraZ > 100.0f)
        viewCameraZ = 100.0f;
}

void renderScene(unsigned int VAO, unsigned int VAO2, GLuint shaderProgram) {
    // Отрисовка
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(shaderProgram);
    //glUseProgram(shaderProgramEdges);

    float projection[16];
    setPerspective(projection, 45.0f * 3.14159265f / 180.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    //glm::mat4 projection = glm::perspective(glm::radians(45.0f * 3.14159265f / 180.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

    glm::mat4 view = glm::mat4(1.0f);
    //float view[16];
    setView(view, viewCameraX, viewCameraY, viewCameraZ);

    // Обновите uniform-переменные
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, projection);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    
    // Обновление модели
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f)); // Вращение по оси X
    model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));   // Вращение по оси Y
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 12 + 3 * 2 * 4, GL_UNSIGNED_INT, 0); // поменять на GL_TRIANGLES

    glBindVertexArray(VAO2);
    glDrawElements(GL_TRIANGLES, 12 + 3 * 2 * 4, GL_UNSIGNED_INT, 0);
}

int main(int argc, char* argv[]) {
    // Инициализация GLFW
    if (!glfwInit()) {
        std::cerr << "Не удалось инициализировать GLFW" << std::endl;
        return -1;
    }
    for (int i = 0; i < argc; i++) {
        std::cout << argv[i] << std::endl;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Куб", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Не удалось создать окно GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Инициализация GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Не удалось инициализировать GLAD" << std::endl;
        return -1;
    }

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window,lconrol_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSwapInterval(1);

    Object* object = new Object();
    object->generateQuadrangle({-1.f, -1.f, 0.f}, {1.f, -1.f, 0.f}, {1.f, 1.f, 0.f}, {-1.f, 1.f, 0.f});
    object->extrudeObject(-2.f);
    
    Object* object2 = new Object();
    object2->generateQuadrangle({-0.5f, -0.5f, 2.f}, {0.5f, -0.5f, 2.f}, {0.5f, 0.5f, 2.f}, {-0.5f, 0.5f, 2.f});
    object2->extrudeObject(1.f);

    GLuint shaderProgram = createShaderProgram();
    GLuint shaderProgramEdges = createShaderProgramEdges();

    // Инициализация ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("src\\imgui\\fonts\\Roboto-Medium.ttf", 12.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    io.Fonts->Build();

    // Имплементация ImGui для GLFW и OpenGL
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Начало нового кадра ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Создание окна управления
        ImGui::Begin(u8"Контроль отдаления");
        ImGui::SliderFloat(u8"отдаление", &viewCameraZ, 0.0f, 10.0f);
        ImGui::End();

        // Отрисовка
        renderScene(object->VAO, object2->VAO, shaderProgram);

        // Отрисовка ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Освобождение ресурсов
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete object;
    delete object2;

    glfwTerminate();
    return 0;
}