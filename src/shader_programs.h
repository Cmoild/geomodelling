#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLuint createShaderProgramObject() {
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

    uniform vec4 FragColorUniform;

    void main() {
        FragColor = FragColorUniform; // Оранжевый цвет
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
    }
    )";

    const char* fragmentShaderEdgesSource = R"(
    #version 330 core
    out vec4 fragColor;

    void main() {
        fragColor = vec4(1.0, 1.0, 1.0, 1.0); // Черный цвет
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

GLuint createShaderProgramLighting(){
    const char* vertexShaderLightingSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;

    out vec3 FragPos;
    out vec3 Normal;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = aNormal;  

        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
    )";

    const char* fragmentShaderLightingSource = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 Normal;  
    in vec3 FragPos;  
    
    uniform vec3 lightPos; 
    uniform vec3 lightColor;
    uniform vec3 objectColor;

    void main()
    {
        // ambient
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * lightColor;
        
        // diffuse 
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
                
        vec3 result = (ambient + diffuse) * objectColor;
        FragColor = vec4(result, 1.0);
    }
    )";

    GLuint vertexShaderLighting = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderLighting, 1, &vertexShaderLightingSource, nullptr);
    glCompileShader(vertexShaderLighting);

    GLuint fragmentShaderLighting = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderLighting, 1, &fragmentShaderLightingSource, nullptr);
    glCompileShader(fragmentShaderLighting);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShaderLighting);
    glAttachShader(shaderProgram, fragmentShaderLighting);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShaderLighting);
    glDeleteShader(fragmentShaderLighting);

    return shaderProgram;
}