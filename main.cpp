/*  Base code by Alun Evans 2016 LaSalle (aevanss@salleurl.edu) modified by: Conrado Ruiz, Ferran Ruiz 2024
    student name: Lucia Perez y Nieves Yashuang Lopez
    description: Graphics scene of a man looking at some strange events. Implementing PBR textures, shadow mapping,  visual lighting, animation, anti aliasing, etc.
*/

// ============================================================
// LIBRARIES
// ============================================================

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glfunctions.h"
#include "Shader.h"
#include "Scene.h"      

#include "tiny_obj_loader.h"
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

// ============================================================
// GLOBALS
// ============================================================

// Viewport
int g_ViewportWidth = 1000;
int g_ViewportHeight = 1000;
double mouse_x, mouse_y;
const vec3 g_backgroundColor(0.2f, 0.2f, 0.2f);

//Anti aliasing
unsigned int samples = 8;

// Shadow mapping
const unsigned int SHADOW_W = 2048; // Res
const unsigned int SHADOW_H = 2048;
GLuint g_shadowFBO = 0;             // Framebuffer
GLuint g_shadowDepthTex = 0;
GLuint g_shadowShader = 0;

// Shaders
GLuint g_simpleShader = 0;        // PBR shader
GLuint g_simpleShader_sky = 0;    // Sky shader

// Scene (PBR objects)
Scene g_scene;

// ------------------------------------------------------------
// CAMERA VARS 

// Type
bool isFPSCamera = false;
bool isPerspective = true;

// Orbital camera
float orbitalRadius = 17.0f;
float orbitalAngleH = -5.0f;
float orbitalAngleV = -5.0f;
vec3 orbitalTarget = vec3(0.0f, 3.0f, 0.0f);

// FPS camera
vec3 cameraPos = vec3(0.0f, -1.0f, 0.0f);
vec3 cameraCenter = vec3(0.0f, 2.0f, 0.0f);
vec3 cameraUp = vec3(0.0f, 0.0f, -1.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
float cameraSpeed = 0.1f;

// FPS mouse ctl
float horMov = -120.0f;
float vertMov = 0.0f;
float lastMouseX = 256.0f;
float lastMouseY = 256.0f;

// ------------------------------------------------------------
// LIGHTS 

// Light01 - Main light arco 
vec3 g_light_dir4(-2.5f, 0.0f, -17.5f);
vec3 g_light_color4(0.7f, 0.1f, 0.15f);
float g_light_intensity4 = 2.5f;

// Light02 - Arthur fill
vec3 g_light_dir2(-0.2f, -20.0f, 50.0f);
vec3 g_light_color2(0.3f, 0.4f, 0.6f);
float g_light_intensity2 = 0.8f;

// Light03 - Bones and skulls fill
vec3 g_light_dir3(0.0f, 8.0f, 0.0f);
vec3 g_light_color3(0.3f, 0.5f, 0.7f);
float g_light_intensity3 = 0.6f;

// Light04 - Main Light 
vec3 g_light_dir(-2.5f, 7.0f, -37.5f);
vec3 g_light_color(0.7f, 0.1f, 0.15f);
float g_light_intensity = 1.3f;


// ------------------------------------------------------------
// SKYBOX + GHOST 

// Skybox
const char sphereObj[] = "assets/sphere.obj";
vector<tinyobj::shape_t> shapes_sky;
GLuint g_Vao_sky = 0;
GLuint g_NumTriangles_sky = 0;
GLuint texture_id_sky = 0;

//Ghost plane alpha
const char planeObj[] = "assets/plane.obj";
vector<tinyobj::shape_t> shapePlane;
GLuint plane_Vao = 0;
GLuint plane_NumTriangles = 0;
GLuint texture_id_plane = 0;

// ============================================================
// FUNCTIONS
// ============================================================

static void updateOrbitalCamera()
//Updates orbital camera
{
    float radH = glm::radians(orbitalAngleH);
    float radV = glm::radians(orbitalAngleV);

    cameraPos.x = orbitalTarget.x + orbitalRadius * cos(radV) * sin(radH);
    cameraPos.y = orbitalTarget.y + orbitalRadius * sin(radV);
    cameraPos.z = orbitalTarget.z + orbitalRadius * cos(radV) * cos(radH);

    cameraCenter = orbitalTarget;
    cameraUp = vec3(0.0f, 1.0f, 0.0f);
}


static GLuint LoadTexture2D(const char* path, bool flip = true)
//Loads textures with STB_image, flips them
{
    int w, h, n;
    stbi_set_flip_vertically_on_load(flip ? 1 : 0);
    unsigned char* pixels = stbi_load(path, &w, &h, &n, 0);

    if (!pixels)
    {
        cout << "ERROR: texture map couldn't be loaded: " << path
            << " -> " << stbi_failure_reason() << endl;
        return 0;
    }

    GLenum fmt = GL_RGB;
    if (n == 1) fmt = GL_RED;
    else if (n == 3) fmt = GL_RGB;
    else if (n == 4) fmt = GL_RGBA;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, pixels);

    stbi_image_free(pixels);
    return tex;
}

