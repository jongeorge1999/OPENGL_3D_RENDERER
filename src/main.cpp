#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Renderer.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

const unsigned int SCR_WIDTH = 2400;
const unsigned int SCR_HEIGHT = 1800;

//controller
Controller controller;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//main function
int main () {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    //create the window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OPENGL 3D RENDERER", NULL, NULL);
    if (window == NULL) { std::cout << "Failed to create GLFW window" << std::endl; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback); 
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cout << "Failed to initialize GLAD" << std::endl; return -1; }

    //render the scene
    Renderer renderer;
    renderer.Render(window, &camera, &controller);

    //terminate program
    glfwTerminate();
    return 0;
}


//CALLBACKS AND MOUSE CONTROL
void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); } 
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { camera.ProcessMouseScroll(static_cast<float>(yoffset)); }
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;
    if(controller.isCursorDisabled()) { camera.ProcessMouseMovement(xoffset, yoffset); } 
    else { camera.ProcessMouseMovement(0, 0); }
}