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
#include <filesystem>

#include "Renderer.hpp"
#include "Model.hpp"
#include "SceneReader.hpp"
#include "TexLoader.hpp"
#include "PrimitiveHelper.hpp"
#include "Shader.hpp"
#include "Object.hpp"

//Texture loader
TexLoader tl;

// primitve helper
PrimitiveHelper ph;

//scene reader
SceneReader sr;

//creating game objects
Object st;

//screen width and height
const unsigned int SCR_WIDTH = 2400;
const unsigned int SCR_HEIGHT = 1800;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// directional light position
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
    Shader normalShader("../src/shaders/normals.vert", "../src/shaders/normals.frag",  "../src/shaders/normals.geom");

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

    unsigned int lightVAO, VBO, transparentVAO, transparentVBO, grassVAO, grassVBO, quadVAO, quadVBO, skyboxVAO, skyboxVBO;

    // Light
    setupVAOandVBO(lightVAO, VBO, ph.blandVertsNormalsTex, {3}, 8);

    // Transparent
    setupVAOandVBO(transparentVAO, transparentVBO, ph.transparentVertices, {3, 2}, 5);

    // Grass
    setupVAOandVBO(grassVAO, grassVBO, ph.grassVerts, {3, 2}, 5);

    // Screen Quad
    setupVAOandVBO(quadVAO, quadVBO, ph.quadVertices, {2, 2}, 4);

    // Skybox
    setupVAOandVBO(skyboxVAO, skyboxVBO, ph.skyboxVertices, {3}, 3);

    //Creating the framebuffer
    Framebuffer framebuffer = createFramebuffer(SCR_WIDTH, SCR_HEIGHT);
    Framebuffer msaa = createMSAAFrameBuffer(SCR_WIDTH, SCR_HEIGHT);

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
        if (wireFrame) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
        else { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

        // sort the transparent windows
        vector<glm::vec3> windows = sr.getWindows();
        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < windows.size(); i++) {
            float distance = glm::length(camera->Position - windows[i]);
            sorted[distance] = windows[i];
        }

        // draw MSAA
        if(renderToTexture) {
            glBindFramebuffer(GL_FRAMEBUFFER, msaa.ID);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

        // // render the loaded stormtrooper model (rotate using quaternions)
        // model = st.getModelMatrix();
        // //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
        // static glm::quat stormtrooperRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
        // if (rotateModels) { stormtrooperRotation = rotate(deltaTime, spinSpeed, stormtrooperRotation); }
        // model = model * glm::mat4(stormtrooperRotation); // Apply rotation
        // //model = glm::scale(model, glm::vec3(1.0f));
        // objectShader.setMat4("model", model);
        // st.setModelMatrix(model);
        // stormtrooper_obj.Draw(objectShader);

        //static glm::quat stormtrooperRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
        //if (rotateModels) { stormtrooperRotation = rotate(deltaTime, spinSpeed, stormtrooperRotation); }

        //st.setPosition(glm::vec3(0.0f, 0.0f, 1.0f)); // Example position
        //st.setRotation(stormtrooperRotation);
        //st.setScale(glm::vec3(1.0f));         
        model = st.getModelMatrix();
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

        // blit multisampled buffer(s) to normal colorbuffer of intermediate FBO. Image is stored in screenTexture
        if(renderToTexture) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa.ID);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.ID);
            glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
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
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, framebuffer.texture);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        } else {
            glEnable(GL_DEPTH_TEST); // RE-ENABLE if not rendering to texture
        }

         

        // IMGUI BUTTONS
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
                ImGui::Checkbox("Use Flashlight?", &useFlashlight);
                ImGui::Checkbox("Use Directional Light?", &useDirectionalLight); 
                ImGui::Checkbox("Use Point Light?", &usePointLight); 
                static const char* light[] = { "1", "2", "3", "4" };
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
                // Add more cases for other lights as needed


                //ImGui::SliderFloat3("Light 1 Position", &sr.getpointLights()[0].x, -20.0f, 20.0f);
                //ImGui::SliderFloat3("Light 2 Position", &sr.getpointLights()[1].x, -20.0f, 20.0f);
                //ImGui::SliderFloat3("Light 3 Position", &sr.getpointLights()[2].x, -20.0f, 20.0f);
                //ImGui::SliderFloat3("Light 4 Position", &sr.getpointLights()[3].x, -20.0f, 20.0f);
                ImGui::ColorEdit3("clear color", (float*)&clear_color);

                ImGui::SliderFloat("Flashlight Intensity", &flashlightIntensity, 0.0f, 4.0f);
                ImGui::SliderFloat("Pointlight Intensity", &pointLightIntensity, 0.0f, 4.0f);
                ImGui::SliderFloat("Directional Light Intensity", &directionLightIntensity, 0.0f, 4.0f);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Post-Processing Options"))
            {
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

            //ImGui::Checkbox("Rotate Models?", &rotateModels);
            //ImGui::SliderFloat("Rotation Speed", &spinSpeed, 0.0f, 500.0f);
            if (ImGui::SliderFloat3("Stormtrooper Position", &st.getPosition().x, -20.0f, 20.0f)) {
                st.setPosition(st.getPosition()); // Update the position and model matrix
            }

            static float scaleScalar = 1.0f;
            if (ImGui::SliderFloat("Stormtrooper Scale", &scaleScalar, 0.1f, 10.0f)) {
                st.setScale(glm::vec3(scaleScalar, scaleScalar, scaleScalar)); // Update the position and model matrix
            }

            static glm::vec3 eulerRotation(0.0f, 0.0f, 0.0f); // Store Euler angles for ImGui input
            if (ImGui::SliderFloat3("Rotation (Pitch, Yaw, Roll)", &eulerRotation.x, -180.0f, 180.0f)) {
                glm::quat quatRotation = glm::quat(glm::radians(eulerRotation));
                st.setRotation(quatRotation);
            }


            static const char* items[]{"Day","Night"};
            static int Selecteditem = 0;
            if (ImGui::Combo("Skybox Selector", &Selecteditem, items, IM_ARRAYSIZE(items)))
            {
                // Here event is fired
                if(Selecteditem == 0) { currSkybox = cubemapTextureDay; }
                else if(Selecteditem == 1) { currSkybox = cubemapTextureNight; }
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
    deleteFramebuffer(framebuffer);
    deleteFramebuffer(msaa);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}


// Helper function to set up VAO and VBO
void Renderer::setupVAOandVBO(unsigned int &VAO, unsigned int &VBO, 
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