// --------------------------------------------------------------------------------------------

static void load()
{// Loads models, textures, shaders and shadow maps
    // -----------------------------
    // Load OBJs (sky + plane)

    bool retSky = tinyobj::LoadObj(shapes_sky, sphereObj);
    cout << (retSky ? "OBJ File loaded: " : "OBJ File FAILED: ") << sphereObj << "\n";

    bool retPlane = tinyobj::LoadObj(shapePlane, planeObj);
    cout << (retPlane ? "OBJ File loaded: " : "OBJ File FAILED: ") << planeObj << "\n";

    // -----------------------------
    // Shaders

    Shader simpleShader("src/shader.vert", "src/shader.frag");
    g_simpleShader = simpleShader.program;

    Shader simpleShader_sky("src/shader_sky.vert", "src/shader_sky.frag");
    g_simpleShader_sky = simpleShader_sky.program;

    // Shadow depth shader
    Shader shadowShader("src/shadow.vert", "src/shadow.frag");
    g_shadowShader = shadowShader.program;

    //Texture depth
    glGenTextures(1, &g_shadowDepthTex);
    glBindTexture(GL_TEXTURE_2D, g_shadowDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    //Shadow map sampling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create FBO
    glGenFramebuffers(1, &g_shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, g_shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_shadowDepthTex, 0);

    //Only depth buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // -----------------------------
    // SKYBOX VAO

    g_Vao_sky = gl_createAndBindVAO();

    // Pos
    gl_createAndBindAttribute(&(shapes_sky[0].mesh.positions[0]),
        (int)shapes_sky[0].mesh.positions.size() * (int)sizeof(float),
        g_simpleShader_sky,
        "a_vertex", 3);

    // Index
    gl_createIndexBuffer(&(shapes_sky[0].mesh.indices[0]),
        (int)shapes_sky[0].mesh.indices.size() * (int)sizeof(unsigned int));

    // UV
    gl_createAndBindAttribute(&(shapes_sky[0].mesh.texcoords[0]),
        (int)shapes_sky[0].mesh.texcoords.size() * (int)sizeof(float),
        g_simpleShader_sky,
        "a_uv", 2);

    gl_unbindVAO();
    g_NumTriangles_sky = (GLuint)shapes_sky[0].mesh.indices.size() / 3;


    // -----------------------------
    // PLANE VAO (ghost)

    plane_Vao = gl_createAndBindVAO();

    gl_createAndBindAttribute(&(shapePlane[0].mesh.positions[0]),
        (int)shapePlane[0].mesh.positions.size() * (int)sizeof(float),
        g_simpleShader_sky, "a_vertex", 3);

    gl_createAndBindAttribute(&(shapePlane[0].mesh.texcoords[0]),
        (int)shapePlane[0].mesh.texcoords.size() * (int)sizeof(float),
        g_simpleShader_sky, "a_uv", 2);

    gl_createIndexBuffer(&(shapePlane[0].mesh.indices[0]),
        (int)shapePlane[0].mesh.indices.size() * (int)sizeof(unsigned int));

    gl_unbindVAO();
    plane_NumTriangles = (GLuint)shapePlane[0].mesh.indices.size() / 3;

    // -----------------------------
    // Textures (sky / ghost)

    texture_id_sky = LoadTexture2D("textures/skydome.jpg", true);
    texture_id_plane = LoadTexture2D("textures/ghost-image.png", true);

    // -----------------------------
    // Load ALL PBR objects (Scene)

    bool ok = g_scene.LoadAll(g_simpleShader);
    if (!ok) cout << "WARNING: Scene::LoadAll() devolvió false (algún recurso falló)\n";
}

// ---------------------------------------------------------------------------------------------------------

static void draw()
{//Renders the whole scene (shadow + main)

    float tiempo = (float)glfwGetTime();

    // =======================================================
    // PROJECTION
    // =======================================================
    mat4 projection_matrix;
    if (isPerspective) {
        projection_matrix = glm::perspective(90.0f, 1.0f, 0.1f, 50.0f);
    }
    else {
        projection_matrix = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, 0.1f, 50.0f);
    }

    // =======================================================
    // VIEW
    // =======================================================
    mat4 view_matrix;
    if (isFPSCamera) {
        view_matrix = glm::lookAt(cameraPos, cameraCenter, cameraUp);
    }
    else {
        updateOrbitalCamera();
        view_matrix = glm::lookAt(cameraPos, cameraCenter, cameraUp);
    }

    // =======================================================
    // ANIMATION 
    // =======================================================

    //Time
    float t = (float)glfwGetTime();

    // Circle center
    vec3 floatingCenter = vec3(-1.0f, -2.7f, 0.0f);

    // Extra movement attributes
    float orbitSpeed = t * 0.3f;          //speed
    float outerRadius = 2.8f;             //outer radius
    float innerRadius = 0.7f;             //inner radius
    float baseHeight = 3.5f;              //mov height
    float waveHeight = 0.4f;              //wave amplitude

    // BoneLeg01

    mat4 boneLeg1(1.0f);

    {
        float angleOffset = 0.0f;               //initial pos
        float angle = orbitSpeed + angleOffset; //angle rot

        // pos in the circle (based on radius)
        float x = cos(angle) * outerRadius;
        float z = sin(angle) * outerRadius;
        float y = baseHeight + sin(angle) * waveHeight;  // wave length

        boneLeg1 = translate(mat4(1.0f), floatingCenter + vec3(x, y, z)); // whole translation
        boneLeg1 = boneLeg1 * rotate(mat4(1.0f), t * 0.4f, vec3(0, 1, 0));  //self rotation
    }

    // BoneLeg02

    mat4 boneLeg2(1.0f);
    {
        float angleOffset = 2.0f;               //initial pos
        float angle = orbitSpeed + angleOffset; //angle rot
        // pos in the circle (based on radius)
        float x = cos(angle) * outerRadius;
        float z = sin(angle) * outerRadius;
        float y = baseHeight + sin(angle) * waveHeight;// wave length

        boneLeg2 = translate(mat4(1.0f), floatingCenter + vec3(x, y, z));// whole translation
        boneLeg2 = boneLeg2 * rotate(mat4(1.0f), t * 0.5f, vec3(0, 1, 0)); //self rotation
    }

    // boneArm

    mat4 boneArm(1.0f);
    {
        float angleOffset = 4.0f;                //initial pos
        float angle = orbitSpeed + angleOffset;  //angle rot
        // pos in the circle (based on radius)
        float x = cos(angle) * outerRadius;
        float z = sin(angle) * outerRadius;
        float y = baseHeight + sin(angle) * waveHeight;// wave length

        boneArm = translate(mat4(1.0f), floatingCenter + vec3(x, y, z));// whole translation
        boneArm = boneArm * rotate(mat4(1.0f), t * 0.6f, vec3(0, 1, 0)); //self rotation
    }

    // LambSkull

    mat4 skullL(1.0f);
    {
        float angleOffset = 0.5f;               //initial pos
        float angle = orbitSpeed + angleOffset; //angle rot
        // pos in the circle (based on radius)
        float x = cos(angle) * innerRadius;
        float z = sin(angle) * innerRadius;
        float y = baseHeight + sin(angle) * waveHeight;// wave length

        skullL = translate(mat4(1.0f), floatingCenter + vec3(x, y, z));// whole translation
        skullL = skullL * rotate(mat4(1.0f), t * 0.3f, vec3(0, 1, 0)); //self rotation
    }

    // DeerSkull

    mat4 skullD(1.0f);
    {
        float angleOffset = 3.6f;               //initial pos
        float angle = orbitSpeed + angleOffset; //angle rot
        // pos in the circle (based on radius)
        float x = cos(angle) * innerRadius;
        float z = sin(angle) * innerRadius;
        float y = baseHeight + sin(angle) * waveHeight;// wave length

        skullD = translate(mat4(1.0f), floatingCenter + vec3(x, y, z));// whole translation
        skullD = skullD * rotate(mat4(1.0f), t * 0.35f, vec3(0, 1, 0)); //self rotation
    }


    // =======================================================
    // SHADOW 
    // =======================================================

    // Lightspace matrix
    glm::vec3 lightPos = g_light_dir;                       //Light_01 pos
    glm::vec3 lightTarget = glm::vec3(0.0f, 1.5f, 0.0f);    //Light_01 look at

    glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0, 1, 0));
    glm::mat4 lightProj = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 1.0f, 60.0f);
    glm::mat4 lightSpace = lightProj * lightView;

    // Shadow  vp
    glViewport(0, 0, SHADOW_W, SHADOW_H);
    glBindFramebuffer(GL_FRAMEBUFFER, g_shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_shadowShader);

    GLint uLS = glGetUniformLocation(g_shadowShader, "u_lightSpace");
    GLint uM = glGetUniformLocation(g_shadowShader, "u_model");
    glUniformMatrix4fv(uLS, 1, GL_FALSE, glm::value_ptr(lightSpace));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    auto DrawDepthObj = [&](const ObjectPBR& obj, const glm::mat4& model)
        {
            if (obj.GetVAO() == 0 || obj.GetNumTriangles() == 0) return;
            glUniformMatrix4fv(uM, 1, GL_FALSE, glm::value_ptr(model));
            gl_bindVAO(obj.GetVAO());
            glDrawElements(GL_TRIANGLES, 3 * obj.GetNumTriangles(), GL_UNSIGNED_INT, 0);
        };



    glm::mat4 I(1.0f);

    // Estáticos
    DrawDepthObj(g_scene.arcoC, I);
    DrawDepthObj(g_scene.paredC, I);
    DrawDepthObj(g_scene.suelo, I);
    DrawDepthObj(g_scene.rocaS, I);
    DrawDepthObj(g_scene.paredD, I);
    DrawDepthObj(g_scene.paredI, I);
    DrawDepthObj(g_scene.arthur, I);

    // Animados 
    DrawDepthObj(g_scene.boneLeg, boneLeg1);
    DrawDepthObj(g_scene.boneLeg, boneLeg2);
    DrawDepthObj(g_scene.skullLamb, skullL);
    DrawDepthObj(g_scene.skullDeer, skullD);
    DrawDepthObj(g_scene.boneArm, boneArm);

    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, g_ViewportWidth, g_ViewportHeight);

    //---------------------------------------------------------------------------------------------------------------

    //MAIN
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_MULTISAMPLE); //AA
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDisable(GL_DEPTH_TEST);

    // -----------------------------
    // SKYBOX
    glUseProgram(g_simpleShader_sky);

    GLuint u_texture_sky_loc = glGetUniformLocation(g_simpleShader_sky, "u_texture");
    glUniform1i(u_texture_sky_loc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id_sky);

    GLuint model_loc_sky = glGetUniformLocation(g_simpleShader_sky, "u_model");
    GLuint view_loc_sky = glGetUniformLocation(g_simpleShader_sky, "u_view");
    GLuint proj_loc_sky = glGetUniformLocation(g_simpleShader_sky, "u_projection");

    // Follows cam pos
    mat4 model_matrix_sky = isPerspective ? translate(mat4(1.0f), cameraPos) : mat4(1.0f);

    glUniformMatrix4fv(model_loc_sky, 1, GL_FALSE, glm::value_ptr(model_matrix_sky));
    glUniformMatrix4fv(view_loc_sky, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(proj_loc_sky, 1, GL_FALSE, glm::value_ptr(projection_matrix));

    gl_bindVAO(g_Vao_sky);
    glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles_sky, GL_UNSIGNED_INT, 0);

    // -----------------------------
    // SCENE (PBR)
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glUseProgram(g_simpleShader);

    // Shadow uniforms / shadow map 
    GLint uLS_pbr = glGetUniformLocation(g_simpleShader, "u_lightSpace");
    if (uLS_pbr >= 0) glUniformMatrix4fv(uLS_pbr, 1, GL_FALSE, glm::value_ptr(lightSpace));

    GLint uShadow = glGetUniformLocation(g_simpleShader, "u_shadowMap");
    if (uShadow >= 0) glUniform1i(uShadow, 4);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_shadowDepthTex);


    // ######################## Lights ##############################

    //Camera pos
    glUniform3f(glGetUniformLocation(g_simpleShader, "u_cam_pos"),
        cameraPos.x, cameraPos.y, cameraPos.z);

    //General light properties
    glUniform3f(glGetUniformLocation(g_simpleShader, "u_ambient"),
        0.1f, 0.03f, 0.07f);
    glUniform3f(glGetUniformLocation(g_simpleShader, "u_specular"),
        0.1f, 0.1f, 0.1f);
    glUniform1f(glGetUniformLocation(g_simpleShader, "u_shininess"), 40.0f);

    //LIGHTS
    //01 - Main
    glUniform3f(glGetUniformLocation(g_simpleShader, "u_light_dir"),
        g_light_dir.x, g_light_dir.y, g_light_dir.z);
    glUniform3f(glGetUniformLocation(g_simpleShader, "u_diffuse"),
        g_light_color.x * g_light_intensity,
        g_light_color.y * g_light_intensity,
        g_light_color.z * g_light_intensity);

    //02 - Arthur fill
    glUniform3f(glGetUniformLocation(g_simpleShader, "u_light_dir2"),
        g_light_dir2.x, g_light_dir2.y, g_light_dir2.z);
    glUniform3f(glGetUniformLocation(g_simpleShader, "u_diffuse2"),
        g_light_color2.x * g_light_intensity2,
        g_light_color2.y * g_light_intensity2,
        g_light_color2.z * g_light_intensity2);

    //03 - Bones fill
    glUniform3f(glGetUniformLocation(g_simpleShader, "u_light_dir3"),
        g_light_dir3.x, g_light_dir3.y, g_light_dir3.z);
    glUniform3f(glGetUniformLocation(g_simpleShader, "u_diffuse3"),
        g_light_color3.x * g_light_intensity3,
        g_light_color3.y * g_light_intensity3,
        g_light_color3.z * g_light_intensity3);

    //04 - Main arco
    glUniform3f(glGetUniformLocation(g_simpleShader, "u_light_dir4"),
        g_light_dir4.x, g_light_dir4.y, g_light_dir4.z);
    glUniform3f(glGetUniformLocation(g_simpleShader, "u_diffuse4"),
        g_light_color4.x * g_light_intensity4,
        g_light_color4.y * g_light_intensity4,
        g_light_color4.z * g_light_intensity4);


    // -----------------------------
    // Estáticos
    mat4 M = mat4(1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    g_scene.arcoC.Draw(M, view_matrix, projection_matrix);
    glDisable(GL_BLEND);

    g_scene.paredC.Draw(M, view_matrix, projection_matrix);
    g_scene.suelo.Draw(M, view_matrix, projection_matrix);
    g_scene.rocaS.Draw(M, view_matrix, projection_matrix);
    g_scene.paredD.Draw(M, view_matrix, projection_matrix);
    g_scene.paredI.Draw(M, view_matrix, projection_matrix);
    g_scene.arthur.Draw(M, view_matrix, projection_matrix);

    // -----------------------------
    // Animados 

    g_scene.boneLeg.Draw(boneLeg1, view_matrix, projection_matrix);
    g_scene.boneLeg.Draw(boneLeg2, view_matrix, projection_matrix);
    g_scene.skullLamb.Draw(skullL, view_matrix, projection_matrix);
    g_scene.skullDeer.Draw(skullD, view_matrix, projection_matrix);
    g_scene.boneArm.Draw(boneArm, view_matrix, projection_matrix);

    // -----------------------------
    // GHOST PLANE (sky shader + alpha)
    glUseProgram(g_simpleShader_sky);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glDisable(GL_CULL_FACE);

    GLuint u_texture_plane_loc = glGetUniformLocation(g_simpleShader_sky, "u_texture");
    glUniform1i(u_texture_plane_loc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id_plane);

    GLuint model_loc_plane = glGetUniformLocation(g_simpleShader_sky, "u_model");
    GLuint view_loc_plane = glGetUniformLocation(g_simpleShader_sky, "u_view");
    GLuint proj_loc_plane = glGetUniformLocation(g_simpleShader_sky, "u_projection");

    mat4 model_plane = translate(mat4(1.0f), vec3(0.0f, -7.0f, 0.0f)) *
        scale(mat4(1.0f), vec3(1.5f, 1.5f, 1.5f));


    glUniformMatrix4fv(model_loc_plane, 1, GL_FALSE, glm::value_ptr(model_plane));
    glUniformMatrix4fv(view_loc_plane, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(proj_loc_plane, 1, GL_FALSE, glm::value_ptr(projection_matrix));

    gl_bindVAO(plane_Vao);
    glDrawElements(GL_TRIANGLES, 3 * plane_NumTriangles, GL_UNSIGNED_INT, 0);

    //Return to normal
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);

    glUseProgram(g_simpleShader);
}


