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
bool lcontrolPressed = false;
double lastMouseX = 0.0, lastMouseY = 0.0;
float angleX = 0.0f, angleY = 0.0f; // Углы вращения по осям X и Y

// Обработчик нажатия мыши
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mousePressed = (action == GLFW_PRESS);
        if (mousePressed) {
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
}

void lconrol_button_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_LEFT_CONTROL) {
        lcontrolPressed = (action == GLFW_PRESS);
    }
}

float viewCameraZ = 3.0f; // Удаление по оси Z

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    viewCameraZ -= yoffset * 0.1f;
    if (viewCameraZ < 1.0f)
        viewCameraZ = 1.0f;
    if (viewCameraZ > 100.0f)
        viewCameraZ = 100.0f;
}

void renderScene(unsigned int VAO, GLuint shaderProgram) {
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
    setView(view, 0.0f, 0.0f, viewCameraZ);

    // Обновите uniform-переменные
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, projection);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    
    // Обновление модели
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f)); // Вращение по оси X
    model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));   // Вращение по оси Y
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0); // поменять на GL_TRIANGLES
}

int main() {
    // Инициализация GLFW
    if (!glfwInit()) {
        std::cerr << "Не удалось инициализировать GLFW" << std::endl;
        return -1;
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


    // Настройка вершин куба
    float vertices[] = {
        // Передняя грань
        -5.f, -5.f,  5.f,
         5.f, -5.f,  5.f,
         5.f,  5.f,  5.f,
        -5.f,  5.f,  5.f,
        // Задняя грань
        //-0.5f, -0.5f, -0.5f,
        //-0.5f,  0.5f, -0.5f,
        // 0.5f,  0.5f, -0.5f,
        // 0.5f, -0.5f, -0.5f,
        0.f, 0.f, -10.f,
    };

    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0, // Передняя грань
        //4, 5, 6, 6, 7, 4, // Задняя грань
        //0, 1, 7, 7, 4, 0, // Левая грань
        //1, 2, 6, 6, 7, 1, // Правая грань
        //2, 3, 5, 5, 6, 2, // Верхняя грань
        //3, 0, 4, 4, 5, 3  // Нижняя грань
        0, 1, 4,
        1, 2, 4,
        2, 3, 4,
        3, 0, 4
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glfwSwapInterval(1);

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
        renderScene(VAO, shaderProgram);

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

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}