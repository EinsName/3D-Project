#include "playground.h"

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

#include <vector>


// Include GLM
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>

float scale_factor = 1.0f;
int mode = 0; // for camera moving mode or interaction mode

glm::vec3 cameraPos = glm::vec3(4, 3, -3);
glm::vec3 cameraTarget = glm::vec3(0, 0, 0);
glm::vec3 cameraUp = glm::vec3(0, 1, 0);

float rotationX = 0.0f;
float rotationY = 0.0f;
float rotationZ = 0.0f;


int main(void)
{
    //Initialize window
    bool windowInitialized = initializeWindow();
    if (!windowInitialized) return -1;

    //Initialize vertex buffer
    bool vertexbufferInitialized = initializeVertexbuffer();
    if (!vertexbufferInitialized) return -1;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

    initializeMVPTransformation();

    curr_x = 0;
    curr_y = 0;

    

    

    //start animation loop until escape key is pressed
    do {

        updateAnimationLoop();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);


    //Cleanup and close window
    cleanupVertexbuffer();
    glDeleteProgram(programID);
    closeWindow();

    return 0;
}

void updateAnimationLoop()

{
    
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(programID);

    if (mode == 0) {    //if in interaction mode

        //movement from example
        if (glfwGetKey(window, GLFW_KEY_W)) curr_y += 0.01;
        else if (glfwGetKey(window, GLFW_KEY_S)) curr_y -= 0.01;
        else if (glfwGetKey(window, GLFW_KEY_A)) curr_x -= 0.01;
        else if (glfwGetKey(window, GLFW_KEY_D)) curr_x += 0.01;
        //
        
        //rotation around the 3 different axes
        if (glfwGetKey(window, GLFW_KEY_X)) rotationX += 0.1;
        if (glfwGetKey(window, GLFW_KEY_Y)) rotationY += 0.1;
        if (glfwGetKey(window, GLFW_KEY_Z)) rotationZ += 0.1;


        if (scale_factor >= 3.0) {   //restriction on how much we can sclae
            if (glfwGetKey(window, GLFW_KEY_O)) scale_factor -= 0.01f;   
        }
        else if (scale_factor <= 0.3) {
            if (glfwGetKey(window, GLFW_KEY_L)) scale_factor += 0.01f;
        }
        else {
            if (glfwGetKey(window, GLFW_KEY_L)) scale_factor += 0.01f;  //larger
            if (glfwGetKey(window, GLFW_KEY_O)) scale_factor -= 0.01f;  //smaller
        }

        if (glfwGetKey(window, GLFW_KEY_T)) scale_factor = 1.0f;
        if (glfwGetKey(window, GLFW_KEY_M)) mode = 1;

        initializeMVPTransformation();
    }
    else if (mode == 1) {   //if in kameramovement mode
        //for moving the camera, it is not realy intuitve
        if (glfwGetKey(window, GLFW_KEY_W)) cameraPos.z += 0.01f;
        if (glfwGetKey(window, GLFW_KEY_S)) cameraPos.z -= 0.01f;
        if (glfwGetKey(window, GLFW_KEY_A)) cameraPos.x += 0.01f;
        if (glfwGetKey(window, GLFW_KEY_D)) cameraPos.x -= 0.01f;

        if (glfwGetKey(window, GLFW_KEY_M)) mode = 0;

        initializeMVPTransformation();
    }

    // Send our transformation to the currently bound shader, 
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    //colorbuffer config
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glVertexAttribPointer(
        1,                                // attribute. No particular reason for 0, but must match the layout in the shader.
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, vertexbuffer_size * 3); // 3 indices starting at 0 -> 1 triangle

    glDisableVertexAttribArray(0);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

bool initializeWindow()
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Demo: Cube", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return false;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // white background
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    return true;
}

