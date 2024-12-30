#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.hpp"
#include "Camera.hpp"

class SceneReader {    
    private:
        // positions of the point lights
        glm::vec3 pointLightPositions[4] = {
            glm::vec3( 0.7f,  0.2f,  2.0f),
            glm::vec3( 2.3f, 3.3f, -4.0f),
            glm::vec3(-4.0f,  2.0f, -12.0f),
            glm::vec3( -5.0f,  0.0f, -5.0f)
        };

        // transparent window locations
        vector<glm::vec3> windows {
            glm::vec3(-1.5f, 0.0f, -0.48f),
            glm::vec3( 1.5f, 0.0f, 0.51f),
            glm::vec3( 0.0f, 0.0f, 0.7f),
            glm::vec3(-0.3f, 0.0f, -2.3f),
            glm::vec3( 0.5f, 0.0f, -0.6f)
        };

        // transparent vegetation locations
        std::vector<glm::vec3> vegetation {
            glm::vec3(-3.5f, -0.4f, -4.48f),
            glm::vec3( 3.5f, -0.4f, 4.51f),
            glm::vec3( 1.0f, -0.4f, 4.7f),
            glm::vec3(-1.3f, -0.4f, -3.3f),
            glm::vec3 (6.5f, -0.4f, -5.6f)
        };

    public:
        SceneReader(){}
        ~SceneReader(){}
        void setParams(Shader objectShader, Camera camera) {

            // be sure to activate shader when setting uniforms/drawing objects
            objectShader.setVec3("viewPos", camera.Position);
            objectShader.setFloat("material.shininess", 32.0f);

            // directional light
            objectShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
            objectShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
            objectShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
            objectShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
            // point light 1
            objectShader.setVec3("pointLights[0].position", pointLightPositions[0]);
            objectShader.setVec3("pointLights[0].ambient", 0.05f, 0.00f, 0.00f);
            objectShader.setVec3("pointLights[0].diffuse", 1.0f, 0.0f, 0.0f);
            objectShader.setVec3("pointLights[0].specular", 1.0f, 0.7f, 0.7f);
            objectShader.setFloat("pointLights[0].constant", 1.0f);
            objectShader.setFloat("pointLights[0].linear", 0.09f);
            objectShader.setFloat("pointLights[0].quadratic", 0.032f);
            // point light 2
            objectShader.setVec3("pointLights[1].position", pointLightPositions[1]);
            objectShader.setVec3("pointLights[1].ambient", 0.00f, 0.00f, 0.05f);
            objectShader.setVec3("pointLights[1].diffuse", 0.0f, 0.0f, 1.0f);
            objectShader.setVec3("pointLights[1].specular", 0.7f, 0.7f, 1.0f);
            objectShader.setFloat("pointLights[1].constant", 1.0f);
            objectShader.setFloat("pointLights[1].linear", 0.09f);
            objectShader.setFloat("pointLights[1].quadratic", 0.032f);
            // point light 3
            objectShader.setVec3("pointLights[2].position", pointLightPositions[2]);
            objectShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
            objectShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
            objectShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
            objectShader.setFloat("pointLights[2].constant", 1.0f);
            objectShader.setFloat("pointLights[2].linear", 0.09f);
            objectShader.setFloat("pointLights[2].quadratic", 0.032f);
            // point light 4
            objectShader.setVec3("pointLights[3].position", pointLightPositions[3]);
            objectShader.setVec3("pointLights[3].ambient", 0.00f, 0.05f, 0.00f);
            objectShader.setVec3("pointLights[3].diffuse", 0.0f, 1.0f, 0.0f);
            objectShader.setVec3("pointLights[3].specular", 0.7f, 1.0f, 0.7f);
            objectShader.setFloat("pointLights[3].constant", 1.0f);
            objectShader.setFloat("pointLights[3].linear", 0.09f);
            objectShader.setFloat("pointLights[3].quadratic", 0.032f);
            // spotLight
            objectShader.setVec3("spotLight.position", camera.Position);
            objectShader.setVec3("spotLight.direction", camera.Front);
            objectShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
            objectShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
            objectShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
            objectShader.setFloat("spotLight.constant", 1.0f);
            objectShader.setFloat("spotLight.linear", 0.09f);
            objectShader.setFloat("spotLight.quadratic", 0.032f);
            objectShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
            objectShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
        }

        glm::vec3* getpointLights() { return pointLightPositions; }
        vector<glm::vec3> getWindows() { return windows; }
        vector<glm::vec3> getVegetation() { return vegetation; }
};