#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Camera.hpp"
#include "Controller.hpp"

class Renderer {
    private:
    public:
        Renderer(){}
        ~Renderer(){}

        void Render(GLFWwindow* window, Camera* camera, Controller* controller);
};