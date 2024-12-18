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

std::vector<Object*> objects; // Объекты для отрисовки (квадрат, сфера, усечённый конус, пересечение)

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
    viewCameraZ -= yoffset * 0.5f;
    if (viewCameraZ < 0.0f)
        viewCameraZ = 0.0f;
    if (viewCameraZ > 100.0f)
        viewCameraZ = 100.0f;
}

// Функция для проекции 3D объекта на 2D экран
bool projectToScreen(const glm::vec3& objPos, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, GLdouble& winX, GLdouble& winY) {
    glm::vec4 worldPos = glm::vec4(objPos, 1.0f);

    // Создаем модельно-видовую матрицу
    glm::mat4 modelview = view * model;

    // Преобразуем в координаты клип-спейса
    glm::vec4 clipSpacePos = projection * modelview * worldPos;

    // Преобразуем в NDC (нормализованные координаты устройства)
    if (clipSpacePos.w != 0.0f) {
        clipSpacePos /= clipSpacePos.w;
    } else {
        return false; // Невозможно преобразовать, объект вне видимости
    }

    // Получаем параметры области вывода
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Преобразуем в оконные координаты
    winX = viewport[0] + (1 + clipSpacePos.x) * viewport[2] / 2;
    winY = viewport[1] + (1 + clipSpacePos.y) * viewport[3] / 2;

    return true;
}

// Проверка нахождения курсора над объектом
bool isCursorOverObject(const glm::vec3& objPos, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, GLFWwindow* window) {
    GLdouble winX, winY;
    if (!projectToScreen(objPos, model, view, projection, winX, winY)) {
        return false;
    }

    // Получаем позицию курсора
    double cursorX, cursorY;
    glfwGetCursorPos(window, &cursorX, &cursorY);
    
    // Отражаем Y относительно высоты окна
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    cursorY = viewport[3] - cursorY;

    // Допустимый радиус вокруг объекта
    float threshold = 5.0f;

    // Проверка, находится ли курсор в пределах радиуса
    return (abs(cursorX - winX) < threshold && abs(cursorY - winY) < threshold);
}

std::vector<int> skipRender;

void renderScene(std::vector<Object*> objects, GLuint shaderProgram, GLuint shaderProgramEdges, GLFWwindow* window) {
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
        if (skipRender[i]) continue;
        objects[i]->model = model;
        // objects[i]->model = glm::translate(objects[i]->model, objects[i]->position);
        objects[i]->model = glm::rotate(objects[i]->model, glm::radians(objects[i]->rotation[0]), glm::vec3(1.0f, 0.0f, 0.0f));
        objects[i]->model = glm::rotate(objects[i]->model, glm::radians(objects[i]->rotation[1]), glm::vec3(0.0f, 1.0f, 0.0f));
        objects[i]->model = glm::rotate(objects[i]->model, glm::radians(objects[i]->rotation[2]), glm::vec3(0.0f, 0.0f, 1.0f));
        objects[i]->model = glm::translate(objects[i]->model, objects[i]->position);
    }

    for (int i = 0; i < objects.size(); i++) {
        if (skipRender[i]) continue;

        glUseProgram(shaderProgram);

        // Обновите uniform-переменные
        glm::vec4 FragColorUniform = objects[i]->FragColor;
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(objects[i]->model));
        glUniform4fv(glGetUniformLocation(shaderProgram, "FragColorUniform"), 1, glm::value_ptr(FragColorUniform));

        glBindVertexArray(objects[i]->VAO);
        if (!(objects[i]->type == OBJECT_NONE)) glDrawElements(GL_TRIANGLES, objects[i]->indices.size(), GL_UNSIGNED_INT, 0);
        // glDrawElements(GL_TRIANGLES, objects[i]->indices.size(), GL_UNSIGNED_INT, 0);
        //std::cout << isCursorOverObject(glm::vec3(model[3]), model, view, projection, window) << std::endl;

        glUseProgram(shaderProgramEdges);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgramEdges, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgramEdges, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgramEdges, "model"), 1, GL_FALSE, glm::value_ptr(objects[i]->model));

        glBindVertexArray(objects[i]->VAO);
        if (objects[i]->type == OBJECT_CIRCLE)
            glDrawElements(GL_POINTS, objects[i]->indices.size(), GL_UNSIGNED_INT, 0);
        else if (objects[i]->type == OBJECT_NONE)
            glDrawElements(GL_POINTS, objects[i]->indices.size(), GL_UNSIGNED_INT, 0);
        else
            glDrawElements(GL_POINTS, objects[i]->indices.size(), GL_UNSIGNED_INT, 0);
    }

}