// ------------------------------------------------------------
// INPUT

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{// Key callback to control the scene

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) //Exit
        glfwSetWindowShouldClose(window, 1);

    if (key == GLFW_KEY_R && action == GLFW_PRESS) //Reload
        load();

    if (key == GLFW_KEY_P && action == GLFW_PRESS) //Ortho/persp
        isPerspective = !isPerspective;

    if (key == GLFW_KEY_O && action == GLFW_PRESS) //Orbital
        isFPSCamera = false;

    if (key == GLFW_KEY_F && action == GLFW_PRESS) //FPS
        isFPSCamera = true;

    // Orbital controls
    if (!isFPSCamera)
    {
        if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
            orbitalAngleH -= 5.0f;
        if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
            orbitalAngleH += 5.0f;

        if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            orbitalAngleV += 5.0f;
            if (orbitalAngleV > 80.0f) orbitalAngleV = 80.0f;
        }
        if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            orbitalAngleV -= 5.0f;
            if (orbitalAngleV < -80.0f) orbitalAngleV = -80.0f;
        }

        if (key == GLFW_KEY_PAGE_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            orbitalRadius -= 0.5f;
            if (orbitalRadius < 1.0f) orbitalRadius = 1.0f;
        }
        if (key == GLFW_KEY_PAGE_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            orbitalRadius += 0.5f;
            if (orbitalRadius > 20.0f) orbitalRadius = 20.0f;
        }
    }
    else    //FPS controls
    {
        vec3 F = glm::normalize(cameraCenter - cameraPos);
        vec3 S = glm::normalize(glm::cross(F, cameraUp));

        if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            cameraPos += F * cameraSpeed;
            cameraCenter += F * cameraSpeed;
        }
        if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            cameraPos -= F * cameraSpeed;
            cameraCenter -= F * cameraSpeed;
        }
        if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            cameraPos -= S * cameraSpeed;
            cameraCenter -= S * cameraSpeed;
        }
        if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            cameraPos += S * cameraSpeed;
            cameraCenter += S * cameraSpeed;
        }
    }
}