bool initializeMVPTransformation()
{
    // Get a handle for our "MVP" uniform
    GLuint MatrixIDnew = glGetUniformLocation(programID, "MVP");
    MatrixID = MatrixIDnew;


    // Projection matrix : 90  Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(90.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    //glm::mat4 Projection = glm::frustum(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);
    // Camera matrix
    glm::mat4 View = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor, scale_factor, scale_factor)); //Matrix for scaling

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);

    Model = glm::rotate(Model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f)); //rotation x-axis
    Model = glm::rotate(Model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f)); //rotation y-axis
    Model = glm::rotate(Model, glm::radians(rotationZ), glm::vec3(1.0f, 0.0f, 1.0f)); //rotation z-axis

    Model = glm::rotate(Model, curr_angle, glm::vec3(0.0f, 1.0f, 1.0f)) * scale_matrix; //apply scale matrix

    glm::mat4 transformation;//additional transformation for the model
    transformation[0][0] = 1.0; transformation[1][0] = 0.0; transformation[2][0] = 0.0; transformation[3][0] = curr_x;
    transformation[0][1] = 0.0; transformation[1][1] = 1.0; transformation[2][1] = 0.0; transformation[3][1] = curr_y;
    transformation[0][2] = 0.0; transformation[1][2] = 0.0; transformation[2][2] = 1.0; transformation[3][2] = 0.0;
    transformation[0][3] = 0.0; transformation[1][3] = 0.0; transformation[2][3] = 0.0; transformation[3][3] = 1.0;

    // Our ModelViewProjection : multiplication of our 3 matrices
    MVP = Projection * View * Model * transformation; // Remember, matrix multiplication is the other way around


    return true;

}

bool initializeVertexbuffer()
{
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    vertexbuffer_size = 12;
    static const GLfloat g_vertex_buffer_data[] = {
      -1.0f,-1.0f,-1.0f, // triangle 1 : begin
      -1.0f,-1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f, // triangle 1 : end
      1.0f, 1.0f,-1.0f, // triangle 2 : begin
      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f,-1.0f, // triangle 2 : end
      1.0f,-1.0f, 1.0f,
      -1.0f,-1.0f,-1.0f,
      1.0f,-1.0f,-1.0f,
      1.0f, 1.0f,-1.0f,
      1.0f,-1.0f,-1.0f,
      -1.0f,-1.0f,-1.0f,
      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f,-1.0f,
      1.0f,-1.0f, 1.0f,
      -1.0f,-1.0f, 1.0f,
      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f,-1.0f, 1.0f,
      1.0f,-1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f,-1.0f,-1.0f,
      1.0f, 1.0f,-1.0f,
      1.0f,-1.0f,-1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f,-1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f,-1.0f,
      -1.0f, 1.0f,-1.0f,
      1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f,-1.0f,
      -1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f,
      1.0f,-1.0f, 1.0f
    };

    //attribute for colors
    static const GLfloat g_color_buffer_data[] = {
    0.583f,  0.771f,  0.014f,
    0.609f,  0.115f,  0.436f,
    0.327f,  0.483f,  0.844f,
    0.822f,  0.569f,  0.201f,
    0.435f,  0.602f,  0.223f,
    0.310f,  0.747f,  0.185f,
    0.597f,  0.770f,  0.761f,
    0.559f,  0.436f,  0.730f,
    0.359f,  0.583f,  0.152f,
    0.483f,  0.596f,  0.789f,
    0.559f,  0.861f,  0.639f,
    0.195f,  0.548f,  0.859f,
    0.014f,  0.184f,  0.576f,
    0.771f,  0.328f,  0.970f,
    0.406f,  0.615f,  0.116f,
    0.676f,  0.977f,  0.133f,
    0.971f,  0.572f,  0.833f,
    0.140f,  0.616f,  0.489f,
    0.997f,  0.513f,  0.064f,
    0.945f,  0.719f,  0.592f,
    0.543f,  0.021f,  0.978f,
    0.279f,  0.317f,  0.505f,
    0.167f,  0.620f,  0.077f,
    0.347f,  0.857f,  0.137f,
    0.055f,  0.953f,  0.042f,
    0.714f,  0.505f,  0.345f,
    0.783f,  0.290f,  0.734f,
    0.722f,  0.645f,  0.174f,
    0.302f,  0.455f,  0.848f,
    0.225f,  0.587f,  0.040f,
    0.517f,  0.713f,  0.338f,
    0.053f,  0.959f,  0.120f,
    0.393f,  0.621f,  0.362f,
    0.673f,  0.211f,  0.457f,
    0.820f,  0.883f,  0.371f,
    0.982f,  0.099f,  0.879f
    };

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    //bind colorbuffer
    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

    return true;
}



bool cleanupVertexbuffer()
{
    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    return true;
}

bool closeWindow()
{
    glfwTerminate();
    return true;
}