// Перечисление типов фигур
enum class ShapeType {
    None,
    Circle,
    Rectangle,
    SemiCircle
};

enum class StandardShapeType {
    None,
    TruncaredCone,
    Sphere
};

// Перечисление режимов создания
enum class ObjCreatingMode {
    None,
    Extrude,
    Revolve,
    Standard
};

// Параметры фигуры
struct ShapeParameters {
    ImVec4 color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Красный цвет по умолчанию
    float radius = 0.0f;  // Размер фигуры по умолчанию
    float radius2 = 0.0f;
    float height = 0.0f;
    float rotatingRadius = 0.0f;  // Размер фигуры по умолчанию
    float length = 0.0f;  // Длина фигуры по умолчанию
};

// Текущая переменная для хранения выбранной фигуры
ShapeType currentShape = ShapeType::None;

// Текущая переменная для хранения выбранной стандартной фигуры
StandardShapeType currentStandardShape = StandardShapeType::None;

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
            ImGui::InputFloat("##00", &shapeParams.radius, 0.0f, 100.0f);
            ImGui::Separator();
            //ImGui::PopID();
        }
        if (currentCreatingMode == ObjCreatingMode::Revolve) {
            ImGui::Text("Радиус вращения:");
            ImGui::InputFloat("##01", &shapeParams.rotatingRadius, 0.0f, 100.0f);
            ImGui::Separator();
            //ImGui::PopID();
        }

        if (currentCreatingMode == ObjCreatingMode::Extrude) {
            ImGui::Text("Длина выдавливания:");
            ImGui::InputFloat("##02", &shapeParams.length, 0.0f, 100.0f);
            ImGui::Separator();
            //ImGui::PopID();
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
        ImGui::SameLine();
        if (ImGui::Button("Создать")) {
            
        }

        ImGui::EndPopup();
    }
}

