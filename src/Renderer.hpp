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
        const unsigned int SHADOW_WIDTH = 4096;
        const unsigned int SHADOW_HEIGHT = 4096;
        const unsigned int POINT_SHADOW_WIDTH = 4096;
        const unsigned int POINT_SHADOW_HEIGHT = 4096;

        const int NUM_POINT_LIGHTS = 4;

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

        glm::vec3 lightPos = glm::vec3(-17.35f, 12.35f, 14.55f);

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
        float directionLightIntensity = 0.1f;
        float pointLightIntensity = 1.0f;
        unsigned int currSkybox = 0;
        bool useBlinn = true;
        bool gammaCorrection = true;
        bool useShadows = true;
        bool showDepthMap = false;
        int shadowItem = 24;
        bool useMSAA = true;
        bool useNormalMaps = true;
        float shadowFactor = 0.4;
        bool useSmoothShadows = true;
        float exposure = 1.0;
        float shadowBias = 0.05;
        float pointLightRadius = 25.0;

        void Render(GLFWwindow* window, Camera* camera, Controller* controller);

        struct Framebuffer {
            unsigned int ID;            // Framebuffer ID
            unsigned int texture;       // Texture attachment
            unsigned int renderbuffer;  // Renderbuffer attachment
        };
        
        unsigned int CopyTexture(GLuint srcTexture, GLenum target, int width, int height)
        {
            // Create a new texture
            unsigned int dstTexture;
            glGenTextures(1, &dstTexture);
            glBindTexture(target, dstTexture);

            // Copy the texture parameters
            glBindTexture(target, srcTexture);
            GLint wrapS, wrapT, minFilter, magFilter;
            glGetTexParameteriv(target, GL_TEXTURE_WRAP_S, &wrapS);
            glGetTexParameteriv(target, GL_TEXTURE_WRAP_T, &wrapT);
            glGetTexParameteriv(target, GL_TEXTURE_MIN_FILTER, &minFilter);
            glGetTexParameteriv(target, GL_TEXTURE_MAG_FILTER, &magFilter);
            
            glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);

            // Copy the texture data
            glTexImage2D(target, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glCopyImageSubData(srcTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
                            dstTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
                            width, height, 1);

            glBindTexture(target, 0);

            return dstTexture;
        }

        glm::quat eulerDegreesToQuat(const glm::vec3& eulerDegrees) {
            glm::vec3 radians = glm::radians(eulerDegrees);
            return glm::normalize(glm::quat(radians));
        }

        unsigned int CopyCubemap(unsigned int srcDepthCubemap, int width, int height) {
            // Create a new depth cubemap
            unsigned int dstDepthCubemap;
            glGenTextures(1, &dstDepthCubemap);
            glBindTexture(GL_TEXTURE_CUBE_MAP, dstDepthCubemap);

            for (unsigned int i = 0; i < 6; ++i) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            }

            // Copy the depth data for all six faces
            for (unsigned int i = 0; i < 6; ++i) {
                glCopyImageSubData(
                    srcDepthCubemap, GL_TEXTURE_CUBE_MAP, 0, 0, 0, i,
                    dstDepthCubemap, GL_TEXTURE_CUBE_MAP, 0, 0, 0, i,
                    width, height, 1
                );
            }

            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            return dstDepthCubemap;
        }




        inline Framebuffer createFramebuffer(int width, int height, GLenum textureFormat = GL_RGB, GLenum depthStencilFormat = GL_DEPTH24_STENCIL8) {
            Framebuffer fb;

            // Generate framebuffer
            glGenFramebuffers(1, &fb.ID);
            glBindFramebuffer(GL_FRAMEBUFFER, fb.ID);

            // Create color attachment texture
            glGenTextures(1, &fb.texture);
            glBindTexture(GL_TEXTURE_2D, fb.texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, 
                        (textureFormat == GL_RGB ? GL_RGB : GL_RGBA), GL_FLOAT, NULL);
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

        inline Framebuffer createDepthMapBuffer() {
            Framebuffer fb;

            // configure depth map FBO
            glGenFramebuffers(1, &fb.ID);
            // create depth texture
            unsigned int depthMap;
            glGenTextures(1, &fb.texture);
            glBindTexture(GL_TEXTURE_2D, fb.texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            // attach depth texture as FBO's depth buffer
            glBindFramebuffer(GL_FRAMEBUFFER, fb.ID);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb.texture, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return fb;
        }

        inline Framebuffer createDepthCubemapBuffer() {
            Framebuffer fb;
            glGenFramebuffers(1, &fb.ID);
            // create depth cubemap texture
            unsigned int depthCubemap;
            glGenTextures(1, &fb.texture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, fb.texture);
            for (unsigned int i = 0; i < 6; ++i)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, POINT_SHADOW_WIDTH, POINT_SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            // attach depth texture as FBO's depth buffer
            glBindFramebuffer(GL_FRAMEBUFFER, fb.ID);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fb.texture, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return fb;
        }

        // Function to delete a framebuffer
        inline void deleteFramebuffer(const Framebuffer &fb) {
            glDeleteFramebuffers(1, &fb.ID);
            glDeleteTextures(1, &fb.texture);
            glDeleteRenderbuffers(1, &fb.renderbuffer);
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