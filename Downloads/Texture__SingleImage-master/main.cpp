

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <sys/stat.h>
#include "lib/glfw/include/GLFW/glfw3.h"

static void error_callback(int error, const char *description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

double cx = 0.0, cy = 0.0, zoom = 1.0;
int itr = 100;
int fps = 0;
int z_exp = 2;

GLFWwindow *window = nullptr;

int w = 640;
int h = 480;

GLuint program;
GLuint shader;

double last_time = 0, current_time = 0;
unsigned int ticks = 0;

bool keys[1024] = { 0 };


const char *vertexShaderSource ="#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "out vec3 ourColor;\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos, 1.0);\n"
        "   ourColor = aColor;\n"
        "   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
        "}\0";

/*
const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "uniform sampler2D texture2;\n"
        "void main()\n"
        "{\n"
        "   FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);\n"
        "}\n\0";

*/
const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "void main()\n"
        "{\n"
        "   FragColor = texture(texture1, TexCoord);\n"
        "}\n\0";


static void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    double xr = 2.0 * (xpos / (double)w - 0.5);
    double yr = 2.0 * (ypos / (double)h - 0.5);

    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        cx += (xr - cx) / zoom / 2.0;
        cy -= (yr - cy) / zoom / 2.0;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoom += yoffset * 0.1 * zoom;
    if(zoom < 0.1) {
        zoom = 0.1;
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const double d = 0.1 / zoom;

    if(action == GLFW_PRESS) {
        keys[key] = true;
    } else if(action == GLFW_RELEASE) {
        keys[key] = false;
    }

    if(keys[GLFW_KEY_ESCAPE]) {
        glfwSetWindowShouldClose(window, 1);
    } else if(keys[GLFW_KEY_A]) {
        cx -= d;
    } else if(keys[GLFW_KEY_D]) {
        cx += d;
    } else if(keys[GLFW_KEY_W]) {
        cy += d;
    } else if(keys[GLFW_KEY_S]) {
        cy -= d;
    } else if(keys[GLFW_KEY_MINUS]) {
        itr += 10;
    } else if(keys[GLFW_KEY_EQUAL]) {
        itr -= 10;
        if(itr <= 0) {
            itr = 0;
        }
    }else if(keys[GLFW_KEY_2])
    {
        z_exp = 2;
    }
    else if(keys[GLFW_KEY_3])
    {
        z_exp = 3;
    }
    else if(keys[GLFW_KEY_4])
    {
        z_exp = 4;
    }
    else if(keys[GLFW_KEY_5])
    {
        z_exp = 5;
    }
    else if(keys[GLFW_KEY_6])
    {
        z_exp = 6;
    }
    else if(keys[GLFW_KEY_7])
    {
        z_exp = 7;
    }
    else if(keys[GLFW_KEY_8])
    {
        z_exp = 8;
    }
}

const char* vertex_shader =
        "#version 410\n"
                "in vec3 vp;"
                "void main () {"
                "  gl_Position = vec4 (vp, 1.0);"
                "}";

static void update_window_title()
{
    std::ostringstream ss;
    //ss << "Mandelbrot Renderer";
    ss << "Rohith Unnikrishnan - Mandelbrot";
    ss << ", Exp: " << z_exp;
    ss << ", Iterations: " << itr;
    ss << ", Zoom: " << zoom;
    ss << ", At: (" << std::setprecision(8) << cx << " + " << cy << "i)";
    glfwSetWindowTitle(window, ss.str().c_str());
}

static void compile_shader(GLuint &prog)
{
    GLuint vs = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource (vs, 1, &vertex_shader, NULL);
    glCompileShader (vs);

    std::ifstream t("../shader.glsl");
    if(!t.is_open()) {
        std::cerr << "Cannot open shader.glsl!" << std::endl;
        return;
    }
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    const char *src  = str.c_str();

    GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (fs, 1, &src, NULL);
    glCompileShader (fs);

    int success;
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if(!success) {
        int s;
        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &s);

        char *buf = new char[s];
        glGetShaderInfoLog(fs, s, &s, buf);

        std::cerr << buf << std::endl;
        delete [] buf;
        return;
    }

    prog = glCreateProgram ();
    glAttachShader (prog, fs);
    glAttachShader (prog, vs);
    glLinkProgram (prog);

    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if(!success) {
        int s;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &s);

        char *buf = new char[s];
        glGetProgramInfoLog(prog, s, &s, buf);

        std::cerr << buf << std::endl;
        delete [] buf;
        return;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

static time_t last_mtime;

static time_t get_mtime(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) == -1) {
        perror(path);
        exit(1);
    }
    return statbuf.st_mtime;
}