void ShowStandardShapeConfigPopup() {
    // Проверка, запущено ли всплывающее окно
    if (ImGui::BeginPopupModal("Настройки фигуры", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        
        // Выбор типа фигуры
        ImGui::Text("Тип фигуры:");
        if (ImGui::RadioButton("Усечённый конус", currentStandardShape == StandardShapeType::TruncaredCone)) {
            currentStandardShape = StandardShapeType::TruncaredCone;
        }
        if (ImGui::RadioButton("Сфера", currentStandardShape == StandardShapeType::Sphere)) {
            currentStandardShape = StandardShapeType::Sphere;
        }

        ImGui::Separator();

        // Выбор цвета
        ImGui::Text("Цвет:");
        ImGui::ColorEdit4("Цвет фигуры", (float*)&shapeParams.color);
        if (currentStandardShape == StandardShapeType::TruncaredCone) {
            ImGui::Text("Радиус r1 (нижний):");
            ImGui::InputFloat("##00", &shapeParams.radius, 0.0f, 100.0f);
            ImGui::Text("Радиус r2 (верхний):");
            ImGui::InputFloat("##01", &shapeParams.radius2, 0.0f, 100.0f);
            ImGui::Text("Высота:");
            ImGui::InputFloat("##02", &shapeParams.height, 0.0f, 100.0f);
            ImGui::Separator();
        }
        if (currentStandardShape == StandardShapeType::Sphere) {
            ImGui::Text("Радиус:");
            ImGui::InputFloat("##03", &shapeParams.radius, 0.0f, 100.0f);
            ImGui::Separator();
        }

        // Кнопка закрытия всплывающего окна
        if (ImGui::Button("Закрыть")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Создать")) {
            if (currentStandardShape == StandardShapeType::TruncaredCone) {
                Object *tmp = objects[2];
                objects[2] = new TruncCone({0.f, 0.f, 0.f}, shapeParams.radius, shapeParams.radius2, shapeParams.height);
                delete tmp;
                objects[2]->FragColor = glm::vec4(shapeParams.color.x, shapeParams.color.y, shapeParams.color.z, shapeParams.color.w);
                skipRender[2] = false;
            }
            if (currentStandardShape == StandardShapeType::Sphere) {
                Object *tmp = objects[1];
                objects[1] = new Sphere({0.f, 0.f, 0.f}, shapeParams.radius);
                delete tmp;
                objects[1]->FragColor = glm::vec4(shapeParams.color.x, shapeParams.color.y, shapeParams.color.z, shapeParams.color.w);
                skipRender[1] = false;
            }
        }

        ImGui::EndPopup();
    }
}

// bool showEditPopup = false;

void ShowEditPopup() {
    if (ImGui::BeginPopupModal("Изменить", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        
        // Выбор типа фигуры
        ImGui::Text("Тип фигуры:");
        if (ImGui::RadioButton("Усечённый конус", currentStandardShape == StandardShapeType::TruncaredCone)) {
            currentStandardShape = StandardShapeType::TruncaredCone;
        }
        if (ImGui::RadioButton("Сфера", currentStandardShape == StandardShapeType::Sphere)) {
            currentStandardShape = StandardShapeType::Sphere;
        }

        ImGui::Separator();

        if (currentStandardShape == StandardShapeType::TruncaredCone && objects[2]) {
            ImGui::Text("Позиция:");
            ImGui::InputFloat("x##00", &objects[2]->position.x, -100.0f, 100.0f);
            ImGui::SameLine();
            ImGui::InputFloat("y##01", &objects[2]->position.y, -100.0f, 100.0f);
            ImGui::SameLine();
            ImGui::InputFloat("z##02", &objects[2]->position.z, -100.0f, 100.0f);
            ImGui::Text("Угол поворота (в градусах):");
            ImGui::InputFloat("x##03", &objects[2]->rotation.x, 0.0f, 360.0f);
            ImGui::SameLine();
            ImGui::InputFloat("y##04", &objects[2]->rotation.y, 0.0f, 360.0f);
            ImGui::SameLine();
            ImGui::InputFloat("z##05", &objects[2]->rotation.z, 0.0f, 360.0f);
            ImGui::Separator();
        }
        if (currentStandardShape == StandardShapeType::Sphere && objects[1]) {
            ImGui::Text("Позиция:");
            ImGui::InputFloat("x##00", &objects[1]->position.x, -100.0f, 100.0f);
            ImGui::SameLine();
            ImGui::InputFloat("y##01", &objects[1]->position.y, -100.0f, 100.0f);
            ImGui::SameLine();
            ImGui::InputFloat("z##02", &objects[1]->position.z, -100.0f, 100.0f);
            ImGui::Separator();
        }

        // Кнопка закрытия всплывающего окна
        if (ImGui::Button("Закрыть")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

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
    if (ImGui::Button("Стандартная", ImVec2(100, 20))) {
        ImGui::OpenPopup("Настройки фигуры");
        currentCreatingMode = ObjCreatingMode::Standard;
    }

    if (!(currentCreatingMode == ObjCreatingMode::Standard)) ShowShapeConfigPopup();
    else ShowStandardShapeConfigPopup();

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
    if (ImGui::Button("Показать | Cкрыть", ImVec2(100, 20))) {
        // Логика для настройки 2
        if (objects[1] && objects[2]) {
            skipRender[1] = !skipRender[1];
            skipRender[2] = !skipRender[2];
        }
    }
    if (ImGui::Button("Изменить", ImVec2(100, 20))) {
        // Логика для настройки 2
        // showEditPopup = true;
        ImGui::OpenPopup("Изменить");
    }
    if (ImGui::Button("Найти пересечение", ImVec2(100, 20)) && objects[1] && objects[2]) {

        Object* tmp = objects[3];
        TruncCone* c = (TruncCone*)objects[2];
        Sphere* s = (Sphere*)objects[1];
        objects[3] = c->checkIntersection(s);
        skipRender[3] = false;
        delete tmp;
    }

    ShowEditPopup();

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
    
    Object* object3 = new Object();
    object3->generateQuadrangle({-10.f, 0.f, 10.f}, {10.f, 0.f, 10.f}, {10.f, 0.f, -10.f}, {-10.f, 0.f, -10.f});
    object3->FragColor = {0.f, 0.f, 1.f, 1.f};
    skipRender.push_back(0);

    // Sphere* sphere = new Sphere({0.f, 0.f, 0.f}, 0.5f);
    // sphere->FragColor = {1.f, 1.f, 0.f, 1.f};
    // sphere->position = {0.f, 0.6f, -0.3f};
    skipRender.push_back(1);

    // TruncCone* cone = new TruncCone({0.f, 0.f, 0.f}, 0.5f, 0.25f, 1.f);
    // cone->FragColor = {1.f, 0.f, 0.f, 1.f};
    // cone->position = {0.f, 0.f, 0.f};
    // cone->rotation = {10.f, 0.f, 0.f};
    skipRender.push_back(1);

    // Object* intersec = cone->checkIntersection(sphere);
    // intersec->FragColor = {1.f, 1.f, 1.f, 1.f};
    skipRender.push_back(1);
    
    objects.push_back(object3);
    // objects.push_back(cone);
    // objects.push_back(sphere);
    // objects.push_back(intersec);
    objects.push_back(nullptr);
    objects.push_back(nullptr);
    objects.push_back(nullptr);


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
        renderScene(objects, shaderProgramObject, shaderProgramEdges, window);

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