static void mouse_movement_callback(GLFWwindow* window, double xpos, double ypos)
{//Mouse callback
    if (!isFPSCamera) return;

    float xoff = (float)(xpos - lastMouseX) * 0.1f;
    float yoff = (float)(lastMouseY - ypos) * 0.1f;

    //Calc pos
    lastMouseX = (float)xpos;
    lastMouseY = (float)ypos;

    //Save pos
    horMov += xoff;
    vertMov += yoff;

    // Avoid gimbal lock
    if (vertMov > 89.0f) vertMov = 89.0f;
    if (vertMov < -89.0f) vertMov = -89.0f;

    //Calc cam dir
    vec3 direction;
    direction.x = cos(glm::radians(horMov)) * cos(glm::radians(vertMov));
    direction.y = sin(glm::radians(vertMov));
    direction.z = sin(glm::radians(horMov)) * cos(glm::radians(vertMov));

    cameraFront = normalize(direction);
    cameraCenter = cameraPos + normalize(direction);
}

// ------------------------------------------------------------------------------------------------------

int main(void)
{
    GLFWwindow* window;
    if (!glfwInit()) return -1;

    //window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, samples);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(g_ViewportWidth, g_ViewportHeight, "a bad dream", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    glewInit();

    cout << "P = Pasar de perspectiva a ortografica\nF/O = Cambiar FPS / Orbital\n" << endl;

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_movement_callback);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

    glClearColor(g_backgroundColor.x, g_backgroundColor.y, g_backgroundColor.z, 1.0f);

    load();

    while (!glfwWindowShouldClose(window))
    {
        draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
    }

    glfwTerminate();
    return 0;
}
