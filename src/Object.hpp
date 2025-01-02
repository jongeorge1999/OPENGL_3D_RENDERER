using namespace std;

class Object {
private:
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    glm::mat4 modelMatrix;
    std::string name;

    // Recalculate the model matrix whenever transformations change
    void updateModelMatrix() {
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix *= glm::mat4(rotation);
        modelMatrix = glm::scale(modelMatrix, scale);
    }

public:
    Object(const std::string& objName = "", glm::vec3 objPos = glm::vec3(0.0f), glm::vec3 objScale = glm::vec3(1.0f))
        : position(objPos), 
          rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)), 
          scale(objScale), modelMatrix(glm::mat4(1.0f)), 
          name(objName) 
          { 
            updateModelMatrix();
        }

    glm::vec3& getPosition() { 
        return position; // Return a copy (const behavior)
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
};

