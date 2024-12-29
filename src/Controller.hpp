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
        bool firstMouse = true;
    public:

        Controller(){}

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
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                camera->ProcessKeyboard(FORWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                camera->ProcessKeyboard(BACKWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                camera->ProcessKeyboard(LEFT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                camera->ProcessKeyboard(RIGHT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
                camera->ProcessKeyboard(UP, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
                camera->ProcessKeyboard(DOWN, deltaTime);

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
                justPressed = false;
        }

        bool isCursorDisabled() { return cursorDisabled; }
        bool wasJustPressed() { return justPressed; }
        bool isFirstMouse() { return firstMouse; }
};