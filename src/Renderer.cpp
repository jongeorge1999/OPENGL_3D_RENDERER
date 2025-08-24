#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imGui/imgui.h>
#include <imGui/imgui_impl_glfw.h>
#include <imGui/imgui_impl_opengl3.h>
#include <imGui/imgui_internal.h>
#include <imGui/ImGuizmo.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <format>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include "Renderer.hpp"
#include "Shader.hpp"

void renderQuad();

static ImGuizmo::OPERATION gizmoOp  = ImGuizmo::TRANSLATE;
static ImGuizmo::MODE      gizmoMode= ImGuizmo::WORLD;
static bool useSnap = false;
static float snapTranslate[3] = {0.5f, 0.5f, 0.5f};
static float snapRotateDeg    = 15.0f;
static float snapScale[3]     = {0.1f, 0.1f, 0.1f};

//UI State Machine
struct UIState {
    Object* selected = nullptr;
    int     selectedIndex = -1;
    char    filter[64] = "";
    char    nameBuf[256] = {};
    Object* nameBufOwner = nullptr;
} ui;

//MARK: Render
void Renderer::Render(GLFWwindow* window, Camera* camera, Controller* controller) {

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


    //create game objects
    Object stormtrooper(&objectShader, "../models/stormtrooper/stormtrooper.obj", &objects, "Stormtrooper", glm::vec3(4.0f, -0.9f, -2.5f));
    Object backpack(&objectShader, "../models/backpack/backpack.obj", &objects, "backpack", glm::vec3(-9.5f, 0.1f, 1.5f), glm::vec3(0.5f));
    Object brickWall(&objectShader, "../models/brick_wall/brick_wall.obj", &objects, "Brick Wall", glm::vec3(-7.5f, 0.5f, -3.5f), glm::vec3(0.1f));
    Object floor(&objectShader, "../models/wood_floor/wood_floor.obj", &objects, "floor", glm::vec3(0.0f, -1.0f, 0.0f));
    Object reflectiveST(&reflectiveShader, "../models/stormtrooper/stormtrooper.obj", &objects, "Reflective ST", glm::vec3(2.0f, 0.0f, 2.0f));
    Object parallaxWall(&parallaxShader, "../models/parallax_wall/parallax.obj", &objects, "Parallax Wall", glm::vec3(-14.5f, 0.5f, -3.5f), glm::vec3(0.1f));
    Object parallaxToy(&parallaxShader, "../models/parallax_toy/parallax.obj", &objects, "Parallax Toy", glm::vec3(-0.5f, 0.5f, -3.5f), glm::vec3(0.1f));
    Object wallLeft(&objectShader, "../models/wood_floor/wood_floor.obj", &objects, "WallLeft", glm::vec3(14.0f, 1.0f, -0.3f), glm::vec3(0.1f, 0.1f, 1.0f), eulerDegreesToQuat(glm::vec3(0.0f, 0.0f, 90.0f)));
    Object wallRight(&objectShader, "../models/wood_floor/wood_floor.obj", &objects, "WallRight", glm::vec3(18.0f, 1.0f, -0.3f), glm::vec3(0.1f, 0.1f, 1.0f), eulerDegreesToQuat(glm::vec3(0.0f, 0.0f, 90.0f)));
    Object wallTop(&objectShader, "../models/wood_floor/wood_floor.obj", &objects, "WallTop", glm::vec3(16.0f, 3.0f, -0.3f), glm::vec3(0.1f, 0.1f, 1.0f), eulerDegreesToQuat(glm::vec3(0.0f, 0.0f, 0.0f)));
    Object wallBack(&objectShader, "../models/wood_floor/wood_floor.obj", &objects, "WallBack", glm::vec3(16.0f, 3.0f, -17.0f), glm::vec3(0.1f, 0.1f, 1.0f), eulerDegreesToQuat(glm::vec3(90.0f, 0.0f, 0.0f)));
    Object wallTest(&objectShader, "../models/wood_floor/wood_floor.obj", &objects, "WallTest", glm::vec3(0.0f, 1.0f, -15.0f), glm::vec3(0.25f, 0.25f, 0.25f), eulerDegreesToQuat(glm::vec3(0.0f, 0.0f, 90.0f)));

    // openGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);  
    glCullFace(GL_BACK); 
    glFrontFace(GL_CCW); 

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

    ImVec2 lastSceneSize = ImVec2(SCR_WIDTH, SCR_HEIGHT);
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
    style.WindowPadding = ImVec2(0.0f, 0.0f);       
    style.WindowBorderSize = 0.0f;                
    style.FramePadding = ImVec2(4.0f, 4.0f); 
    style.ItemSpacing = ImVec2(4.0f, 4.0f);     
    style.TabBorderSize = 0.0f;   
    style.WindowRounding = 0.0f;  

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
        ImGuizmo::BeginFrame();
        ImGuiWindow* sceneWindow = ImGui::FindWindowByName("Scene");
        if (sceneWindow) {lastSceneSize = sceneWindow->ContentRegionRect.GetSize();}
        int newWidth = (int)lastSceneSize.x;
        int newHeight = (int)lastSceneSize.y;
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

        // dirLight Anim
        static bool  animateDirLight = true; 
        static float orbitSpeed      = 0.25f;
        static float orbitRadius     = 32.0f;
        static glm::vec2 orbitCenter = {-16.0f, 16.0f};

        if (animateDirLight) {
            float t = glfwGetTime() * orbitSpeed;
            float y = lightPos.y;              
            lightPos.x = orbitCenter.x + orbitRadius * std::cos(t);
            lightPos.z = orbitCenter.y + orbitRadius * std::sin(t);
            lightPos.y = y;
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
        for (Object* obj : objects) {obj->Draw(depthShader);}
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
            for (Object* obj : objects) {obj->Draw(pointDepthShader);}
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
            if (useMSAA) {glBindFramebuffer(GL_FRAMEBUFFER, msaa.ID);} 
            else {glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.ID);}
        } else { glBindFramebuffer(GL_FRAMEBUFFER, 0);}
        
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
        objectShader.setFloat("dirShadowBias", dirShadowBias);
        objectShader.setFloat("pointLightRadius", pointLightRadius);

        parallaxShader.use();
        parallaxShader.setVec3("lightPos", lightPos);
        parallaxShader.setVec3("viewPos", camera->Position);
        parallaxShader.setFloat("heightScale", 0.1f);

        //render the objects normally (second pass)
        glCullFace(GL_BACK);
        for (Object* obj : objects) {obj->Draw();}

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
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.3f, nullptr, &dock_main_id);
            ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.3f, nullptr, &dock_main_id);
            ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.3f, nullptr, &dock_main_id);
            ImGuiID dock_right_inspector = ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Right, 0.50f, nullptr, &dock_right);

            ImGui::DockBuilderDockWindow("Scene", dock_main_id);
            ImGui::DockBuilderDockWindow("George Engine Debug Menu", dock_bottom);
            ImGui::DockBuilderDockWindow("Hierarchy", dock_left);
            ImGui::DockBuilderDockWindow("Inspector", dock_right_inspector);

            ImGui::DockBuilderFinish(dockspace_id);
            first_time = false;
        }
        ImGui::End();

        ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_None);
        const float toolbarH = ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("##Toolbar", ImVec2(0, toolbarH), false, ImGuiWindowFlags_NoScrollbar);

        ImGui::TextUnformatted("Transform:");
        ImGui::SameLine();
        if (ImGui::RadioButton("Translate", gizmoOp == ImGuizmo::TRANSLATE)) gizmoOp = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", gizmoOp == ImGuizmo::ROTATE)) gizmoOp = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", gizmoOp == ImGuizmo::SCALE)) gizmoOp = ImGuizmo::SCALE;

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        if (ImGui::RadioButton("World", gizmoMode == ImGuizmo::WORLD)) gizmoMode = ImGuizmo::WORLD;
        ImGui::SameLine();
        if (ImGui::RadioButton("Local", gizmoMode == ImGuizmo::LOCAL)) gizmoMode = ImGuizmo::LOCAL;

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        ImGui::Checkbox("Snap", &useSnap);
        if (useSnap) {
            if (gizmoOp == ImGuizmo::TRANSLATE)      { ImGui::SameLine(); ImGui::InputFloat3("ΔT", snapTranslate); }
            else if (gizmoOp == ImGuizmo::ROTATE)    { ImGui::SameLine(); ImGui::InputFloat ("ΔR (deg)", &snapRotateDeg); }
            else                                     { ImGui::SameLine(); ImGui::InputFloat3("ΔS", snapScale); }
        }
        ImGui::EndChild();

        ImGui::BeginChild("##Viewport",
                        ImVec2(0, 0),
                        false,
                        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImVec2 vAvail = ImGui::GetContentRegionAvail();
        ImGui::Image((void*)(intptr_t)postProcessFramebuffer.texture, vAvail, ImVec2(0,1), ImVec2(1,0));
        if (ui.selected) {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList()); 
            const ImVec2 vPos   = ImGui::GetWindowPos();
            const ImVec2 vMin   = ImGui::GetWindowContentRegionMin();
            const ImVec2 vMax   = ImGui::GetWindowContentRegionMax();
            const ImVec2 tl     = ImVec2(vPos.x + vMin.x, vPos.y + vMin.y);
            const float  gW     = vMax.x - vMin.x;
            const float  gH     = vMax.y - vMin.y;
            ImGuizmo::SetRect(tl.x, tl.y, gW, gH);
            glm::mat4 view = camera->GetViewMatrix();
            glm::mat4 proj = camera->GetProjectionMatrix(gW, gH);
            glm::mat4 model = ui.selected->getModelMatrix();
            const float* snapPtr = nullptr;
            if (useSnap) {
                if      (gizmoOp == ImGuizmo::TRANSLATE) snapPtr = snapTranslate;
                else if (gizmoOp == ImGuizmo::ROTATE)    snapPtr = &snapRotateDeg;
                else                                     snapPtr = snapScale; 
            }

            ImGuizmo::Manipulate(glm::value_ptr(view),
                                glm::value_ptr(proj),
                                gizmoOp,
                                gizmoMode,
                                glm::value_ptr(model),
                                nullptr,
                                snapPtr);

            if (ImGuizmo::IsUsing()) {
                const glm::vec3 T(model[3].x, model[3].y, model[3].z);
                glm::mat3 B(model);
                glm::vec3 S(glm::length(B[0]), glm::length(B[1]), glm::length(B[2]));
                const float kEps = 1e-8f;
                if (S.x < kEps) S.x = kEps;
                if (S.y < kEps) S.y = kEps;
                if (S.z < kEps) S.z = kEps;
                B[0] /= S.x;
                B[1] /= S.y;
                B[2] /= S.z;

                if (glm::determinant(B) < 0.0f) {
                    S.x = -S.x;
                    B[0] = -B[0];
                }

                glm::quat R_from_matrix = glm::normalize(glm::quat_cast(B));

                glm::quat R = (gizmoOp == ImGuizmo::SCALE)
                                ? ui.selected->getRotation()
                                : R_from_matrix;

                ui.selected->setPosition(T);
                ui.selected->setRotation(R); 
                ui.selected->setScale(S);
            }
        }

        ImGui::EndChild();
        ImGui::End();

        // SHADOW MAP VIEW WINDOW
        // ImGui::Begin("Shadow Map");
        // ImVec2 shadowSize = ImGui::GetContentRegionAvail();
        // ImGui::Image((void*)(intptr_t)depthMapBuffer.texture, shadowSize, ImVec2(0, 1), ImVec2(1, 0),
        //      ImVec4(1,1,1,1), ImVec4(0,0,0,0));
        // ImGui::End();

        //inspector view window
        // ImGui::Begin("Inspector");
        // if (ImGui::Button("Close Application")) { glfwSetWindowShouldClose(window, true); }
        // ImGui::End();

        // --- HIERARCHY WINDOW ---
        ImGui::Begin("Hierarchy");
        ImGui::InputTextWithHint("##filter", "Filter objects...", ui.filter, IM_ARRAYSIZE(ui.filter));
        ImGui::Separator();

        for (int i = 0; i < (int)objects.size(); ++i) {
            Object* obj = objects[i];
            const std::string& name = obj->getName();

            // simple filter
            if (ui.filter[0] != '\0' &&
                name.find(ui.filter) == std::string::npos) {
                continue;
            }

            bool isSelected = (ui.selected == obj);
            if (ImGui::Selectable(name.c_str(), isSelected)) {
                ui.selected      = obj;
                ui.selectedIndex = i;
                std::snprintf(ui.nameBuf, sizeof(ui.nameBuf), "%s", obj->getName().c_str());
                ui.nameBufOwner = obj;
                ImGui::SetWindowFocus("Inspector");
            }

            //save for later
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {}
        }

        ImGui::End();


        // Inspector window
        ImGui::Begin("Inspector");

        if (!ui.selected) {
            ImGui::TextUnformatted("No object selected.");
            ImGui::End();
        } else {
            Object* obj = ui.selected;
            if (ui.nameBufOwner != obj) {
                std::snprintf(ui.nameBuf, sizeof(ui.nameBuf), "%s", obj->getName().c_str());
                ui.nameBufOwner = obj;
            }

            // Editable name
            ImGuiInputTextFlags nameFlags =
                ImGuiInputTextFlags_AutoSelectAll |
                ImGuiInputTextFlags_EnterReturnsTrue;

            bool submitted = ImGui::InputText("Name", ui.nameBuf, IM_ARRAYSIZE(ui.nameBuf), nameFlags);
            if (submitted || (ImGui::IsItemDeactivatedAfterEdit())) {
                std::string newName = ui.nameBuf;
                auto l = newName.find_first_not_of(" \t\r\n");
                auto r = newName.find_last_not_of(" \t\r\n");
                if (l == std::string::npos) newName.clear();
                else newName = newName.substr(l, r - l + 1);

                if (!newName.empty() && newName != obj->getName()) {
                    obj->setName(newName);
                }
            }

            ImGui::Separator();

            glm::vec3 pos = obj->getPosition();
            if (ImGui::DragFloat3("Position", &pos.x, 0.05f)) obj->setPosition(pos);

            glm::vec3 eulerDeg = glm::degrees(glm::eulerAngles(obj->getRotation()));
            if (ImGui::DragFloat3("Rotation (deg)", &eulerDeg.x, 0.5f))
                obj->setRotation(eulerDegreesToQuat(eulerDeg));

            glm::vec3 scale = obj->getScale();
            if (ImGui::DragFloat3("Scale", &scale.x, 0.01f))
                obj->setScale(scale);

            if (ImGui::Button("Center on Origin")) {
                obj->setPosition(glm::vec3(0.0f));
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset Rotation")) {
                obj->setRotation(eulerDegreesToQuat(glm::vec3(0.0f)));
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset Scale")) {
                obj->setScale(glm::vec3(1.0f));
            }

            ImGui::End();
        }


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
            if(ImGui::TreeNode("Directional Light Options")) 
            {
                ImGui::Checkbox("Use Directional Light?", &useDirectionalLight); 
                ImGui::Checkbox("Animate Dir Light", &animateDirLight);
                ImGui::SliderFloat("Dir Orbit Speed", &orbitSpeed, 0.0f, 2.0f);
                ImGui::SliderFloat("Dir Orbit Radius", &orbitRadius, 0.0f, 128.0f);
                ImGui::SliderFloat2("Dir Orbit Center (x,z)", &orbitCenter.x, -64.0f, 64.0f);
                ImGui::SliderFloat3("Direction Light Position (NOT ROTATION)", &lightPos.x, -20.0f, 20.0f);
                ImGui::SliderFloat("Directional Light Intensity", &directionLightIntensity, 0.0f, 4.0f);
                ImGui::TreePop();
            }
            ImGui::Checkbox("Use Point Light?", &usePointLight); 
            ImGui::Checkbox("Use Shadows?", &useShadows);
            ImGui::Checkbox("Use Smooth Shadows?", &useSmoothShadows);
            ImGui::Checkbox("Use Normal Maps?", &useNormalMaps);
            ImGui::SliderFloat("Shadow Blending", &shadowFactor, 0.0f, 1.0f);
            ImGui::SliderFloat("Shadow Bias", &shadowBias, 0.0f, 0.2f);
            ImGui::SliderFloat("Dir. Shadow Bias", &dirShadowBias, 0.0f, 0.2f);
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