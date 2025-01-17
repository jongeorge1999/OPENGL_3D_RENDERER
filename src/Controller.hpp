#pragma once

#include <GLFW/glfw3.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.hpp"
#include "Camera.hpp"

class Controller {
    private:
        bool cursorDisabled = true;
        bool justPressed = false;
        bool fast = false;
    public:

        Controller(){}
        ~Controller(){}

        void processInput(GLFWwindow *window, float deltaTime, Camera *camera) {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !cursorDisabled && !justPressed) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                justPressed = true;
                cursorDisabled = true;
            } else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && cursorDisabled && !justPressed) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                justPressed = true;
                cursorDisabled = false;
            }
            if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) fast = true;
            if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) fast = false;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera->ProcessKeyboard(FORWARD, deltaTime, fast);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera->ProcessKeyboard(BACKWARD, deltaTime, fast);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera->ProcessKeyboard(LEFT, deltaTime, fast);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera->ProcessKeyboard(RIGHT, deltaTime, fast);
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera->ProcessKeyboard(UP, deltaTime, fast);
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera->ProcessKeyboard(DOWN, deltaTime, fast);
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) justPressed = false;
        }

        bool isCursorDisabled() { return cursorDisabled; }
        bool wasJustPressed() { return justPressed; }
};