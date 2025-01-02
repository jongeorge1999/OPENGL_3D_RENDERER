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

class Renderer {
    private:
    public:
        Renderer(){}
        ~Renderer(){}

        void Render(GLFWwindow* window, Camera* camera, Controller* controller);

        // Helper function to set up VAO and VBO
        void setupVAOandVBO(unsigned int &VAO, unsigned int &VBO, 
                            const std::vector<float> &data, 
                            const std::vector<int> &attributes, 
                            int stride);

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


};