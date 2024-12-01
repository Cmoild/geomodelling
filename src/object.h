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
#define OBJECT_NONE 2

class Object
{
private:
    std::vector<float> vertices2d;
    std::vector<unsigned int> indices2d;
    std::vector<float> verticesWithNormals{0 , 0};

    inline void calculateNormals(){
        verticesWithNormals.clear();
        //std::vector<glm::vec3> normals(vertices.size() / 3, glm::vec3(0.0f, 0.0f, 0.0f));
        //std::vector<float> normals{0 , 0};
        //normals.resize(vertices.size(), 0.0f);
        float *normals = new float[vertices.size()];
        for (int i = 0; i < indices.size(); i += 3){
            unsigned int i1 = indices[i];
            unsigned int i2 = indices[i + 1];
            unsigned int i3 = indices[i + 2];

            glm::vec3 v1 = glm::vec3(vertices[i1 * 3], vertices[i1 * 3 + 1], vertices[i1 * 3 + 2]);
            glm::vec3 v2 = glm::vec3(vertices[i2 * 3], vertices[i2 * 3 + 1], vertices[i2 * 3 + 2]);
            glm::vec3 v3 = glm::vec3(vertices[i3 * 3], vertices[i3 * 3 + 1], vertices[i3 * 3 + 2]);

            glm::vec3 normal = glm::cross(v2 - v1, v3 - v1);

            normals[i1 * 3] += normal.x;
            normals[i1 * 3 + 1] += normal.y;
            normals[i1 * 3 + 2] += normal.z;

            normals[i2 * 3] += normal.x;
            normals[i2 * 3 + 1] += normal.y;
            normals[i2 * 3 + 2] += normal.z;
            
            normals[i3 * 3] += normal.x;
            normals[i3 * 3 + 1] += normal.y;
            normals[i3 * 3 + 2] += normal.z;
        }
        for (int i = 0; i < vertices.size(); i+= 3){
            //normals[i] = glm::normalize(normals[i]);
            glm::vec3 normalized = glm::normalize(glm::vec3(normals[i], normals[i + 1], normals[i + 2]));
            normals[i] = normalized.x;
            normals[i + 1] = normalized.y;
            normals[i + 2] = normalized.z;
        }

        
        std::cout << "tmp size " << vertices.size() << std::endl;
        verticesWithNormals.resize(vertices.size() * 2, 0);
        for (int i = 0; i < vertices.size(); i += 3){
            //std::cout << "tmp size " << tmp.size() << std::endl;
            // verticesWithNormals.push_back(vertices[i]);
            // verticesWithNormals.push_back(vertices[i + 1]);
            // verticesWithNormals.push_back(vertices[i + 2]);
            // verticesWithNormals.push_back(normals[i / 3].x);
            // verticesWithNormals.push_back(normals[i / 3].y);
            // verticesWithNormals.push_back(normals[i / 3].z);
            // verticesWithNormals.push_back(normals[i]);
            // verticesWithNormals.push_back(normals[i + 1]);
            // verticesWithNormals.push_back(normals[i + 2]);

            verticesWithNormals[2 * i] = vertices[i];
            verticesWithNormals[2 * i + 1] = vertices[i + 1];
            verticesWithNormals[2 * i + 2] = vertices[i + 2];
            verticesWithNormals[2 * i + 3] = normals[i];
            verticesWithNormals[2 * i + 4] = normals[i];
            verticesWithNormals[2 * i + 5] = normals[i];
        }
        //normals.clear();
        delete [] normals;
        std::cout << "vertices size " << verticesWithNormals.size() << std::endl;
    }
public:
    void generateBuffers(){
        // std::cout << "Calculate normals" << std::endl;
        // try
        // {
        //     calculateNormals();
        // }
        // catch(const std::exception& e)
        // {
        //     std::cerr << e.what() << '\n';
        // }
        
        // //calculateNormals();

        // std::cout << "Generate buffers" << std::endl;

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

        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        // glEnableVertexAttribArray(1);

        // glGenVertexArrays(1, &lightVAO);
        // glBindVertexArray(lightVAO);

        // glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        
    }
public:
    std::vector<float> vertices{0, 0};
    std::vector<unsigned int> indices{0, 0};
    unsigned int VAO, VBO, EBO, lightVAO;
    GLint shaderProgram;
    int type;
    glm::vec4 FragColor;
    glm::mat4 model;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);

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
        vertices2d = vertices;
        indices2d = indices;
        type = OBJECT_QUADRANGLE;
    }

    void generateCircle(Coordinate c, float r){
        //vertices.push_back(c.x); vertices.push_back(c.y); vertices.push_back(c.z);
        vertices.resize(360 * 3, 0);
        for (int t = 0; t < 360; t++){
            vertices[t * 3] = (c.x + r * cos((t - 90) * 3.14159265f / 180));
            vertices[t * 3 + 1] = (c.y + r * sin((t - 90) * 3.14159265f / 180));
            vertices[t * 3 + 2] = (c.z);
        }
        indices.clear();
        for (int i = 1; i < vertices.size() / 3; i++){
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }
        generateBuffers();
        vertices2d = vertices;
        indices2d = indices;
        type = OBJECT_CIRCLE;
    }

    void generateHalfCircle(Coordinate c, float r){
        vertices.resize(180 * 3, 0);
        for (int t = 0; t < 180; t++){
            //vertices[t] = (c.x + r * cos(t * 3.14159265f / 180));
            //vertices[t + 1] = (c.y + r * sin(t * 3.14159265f / 180));
            //vertices[t + 2] = (c.z);
            vertices[t * 3] = (c.x + r * cos((t - 90) * 3.14159265f / 180));
            vertices[t * 3 + 1] = (c.y + r * sin((t - 90) * 3.14159265f / 180));
            vertices[t * 3 + 2] = (c.z);
        }
        indices.clear();
        for (int i = 1; i < vertices.size() / 3; i++){
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }
        generateBuffers();
        vertices2d = vertices;
        indices2d = indices;
        type = OBJECT_CIRCLE;
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
        
        for (int i = 0; i < vertices.size(); i+=3) {
            vertices[i] += r;
        }

        int vOldSize = vertices.size();
        int iOldSize = indices.size();
        for (int t = 1; t < angle + 1; t++){
            int curVSize = vertices.size();
            int curISize = indices.size();
            for (int i = vOldSize * t; i < vOldSize * (t + 1); i+= 3){
                vertices.push_back(vertices2d[i - vOldSize * t] * cos(3.14159265f * t / 180) + r * cos(3.14159265f * t / 180));
                vertices.push_back(vertices2d[i - vOldSize * t + 1]);
                vertices.push_back(vertices2d[i - vOldSize * t] * sin(3.14159265f * t / 180) + r * sin(3.14159265f * t / 180));
            }

            for (int i = vOldSize / 3 * t; i < vOldSize / 3 * (t + 1) - 1; i++){
                indices.push_back(i);
                indices.push_back(i + 1);
                indices.push_back(i - vOldSize / 3);
                indices.push_back(i - vOldSize / 3);
                indices.push_back(i - vOldSize / 3 + 1);
                indices.push_back(i + 1);
            }
            indices.push_back(vOldSize / 3 * (t + 1) - 1);
            indices.push_back(vOldSize / 3 * t);
            indices.push_back(vOldSize / 3 * t - 1);
            indices.push_back(vOldSize / 3 * (t + 1) - 2 * vOldSize / 3 + 1);
            //std::cout << "sdffd " << vOldSize / 3 * (t + 1) << ' ' << 2 * vOldSize / 3 << std::endl;
            indices.push_back(vOldSize / 3 * t);
            indices.push_back(vOldSize / 3 * t - 1);
        }

        generateBuffers();
    }

    ~Object(){
        glDeleteVertexArrays(1, &lightVAO);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        delete this;
    }

};

