#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "shader.h"
#include "filesystem.h"
#include "camera.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <map>

#include <irrKlang.h>
using namespace irrklang;
#pragma comment(lib, "irrklang.lib")

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void generateMaze();

void processReplay(GLFWwindow *window);

void processInput(GLFWwindow *window);

void mouse_move_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

bool is_world_hit(GLFWwindow *window, glm::vec3 next);

bool is_world_in_air(GLFWwindow *window, glm::vec3 next);

bool is_world_arriving(GLFWwindow *window);

bool is_world_dangerous(GLFWwindow *window);

void RenderText(Shader &s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

void consume(float number);

glm::vec3 getNextPositionByProcessInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int GamePhysical = 100;
int ElevateStatus = 0; // -1=fail 1=climb


unsigned int VBO, VAO, EBO;

unsigned int TEXTVBO, TEXTVAO, TEXTEBO;

unsigned int RAINBOWVBO, RAINBOWVAO, RAINBOWEBO;

struct Character {
    GLuint TextureID;  // 字形纹理的ID
    glm::ivec2 Size;       // 字形大小
    glm::ivec2 Bearing;    // 从基准线到字形左部/顶部的偏移值
    GLuint Advance;    // 原点距下一个字形原点的距离
};

std::map <GLchar, Character> Characters;

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

// camera
Camera camera(glm::vec3(0.0f, 10.0f, 3.0f));
float lastX = SCR_WIDTH / 2, lastY = SCR_HEIGHT / 2;
bool firstMouse = true;

int GameStatus = -1; // -1=init 0=normal 1=win -10=dangerous

glm::vec3 cubePositions[5000 + 816];

glm::vec3 exitPosition[] = {
        glm::vec3(-51.0f, 1.5f, -51.0f),
        glm::vec3(-50.0f, 1.5f, -51.0f),
        glm::vec3(-49.0f, 1.5f, -51.0f),
};

glm::vec3 rainbowPosition = glm::vec3(-57.0f, 0.0f, -58.0f);

int main() {

    ISoundEngine* engine = createIrrKlangDevice();

    if (!engine)
        return 0; // error starting up the engine

    // play some sound stream, looped
    engine->play2D(FileSystem::getPath("breakout.mp3").c_str(), true);

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_move_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader(FileSystem::getPath("texture.vs").c_str(), FileSystem::getPath("texture.fs").c_str());

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
    };

    float rainbowVertices[] = {
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,

            -0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,

            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
    };


    generateMaze();

    glEnable(GL_DEPTH_TEST);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int texture1;
    glGenTextures(1, &texture1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    // 为当前绑定的纹理对象设置环绕、过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 加载并生成纹理
    int width, height, nrChannels;
    unsigned char *data = stbi_load(FileSystem::getPath("container.jpeg").c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);;

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------

    glm::mat4 model = glm::mat4(1.0f);

    ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
    ourShader.setInt("texture1", 0);

    /* For Text Rendering */
    Shader textShader(FileSystem::getPath("text-texture.vs").c_str(), FileSystem::getPath("text-texture.fs").c_str());
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    textShader.use();
    glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    // --------
    // All functions return a value different than 0 whenever an error occurred
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, FileSystem::getPath("Antonio-Bold.ttf").c_str(), 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

    FT_Set_Pixel_Sizes(face, 0, 48);

    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
    // render loop
    // -----------

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //禁用字节对齐限制

    // Create Rainbow Exit Cube
    Shader rainbowShader(FileSystem::getPath("rainbow-texture.vs").c_str(),
                         FileSystem::getPath("rainbow-texture.fs").c_str());

    rainbowShader.use();
    glEnable(GL_DEPTH_TEST);

    glGenVertexArrays(1, &RAINBOWVAO);
    glGenBuffers(1, &RAINBOWVBO);
    glGenBuffers(1, &RAINBOWEBO);

    glBindVertexArray(RAINBOWVAO);

    glBindBuffer(GL_ARRAY_BUFFER, RAINBOWVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rainbowVertices), rainbowVertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // Text Shader
    textShader.use();

    for (GLubyte c = 0; c < 128; c++) {
        // 加载字符的字形
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // 生成纹理
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
        );
        // 设置纹理选项
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 储存字符供之后使用
        Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<GLuint>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenVertexArrays(1, &TEXTVAO);
    glGenBuffers(1, &TEXTVBO);
    glBindVertexArray(TEXTVAO);
    glBindBuffer(GL_ARRAY_BUFFER, TEXTVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processReplay(window);

        // 💥 处理
        processInput(window);
        if (GameStatus == -1) {
            // do nothing
        } else if (GameStatus == 1) {
            // do nothing
        } else if (is_world_arriving(window)) {
            GameStatus = 1;
        } else if (is_world_dangerous(window)) {
            GameStatus = -10;
        } else {
            GameStatus = 0;
        }

        // render
        glClearColor(0.6f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        ourShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f,
                                                100.0f);


        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();


        int transformLoc = glGetUniformLocation(ourShader.ID, "transform");

        int modelLoc = glGetUniformLocation(ourShader.ID, "model");

        int viewLoc = glGetUniformLocation(ourShader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // trans
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(0.0f, -0.25f, 0.0f));
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE,
                           &transform[0][0]); // this time take the matrix value array's first element as its memory pointer value
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        for (unsigned int i = 0; i < (sizeof(cubePositions) / sizeof(cubePositions[0])); i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        rainbowShader.use();

        int rainbowTransformLoc = glGetUniformLocation(rainbowShader.ID, "transform");
        glm::mat4 rainbowTransform = glm::mat4(1.0f);
        rainbowTransform = glm::scale(rainbowTransform, glm::vec3(0.5f, 0.5f, 0.5f));
        rainbowTransform = glm::rotate(rainbowTransform, (float)glfwGetTime(), glm::vec3(1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(rainbowTransformLoc, 1, GL_FALSE,
                           &rainbowTransform[0][0]); // this time take the matrix value array's first element as its memory pointer value

        int rainbowModelLoc = glGetUniformLocation(rainbowShader.ID, "model");

        int rainbowViewLoc = glGetUniformLocation(rainbowShader.ID, "view");
        glUniformMatrix4fv(rainbowViewLoc, 1, GL_FALSE, glm::value_ptr(view));

        int rainbowProjectionLoc = glGetUniformLocation(rainbowShader.ID, "projection");
        glUniformMatrix4fv(rainbowProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // rainbow model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, rainbowPosition);
        glUniformMatrix4fv(rainbowModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        RenderText(textShader, "(" + std::to_string(GamePhysical) + "%)", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));

        if (GameStatus == 1) {
            RenderText(textShader, "YOU WIN", SCR_WIDTH / 2 - 100, SCR_HEIGHT / 2, 1.0f, glm::vec3(1.0, 1.0f, 1.0f));
        } else if (GameStatus == -1) {
            RenderText(textShader, "* MAZE GAME *", SCR_WIDTH / 2 - 140, SCR_HEIGHT / 2, 1.0f, glm::vec3(1.0, 1.0f, 1.0f));
        } else if (GameStatus == -10) {
            RenderText(textShader, "DANGEROUS", SCR_WIDTH / 2 - 100, SCR_HEIGHT / 2, 1.0f, glm::vec3(1.0, 0.0f, 0.0f));
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    engine->drop(); // delete engine

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    // glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}


void generateMaze() {
    unsigned int seed = ((unsigned) time(NULL));

    for (unsigned int i = 0; i < 5000; i++) {
        int x = rand() % 101 - 50;
        int z = rand() % 101 - 50;
        cubePositions[i] = glm::vec3(x, 0.0f, z);
    }

    for (int i = 5000, j = 0; j < 102; i += 2, j++) {
        cubePositions[i] = glm::vec3(-51, 0.0f, 51 - j);
        cubePositions[i + 1] = glm::vec3(-51, 1.0f, 51 - j);
    }

    cubePositions[5204] = glm::vec3(-52.0f, 0.0f, -51.0f);
    cubePositions[5205] = glm::vec3(-53.0f, 0.0f, -52.0f);
    cubePositions[5206] = glm::vec3(-54.0f, 0.0f, -53.0f);
    cubePositions[5207] = glm::vec3(-49.0f, 0.0f, -52.0f);
    cubePositions[5208] = glm::vec3(-50.0f, 0.0f, -53.0f);
    cubePositions[5209] = glm::vec3(-51.0f, 0.0f, -54.0f);

    for (int i = 5210, j = 0; j < 99; i += 2, j++) {
        cubePositions[i] = glm::vec3(-48 + j, 0.0f, -51);
        cubePositions[i + 1] = glm::vec3(-48 + j, 1.0f, -51);
    }

    for (int i = 5408, j = 0; j < 102; i += 2, j++) {
        cubePositions[i] = glm::vec3(51, 0.0f, -51 + j);
        cubePositions[i + 1] = glm::vec3(51, 1.0f, -51 + j);
    }

    for (int i = 5612, j = 0; j < 102; i += 2, j++) {
        cubePositions[i] = glm::vec3(51 - j, 0.0f, 51);
        cubePositions[i + 1] = glm::vec3(51 - j, 1.0f, 51);
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3 nextPosition = getNextPositionByProcessInput(window);

    if (is_world_hit(window, nextPosition)) {
        ElevateStatus = 1;
        camera.Climb(deltaTime);
        return;
    }

    if (is_world_in_air(window, nextPosition)) {
        ElevateStatus = -1;
        camera.Fall(deltaTime * 1.5);
        return;
    }

    if (GameStatus == -1) {
        GameStatus = 0;
    }

    if (ElevateStatus == 1) {
        ElevateStatus = 0;
        consume(rand() % 10);
    }

    if (ElevateStatus == -1) {
        ElevateStatus = 0;
    }

    camera.To(nextPosition);
}

void consume(float number) {
    if (GameStatus != 1 && (GameStatus != -1)) {
        GamePhysical -= number;
    }
}

void processReplay(GLFWwindow *window) {
    if ((glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) || GamePhysical <= 0) {
        GameStatus = -1;
        GamePhysical = 100;
        generateMaze();
        camera.To(glm::vec3(0.0f, 10.0f + rand() % 3, 3.0f));
    }
}

glm::vec3 getNextPositionByProcessInput(GLFWwindow *window) {
    glm::vec3 next = camera.Position;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        next = camera.getKeyboardNextPositionValue(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        next = camera.getKeyboardNextPositionValue(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        next = camera.getKeyboardNextPositionValue(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        next = camera.getKeyboardNextPositionValue(RIGHT, deltaTime);
    return next;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_move_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

bool is_world_hit(GLFWwindow *window, glm::vec3 next) {

    for (unsigned int i = 0; i < (sizeof(cubePositions) / sizeof(cubePositions[0])); i++) {
        glm::vec3 cube = cubePositions[i];
        if ((abs(next.x - cube.x) <= 0.705) && (abs(next.y - cube.y) <= 0.705) && (abs(next.z - cube.z) <= 0.705)) {
            return true;
        }
    }
//    std::cout << " go through " << " x=" << next.x << " y=" << next.y << " z=" << next.z << std::endl;
    return false;
}


bool is_world_in_air(GLFWwindow *window, glm::vec3 next) {

    if (next.y == 0) {
        return false;
    }

    bool is_on = false;

    for (unsigned int i = 0; i < (sizeof(cubePositions) / sizeof(cubePositions[0])); i++) {
        glm::vec3 cube = cubePositions[i];
        if ((abs(next.x - cube.x) <= 0.705) && (abs(next.y - cube.y) <= 0.8) && (abs(next.z - cube.z) <= 0.705)) {
            is_on = true;
        }
    }
    return !is_on;
}

bool is_world_arriving(GLFWwindow *window) {
    glm::vec3 position = camera.Position;

    for (unsigned int i = 0; i < (sizeof(exitPosition) / sizeof(exitPosition[0])); i++) {
        glm::vec3 exit = exitPosition[i];
        if ((abs(position.x - exit.x) <= 0.705) && (abs(position.z - exit.z) <= 0.705)) {
            return true;
        }
    }

    return false;
}

bool is_world_dangerous(GLFWwindow *window) {
    glm::vec3 position = camera.Position;

    if (abs(position.x) > 51 || abs(position.z) > 51) {
        return true;
    }

    return false;
}


void RenderText(Shader &s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
    // 激活对应的渲染状态
    s.use();
    glUniform3f(glGetUniformLocation(s.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(TEXTVAO);

    // 遍历文本中所有的字符
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // 对每个字符更新VBO
        GLfloat vertices[6][4] = {
                {xpos,     ypos + h, 0.0, 0.0},
                {xpos,     ypos,     0.0, 1.0},
                {xpos + w, ypos,     1.0, 1.0},

                {xpos,     ypos + h, 0.0, 0.0},
                {xpos + w, ypos,     1.0, 1.0},
                {xpos + w, ypos + h, 1.0, 0.0}
        };
        // 在四边形上绘制字形纹理
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // 更新VBO内存的内容
        glBindBuffer(GL_ARRAY_BUFFER, TEXTVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // 绘制四边形
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // 更新位置到下一个字形的原点，注意单位是1/64像素
        x += (ch.Advance >> 6) * scale; // 位偏移6个单位来获取单位为像素的值 (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}