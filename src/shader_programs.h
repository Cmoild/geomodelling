#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLuint createShaderProgram() {
    const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;

    uniform mat4 projection;
    uniform mat4 view;
    uniform mat4 model;

    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
    )";

    const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    void main() {
        FragColor = vec4(1.0, 0.5, 0.2, 0.5); // Оранжевый цвет
    }
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint createShaderProgramEdges(){
    const char* vertexShaderEdgesSource = R"(
    #version 330 core
    layout(location = 0) in vec3 position;

    uniform mat4 projection;
    uniform mat4 view;
    uniform mat4 model;

    void main() {
        gl_Position = projection * view * model * vec4(position, 1.0);
        gl_LineWidth = 2.0; // Установите ширину линий
    }
    )";

    const char* fragmentShaderEdgesSource = R"(
    #version 330 core
    out vec4 fragColor;

    void main() {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0); // Черный цвет
    }
    )";

    GLuint vertexShaderEdges = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderEdges, 1, &vertexShaderEdgesSource, nullptr);
    glCompileShader(vertexShaderEdges);

    GLuint fragmentShaderEdges = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderEdges, 1, &fragmentShaderEdgesSource, nullptr);
    glCompileShader(fragmentShaderEdges);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShaderEdges);
    glAttachShader(shaderProgram, fragmentShaderEdges);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShaderEdges);
    glDeleteShader(fragmentShaderEdges);

    return shaderProgram;
}
