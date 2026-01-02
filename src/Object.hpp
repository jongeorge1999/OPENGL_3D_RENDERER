#pragma once

#include "Model.hpp"
#include <string>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <random>

class Shader;

using namespace std;

class Object {
private:
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    glm::mat4 modelMatrix;
    std::string name;
    std::vector<Object*>* objects;
    Model model;
    Shader* shaderStored;
    bool isLight;
    glm::vec3 lightColor;

    // Recalculate the model matrix whenever transformations change
    void updateModelMatrix() {
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix *= glm::mat4(rotation);
        modelMatrix = glm::scale(modelMatrix, scale);
    }

public:
    Object(Shader* shaderIn = nullptr,
        const std::string& modelPath = "",
        const std::string& objName = "",
        glm::vec3 objPos   = glm::vec3(0.0f),
        glm::vec3 objScale = glm::vec3(1.0f),
        glm::quat objRot   = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        bool is_light      = false)
        :
        position(objPos),
        rotation(objRot),
        scale(objScale),
        modelMatrix(glm::mat4(1.0f)),
        name(objName),
        model(modelPath),
        shaderStored(shaderIn),
        isLight(is_light)
    {
        updateModelMatrix();

        if (isLight) {
            static std::mt19937 rng{ std::random_device{}() };
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            lightColor = glm::vec3(dist(rng), dist(rng), dist(rng));
        }
    }

    glm::vec3& getLightColor() {
        return lightColor;
    }

    void setLightColor(glm::vec3 newColor) {
        lightColor = newColor;
    }

    glm::vec3& getPosition() { 
        return position;
    }

    bool is_light() {
        if (isLight) {
            return true;
        }
        return false;
    }

    void Draw(Shader shader) {
       model.Draw(shader, this->getModelMatrix());
    }

    void Draw() {
        model.Draw(*shaderStored, this->getModelMatrix());
    }

    void setPosition(const glm::vec3& newPosition) {
        position = newPosition;
        updateModelMatrix();
    }

    glm::quat& getRotation() {
        return rotation;
    }

    void setRotation(const glm::quat& newRotation) {
        rotation = newRotation;
        updateModelMatrix();
    }

    glm::vec3& getScale() {
        return scale;
    }

    void setScale(const glm::vec3& newScale) {
        scale = newScale;
        updateModelMatrix();
    }

    glm::mat4 getModelMatrix() const {
        return modelMatrix;
    }

    void setModelMatrix(glm::mat4 matrix) {
        modelMatrix = matrix;
    }

    string getName() {
        return name;
    }

    void setName(const std::string& s) { name = s; }
};

