#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <imGui/imgui.h>
#include <imGui/imgui_impl_glfw.h>
#include <imGui/imgui_impl_opengl3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <format>
#include <filesystem>

#include "Renderer.hpp"
#include "Model.hpp"
#include "SceneReader.hpp"
#include "TexLoader.hpp"
#include "PrimitiveHelper.hpp"
#include "Shader.hpp"

//Texture loader
TexLoader tl;

// primitve helper
PrimitiveHelper ph;

//scene reader
SceneReader sr;

//screen width and height
const unsigned int SCR_WIDTH = 2400;
const unsigned int SCR_HEIGHT = 1800;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// imgui variables
ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
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

void Renderer::Render(GLFWwindow* window, Camera* camera, Controller* controller) {
    //texture flip
    stbi_set_flip_vertically_on_load(true);

    // z buffer
    glEnable(GL_DEPTH_TEST);

    //stencil testing
    glEnable(GL_STENCIL_TEST);

    //blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //face culling
    glEnable(GL_CULL_FACE);  
    glCullFace(GL_BACK); 
    glFrontFace(GL_CCW); 

    // loading shaders
    Shader objectShader("../src/shaders/shader.vert", "../src/shaders/shader.frag");
    Shader pointlightcube("../src/shaders/pointlightcube.vert", "../src/shaders/pointlightcube.frag");
    Shader transparentShader("../src/shaders/transparent.vert", "../src/shaders/transparent.frag");
    Shader grassShader("../src/shaders/grass.vert", "../src/shaders/grass.frag");
    Shader screenShader("../src/shaders/screenBuffer.vert", "../src/shaders/screenBuffer.frag");
    Shader skyboxShader("../src/shaders/skybox.vert", "../src/shaders/skybox.frag");
    Shader reflectiveShader("../src/shaders/reflectiveCubemap.vert", "../src/shaders/reflectiveCubemap.frag");

    // load models
    Model backpack_obj("../models/backpack/backpack.obj");
    Model stormtrooper_obj("../models/stormtrooper/stormtrooper.obj");
    Model wood_floor_obj("../models/wood_floor/wood_floor.obj");


    //load textures
    unsigned int transparentTexture = tl.loadTexture("../textures/red_window.png");
    unsigned int grassTexture = tl.loadTexture("../textures/grass.png");

    //skybox-day texture
    vector<std::string> faces_day
    {
        "../textures/skybox_day/right.jpg",
        "../textures/skybox_day/left.jpg",
        "../textures/skybox_day/top.jpg",
        "../textures/skybox_day/bottom.jpg",
        "../textures/skybox_day/front.jpg",
        "../textures/skybox_day/back.jpg"
    };
    unsigned int cubemapTextureDay = tl.loadCubemap(faces_day);
    currSkybox = cubemapTextureDay;

    //skybox-night texture
    vector<std::string> faces_night
    {
        "../textures/skybox_night/right.jpg",
        "../textures/skybox_night/left.jpg",
        "../textures/skybox_night/top.jpg",
        "../textures/skybox_night/bottom.jpg",
        "../textures/skybox_night/front.jpg",
        "../textures/skybox_night/back.jpg"
    };
    unsigned int cubemapTextureNight = tl.loadCubemap(faces_night);

    // bind buffers and VAOs
    unsigned int VBO, lightVAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ph.blandVertsNormalsTex), ph.blandVertsNormalsTex.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); 

    //transparent
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ph.transparentVertices), ph.transparentVertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    //grass
    unsigned int grassVAO, grassVBO;
    glGenVertexArrays(1, &grassVAO);
    glGenBuffers(1, &grassVBO);
    glBindVertexArray(grassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ph.grassVerts), ph.grassVerts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);


    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ph.quadVertices), ph.quadVertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


     // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ph.skyboxVertices), ph.skyboxVertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    // framebuffer configuration
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    //render buffer object
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl; }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    //****************************************************************************************************************************
    //**************************************************************************************************************************** 
    // MAIN RENDER LOOP
    while(!glfwWindowShouldClose(window)) {

        //delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //input
        controller->processInput(window, deltaTime, camera);

        // imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // wireframe
        if (wireFrame) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // sort the transparent windows
        vector<glm::vec3> windows = sr.getWindows();
        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < windows.size(); i++) {
            float distance = glm::length(camera->Position - windows[i]);
            sorted[distance] = windows[i];
        }

        // render to screen or to texture
        if(renderToTexture) {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glEnable(GL_DEPTH_TEST);
        }

        // clear the buffers
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        //do objects first
        objectShader.use();

        // doing a lot of the work in the scene reader
        sr.setParams(objectShader, *camera);
        
        // imgui uniforms
        objectShader.setBool("useAmbient", useAmbient);
        objectShader.setBool("useDiffuse", useDiffuse);
        objectShader.setBool("useSpecular", useSpecular);
        objectShader.setBool("useFlashlight", useFlashlight);
        objectShader.setBool("useDirectionalLight", useDirectionalLight);
        objectShader.setBool("usePointLight", usePointLight);
        objectShader.setBool("showDepthBuffer", showDepthBuffer);
        objectShader.setFloat("flashlightIntensity", flashlightIntensity);
        objectShader.setFloat("directionalLightIntensity", directionLightIntensity);
        objectShader.setFloat("pointLightIntensity", pointLightIntensity);
       
        // setting the object transform
        glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera->GetViewMatrix();
        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);

        // render the loaded backpack model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        objectShader.setMat4("model", model);
        backpack_obj.Draw(objectShader);

        // render the loaded stormtrooper model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        if(rotateModels) { model = glm::rotate(model, (float)glfwGetTime() * glm::radians(spinSpeed), glm::vec3(0.0f, 1.0f, 0.0f)); }
        objectShader.setMat4("model", model);
        stormtrooper_obj.Draw(objectShader);

        // render the loaded floor model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f));	
        objectShader.setMat4("model", model);
        wood_floor_obj.Draw(objectShader);

        //disable face culling for transparent objects
        glDisable(GL_CULL_FACE);

        // change to using the pointLightcube shader
        pointlightcube.use();
        pointlightcube.setMat4("projection", projection);
        pointlightcube.setMat4("view", view);
        glBindVertexArray(lightVAO);
        glm::vec3* pointLights = sr.getpointLights();
        // draw all the lamp objects for every light with correct color
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

        // switch to the grass shader
        grassShader.use();
        grassShader.setMat4("projection", projection);
        grassShader.setMat4("view", view);
        grassShader.setInt("texture1", 0);
        glBindVertexArray(grassVAO);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        // draw vegetation
        vector<glm::vec3> vegetation = sr.getVegetation();
        for (unsigned int i = 0; i < vegetation.size(); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, vegetation[i]);
            grassShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        //render reflective stormtrooper model
        reflectiveShader.use();
        reflectiveShader.setMat4("projection", projection);
        reflectiveShader.setMat4("view", view);
        reflectiveShader.setInt("texture1", 0);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 4.0f, -6.0f));
        if(rotateModels) { model = glm::rotate(model, (float)glfwGetTime() * glm::radians(spinSpeed), glm::vec3(0.0f, 1.0f, 0.0f)); }
        model = glm::scale(model, glm::vec3(0.5f));
        reflectiveShader.setMat4("model", model);
        stormtrooper_obj.Draw(reflectiveShader);

        // draw skybox (LAST BUT BEFORE TRANSPARENT)
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(camera->GetViewMatrix())));
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, currSkybox);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);


        // switch to the transparency shader
        transparentShader.use();
        transparentShader.setMat4("projection", projection);
        transparentShader.setMat4("view", view);
        transparentShader.setInt("texture1", 0);
        glBindVertexArray(transparentVAO);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);
        // draw all the windows from furthest to nearest
        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            transparentShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // if rendering onto a quad
        if(renderToTexture) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDisable(GL_DEPTH_TEST);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            screenShader.use();
            screenShader.setBool("inverted", inverted);
            screenShader.setBool("grayscale", grayscale);
            screenShader.setBool("sharpen", sharpen);
            screenShader.setBool("blur", blur);
            screenShader.setBool("edgeDetection", edgeDetection);
            glBindVertexArray(quadVAO);
            glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        } else {
            glEnable(GL_DEPTH_TEST); // RE-ENABLE if not rendering to texture
        }

         

        // IMGUI BUTTONS
        {
            static int counter = 0;

            ImGui::Begin("George Engine Debug Menu");            

            ImGui::Text("Debug controls");       
            ImGui::Checkbox("Rotate Models?", &rotateModels);
            ImGui::Checkbox("Use Ambient?", &useAmbient);
            ImGui::Checkbox("Use Diffuse?", &useDiffuse);  
            ImGui::Checkbox("Use Specular?", &useSpecular);  
            ImGui::Checkbox("Use Flashlight?", &useFlashlight);
            ImGui::Checkbox("Use Directional Light?", &useDirectionalLight); 
            ImGui::Checkbox("Use Point Light?", &usePointLight); 
            ImGui::Checkbox("Show depth buffer?", &showDepthBuffer);
            ImGui::Checkbox("wireframe?", &wireFrame);
            ImGui::Checkbox("Render to texture?", &renderToTexture);
            ImGui::Checkbox("invert?", &inverted);
            ImGui::Checkbox("grayscale?", &grayscale);
            ImGui::Checkbox("sharpen?", &sharpen);
            ImGui::Checkbox("blur?", &blur);
            ImGui::Checkbox("edge detection?", &edgeDetection);

            ImGui::SliderFloat("Rotation Speed", &spinSpeed, 0.0f, 500.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);

            ImGui::SliderFloat("Flashlight Intensity", &flashlightIntensity, 0.0f, 4.0f);
            ImGui::SliderFloat("Pointlight Intensity", &pointLightIntensity, 0.0f, 4.0f);
            ImGui::SliderFloat("Directional Light Intensity", &directionLightIntensity, 0.0f, 4.0f);


            static const char* items[]{"Day","Night"};
            static int Selecteditem = 0;
            if (ImGui::Combo("Skybox Selector", &Selecteditem, items, IM_ARRAYSIZE(items)))
            {
                // Here event is fired
                if(Selecteditem == 0) {
                    currSkybox = cubemapTextureDay; //set skybox to day
                } else if(Selecteditem == 1) {
                    currSkybox = cubemapTextureNight; //set skybox to night
                }
            }

            //if (ImGui::Button("Button"))
            //    counter++;
            //ImGui::SameLine();
            //ImGui::Text("counter = %d", counter);

            if (ImGui::Button("Close Application")) { glfwSetWindowShouldClose(window, true); }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
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
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &framebuffer);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}