int main(int argc, char *argv[])
{
    if(!glfwInit()) {
        std::cerr << "Failed to init GLFW" << std::endl;
        return 1;
    }

    atexit(glfwTerminate);

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(w, h, "Mandelbrot", NULL, NULL);
    if(!window) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR_NORMAL, GLFW_STICKY_KEYS);

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    GLuint prog;
    compile_shader(prog);

    last_mtime = get_mtime("../shader.glsl");


    float points[] = {
            -1.0f,  1.0f,  0.0f,         //top left
            -1.0f,  -1.0f,  0.0f,        //bottom left
            1.0f,  -1.0f,  0.0f,         //bottom right

            -1.0f,  1.0f,  0.0f,         //top left
            1.0f,  -1.0f,  0.0f,         //bottom right
            1.0f,  1.0f,  0.0f,          //top right
    };

/*
    float points[] = {
            -0.5f,  1.0f,  0.0f,
            -0.5f,  -1.0f,  0.0f,
            1.0f,  -1.0f,  0.0f,

            -0.5f,  1.0f,  0.0f,
            1.0f,  -1.0f,  0.0f,
            1.0f,  1.0f,  0.0f,
    };
*/

    GLuint vbo = 0;
    glGenBuffers (1, &vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glBufferData (GL_ARRAY_BUFFER, 2 * 9 * sizeof (float), points, GL_STATIC_DRAW);

    GLuint vao = 0;
    glGenVertexArrays (1, &vao);
    glBindVertexArray (vao);
    glEnableVertexAttribArray (0);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glUseProgram (prog);

    last_time = glfwGetTime();

    glBindVertexArray (vao);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
            // positions          // colors           // texture coords
            -0.5f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
            -0.5f,  0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
            -1.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
            -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);



    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


//    glBindBuffer(GL_ARRAY_BUFFER_ARB, VBO);
//    glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices) ,vertices, GL_STATIC_DRAW_ARB);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // load and create a texture
    // -------------------------
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("../Name.jpg", &width, &height, &nrChannels, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);



    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int count = 0;

    while(!glfwWindowShouldClose(window)) {
        time_t new_time = get_mtime("../shader.glsl");
        if(new_time != last_mtime) {
            glDeleteProgram(prog);
            compile_shader(prog);
            glUseProgram(prog);
            last_mtime = new_time;

            std::cout << "Reloaded shader: " << last_mtime << std::endl;
        }

        count++;

//
//        {
//            ////////////////////////////////////////////////////////////////////////////////////
//            GLuint vbo = 0;
//            glGenBuffers (1, &vbo);
//            glBindBuffer (GL_ARRAY_BUFFER, vbo);
//            glBufferData (GL_ARRAY_BUFFER, 2 * 9 * sizeof (float), points, GL_STATIC_DRAW);
//
//            GLuint vao = 0;
//            glGenVertexArrays (1, &vao);
//            glBindVertexArray (vao);
//            glEnableVertexAttribArray (0);
//            glBindBuffer (GL_ARRAY_BUFFER, vbo);
//            glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
//
//            glUseProgram (prog);
//
//            last_time = glfwGetTime();
//
//            glBindVertexArray (vao);
//            ///////////////////////////////////////////////////////////////////////////////////////
//
//            glfwGetWindowSize(window, &w, &h);
//            glUniform2d(glGetUniformLocation(prog, "screen_size"), (double)w, (double)h);
//            glUniform1d(glGetUniformLocation(prog, "screen_ratio"), (double)w / (double)h);
//            glUniform2d(glGetUniformLocation(prog, "center"), cx, cy);
//            glUniform1d(glGetUniformLocation(prog, "zoom"), zoom);
//            glUniform1i(glGetUniformLocation(prog, "itr"), itr);
//            glUniform1i(glGetUniformLocation(prog, "z_exp"), z_exp);
//
//            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//            ///////////////////////////////////////////////////
//            //glBindVertexArray(vao);
//            //////////////////////////////////////////////////
//            glDrawArrays (GL_TRIANGLES, 0, 6);
//
//        }



            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if(count%2==0)
        {
            /////////////////////////////////////////////////////////////////////////////////////
            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


//    glBindBuffer(GL_ARRAY_BUFFER_ARB, VBO);
//    glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices) ,vertices, GL_STATIC_DRAW_ARB);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            // color attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            // texture coord attribute
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);
            /////////////////////////////////////////////////////////////////////////////////////

            // bind Texture
            glBindTexture(GL_TEXTURE_2D, texture);
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }


        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        glfwSwapBuffers(window);
        glfwPollEvents();

        ticks++;
        current_time = glfwGetTime();
        if(current_time - last_time > 1.0) {
            fps = ticks;
            update_window_title();
            last_time = glfwGetTime();
            ticks = 0;
        }
    }

    glfwDestroyWindow(window);
}
