#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void FramebufferSizeCallback(GLFWwindow* windowHandle, int width, int height);
void ProcessInput(GLFWwindow* windowHandle);

int main()
{
    if (glfwInit() == false)
    {
        throw std::runtime_error{"Failed to intialize GLFW"};
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    const int windowWidth = 800;
    const int windowHeight = 600;

    GLFWwindow* windowHandle = glfwCreateWindow(windowWidth, windowHeight, "OpenGL Model Viewer", nullptr, nullptr);
    if (windowHandle == nullptr)
    {
        glfwTerminate();

        throw std::runtime_error{"failed to create GLFW window"};
    }

    glfwMakeContextCurrent(windowHandle);
    
    glfwSetFramebufferSizeCallback(windowHandle, FramebufferSizeCallback);

    if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == false)
    {
        glfwSetWindowShouldClose(windowHandle, true);
        glfwDestroyWindow(windowHandle);
        glfwTerminate();

        throw std::runtime_error{"failed to initialize GLAD"};
    }

    glViewport(0, 0, windowWidth, windowHeight);

    // layout: position.x, position.y, position.z, color.r, color.g, color.b 
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // enable position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // enable color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    const char* vertexShaderSource = R"(
        #version 330 core

        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;

        out vec3 vertexColor;

        void main()
        {
            gl_Position = vec4(aPos, 1.0);
            vertexColor = aColor;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core

        in vec3 vertexColor;

        out vec4 FragColor;

        void main()
        {
            FragColor = vec4(vertexColor, 1.0);
        }
    )";

    int success;
    char log[512];

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (success != true)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, log);

        throw std::runtime_error{"vertex shader compilation failed"};
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (success != true)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, log);

        throw std::runtime_error{"fragment shader compilation failed"};
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (success != true)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, log);

        throw std::runtime_error{"shader program linking failed"};
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    while (glfwWindowShouldClose(windowHandle) == false)
    {
        ProcessInput(windowHandle);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glfwSwapBuffers(windowHandle);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(windowHandle);
    glfwTerminate();

    return 0;
}

void FramebufferSizeCallback(GLFWwindow* windowHandle, int width, int height)
{
    glViewport(0, 0, width, height);
}

void ProcessInput(GLFWwindow* windowHandle)
{
    if (glfwGetKey(windowHandle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(windowHandle, true);
    }
}