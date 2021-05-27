#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>

 
//method declaration

// framebuffer size function takes a GLFWwindow as its first argument and two integers indicating the new window dimensions. 
// Whenever the window changes size, GLFW calls this function and fills in the proper arguments for you to process
//pass in 3 parameters: a pointer to your GLFWwindow, width, and height 
void framebuffer_size_callback(GLFWwindow* window, int width, int height); 

//method to process input
void processInput(GLFWwindow* window);

std::string  loadShaderSrc(const char* filename); //this method will be used to load the source code of our glsl shaders 


int main() {

    int success; //success of the compilation of shader code 
    char infolog[512]; //to store error message if there is one 

    //glm test (test the translation, scale, and rotation features of the glm libraries)

    glm::vec4 vec(1.0f, 1.0f, 1.0f, 1.0f); //original vector with homogeneous coordinate, w, set to 1
    glm::mat4 trans = glm::mat4(1.0f); //4x4 identity matrix
    //trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f)); // matrix that translates by (1,1,0)
    trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0)); //Rotation matrix. pass in the angle of rotation and the vector to act upon
    trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5)); //translation matrix to scale by (0.5,0.5,0.5)
    vec = trans * vec; //translate the original vector by multiplying it by the transformation matrix
    std::cout << vec.x  << ' ' << vec.y << ' ' << vec.z << ' ' << std::endl; //print out coordinates of transformed matrix

    glfwInit(); //initializes glfw

    // openGL version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //pass hints to the window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 

    //use core profile for GLFW
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //initialize window object 

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL tutorial", NULL, NULL); //create a pointer to our window, specify width, height, title, monitor(null)
     
    //check to see if window was created by checking if the pointer is null
    if (window == NULL) { //window not created
        std::cout << "could not create window." << std::endl;
        glfwTerminate(); //this function terminates glfw
        return -1; 
    }

    //set the focus of GLFW to our window
    glfwMakeContextCurrent(window); 

    //glad code 

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { //Glad didn't initialize
    std::cout << "Failed to initialize GLAD" << std::endl; 
    glfwTerminate();
    return -1;
}

    //set viewport. Tell OpenGL how to display/ how to render data
    // processed coordinates in OpenGL are between -1 and 1, so we effectively map from the range (-1,1) to our window's coordinates, for example (0,800) and (0,600)
    
    glViewport(0, 0, 800, 600); //set the position and dimensions of the window, (x position of lower left corner y position of lower left corner, window width,window height) 

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 


    /*
           shaders
    */

    //compile vertex shader
    unsigned int vertexShader; //vertex shader object

    vertexShader = glCreateShader(GL_VERTEX_SHADER); //create a shader object named vertexShader of type GL Vertex Shader
    std::string vertShaderSrc = loadShaderSrc("assets/vertex_core.glsl"); //get the vertex shader source code as a string
    const GLchar* vertShader = vertShaderSrc.c_str(); //convert string to const char*
    glShaderSource(vertexShader, 1, &vertShader, NULL); //set the source in the shader object.(current shader, count, reference to vertShader which holds the source code, length=NULL)
    glCompileShader(vertexShader); //compile the shader 

    //catch error
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); //tells openGL to get the shader value GL_COMPILE_STATUS to tell us if it was successful 
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infolog);
        std::cout << "Error with vertex shader compilation. : " << std::endl << infolog << std::endl; 
    }

    //compile fragment shader
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragShaderSrc = loadShaderSrc("assets/fragment_core.glsl");
    const GLchar* fragShader = fragShaderSrc.c_str();
    glShaderSource(fragmentShader, 1, &fragShader, NULL);
    glCompileShader(fragmentShader); 

    //catch error
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success); //tells openGL to get the shader value GL_COMPILE_STATUS to tell us if it was successful 
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
        std::cout << "Error with fragment shader compilation. : " << std::endl << infolog << std::endl; 
    }

    //create shader program to link the vertex shader with the fragment shader 
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram); 

    //catch errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infolog); 
        std::cout << "Linking error: " << std::endl << infolog << std::endl; 
    }

    //we have compiled and linked our shaders into a program. We don't need them anymore so we can delete them. 
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //vertex array
    float vertices[] = { //vertex data for a square made of 2 triangles

        //first triangle
        0.5f, 0.5f, 0.0f, //top right 
        -0.5f, 0.5f, 0.0f, //top left
        -0.5f, -0.5f, 0.0f, //bottom left

        //second triangle
        -0.5f, -0.5f, 0.0f, //bottom left
        0.5f, -0.5f, 0.0f, //bottom right
        0.5f, 0.5f, 0.0f //top right 

    };

    //VAO, VBO

    unsigned int VAO, VBO; 
    glGenVertexArrays(1, &VAO); //Vertex array object (VAO), contains pointers to the vertex buffers
    glGenBuffers(1, &VBO); //Vertex buffer object (VBO), buffer containining vertex data 

    //bind VAO
    glBindVertexArray(VAO); // passes in an integer and openGL will know which array object is the active one

    //bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //static draw teells openGL that we won't be modifying this data much

    //set attribute pointer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // (index, number of values per vertex, type of data, bool normalized, size,offset)
    glEnableVertexAttribArray(0); //index=0 







    //while loop that runs the program
    while (!glfwWindowShouldClose(window)) {
        //process input
        processInput(window);

        //render

         //clear the entire window and set color to specified color in RGBA format
        glClearColor(0.1f, 0.3f, 0.3f, 1.0f); //background color 
        glClear(GL_COLOR_BUFFER_BIT); 


        // draw shapes

        glBindVertexArray(VAO); //openGL now knows which vertex array object to look at, and as a result knows which vertex buffer data to look at 
        glUseProgram(shaderProgram); //use the shader
        glDrawArrays(GL_TRIANGLES, 0, 6); //(first index, number of vertices)


        // send new frame to window
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

//defines the method framebuffer_size_callback 
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height); //reset the glViewport every time it is resized 
}

//defines the method processInput

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { //test if escape key is pressed down
        glfwSetWindowShouldClose(window, true); //this will end the while loop, closing the window if the escape key is pressed 
    }
}


//pass in a file, read the file 
std::string loadShaderSrc(const char* filename) {
    std::ifstream file; 
    std::stringstream buf; //buffer to read the data into

    std::string ret = ""; //placeholder for the return variable 
    
    file.open(filename); //open the shader code file

    if (file.is_open()) { //if the file is open, read the file into the buffer
        buf << file.rdbuf(); //pipe readbuffer into stringbuffer 
        ret = buf.str(); 
    }
    else {
        std::cout << "Could not open " << filename << std::endl; 
    }

    file.close(); 

    return ret; 
}