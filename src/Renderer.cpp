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
#include <imGui/imgui_internal.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <format>

#include "Renderer.hpp"
#include "Model.hpp"
#include "Shader.hpp"

void renderQuad();

//MARK: Render
void Renderer::Render(GLFWwindow* window, Camera* camera, Controller* controller) {
    //create game objects
    Object stormtrooper("Stormtrooper", glm::vec3(4.0f, -0.9f, -2.5f));
    Object backpack("backpack", glm::vec3(-9.5f, 0.1f, 1.5f), glm::vec3(0.5f));
    Object brickWall("Brick Wall", glm::vec3(-7.5f, 0.5f, -3.5f), glm::vec3(0.1f));
    Object floor("floor", glm::vec3(0.0f, -1.0f, 0.0f));
    Object reflectiveST("Reflective ST", glm::vec3(2.0f, 0.0f, 2.0f));
    Object parallaxWall("Parallax Wall", glm::vec3(-14.5f, 0.5f, -3.5f), glm::vec3(0.1f));
    Object parallaxToy("Parallax Toy", glm::vec3(-0.5f, 0.5f, -3.5f), glm::vec3(0.1f));
    Object wallLeft("WallLeft", glm::vec3(14.0f, 1.0f, -0.3f), glm::vec3(0.1f, 0.1f, 1.0f), eulerDegreesToQuat(glm::vec3(0.0f, 0.0f, 90.0f)));
    Object wallRight("WallRight", glm::vec3(18.0f, 1.0f, -0.3f), glm::vec3(0.1f, 0.1f, 1.0f), eulerDegreesToQuat(glm::vec3(0.0f, 0.0f, 90.0f)));
    Object wallTop("WallTop", glm::vec3(16.0f, 3.0f, -0.3f), glm::vec3(0.1f, 0.1f, 1.0f), eulerDegreesToQuat(glm::vec3(0.0f, 0.0f, 0.0f)));
    Object wallBack("WallBack", glm::vec3(16.0f, 3.0f, -17.0f), glm::vec3(0.1f, 0.1f, 1.0f), eulerDegreesToQuat(glm::vec3(90.0f, 0.0f, 0.0f)));
    Object wallTest("WallTest", glm::vec3(0.0f, 1.0f, -15.0f), glm::vec3(0.25f, 0.25f, 0.25f), eulerDegreesToQuat(glm::vec3(0.0f, 0.0f, 90.0f)));

    //organize game objects
    objects.push_back(&stormtrooper);
    objects.push_back(&backpack);
    objects.push_back(&floor);
    objects.push_back(&reflectiveST);
    objects.push_back(&brickWall);
    objects.push_back(&parallaxWall);
    objects.push_back(&parallaxToy);
    objects.push_back(&wallLeft);
    objects.push_back(&wallRight);
    objects.push_back(&wallTop);
    objects.push_back(&wallBack);
    objects.push_back(&wallTest);

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
    Shader pointDepthShader("../src/shaders/pointDepthShader.vert", "../src/shaders/pointDepthShader.frag",  "../src/shaders/pointDepthShader.geom");
    Shader normalMapShader("../src/shaders/normalMap.vert", "../src/shaders/normalMap.frag");
    Shader parallaxShader("../src/shaders/parallaxMapping.vert", "../src/shaders/parallaxMapping.frag");

    // load models
    Model backpack_obj("../models/backpack/backpack.obj");
    Model stormtrooper_obj("../models/stormtrooper/stormtrooper.obj");
    Model wood_floor_obj("../models/wood_floor/wood_floor.obj");
    Model brick_wall_obj("../models/brick_wall/brick_wall.obj");
    Model parallax_wall_obj("../models/parallax_wall/parallax.obj");
    Model parallax_toy_obj("../models/parallax_toy/parallax.obj");

    //load textures
    unsigned int transparentTexture = tl.loadTexture("../textures/red_window.png");
    unsigned int grassTexture = tl.loadTexture("../textures/grass.png");
    unsigned int brickTexture = tl.loadTexture("../textures/brick.jpg");
    unsigned int brickwallTexture = tl.loadTexture("../textures/brickwall.jpg");
    unsigned int brickwallNormalTexture = tl.loadTexture("../textures/brickwall_normal.jpg");

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
    unsigned int uniformBlockNormalMapShader = glGetUniformBlockIndex(normalMapShader.ID, "Matrices");
    glUniformBlockBinding(normalMapShader.ID, uniformBlockNormalMapShader, 0);
    unsigned int uniformBlockParallaxMapShader = glGetUniformBlockIndex(parallaxShader.ID, "Matrices");
    glUniformBlockBinding(parallaxShader.ID, uniformBlockParallaxMapShader, 0);

    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

    ImVec2 lastSceneSize = ImVec2(SCR_WIDTH, SCR_HEIGHT); // default value before ImGui updates it
    int fbWidth = (int)lastSceneSize.x;
    int fbHeight = (int)lastSceneSize.y;

    // framebuffers
    Framebuffer framebuffer = createFramebuffer(SCR_WIDTH, SCR_HEIGHT);
    Framebuffer msaa = createMSAAFrameBuffer(SCR_WIDTH, SCR_HEIGHT);
    Framebuffer depthMapBuffer = createDepthMapBuffer();
    Framebuffer postProcessFramebuffer = createFramebuffer(fbWidth, fbHeight);


    Framebuffer pointLightsBuffers[NUM_POINT_LIGHTS];
    // depth cube map
    for (int i = 0; i < NUM_POINT_LIGHTS; i++) { Framebuffer pointBuffer = createDepthCubemapBuffer(); pointLightsBuffers[i] = pointBuffer; }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(0.0f, 0.0f);       // No inner window padding
    style.WindowBorderSize = 0.0f;                  // No window borders
    style.FramePadding = ImVec2(4.0f, 4.0f);         // Optional: tweak for buttons
    style.ItemSpacing = ImVec2(4.0f, 4.0f);          // Optional: tweak for controls
    style.TabBorderSize = 0.0f;                      // No border between tabs
    style.WindowRounding = 0.0f;                     // Flat corners

    // perspective (only needs to be set once unless you want to change projection)
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 model;
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // MARK: MAIN LOOP
    while(!glfwWindowShouldClose(window)) {
        glEnable(GL_CULL_FACE);  
        glCullFace(GL_BACK); 
        glFrontFace(GL_CCW); 

        //gamma correction
        if (gammaCorrection) {glEnable(GL_FRAMEBUFFER_SRGB);} else { glDisable(GL_FRAMEBUFFER_SRGB);}

        //delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //input
        controller->processInput(window, deltaTime, camera);

        // Start new ImGui frame early so we can query the scene window size before rendering
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Get pointer to Scene window to get size, without rendering
        ImGuiWindow* sceneWindow = ImGui::FindWindowByName("Scene");
        if (sceneWindow) {
            lastSceneSize = sceneWindow->ContentRegionRect.GetSize();
        }

        int newWidth = (int)lastSceneSize.x;
        int newHeight = (int)lastSceneSize.y;

        // Only reallocate if size changed
        if (newWidth != fbWidth || newHeight != fbHeight) {
            fbWidth = newWidth;
            fbHeight = newHeight;

            deleteFramebuffer(framebuffer);
            deleteFramebuffer(msaa);
            deleteFramebuffer(postProcessFramebuffer);

            framebuffer = createFramebuffer(fbWidth, fbHeight);
            msaa = createMSAAFrameBuffer(fbWidth, fbHeight);
            postProcessFramebuffer = createFramebuffer(fbWidth, fbHeight);
        }

        // Update projection matrix to match ImGui Scene window size
        float aspect = (float)fbWidth / (float)fbHeight;
        glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), aspect, 0.1f, 100.0f);
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

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
        float near_plane = 0.1f, far_plane = 100.0f;
        lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
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
        brick_wall_obj.Draw(depthShader, brickWall);
        parallax_wall_obj.Draw(depthShader, parallaxWall);
        parallax_toy_obj.Draw(depthShader, parallaxToy);
        wood_floor_obj.Draw(depthShader, wallTop);
        wood_floor_obj.Draw(depthShader, wallBack);
        wood_floor_obj.Draw(depthShader, wallLeft);
        wood_floor_obj.Draw(depthShader, wallRight);
        wood_floor_obj.Draw(depthShader, wallTest);
        wood_floor_obj.Draw(depthShader, floor);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.ID);
        glViewport(0, 0, fbWidth, fbHeight);

        // clear the buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // create depth cubemap transformation matrices
        float point_near_plane = 0.1f;
        float point_far_plane = 25.0f;

        // render scene to depth cubemap for each point light
        for (int i = 0; i < NUM_POINT_LIGHTS; i++) {
            glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)POINT_SHADOW_WIDTH / (float)POINT_SHADOW_HEIGHT, point_near_plane, point_far_plane);
            std::vector<glm::mat4> shadowTransforms;
            shadowTransforms.push_back(shadowProj * glm::lookAt(sr.pointLightPositions[i], sr.pointLightPositions[i] + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(sr.pointLightPositions[i], sr.pointLightPositions[i] + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(sr.pointLightPositions[i], sr.pointLightPositions[i] + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(sr.pointLightPositions[i], sr.pointLightPositions[i] + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(sr.pointLightPositions[i], sr.pointLightPositions[i] + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(sr.pointLightPositions[i], sr.pointLightPositions[i] + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
            glViewport(0, 0, POINT_SHADOW_WIDTH, POINT_SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, pointLightsBuffers[i].ID);
            glClear(GL_DEPTH_BUFFER_BIT);
            pointDepthShader.use();
            for (unsigned int i = 0; i < 6; ++i) { pointDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]); }
            pointDepthShader.setFloat("far_plane", point_far_plane);
            pointDepthShader.setVec3("lightPos", sr.pointLightPositions[i]);
            backpack_obj.Draw(pointDepthShader, backpack);
            stormtrooper_obj.Draw(pointDepthShader, stormtrooper);
            stormtrooper_obj.Draw(pointDepthShader, reflectiveST);
            brick_wall_obj.Draw(pointDepthShader, brickWall);
            parallax_wall_obj.Draw(pointDepthShader, parallaxWall);
            parallax_toy_obj.Draw(pointDepthShader, parallaxToy);
            wood_floor_obj.Draw(pointDepthShader, wallTop);
            wood_floor_obj.Draw(pointDepthShader, wallBack);
            wood_floor_obj.Draw(pointDepthShader, wallLeft);
            wood_floor_obj.Draw(pointDepthShader, wallRight);
            wood_floor_obj.Draw(pointDepthShader, wallTest);
            wood_floor_obj.Draw(pointDepthShader, floor);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, fbWidth, fbHeight);
            glActiveTexture(GL_TEXTURE25 + i);
            glBindTexture(GL_TEXTURE_CUBE_MAP, pointLightsBuffers[i].texture);
        }

        //setting shadow texture
        glActiveTexture(GL_TEXTURE24);
        glBindTexture(GL_TEXTURE_2D, depthMapBuffer.texture);

        // draw to non-default framebuffer
        // Only bind and clear the framebuffer if we're actually rendering to it
        if (renderToTexture) {
            if (useMSAA) {
                glBindFramebuffer(GL_FRAMEBUFFER, msaa.ID);
            } else {
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.ID);
            }
        } else {
            // Not rendering to texture — assume full-screen mode
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        
        // clear the buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // MARK: UNIFORM HELL
        objectShader.use();
        sr.setParams(objectShader, *camera);
        objectShader.setVec3("lightPos", lightPos);
        objectShader.setFloat("far_plane", point_far_plane);
        objectShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        objectShader.setInt("shadowMap", shadowItem);
        for (int i = 0; i < NUM_POINT_LIGHTS; i++) { objectShader.setVec3("pointLightPos[" + std::to_string(i) + "]", sr.pointLightPositions[i]); }
        for (int i = 0; i < NUM_POINT_LIGHTS; i++) { objectShader.setInt("depthCubeMap[" + std::to_string(i) + "]", 25 + i); }
        
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
        objectShader.setBool("useNormalMaps", useNormalMaps);
        objectShader.setFloat("shadowFactor", shadowFactor);
        objectShader.setBool("useSmoothShadows", useSmoothShadows);
        objectShader.setFloat("exposure", exposure);
        objectShader.setFloat("shadowBias", shadowBias);
        objectShader.setFloat("pointLightRadius", pointLightRadius);

        //render the objects normally (second pass)
        glCullFace(GL_BACK);
        backpack_obj.Draw(objectShader, backpack);
        stormtrooper_obj.Draw(objectShader, stormtrooper);
        wood_floor_obj.Draw(objectShader, floor);
        brick_wall_obj.Draw(objectShader, brickWall);
        stormtrooper_obj.Draw(reflectiveShader, reflectiveST);
        wood_floor_obj.Draw(objectShader, wallLeft);
        wood_floor_obj.Draw(objectShader, wallRight);
        wood_floor_obj.Draw(objectShader, wallTop);
        wood_floor_obj.Draw(objectShader, wallBack);
        wood_floor_obj.Draw(objectShader, wallTest);
        //parallax_wall_obj.Draw(objectShader, parallaxWall);

        // parallax uniforms
        parallaxShader.use();
        parallaxShader.setVec3("lightPos", lightPos);
        parallaxShader.setVec3("viewPos", camera->Position);
        parallaxShader.setFloat("heightScale", 0.1f);
        parallax_wall_obj.Draw(parallaxShader, parallaxWall);
        parallax_toy_obj.Draw(parallaxShader, parallaxToy);

        // Pointlight cubes
        pointlightcube.use();
        glDisable(GL_CULL_FACE);
        glBindVertexArray(lightVAO);
        vector<glm::vec3> pointLights = sr.getpointLights();
        for (unsigned int i = 0; i < NUM_POINT_LIGHTS; i++) {
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
            glBlitFramebuffer(0, 0, fbWidth, fbHeight, 0, 0, fbWidth, fbHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
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
            glBindFramebuffer(GL_FRAMEBUFFER, postProcessFramebuffer.ID);
            glDisable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            screenShader.use();
            screenShader.setBool("inverted", inverted);
            screenShader.setBool("grayscale", grayscale);
            screenShader.setBool("sharpen", sharpen);
            screenShader.setBool("blur", blur);
            screenShader.setBool("edgeDetection", edgeDetection);
            glBindVertexArray(quadVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, framebuffer.texture); // color
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glEnable(GL_DEPTH_TEST);
        } else {
            glEnable(GL_DEPTH_TEST); // RE-ENABLE if not rendering to texture
        }

        // MARK: imgui
        // Set up fullscreen host window for DockSpace
        ImGuiWindowFlags dockspace_window_flags = 0;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        dockspace_window_flags |= ImGuiWindowFlags_NoTitleBar 
                                | ImGuiWindowFlags_NoCollapse 
                                | ImGuiWindowFlags_NoResize 
                                | ImGuiWindowFlags_NoMove 
                                | ImGuiWindowFlags_NoBringToFrontOnFocus 
                                | ImGuiWindowFlags_NoNavFocus;


        //DOCKSPACE
        ImGui::Begin("DockSpaceRoot", nullptr, dockspace_window_flags);
        ImGui::PopStyleVar(3);
        ImGuiID dockspace_id = ImGui::GetID("GeorgeDockspace");
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        static bool first_time = true;
        if (first_time) {
            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any existing layout
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            // Split into left (scene) and right (debug)
            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.3f, nullptr, &dock_main_id);
            ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.3f, nullptr, &dock_main_id);

            // Assign windows to each side
            ImGui::DockBuilderDockWindow("Scene", dock_main_id);
            ImGui::DockBuilderDockWindow("George Engine Debug Menu", dock_right);
            ImGui::DockBuilderDockWindow("Shadow Map", dock_bottom);

            ImGui::DockBuilderFinish(dockspace_id);
            first_time = false;
        }
        ImGui::End();

        //SCENE WINDOW
        static GLuint sceneFBO = 0, sceneTex = 0, sceneRBO = 0;
        static int fbWidth = 2400, fbHeight = 1800;
        ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_None);
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImGui::Image((void*)(intptr_t)postProcessFramebuffer.texture, avail, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();

        // SHADOW MAP VIEW WINDOW
        ImGui::Begin("Shadow Map");
        ImVec2 shadowSize = ImGui::GetContentRegionAvail();
        ImGui::Image((void*)(intptr_t)depthMapBuffer.texture, shadowSize, ImVec2(0, 1), ImVec2(1, 0),
             ImVec4(1,1,1,1), ImVec4(0,0,0,0));
        ImGui::End();


        //DEBUG WINDOW
        static int counter = 0;
        ImGui::Begin("George Engine Debug Menu", nullptr, ImGuiWindowFlags_None);  
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
            ImGui::Checkbox("Use Smooth Shadows?", &useSmoothShadows);
            ImGui::Checkbox("Use Normal Maps?", &useNormalMaps);
            ImGui::SliderFloat("Shadow Blending", &shadowFactor, 0.0f, 1.0f);
            ImGui::SliderFloat("Shadow Bias", &shadowBias, 0.0f, 0.2f);
            ImGui::SliderFloat("Point Light Radius", &pointLightRadius, 0.0f, 100.0f);
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
            ImGui::SliderFloat3("Direction Light Position (NOT ROTATION)", &lightPos.x, -20.0f, 20.0f);
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
            ImGui::SliderFloat("exposure", &exposure, 0.1, 1.0f);
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
                    static glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(object->getRotation()));
                    if (ImGui::InputFloat3((object->getName() + " Rotation").c_str(), &eulerRotation.x)) {
                        glm::quat quatRotation = eulerDegreesToQuat(eulerRotation);
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
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // swap chain and IO handling
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    // MARK: CLEANUP
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