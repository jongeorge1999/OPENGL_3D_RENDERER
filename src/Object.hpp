#pragma once

#include "Model.hpp"
#include <string>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp> // for glm::mat4_cast

class Shader; // forward-declare only if you have methods taking Shader&

using namespace std;

class Object {
private:
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    glm::mat4 modelMatrix;
    std::string name;
    std::vector<Object*>* objects;
    //const std::string& path;
    Model model;
    Shader* shaderStored;

    // Recalculate the model matrix whenever transformations change
    void updateModelMatrix() {
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix *= glm::mat4(rotation);
        modelMatrix = glm::scale(modelMatrix, scale);
    }

public:
    Object(Shader* shaderIn = nullptr, const std::string& modelPath = "", std::vector<Object*> *vec = nullptr, const std::string& objName = "", glm::vec3 objPos = glm::vec3(0.0f), glm::vec3 objScale = glm::vec3(1.0f), glm::quat objRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
        : 
          objects(vec),
          position(objPos), 
          rotation(objRot), 
          scale(objScale), modelMatrix(glm::mat4(1.0f)), 
          name(objName),
          model(modelPath),
          shaderStored(shaderIn)
          { 
            updateModelMatrix();
            //model = Model(modelPath);
            vec->push_back(this);
        }

    glm::vec3& getPosition() { 
        return position; // Return a copy (const behavior)
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
        updateModelMatrix(); // Update modelMatrix
    }

    glm::vec3& getScale() {
        return scale;
    }

    void setScale(const glm::vec3& newScale) {
        scale = newScale;
        updateModelMatrix(); // Update modelMatrix
    }

    glm::mat4 getModelMatrix() const {
        return modelMatrix; // Return the latest model matrix
    }

    string getName() {
        return name;
    }

    void setName(const std::string& s) { name = s; }
};