#define OBJECT_SPHERE 0
#define OBJECT_CONE 1

class TruncCone;

class Sphere : public Object {
private:
    
public:
    int type3d;
    float r;

    Sphere(Coordinate c, float r) {
        generateHalfCircle(c, r);
        revolveObject(360, 0.f);
        type3d = OBJECT_SPHERE;
        this->r = r;
    }

    void checkIntersection(TruncCone *s) {
        
    }
};

class TruncCone : public Object {
private:
    float r1, r2, h;
public:
    int type3d;

    TruncCone(Coordinate c, float r1, float r2, float h) {
        generateQuadrangle(c, {c.x + r1, c.y, c.z}, {c.x + r2, c.y + h, c.z}, {c.x, c.y + h, c.z});
        revolveObject(360, 0.f);
        type3d = OBJECT_CONE;
        this->r1 = r1;
        this->r2 = r2;
        this->h = h;
    }

    Object* checkIntersection(Sphere *s) {
        Object* obj = new Object();
        obj->vertices.clear();
        int n = 0;
        glm::mat4 model = glm::mat4(1.0f);
        // model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation[0]), glm::vec3(1.0f, 0.0f, 0.0f)); // Вращение по оси X
        model = glm::rotate(model, glm::radians(rotation[1]), glm::vec3(0.0f, 1.0f, 0.0f)); // Вращение по оси Y
        model = glm::rotate(model, glm::radians(rotation[2]), glm::vec3(0.0f, 0.0f, 1.0f)); // Вращение по оси Z
        model = glm::translate(model, position);
        
        bool res = false;
        for (int i = 0; i < vertices.size(); i += 3) {
            glm::vec4 vertex = model * glm::vec4(vertices[i], vertices[i + 1], vertices[i + 2], 1.0f);
            // if (sqrt(pow(vertices[i] + position.x - s->position.x, 2) + pow(vertices[i + 1] + position.y - s->position.y, 2) + pow(vertices[i + 2] + position.z - s->position.z, 2)) <= s->r) {
            //     if (!res) {
            //         res = true;
            //         std::cout << ' ' << s->position.x << ' ' << s->position.y << ' ' << s->position.z << std::endl;
            //         std::cout << sqrt(pow(vertices[i] + position.x - s->position.x, 2) + pow(vertices[i + 1] + position.y - s->position.y, 2) + pow(vertices[i + 2] + position.z - s->position.z, 2)) << std::endl;
            //         std::cout << sqrt(pow(vertex[0] - s->position.x, 2) + pow(vertex[1] - s->position.y, 2) + pow(vertex[2] - s->position.z, 2)) << std::endl;
            //     }
            // }
            if (sqrt(pow(vertex.x - s->position.x, 2) + pow(vertex.y - s->position.y, 2) + pow(vertex.z - s->position.z, 2)) <= s->r) {
                //std::cout << '!' << std::endl;
                obj->vertices.push_back(vertices[i]);
                obj->vertices.push_back(vertices[i + 1]);
                obj->vertices.push_back(vertices[i + 2]);
                n++;
                std::cout << vertices[i] << ' ' << vertices[i + 1] << ' ' << vertices[i + 2] << std::endl;
                if (!res) {
                    res = true;
                    //std::cout << sqrt(pow(vertex[0] - s->position.x, 2) + pow(vertex[1] - s->position.y, 2) + pow(vertex[2] - s->position.z, 2)) << std::endl;
                }
            }
        }
        if (res) {
            std::cout << "Intersection1" << std::endl;
        }

        glm::mat4 modelSphere = glm::mat4(1.0f);
        modelSphere = glm::translate(modelSphere, s->position);

        res = false;
        for (int i = 0; i < s->vertices.size(); i += 3) {
            glm::vec4 vertex = modelSphere * glm::vec4(s->vertices[i], s->vertices[i + 1], s->vertices[i + 2], 1.0f);
            vertex = glm::inverse(model) * vertex; // локальные координаты
            if (vertex.y <= h && vertex.y >= 0) {
                float locR = (vertex.y - h * r1 / (r1 - r2)) / (h / (r2 - r1));
                if (sqrt(pow(vertex.x, 2) + pow(vertex.z, 2)) <= locR) {
                    //std::cout << '!' << std::endl;
                    obj->vertices.push_back(s->vertices[i] + s->position.x);
                    obj->vertices.push_back(s->vertices[i + 1] + s->position.y);
                    obj->vertices.push_back(s->vertices[i + 2] + s->position.z);
                    
                    if (!res) {
                        res = true;
                        
                    }
                }
            }
        }
        if (res) {
            std::cout << "Intersection2" << std::endl;
        }

        for (int i = 0; i < obj->vertices.size() / 3; i ++) {
            obj->indices.push_back(i);
        }
        std::cout << obj->vertices.size() << ' ' << n << std::endl;
        obj->generateBuffers();
        obj->type = OBJECT_NONE;

        return obj;
    }
};