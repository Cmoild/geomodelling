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
#include <vector>

GLuint WIDTH = 800, HEIGHT = 600;

glm::vec3 lightPos(0.f, 0.f, 4.0f);

void setView(glm::mat4& matrix, float cameraX, float cameraY, float cameraZ) {
    matrix[3][0] = -cameraX; matrix[3][1] = -cameraY; matrix[3][2] = -cameraZ;
}

// Обработчик изменения размера окна
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
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
        angleX += deltaY * 0.1f; // Чувствительность по оси Y
        angleY += deltaX * 0.1f; // Чувствительность по оси X

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

void renderScene(std::vector<Object*> objects, GLuint shaderProgram, GLuint shaderProgramEdges) {
    // Отрисовка
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = glm::mat4(0.0f);
    projection = glm::perspective(glm::radians(90.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

    glm::mat4 view = glm::mat4(1.0f);
    setView(view, viewCameraX, viewCameraY, viewCameraZ);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f)); // Вращение по оси X
    model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));

    for (int i = 0; i < objects.size(); i++) {
        glUseProgram(shaderProgram);

        // Обновите uniform-переменные
        glm::vec4 FragColorUniform = objects[i]->FragColor;
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform4fv(glGetUniformLocation(shaderProgram, "FragColorUniform"), 1, glm::value_ptr(FragColorUniform));

        glBindVertexArray(objects[i]->VAO);
        glDrawElements(GL_TRIANGLES, objects[i]->indices.size(), GL_UNSIGNED_INT, 0);

        glUseProgram(shaderProgramEdges);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgramEdges, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgramEdges, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgramEdges, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(objects[i]->VAO);
        if (objects[i]->type == OBJECT_CIRCLE)
            glDrawElements(GL_POINTS, objects[i]->indices.size(), GL_UNSIGNED_INT, 0);
        else
            glDrawElements(GL_LINES, objects[i]->indices.size(), GL_UNSIGNED_INT, 0);
    }

}

// Перечисление типов фигур
enum class ShapeType {
    None,
    Circle,
    Rectangle,
    SemiCircle
};

// Перечисление режимов создания
enum class ObjCreatingMode {
    None,
    Extrude,
    Revolve
};

// Параметры фигуры
struct ShapeParameters {
    ImVec4 color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Красный цвет по умолчанию
    float radius = 0.0f;  // Размер фигуры по умолчанию
    float rotatingRadius = 0.0f;  // Размер фигуры по умолчанию
    float length = 0.0f;  // Длина фигуры по умолчанию
};

// Текущая переменная для хранения выбранной фигуры
ShapeType currentShape = ShapeType::None;

// Текущий режим создания
ObjCreatingMode currentCreatingMode = ObjCreatingMode::None;

ShapeParameters shapeParams;

// Функция для отображения всплывающего окна
void ShowShapeConfigPopup() {
    // Проверка, запущено ли всплывающее окно
    if (ImGui::BeginPopupModal("Настройки фигуры", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        
        // Выбор типа фигуры
        ImGui::Text("Тип базовой фигуры:");
        if (ImGui::RadioButton("Круг", currentShape == ShapeType::Circle)) {
            currentShape = ShapeType::Circle;
        }
        if (ImGui::RadioButton("Четырехугольник", currentShape == ShapeType::Rectangle)) {
            currentShape = ShapeType::Rectangle;
        }
        if (ImGui::RadioButton("Полукруг", currentShape == ShapeType::SemiCircle)) {
            currentShape = ShapeType::SemiCircle;
        }

        ImGui::Separator();

        // Выбор цвета
        ImGui::Text("Цвет:");
        ImGui::ColorEdit4("Цвет фигуры", (float*)&shapeParams.color);
        if (currentShape == ShapeType::SemiCircle || currentShape == ShapeType::Circle) {
            ImGui::Text("Радиус:");
            //ImGui::SliderFloat("радиус фигуры", &shapeParams.size, 0.0f, 100.0f);
            ImGui::InputFloat(" ", &shapeParams.radius, 0.0f, 100.0f);
            ImGui::Separator();
        }
        if (currentCreatingMode == ObjCreatingMode::Revolve) {
            ImGui::Text("Радиус вращения:");
            ImGui::InputFloat(" ", &shapeParams.rotatingRadius, 0.0f, 100.0f);
            ImGui::Separator();
        }
        if (currentCreatingMode == ObjCreatingMode::Extrude) {
            ImGui::Text("Длина выдавливания:");
            ImGui::InputFloat(" ", &shapeParams.length, 0.0f, 100.0f);
            ImGui::Separator();
        }
        
        // Выбор размера
        // ImGui::Text("Размер:");
        // ImGui::SliderFloat("Размер фигуры", &shapeParams.size, 10.0f, 100.0f);

        // ImGui::Separator();

        // Визуализация фигуры в этом окне

        // Кнопка закрытия всплывающего окна
        if (ImGui::Button("Закрыть")) {
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Создать")) {
            
        }

        ImGui::EndPopup();
    }
}

void VerticalSeparator(float height = 0.0f) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    float line_height = height > 0.0f ? height : ImGui::GetContentRegionAvail().y;

    draw_list->AddLine(
        ImVec2(cursor_pos.x, cursor_pos.y),                   // Начальная точка
        ImVec2(cursor_pos.x, cursor_pos.y + line_height),     // Конечная точка
        ImGui::GetColorU32(ImGuiCol_Separator),               // Цвет линии
        1.0f                                                  // Толщина линии
    );

    // Смещаем курсор вправо, чтобы следующие элементы не перекрывали линию
    ImGui::SetCursorScreenPos(ImVec2(cursor_pos.x + 4.0f, cursor_pos.y));
}


