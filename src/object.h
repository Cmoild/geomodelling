#pragma once

struct Coordinate{
    float x;
    float y;
    float z;
};

#include <vector>
#include <iostream>
#include <glad/glad.h>


#define OBJECT_QUADRANGLE 0
#define OBJECT_CIRCLE 1

class Object
{
private:

    void generateBuffers(){
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
public:
    std::vector<float> vertices{0, 0};
    std::vector<unsigned int> indices{0, 0};
    unsigned int VAO, VBO, EBO;
    GLint shaderProgram;

    void generateQuadrangle(Coordinate c1, Coordinate c2, Coordinate c3, Coordinate c4){
        vertices.resize(12, 0);
        vertices[0] = c1.x; vertices[1] = c1.y; vertices[2] = c1.z;
        vertices[3] = c2.x; vertices[4] = c2.y; vertices[5] = c2.z;
        vertices[6] = c3.x; vertices[7] = c3.y; vertices[8] = c3.z;
        vertices[9] = c4.x; vertices[10] = c4.y; vertices[11] = c4.z;
        indices.resize(6, 0);
        indices[0] = 0; indices[1] = 1; indices[2] = 2;
        indices[3] = 0; indices[4] = 2; indices[5] = 3;

        generateBuffers();
    }

    void generateCircle(Coordinate c, float r){
        //vertices.push_back(c.x); vertices.push_back(c.y); vertices.push_back(c.z);
        vertices.resize(360 * 3, 0);
        for (int t = 0; t < 360 * 3; t += 3){
            vertices[t] = (c.x + r * cos(t * 3.14159265f / 180));
            vertices[t + 1] = (c.y + r * sin(t * 3.14159265f / 180));
            vertices[t + 2] = (c.z);
        }
        indices.clear();
        for (int i = 1; i < vertices.size() / 3; i++){
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }
        generateBuffers();
    }

    void extrudeObject(float length){
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);

        int vOldSize = vertices.size();
        vertices.resize(vertices.size() * 2, 0);
        for (int i = 0; i < vertices.size() / 2; i += 3){
            vertices[vOldSize + i] = vertices[i];
            vertices[vOldSize + i + 1] = vertices[i + 1];
            vertices[vOldSize + i + 2] = vertices[i + 2] + length;
        }
        int iOldSize = indices.size();
        indices.resize(indices.size() * 2, 0);
        for (int i = iOldSize; i < indices.size(); i++){
            indices[i] = indices[i - iOldSize] + vOldSize / 3;
        }
        for (int i = vOldSize / 3; i < vertices.size() / 3 - 1; i++){
            indices.push_back(i);
            indices.push_back(i + 1);
            indices.push_back(i - vOldSize / 3);
            indices.push_back(i - vOldSize / 3);
            indices.push_back(i - vOldSize / 3 + 1);
            indices.push_back(i + 1);
        }
        indices.push_back(vertices.size() / 3 - 1);
        indices.push_back(vOldSize / 3);
        indices.push_back(vOldSize / 3 - 1);
        indices.push_back(0);
        indices.push_back(vOldSize / 3);
        indices.push_back(vOldSize / 3 - 1);

        generateBuffers();
    }

    void revolveObject(float angle, float r){
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);



        generateBuffers();
    }

    ~Object(){
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        delete this;
    }

};