#include <cmath>

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void FramebufferSizeCallback(GLFWwindow* windowHandle, int width, int height);
void ProcessInput(GLFWwindow* windowHandle, float& distanceFromTarget, float& azimuth, float& elevation, float deltaTime);

glm::vec3 CalculateCameraPosition(float distanceFromTarget, float azimuth, float elevation, const glm::vec3& target);

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
        -0.5f, -0.5f, 0.7f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.3f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f
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

        uniform mat4 modelMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 projectionMatrix;

        void main()
        {
            gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos, 1.0);

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
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, log);
        std::cerr << log << std::endl;
        throw std::runtime_error{"vertex shader compilation failed"};
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, log);
        std::cerr << log << std::endl;
        throw std::runtime_error{"fragment shader compilation failed"};
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, log);
        std::cerr << log << std::endl;
        throw std::runtime_error{"shader program linking failed"};
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float cameraDistanceFromTarget = 0.5f;
    float cameraAzimuth = 0.0f;
    float cameraElevation = 0.0f;
    glm::vec3 cameraTarget{0.0f, 0.0f, 0.0f};
    const glm::vec3 cameraUp{0.0f, 1.0f, 0.0f};

    float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    const float fov = glm::radians(45.0f);
    const float distanceToNearPlane = 0.1f;
    const float distanceToFarPlane = 100.0f;

    const int modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
    const int viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
    const int projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");

    glEnable(GL_DEPTH_TEST);

    float lastFrameTime = 0.0f;

    while (glfwWindowShouldClose(windowHandle) == false)
    {
        float currentFrameTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        ProcessInput(windowHandle, cameraDistanceFromTarget, cameraAzimuth, cameraElevation, deltaTime);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 modelMatrix = glm::mat4{1.0f};

        glm::vec3 cameraPos = CalculateCameraPosition(cameraDistanceFromTarget, cameraAzimuth, cameraElevation, cameraTarget);
        glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        
        glm::mat4 projectionMatrix = glm::perspective(fov, aspectRatio, distanceToNearPlane, distanceToFarPlane);

        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

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

void FramebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

void ProcessInput(GLFWwindow* windowHandle, float& distanceFromTarget, float& azimuth, float& elevation, float deltaTime)
{
    if (glfwGetKey(windowHandle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(windowHandle, true);
    }

    const float cameraRotationSpeed = 2.0f;
    const float cameraZoomSpeed = 5.0f;

    // horizontal rotation (left/right around target)
    if (glfwGetKey(windowHandle, GLFW_KEY_A) == GLFW_PRESS)
    {
        azimuth -= cameraRotationSpeed * deltaTime;  // rotate counterclockwise
    }
    if (glfwGetKey(windowHandle, GLFW_KEY_D) == GLFW_PRESS)
    {
        azimuth += cameraRotationSpeed * deltaTime;  // rotate clockwise
    }

    // zoom in/out (change distance from target)
    if (glfwGetKey(windowHandle, GLFW_KEY_W) == GLFW_PRESS)
    {
        distanceFromTarget -= cameraZoomSpeed * deltaTime;  // move closer
        if (distanceFromTarget < 0.5f)
        {
            distanceFromTarget = 0.5f;
        }
    }
    if (glfwGetKey(windowHandle, GLFW_KEY_S) == GLFW_PRESS)
    {
        distanceFromTarget += cameraZoomSpeed * deltaTime;  // move father
        if (distanceFromTarget > 20.0f)
        {
            distanceFromTarget = 20.f;
        }
    }

    // vertical rotation (up/down view angle)
    if (glfwGetKey(windowHandle, GLFW_KEY_Q) == GLFW_PRESS)
    {
        elevation += cameraRotationSpeed * deltaTime;  // move up higher
        if (elevation > glm::radians(89.0f))
        {
            elevation = glm::radians(89.0f);
        }
    }
    if (glfwGetKey(windowHandle, GLFW_KEY_E) == GLFW_PRESS)
    {
        elevation -= cameraRotationSpeed * deltaTime;  // move down lower
        if (elevation < glm::radians(-89.0f))
        {
            elevation = glm::radians(-89.0f);
        }
    }
}

glm::vec3 CalculateCameraPosition(float distanceFromTarget, float azimuth, float elevation, const glm::vec3& target)
{
    // convert spherical coordinates to cartesian offset from target
    const float x = distanceFromTarget * std::cos(elevation) * std::sin(azimuth);
    const float y = distanceFromTarget * std::sin(elevation);
    const float z = distanceFromTarget * std::cos(elevation) * std::cos(azimuth);

    // add the offset to the target position to get the final camera position
    return target + glm::vec3{x, y, z};
}