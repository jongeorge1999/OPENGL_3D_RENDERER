#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Camera.hpp"
#include "Controller.hpp"
#include "Object.hpp"
#include "TexLoader.hpp"
#include "SceneReader.hpp"
#include "PrimitiveHelper.hpp"

class Renderer {
    private:
    public:
        Renderer(){}
        ~Renderer(){}

        //screen width and height
        const unsigned int SCR_WIDTH = 2400;
        const unsigned int SCR_HEIGHT = 1800;

        //Game object manager
        std::vector<Object*> objects;

        //Texture loader
        TexLoader tl;

        //scene reader
        SceneReader sr;

        // primitve helper
        PrimitiveHelper ph;

        // timing
        float deltaTime = 0.0f;
        float lastFrame = 0.0f;

        //ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
        bool rotateModels = true;
        bool show_another_window = false;
        bool useDiffuse = true;
        bool useSpecular = true;
        bool useAmbient = true;
        bool useFlashlight = false;
        bool useDirectionalLight = true;
        bool usePointLight = true;
        bool showDepthBuffer = false;
        bool wireFrame = false;
        bool renderToTexture = true;
        bool inverted = false;
        bool grayscale = false;
        bool sharpen = false;
        bool blur = false;
        bool edgeDetection = false;
        float spinSpeed = 0.0f;
        float flashlightIntensity = 1.0f;
        float directionLightIntensity = 1.0f;
        float pointLightIntensity = 1.0f;
        unsigned int currSkybox = 0;


        void Render(GLFWwindow* window, Camera* camera, Controller* controller);

        struct Framebuffer {
            unsigned int ID;            // Framebuffer ID
            unsigned int texture;       // Texture attachment
            unsigned int renderbuffer;  // Renderbuffer attachment
        };

        inline Framebuffer createFramebuffer(int width, int height, GLenum textureFormat = GL_RGB, GLenum depthStencilFormat = GL_DEPTH24_STENCIL8) {
            Framebuffer fb;

            // Generate framebuffer
            glGenFramebuffers(1, &fb.ID);
            glBindFramebuffer(GL_FRAMEBUFFER, fb.ID);

            // Create color attachment texture
            glGenTextures(1, &fb.texture);
            glBindTexture(GL_TEXTURE_2D, fb.texture);
            glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, width, height, 0, 
                        (textureFormat == GL_RGB ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.texture, 0);

            // Create depth and stencil renderbuffer
            glGenRenderbuffers(1, &fb.renderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, fb.renderbuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, depthStencilFormat, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb.renderbuffer);

            // Check framebuffer status
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
            }

            // Unbind framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return fb;
        }

        inline Framebuffer createMSAAFrameBuffer(int width, int height, GLenum textureFormat = GL_RGB, GLenum depthStencilFormat = GL_DEPTH24_STENCIL8) {
            Framebuffer fb;

            glGenFramebuffers(1, &fb.ID);
            glBindFramebuffer(GL_FRAMEBUFFER, fb.ID);
            // create a multisampled color attachment texture
            glGenTextures(1, &fb.texture);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fb.texture);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, fb.texture, 0);
            // create a (also multisampled) renderbuffer object for depth and stencil attachments
            unsigned int rbo;
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return fb;
        }

        // Function to delete a framebuffer
        inline void deleteFramebuffer(const Framebuffer &fb) {
            glDeleteFramebuffers(1, &fb.ID);
            glDeleteTextures(1, &fb.texture);
            glDeleteRenderbuffers(1, &fb.renderbuffer);
        }

        inline glm::quat rotate(float deltaTime, float spinSpeed, glm::quat objRotation) {
            float angle = deltaTime * glm::radians(spinSpeed); // Incremental rotation angle
            glm::quat incrementalRotation = glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Y-axis rotation
            objRotation = incrementalRotation * objRotation; // Combine rotations
            objRotation = glm::normalize(objRotation); // Normalize to avoid precision issues
            return objRotation;
        }


        // Helper function to set up VAO and VBO
        inline void setupVAOandVBO(unsigned int &VAO, unsigned int &VBO, 
                            const std::vector<float> &data, 
                            const std::vector<int> &attributes, 
                            int stride) {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

            // Set up vertex attributes
            int offset = 0;
            for (size_t i = 0; i < attributes.size(); ++i) {
                glEnableVertexAttribArray(i);
                glVertexAttribPointer(i, attributes[i], GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(offset * sizeof(float)));
                offset += attributes[i]; // Increment offset by the size of this attribute
            }

            glBindVertexArray(0); // Unbind VAO
        }


        vector<std::string> faces_day {
            "../textures/skybox_day/right.jpg",
            "../textures/skybox_day/left.jpg",
            "../textures/skybox_day/top.jpg",
            "../textures/skybox_day/bottom.jpg",
            "../textures/skybox_day/front.jpg",
            "../textures/skybox_day/back.jpg"
         };

        vector<std::string> faces_night {
            "../textures/skybox_night/right.jpg",
            "../textures/skybox_night/left.jpg",
            "../textures/skybox_night/top.jpg",
            "../textures/skybox_night/bottom.jpg",
            "../textures/skybox_night/front.jpg",
            "../textures/skybox_night/back.jpg"
        };

        vector<std::string> faces_space1 {
            "../textures/space_skybox1/space1_left.png",
            "../textures/space_skybox1/space1_right.png",
            "../textures/space_skybox1/space1_up.png",
            "../textures/space_skybox1/space1_down.png",
            "../textures/space_skybox1/space1_front.png",
            "../textures/space_skybox1/space1_back.png"
        };
        
        vector<std::string> faces_space2 {
            "../textures/space_skybox2/space2_left.png",
            "../textures/space_skybox2/space2_right.png",
            "../textures/space_skybox2/space2_top.png",
            "../textures/space_skybox2/space2_bottom.png",
            "../textures/space_skybox2/space2_front.png",
            "../textures/space_skybox2/space2_back.png"
        };


};