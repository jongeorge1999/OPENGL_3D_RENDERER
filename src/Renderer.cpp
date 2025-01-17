#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <imGui/imgui.h>
#include <imGui/imgui_impl_glfw.h>
#include <imGui/imgui_impl_opengl3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <format>

#include "Renderer.hpp"
#include "Model.hpp"
#include "Shader.hpp"

void Renderer::Render(GLFWwindow* window, Camera* camera, Controller* controller) {
    //create game objects
    Object stormtrooper("Stormtrooper", glm::vec3(4.0f, -0.9f, 0.0f));
    Object backpack("backpack", glm::vec3(-9.5f, 0.1f, 1.5f), glm::vec3(0.5f));
    Object floor("floor", glm::vec3(0.0f, -1.0f, 0.0f));
    Object reflectiveST("Reflective ST", glm::vec3(2.0f, 0.0f, 2.0f));

    //organize game objects
    objects.push_back(&stormtrooper);
    objects.push_back(&backpack);
    objects.push_back(&floor);
    objects.push_back(&reflectiveST);

    //texture flip
    stbi_set_flip_vertically_on_load(true);

    // openGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);  
    glCullFace(GL_BACK); 
    glFrontFace(GL_CCW); 

    // load shaders
    Shader objectShader("../src/shaders/shader.vert", "../src/shaders/shader.frag");
    Shader pointlightcube("../src/shaders/pointlightcube.vert", "../src/shaders/pointlightcube.frag");
    Shader transparentShader("../src/shaders/transparent.vert", "../src/shaders/transparent.frag");
    Shader grassShader("../src/shaders/grass.vert", "../src/shaders/grass.frag");
    Shader screenShader("../src/shaders/screenBuffer.vert", "../src/shaders/screenBuffer.frag");
    Shader skyboxShader("../src/shaders/skybox.vert", "../src/shaders/skybox.frag");
    Shader reflectiveShader("../src/shaders/reflectiveCubemap.vert", "../src/shaders/reflectiveCubemap.frag");
    Shader normalShader("../src/shaders/normals.vert", "../src/shaders/normals.frag",  "../src/shaders/normals.geom");
    Shader depthShader("../src/shaders/depthShader.vert", "../src/shaders/depthShader.frag");
    Shader depthTestShader("../src/shaders/depthTestShader.vert", "../src/shaders/depthTestShader.frag");

    // load models
    Model backpack_obj("../models/backpack/backpack.obj");
    Model stormtrooper_obj("../models/stormtrooper/stormtrooper.obj");
    Model wood_floor_obj("../models/wood_floor/wood_floor.obj");

    //load textures
    unsigned int transparentTexture = tl.loadTexture("../textures/red_window.png");
    unsigned int grassTexture = tl.loadTexture("../textures/grass.png");
    unsigned int brickTexture = tl.loadTexture("../textures/brick.jpg");

    //load skyboxes
    unsigned int cubemapTextureDay = tl.loadCubemap(faces_day);
    unsigned int cubemapTextureNight = tl.loadCubemap(faces_night);
    unsigned int cubemapTextureSpace1 = tl.loadCubemap(faces_space1);
    unsigned int cubemapTextureSpace2 = tl.loadCubemap(faces_space2);
    currSkybox = cubemapTextureSpace2;

    // VAOs and VBOs
    unsigned int lightVAO, VBO, transparentVAO, transparentVBO, grassVAO, grassVBO, quadVAO, quadVBO, skyboxVAO, skyboxVBO;
    setupVAOandVBO(lightVAO, VBO, ph.blandVertsNormalsTex, {3}, 8);
    setupVAOandVBO(transparentVAO, transparentVBO, ph.transparentVertices, {3, 2}, 5);
    setupVAOandVBO(grassVAO, grassVBO, ph.grassVerts, {3, 2}, 5);
    setupVAOandVBO(quadVAO, quadVBO, ph.quadVertices, {2, 2}, 4);
    setupVAOandVBO(skyboxVAO, skyboxVBO, ph.skyboxVertices, {3}, 3);

    //ubo
    unsigned int uniformBlockObjectShader = glGetUniformBlockIndex(objectShader.ID, "Matrices");
    glUniformBlockBinding(objectShader.ID, uniformBlockObjectShader, 0);
    unsigned int uniformBlockReflectiveShader = glGetUniformBlockIndex(reflectiveShader.ID, "Matrices");
    glUniformBlockBinding(reflectiveShader.ID, uniformBlockReflectiveShader, 0);
    unsigned int uniformBlockPointLightCubeShader = glGetUniformBlockIndex(pointlightcube.ID, "Matrices");
    glUniformBlockBinding(pointlightcube.ID, uniformBlockPointLightCubeShader, 0);
    unsigned int uniformBlockGrassShader = glGetUniformBlockIndex(grassShader.ID, "Matrices");
    glUniformBlockBinding(grassShader.ID, uniformBlockGrassShader, 0);
    unsigned int uniformBlockTransparentShader = glGetUniformBlockIndex(transparentShader.ID, "Matrices");
    glUniformBlockBinding(transparentShader.ID, uniformBlockTransparentShader, 0);

    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

    // framebuffers
    Framebuffer framebuffer = createFramebuffer(SCR_WIDTH, SCR_HEIGHT);
    Framebuffer msaa = createMSAAFrameBuffer(SCR_WIDTH, SCR_HEIGHT);
    Framebuffer depthMapBuffer = createDepthMapBuffer();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // perspective (only needs to be set once unless you want to change projection)
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 model;
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // MAIN RENDER LOOP
    while(!glfwWindowShouldClose(window)) {

        //gamma correction
        if (gammaCorrection) {glEnable(GL_FRAMEBUFFER_SRGB);} else { glDisable(GL_FRAMEBUFFER_SRGB);}

        //delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //input
        controller->processInput(window, deltaTime, camera);

        // wireframe
        if (wireFrame) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); } else { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

        // view
        glm::mat4 view = camera->GetViewMatrix();
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // sort the transparent windows
        vector<glm::vec3> windows = sr.getWindows();
        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < windows.size(); i++) {
            float distance = glm::length(camera->Position - windows[i]);
            sorted[distance] = windows[i];
        }

        //variables for shadowing
        glm::mat4 lightProjection, lightView, lightSpaceMatrix;
        float near_plane = 0.1f, far_plane = 25.0f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        lightSpaceMatrix = lightProjection * lightView;

        // render scene from light's point of view (first pass)
        glCullFace(GL_FRONT);
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapBuffer.ID);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glClear(GL_DEPTH_BUFFER_BIT);
        backpack_obj.Draw(depthShader, backpack);
        stormtrooper_obj.Draw(depthShader, stormtrooper);
        stormtrooper_obj.Draw(depthShader, reflectiveST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

        // draw to non-default framebuffer
        if(renderToTexture && useMSAA) { glBindFramebuffer(GL_FRAMEBUFFER, msaa.ID); } else if (renderToTexture && !useMSAA) { glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.ID); }
        
        // clear the buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // setting a fuck ton of uniforms
        objectShader.use();
        sr.setParams(objectShader, *camera);
        objectShader.setVec3("viewPos", camera->Position);
        objectShader.setVec3("lightPos", lightPos);
        objectShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        objectShader.setInt("shadowMap", shadowItem);
        
        // imgui uniforms
        objectShader.setBool("useAmbient", useAmbient);
        objectShader.setBool("useDiffuse", useDiffuse);
        objectShader.setBool("useSpecular", useSpecular);
        objectShader.setBool("useBlinn", useBlinn);
        objectShader.setBool("useFlashlight", useFlashlight);
        objectShader.setBool("useDirectionalLight", useDirectionalLight);
        objectShader.setBool("usePointLight", usePointLight);
        objectShader.setBool("showDepthBuffer", showDepthBuffer);
        objectShader.setFloat("flashlightIntensity", flashlightIntensity);
        objectShader.setFloat("directionalLightIntensity", directionLightIntensity);
        objectShader.setFloat("pointLightIntensity", pointLightIntensity);
        objectShader.setBool("useShadows", useShadows);

        //setting shadow texture
        glActiveTexture(GL_TEXTURE24);
        glBindTexture(GL_TEXTURE_2D, depthMapBuffer.texture);

        //render the objects normally (second pass)
        backpack_obj.Draw(objectShader, backpack);
        stormtrooper_obj.Draw(objectShader, stormtrooper);
        wood_floor_obj.Draw(objectShader, floor);
        stormtrooper_obj.Draw(reflectiveShader, reflectiveST);

        // Pointlight cubes
        pointlightcube.use();
        glDisable(GL_CULL_FACE);
        glBindVertexArray(lightVAO);
        vector<glm::vec3> pointLights = sr.getpointLights();
        for (unsigned int i = 0; i < 4; i++) {
            glm::vec3 val;
            glGetUniformfv(objectShader.ID, glGetUniformLocation(objectShader.ID, std::format("pointLights[{}].diffuse", i).c_str()), glm::value_ptr(val));
            pointlightcube.setVec3("color", val);
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLights[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            pointlightcube.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Grass
        grassShader.use();
        glBindVertexArray(grassVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        vector<glm::vec3> vegetation = sr.getVegetation();
        for (unsigned int i = 0; i < vegetation.size(); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, vegetation[i]);
            grassShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // draw skybox (LAST BUT BEFORE TRANSPARENT)
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(camera->GetViewMatrix())));
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, currSkybox);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        // Windows
        transparentShader.use();
        glBindVertexArray(transparentVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);
        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            transparentShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        //blit multisampled buffer(s) to normal colorbuffer of intermediate FBO. Image is stored in screenTexture
        if(renderToTexture && useMSAA) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa.ID);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.ID);
            glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        }

        // showing the perspective of the dirLight for testing
        if(showDepthMap && !renderToTexture) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            depthTestShader.use();
            depthTestShader.setInt("depthMap", 0);
            glBindVertexArray(quadVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthMapBuffer.texture);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // if rendering onto a quad
         if(renderToTexture && !showDepthMap) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDisable(GL_DEPTH_TEST);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            screenShader.use();
            screenShader.setBool("inverted", inverted);
            screenShader.setBool("grayscale", grayscale);
            screenShader.setBool("sharpen", sharpen);
            screenShader.setBool("blur", blur);
            screenShader.setBool("edgeDetection", edgeDetection);
            glBindVertexArray(quadVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, framebuffer.texture);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glEnable(GL_DEPTH_TEST);
        } else {
            glEnable(GL_DEPTH_TEST); // RE-ENABLE if not rendering to texture
        }

        // imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            static int counter = 0;
            ImGui::Begin("George Engine Debug Menu");    
            ImGui::Text("Debug controls"); 

            // Basic options
            if (ImGui::TreeNode("Light Options"))
            {
                ImGui::Checkbox("Use Ambient?", &useAmbient);
                ImGui::Checkbox("Use Diffuse?", &useDiffuse);  
                ImGui::Checkbox("Use Specular?", &useSpecular); 
                ImGui::Checkbox("Use blinn-phong?", &useBlinn); 
                ImGui::Checkbox("Use Gamma Correction?", &gammaCorrection);
                ImGui::Checkbox("Use Flashlight?", &useFlashlight);
                ImGui::Checkbox("Use Directional Light?", &useDirectionalLight); 
                ImGui::Checkbox("Use Point Light?", &usePointLight); 
                ImGui::Checkbox("Use Shadows?", &useShadows);
                static const char* light[] = { "red", "blue", "white", "green" };
                static int selectedLight = 0;

                if (ImGui::Combo("Light Selector", &selectedLight, light, IM_ARRAYSIZE(light)))
                {
                    // Combo logic here if needed (e.g., firing an event)
                }

                // Display the corresponding slider based on the selected light
                if (selectedLight == 0) 
                {
                    ImGui::SliderFloat3("Light 1 Position", &sr.getpointLights()[0].x, -20.0f, 20.0f);
                } 
                else if (selectedLight == 1) 
                {
                    ImGui::SliderFloat3("Light 2 Position", &sr.getpointLights()[1].x, -20.0f, 20.0f);
                }
                else if (selectedLight == 2) 
                {
                    ImGui::SliderFloat3("Light 2 Position", &sr.getpointLights()[2].x, -20.0f, 20.0f);
                }
                else if (selectedLight == 3) 
                {
                    ImGui::SliderFloat3("Light 2 Position", &sr.getpointLights()[3].x, -20.0f, 20.0f);
                }

                //ImGui::ColorEdit3("clear color", (float*)&clear_color);
                ImGui::SliderFloat("Flashlight Intensity", &flashlightIntensity, 0.0f, 4.0f);
                ImGui::SliderFloat("Pointlight Intensity", &pointLightIntensity, 0.0f, 4.0f);
                ImGui::SliderFloat("Directional Light Intensity", &directionLightIntensity, 0.0f, 4.0f);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Post-Processing Options"))
            {
                ImGui::Checkbox("Use MSAA?", &useMSAA);
                ImGui::Checkbox("Show depth buffer?", &showDepthBuffer);
                ImGui::Checkbox("wireframe?", &wireFrame);
                ImGui::Checkbox("Render to texture?", &renderToTexture);
                ImGui::Checkbox("invert?", &inverted);
                ImGui::Checkbox("grayscale?", &grayscale);
                ImGui::Checkbox("sharpen?", &sharpen);
                ImGui::Checkbox("blur?", &blur);
                ImGui::Checkbox("edge detection?", &edgeDetection);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Game objects"))
            {
                for(auto object : objects) {
                    if (ImGui::TreeNode(object->getName().c_str()))
                    {
                        if (ImGui::SliderFloat3((object->getName() + " Position").c_str(), &object->getPosition().x, -20.0f, 20.0f)) {
                            object->setPosition(object->getPosition());
                        }

                        static float scaleScalar = 1.0f;
                        if (ImGui::SliderFloat((object->getName() + " Scale").c_str(), &scaleScalar, 0.1f, 10.0f)) {
                            object->setScale(glm::vec3(scaleScalar, scaleScalar, scaleScalar)); 
                        }

                        static glm::vec3 eulerRotation(0.0f, 0.0f, 0.0f);
                        if (ImGui::SliderFloat3((object->getName() + " Rotation").c_str(), &eulerRotation.x, -180.0f, 180.0f)) {
                            glm::quat quatRotation = glm::quat(glm::radians(eulerRotation));
                            object->setRotation(quatRotation);
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }


            static const char* items[]{"Day","Night", "Space1", "Space2"};
            static int Selecteditem = 4;
            if (ImGui::Combo("Skybox Selector", &Selecteditem, items, IM_ARRAYSIZE(items)))
            {
                // Here event is fired
                if(Selecteditem == 0) { currSkybox = cubemapTextureDay; }
                else if(Selecteditem == 1) { currSkybox = cubemapTextureNight; }
                else if(Selecteditem == 2) { currSkybox = cubemapTextureSpace1; }
                else if(Selecteditem == 3) { currSkybox = cubemapTextureSpace2; }
            }

            ImGui::SliderInt("ShadowTexture", &shadowItem, 0, 36);
            ImGui::Checkbox("Show Depth Map?", &showDepthMap);

            if (ImGui::Button("Close Application")) { glfwSetWindowShouldClose(window, true); }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::Text("CamX %0.1f CamY %0.1f CamZ %0.1f", camera->Position.x, camera->Position.y, camera->Position.z);
            ImGui::Text("CamYaw %0.1f CamPitch %0.1f", std::fmod(camera->Yaw, 360), camera->Pitch);
            ImGui::End();

            // render the imgui window
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        // swap chain and IO handling
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    // cleaning up after ourselves
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteVertexArrays(1, &grassVAO);
    glDeleteVertexArrays(1, &transparentVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &grassVBO);
    glDeleteBuffers(1, &transparentVBO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteBuffers(1, &uboMatrices);
    deleteFramebuffer(framebuffer);
    deleteFramebuffer(msaa);
    deleteFramebuffer(depthMapBuffer);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}