// Функция для создания окна с выбором фигур
void DrawShapesWindow() {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always); // Устанавливаем позицию окна
    ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_Always); // Фиксированный размер окна

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Shapes Window", nullptr, window_flags);

    ImGui::Text("Создать фигуру:");
    if (ImGui::Button("Выдавливанием", ImVec2(100, 20))) {
        ImGui::OpenPopup("Настройки фигуры");
        currentCreatingMode = ObjCreatingMode::Extrude;
    }
    if (ImGui::Button("Вращением", ImVec2(100, 20))) {
        ImGui::OpenPopup("Настройки фигуры");
        currentCreatingMode = ObjCreatingMode::Revolve;
    }

    ShowShapeConfigPopup();

    ImGui::End();
}

// Функция для создания окна с опциями
void DrawOptionsWindow() {
    ImGui::SetNextWindowPos(ImVec2(200, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_Always);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Options Window", nullptr, window_flags);

    ImGui::Text("Опции:");
    if (ImGui::Button("Выбрать", ImVec2(100, 20))) {
        // Логика для настройки 1
    }
    if (ImGui::Button("Удалить", ImVec2(100, 20))) {
        // Логика для настройки 2
    }
    if (ImGui::Button("Изменить", ImVec2(100, 20))) {
        // Логика для настройки 2
    }

    ImGui::End();
}

void SliderWindow() {
    ImGui::SetNextWindowPos(ImVec2(400, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(WIDTH - 400, 100), ImGuiCond_Always);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Slider Window", nullptr, window_flags);

    ImGui::Text("Отдалить:");
    ImGui::SliderFloat(u8" ", &viewCameraZ, 0.0f, 50.0f);

    ImGui::End();
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
    object->generateQuadrangle({-1.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 2.f, 0.f}, {-1.f, 2.f, 0.f});
    object->extrudeObject(-2.f);
    object->FragColor = {0.f, 1.f, 0.f, 1.f};
    
    Object* object2 = new Object();
    object2->generateCircle({0.f, 0.f, 0.f}, 0.5f);
    object2->revolveObject(360.f, 3.f);
    object2->FragColor = {1.f, 0.f, 0.f, 1.f};

    Object* object3 = new Object();
    object3->generateQuadrangle({-10.f, 0.f, 10.f}, {10.f, 0.f, 10.f}, {10.f, 0.f, -10.f}, {-10.f, 0.f, -10.f});
    object3->FragColor = {0.f, 0.f, 1.f, 1.f};


    std::vector<Object*> objects;
    objects.push_back(object);
    objects.push_back(object2);
    objects.push_back(object3);

    GLuint shaderProgramObject = createShaderProgramObject();
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

        // Отрисовка меню
        DrawShapesWindow();
        DrawOptionsWindow();
        SliderWindow();
        

        

        // Отрисовка
        renderScene(objects, shaderProgramObject, shaderProgramEdges);

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

    for (int i = 0; i < objects.size(); i++) {
        delete objects[i];
    }

    glfwTerminate();
    return 0;
}