#include <iostream>
#include "../headers/imgui/imgui.h"
#include "../headers/imgui/imgui_impl_glfw.h"
#include "../headers/imgui/imgui_impl_opengl3.h"
#include "glad.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <filesystem>
#include <string>

#include "../headers/shaderClass.h"
#include "../headers/EBO.h"
#include "../headers/VBO.h"
#include "../headers/VAO.h"
#include "../headers/texture.h"
#include "../headers/camera.h"
#include "../headers/model.h"
#include "../headers/FBO.h"
#include "../headers/guiVariables.h"

#include "../include/stb/stb_image.h"
#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"


const unsigned int width = 800;
const unsigned int height = 800;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
     glViewport(0, 0, width, height);
}

int main(int, char **)
{
     float quadVertices[] = {
          // positions     // texCoords
          -1.0f,  1.0f,  0.0f, 1.0f,
          -1.0f, -1.0f,  0.0f, 0.0f,
           1.0f, -1.0f,  1.0f, 0.0f,

          -1.0f,  1.0f,  0.0f, 1.0f,
          1.0f, -1.0f,  1.0f, 0.0f,
          1.0f,  1.0f,  1.0f, 1.0f
     };
     GLFWwindow *window;

     if (!glfwInit())
     {
         throw 1;
     }
     // Create the window that appears on the screen
     window = glfwCreateWindow(width, height, "Window", NULL, NULL);

     // Tells GLFW to add the window to the current context
     // A context being an object that encapsulates the OpenGL state machine
     glfwMakeContextCurrent(window);

     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
     {
          std::cout << "Couldn't load OpenGL" << std::endl;
          glfwTerminate();
          return -1;
     }
     // Parses the fragment and vertex shader files and wraps them into a shader program
     // The files are compiled to an intermediary language then translated into specific instructions for the GPU
     Shader depthShader("../assets/shaders/shadows/depth.vert", "../assets/shaders/shadows/depth.frag");
     Shader pointDepthShader("../assets/shaders/shadows/pointShadow.vert", "../assets/shaders/shadows/pointShadow.frag");
     pointDepthShader.LinkGeometry("../assets/shaders/shadows/pointShadow.geom");
     Shader framebufferShader("../assets/shaders/framebuffer.vert", "../assets/shaders/framebuffer.frag");
     Shader shadowShader("../assets/shaders/shadows/shadow.vert", "../assets/shaders/default.frag");
     // Models
     Model cafe("../assets/ModularModel/modular.obj");
     Model cube("../assets/Models/cube.obj");
     // quad geometry
     VAO quadVAO;
     VBO quadVBO(quadVertices, sizeof(quadVertices));
     quadVAO.Bind();
     quadVAO.LinkAttrib(quadVBO, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
     quadVAO.LinkAttrib(quadVBO, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
     quadVAO.Unbind();

     //-----------IMAGE VARIABLES-----------
     //---SHADOW VARIABLES---
     const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
     unsigned int depthMapFBO;
     glGenFramebuffers(1, &depthMapFBO);
     // DEPTH TEXTURE MAP
     unsigned int depthMap;
     glGenTextures(1, &depthMap);
     glBindTexture(GL_TEXTURE_2D, depthMap);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
     float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f};
     glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

     glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
     glDrawBuffer(GL_NONE);
     glReadBuffer(GL_NONE);
     glBindFramebuffer(GL_FRAMEBUFFER, 0);
     // DEPTH CUBE MAP
     unsigned int depthCubemap;
     glGenTextures(1, &depthCubemap);

     glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
     for (unsigned int i = 0; i < 6; ++i)
             glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                          SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

     glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
     glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
     glBindFramebuffer(GL_FRAMEBUFFER, 0);
     ///---END OF SHADOW VARIABLES---
      //-----------END OF IMAGE VARIABLES-----------

     glEnable(GL_DEPTH_TEST); // Allows for depth comparison and updates the depth buffer
     //glEnable(GL_CULL_FACE);


     // -----------RENDER LOOP VARIABLES-----------
     Camera camera(width, height, glm::vec3(0.0f, 2.5f, 2.0f));

     GUIVariables GUI;

     float deltaTime = 0.0f;
     float lastFrame = 0.0f;

     //-----------END OF RENDER LOOP VARIABLES-----------
     IMGUI_CHECKVERSION();
     ImGui::CreateContext();
     ImGuiIO &io = ImGui::GetIO();
     (void)io;
     ImGui::StyleColorsClassic();
     ImGui_ImplGlfw_InitForOpenGL(window, true);
     ImGui_ImplOpenGL3_Init("#version 330");

     glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

     shadowShader.Activate();
     shadowShader.SetToInt("depthMap", 1);
     shadowShader.SetToInt("depthCubeMap", 2);

     // Main Render Loop
     while (!glfwWindowShouldClose(window))
     {
          glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

          ImGui_ImplOpenGL3_NewFrame();
          ImGui_ImplGlfw_NewFrame();
          ImGui::NewFrame();

          float crntFrame = glfwGetTime();
          deltaTime = crntFrame - lastFrame;
          lastFrame = crntFrame;

           if (!io.WantCaptureMouse)
               camera.Inputs(window);

          camera.Matrix(45, 0.1, 100);

          //--------------SHADERS & MODEL DRAWING--------------
          //---FIRST PASS (SHADOW MAP)---
          glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
          glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
          glBindTexture(GL_TEXTURE_2D, depthMap);
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
          glClear(GL_DEPTH_BUFFER_BIT);
          // 1. Depth Texture Map
          float nearPlane = 1.0f, farPlane = 7.5f;
          glm::mat4 model(1.0f);
          glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
          glm::mat4 lightView = glm::lookAt(GUI.lightDir, glm::vec3(0.0f), glm::vec3( 0.0f, 1.0f,  0.0f));
          glm::mat4 lightSpaceMatrix = lightProjection * lightView;

          depthShader.Activate();
          depthShader.SetToMat4("lightSpaceMatrix", lightSpaceMatrix);
          depthShader.SetToMat4("model", model);
          cafe.Draw(depthShader);
          // 2. Depth Cube Map
          glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
          glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
          glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
          glClear(GL_DEPTH_BUFFER_BIT);
          float aspect = (float)SHADOW_WIDTH/(float)SHADOW_HEIGHT;
          float near = 1.0f;
          float far = 25.0f;
          glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
          std::vector<glm::mat4> shadowTransforms;
          shadowTransforms.push_back(shadowProj *
                           glm::lookAt(GUI.lightPos, GUI.lightPos + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
          shadowTransforms.push_back(shadowProj *
                           glm::lookAt(GUI.lightPos, GUI.lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
          shadowTransforms.push_back(shadowProj *
                           glm::lookAt(GUI.lightPos, GUI.lightPos + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
          shadowTransforms.push_back(shadowProj *
                           glm::lookAt(GUI.lightPos, GUI.lightPos + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)));
          shadowTransforms.push_back(shadowProj *
                           glm::lookAt(GUI.lightPos, GUI.lightPos + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)));
          shadowTransforms.push_back(shadowProj *
                           glm::lookAt(GUI.lightPos, GUI.lightPos + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)));
          pointDepthShader.Activate();
          pointDepthShader.SetToMat4("model", model);
          for(int i = 0; i < 6; ++i)
          {
              pointDepthShader.SetToMat4(&("shadowMatrices[" + std::to_string(i) + "]")[0], shadowTransforms[i]);
          }
          pointDepthShader.SetToVec3("lightPos", &GUI.lightPos[0]);
          pointDepthShader.SetToFloat("far_plane", farPlane);
          cafe.Draw(pointDepthShader);
          glBindFramebuffer(GL_FRAMEBUFFER, 0);
          //---END OF FIRST PASS---
          //---SECOND PASS (SHADOW RENDERING)---
          glViewport(0, 0, width, height);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

          glm::mat4 view = camera.GetViewMatrix();
          glm::mat4 projection = camera.GetProjMatrix();

          shadowShader.Activate();
          glActiveTexture(GL_TEXTURE1);
          glBindTexture(GL_TEXTURE_2D, depthMap);
          glActiveTexture(GL_TEXTURE2);
          glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

          shadowShader.SetToMat4("view", view);
          shadowShader.SetToMat4("projection", projection);
          shadowShader.SetToMat4("model", model);
          shadowShader.SetToFloat("far_plane", farPlane);
          shadowShader.SetToMat4("lightSpaceMatrix", lightSpaceMatrix);
          shadowShader.SetToVec3("u_viewPos", &camera.Position[0]);
          shadowShader.SetToVec3("dLight.direction", &GUI.lightDir[0]);
          shadowShader.SetToVec3("dLight.ambient", &glm::vec3(0.4)[0]);
          shadowShader.SetToVec3("dLight.diffuse", &glm::vec3(0.4)[0]);
          shadowShader.SetToVec3("dLight.specular", &glm::vec3(0.2)[0]);
          shadowShader.SetToVec3("pLight.position", &GUI.lightPos[0]);
          shadowShader.SetToVec3("pLight.color", &glm::vec3(0.8)[0]);
          shadowShader.SetToFloat("pLight.constant", 1.0f);
          shadowShader.SetToFloat("pLight.linear", 0.32f);
          shadowShader.SetToFloat("pLight.quadratic", 0.09f);
          cafe.Draw(shadowShader);

          model = glm::translate(model, GUI.lightPos);
          model = glm::scale(model, glm::vec3(0.2f));
          shadowShader.SetToMat4("model", model);
          cube.Draw(shadowShader);
          //--END OF SECOND PASS---
          /*framebufferShader.Activate();
          glBindTexture(GL_TEXTURE_2D, depthMap);
          quadVAO.Bind();
          glDrawArrays(GL_TRIANGLES, 0, 6);
          quadVAO.Unbind();*/
          //--------------END OF SHADERS & MODEL DRAWING--------------
          // ---------IMGUI---------
          ImGui::Begin("OpenGL Settings Panel");
          ImGui::Text("Tweaks");

          ImGui::Separator();

          ImGui::Text("Edit Directional Light");
          ImGui::SliderFloat3("Direction", &GUI.lightDir[0], -20.0, 20.0);

          ImGui::Separator();

          ImGui::Text("Edit Point Light");
          ImGui::SliderFloat3("Position", &GUI.lightPos[0], -20.0, 20.0);
          ImGui::End();

          ImGui::Render();
          ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
          // ---------END OF IMGUI---------

          // Swap back buffer with front buffer
          glfwSwapBuffers(window);
          // Makes sure our window is responsive (such as resizing it and moving it)
          glfwPollEvents();
     }
     ImGui_ImplOpenGL3_Shutdown();
     ImGui_ImplGlfw_Shutdown();
     ImGui::DestroyContext();

     // ------------OBJECT DELETION------------

     glfwTerminate();
     return 0;
}
