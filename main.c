#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "config.h"


// Vertex Shader Source Code
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "}\0";

// Function to read shader source from a file
char* readShaderSource(const char* shaderFile) {
    FILE* file = fopen(shaderFile, "r");
    if (!file) {
        fprintf(stderr, "Failed to open shader file %s\n", shaderFile);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* shaderSource = (char*)malloc(length + 1);
    if (!shaderSource) {
        fprintf(stderr, "Failed to allocate memory for shader source\n");
        fclose(file);
        return NULL;
    }

    fread(shaderSource, 1, length, file);
    shaderSource[length] = '\0';
    fclose(file);

    return shaderSource;
}

// Global variable to control the pause state
volatile sig_atomic_t paused = 0;

// Signal handlers
void pause_signal(int signal) {
    paused = 1;
}
void unpause_signal(int signal) {
    paused = 0;
}

// Callback to update mouse position
double mouseX = 0.0, mouseY = 0.0;
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    mouseX = xpos;
    mouseY = ypos;
}

// Load an image as a texture
void load_texture(unsigned int *texture, char *path)
{
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    // Set texture wrapping and filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Force loading the image with 4 channels (RGBA)
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, STBI_rgb_alpha);
    if (data) {
        // Ensure OpenGL expects 4 channels (RGBA)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

    } else {
        fprintf(stderr, "Failed to load texture\n");
    }
    stbi_image_free(data);
}

// Function to check if a file has been modified since the last check
int has_file_changed(const char *filename, time_t *last_mod_time) {
    struct stat file_stat;

    // Get file metadata
    if (stat(filename, &file_stat) == -1) {
        perror("stat");
        return -1; // Error accessing the file
    }

    // Compare the current modification time with the last stored modification time
    if (*last_mod_time != file_stat.st_mtime) {
        *last_mod_time = file_stat.st_mtime;
        return 1; // File has changed
    }

    return 0; // File has not changed
}

int main(int argc, char *argv[]) {
    // Check command line args
    if (argc != 3 && argc != 2) {
        fprintf(stderr, "Usage: GLWall <path_to_shader> [<path_to_texture>]\n");
        return -1;
    }

    const char *battery_capacity_path = "/sys/class/power_supply/BAT0/capacity";

    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // Set the window class for Wayland
    const char* app_id = getenv("GLWALL_CLASS");
    if (!app_id) app_id = "GLWall";
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, app_id);

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "GLWall", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    // Set the cursor position callback
    glfwSetCursorPosCallback(window, cursor_position_callback);


    // Make the OpenGL context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // Read Fragment Shader from file
    char* fragmentShaderSource = readShaderSource(argv[1]);
    if (!fragmentShaderSource) {
        return -1;
    }

    // Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* shaderSource = (const GLchar*)fragmentShaderSource; // Cast to correct type
    glShaderSource(fragmentShader, 1, &shaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // Link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    free(fragmentShaderSource);

    // Set up vertex data (and buffer(s)) and configure vertex attributes
    float vertices[] = {
        // positions
         1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Load and create a texture
    unsigned int texture;
    time_t last_mod_time;
    struct stat file_stat;

    if (argc == 3)
    {
        load_texture(&texture, argv[2]);
        // Get file metadata
        if (stat(argv[2], &file_stat) == -1) {
            perror("stat");
            return -1; // Error accessing the file
        }
        last_mod_time = file_stat.st_mtime;
    }

    // Keep track of changes to the file



    // Get the location of the uniform variables
    GLint resolutionLocation = glGetUniformLocation(shaderProgram, "iResolution");
    GLint mouseLocation = glGetUniformLocation(shaderProgram, "iMouse");
    GLint timeLocation = glGetUniformLocation(shaderProgram, "iTime");
    GLint timeDeltaLocation = glGetUniformLocation(shaderProgram, "iTimeDelta");
    GLint batteryLevelLocation = glGetUniformLocation(shaderProgram, "iBatteryLevel");
    GLint localTimeLocation = glGetUniformLocation(shaderProgram, "iLocalTime");

    // Set up signal handling
    signal(SIGUSR1, pause_signal);
    signal(SIGUSR2, unpause_signal);

    // Time for frame limiter
    double frame_time = 0;
    double time_cyclic = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    unsigned int delta_us = 0;
    double delta = delta_us;
    float glfw_time = 0;

    // Some more miscellaneous vars for uniforms
    int battery_level = 0;
    float local_time = 0;

    int has_battery = 1;
    FILE *fp = fopen(battery_capacity_path, "r");
    if (fp == NULL) {
        perror("Failed to open battery capacity file");
        has_battery = 0;
    } else {
        fclose(fp);
    }

    // Render loop
    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        if (paused) {
            pause();
        }
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);

        // Get window size
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // Render
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the shader program
        glUseProgram(shaderProgram);

        // Update these variables once per second
        if (frame == 0) 
        {
            // Battery
            if (has_battery) {
                fp = fopen(battery_capacity_path, "r");
                if (fscanf(fp, "%d", &battery_level) != 1) {
                    perror("Failed to read battery level");
                    fclose(fp);
                    return -1;
                }
                battery_level /= 100.0;
                fclose(fp);
            }

            // Time
            time_t rawtime;
            struct tm *timeinfo;

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            // Calculate the fraction of the day
            int hours = timeinfo->tm_hour;
            int minutes = timeinfo->tm_min;
            int seconds = timeinfo->tm_sec;

            // Total seconds elapsed in the day
            int total_seconds = hours * 3600 + minutes * 60 + seconds;
            // Total seconds in a day
            int seconds_in_day = 24 * 3600;

            // Fraction of the day
            local_time = (float)total_seconds / seconds_in_day;

            // Texture
            if (REFRESH_TEXTURE && argc == 3 && has_file_changed(argv[2], &last_mod_time))
            {
                printf("Texture file changed, will reload\n");
                load_texture(&texture, argv[2]);
            }

        }

        // Keep GLFW time in a 60 second interval so it doesn't
        // cause any floating point precision error thingies
        // when the program is run for long periods of time
        glfw_time = glfwGetTime();
        if (glfw_time >= 60.0)
        {
            glfw_time = fmod(glfw_time, 60);
            glfwSetTime(glfw_time);
        }
        
        // Pass the window width and time to the shader
        glUniform3f(resolutionLocation, (float)width, (float)height, 1.0f);
        glUniform4f(mouseLocation, (float)mouseX, 1.0 - (float)mouseY, (float)0.0, (float)0.0);
        glUniform1f(timeLocation, (float)glfw_time);
        glUniform1f(timeDeltaLocation, (float)delta);
        glUniform1f(batteryLevelLocation, (float)battery_level);
        glUniform1f(localTimeLocation, (float)local_time);

        // Draw the rectangle
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Swap buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();

                clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;

        if (FRAME_LIMIT > 0)
        {
            if (delta_us < 1000000.0 / FRAME_LIMIT)
            {
                usleep(1000000.0 / FRAME_LIMIT - delta_us);
                delta = 1.0 / FRAME_LIMIT;
            }
            else
            {
                delta = delta_us/1000000.0;
            }
        }
        else
        {
            delta = delta_us/1000000.0;
        }
        frame = (frame + 1) % FRAME_LIMIT;
    }

    // Deallocate resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    // Terminate GLFW
    glfwTerminate();
    return 0;
}

