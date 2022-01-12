#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>

#define LOG(x) std::cout << x << std::endl

using namespace std;

struct ShaderProgramSource {
    string vertexSource;
    string fragmentSource;
};

struct BufferData {
    int id;
    int indices_size;
};

static BufferData buffer;

struct Point {
    float x,y;
};

static ShaderProgramSource parseShader(const string& file_path ) {
    enum ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };
    ifstream stream(file_path);
    if(!stream){
        cout << "Can't open file" << endl;
        exit(EXIT_FAILURE);
    }
    string line;
    stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while(getline(stream,line)) {
        if(line.find("#shader") != string::npos) {
            if(line.find("vertex") != string::npos) {
                type = ShaderType::VERTEX;
            } else if (line.find("fragment") != string::npos){
                type = ShaderType::FRAGMENT;
            }
        } else {
            ss[(int)type] << line << endl;
        }
    }
    return { ss[VERTEX].str(),ss[FRAGMENT].str() };
}

static unsigned int compileShader(unsigned int type, const string& source ) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id,1,&src, nullptr);
    glCompileShader(id);
    int result;
    glGetShaderiv(id,GL_COMPILE_STATUS,&result);
    if(!result){
        int length;
        glGetShaderiv(id,GL_INFO_LOG_LENGTH,&length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id,length,&length,message);
        cout << "ERROR: Failed to compile" << (type == GL_VERTEX_SHADER ? "shader":"vertex") << " shader !";
        cout << message << endl;
    }
    return id;
}

static unsigned int createShader(const string& vertexShader, const string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

static void courbeVonKoch(int n,  Point depart, Point arrivee, vector<float> *vertices){
    if(n == 0) {
        vertices->push_back(depart.x);
        vertices->push_back(depart.y);
        vertices->push_back(arrivee.x);
        vertices->push_back(arrivee.y);
    } else {
        Point p1, p2, p3;

        float longueurX = ( arrivee.x - depart.x ) / 3.0;
        float longueurY = ( arrivee.y - depart.y ) / 3.0;

        p1.x =  depart.x + longueurX;
        p1.y =  depart.y + longueurY;

        p2.x =  depart.x + 1.5*longueurX - 0.8660254* longueurY;
        p2.y =  depart.y + 0.8660254* longueurX + 1.5*longueurY;

        p3.x =  depart.x + 2.0*longueurX;
        p3.y =  depart.y + 2.0*longueurY;

        courbeVonKoch( n - 1, depart, p1, vertices );
        courbeVonKoch( n- 1, p1, p2, vertices );
        courbeVonKoch( n - 1, p2, p3, vertices );
        courbeVonKoch( n - 1, p3, arrivee, vertices );
    };
}

static BufferData computeAndLoadShader(int n) {
        vector<float> vertices = {};
        Point p1 = {-0.9f,0.5f};
        Point p2 = {0.9f,0.5f};
        Point p3 = {0.0,-0.9};
        courbeVonKoch(n,p1,p2, &vertices);
        courbeVonKoch(n,p2,p3, &vertices);
        courbeVonKoch(n,p3,p1, &vertices);

        unsigned int buffer;
        glGenBuffers(1,&buffer);
        glBindBuffer(GL_ARRAY_BUFFER,buffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),vertices.data(),GL_STATIC_DRAW); // <-- DYNAMIC DRAW IN THE FUTURE ?
        vector<unsigned int> indices = {};
        for(int i=0; i < (vertices.size() / 2) - 1;i++) {
            indices.push_back(i);
            indices.push_back(i+1);
        }

        unsigned int ibo;
        glGenBuffers(1,&ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(float) * 2,0);

        ShaderProgramSource source = parseShader("basic.shader");
        int program = createShader(source.vertexSource,source.fragmentSource);
        glUseProgram(program);
        LOG("Render " << indices.size() << " indices !");
        return { id: program, indices_size: (int)indices.size()};
};

static float number_of_iteration = 0;

static bool exit_program = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
        buffer = computeAndLoadShader(--number_of_iteration);
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        buffer = computeAndLoadShader(++number_of_iteration);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        exit_program = true;
}


int main(void)
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(600, 600, "Fractal", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);
    gladLoadGL(glfwGetProcAddress);
    buffer = computeAndLoadShader(0);
    int color = glGetUniformLocation(buffer.id,"u_color");
    glUniform4f(color,0.2f,0.3f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    float b = 1.0f;
    float inc = -0.01f;
    while (!glfwWindowShouldClose(window) && !exit_program)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glUniform4f(color,0.0f,0.0f,b,1.0f);
        b += inc;

        glDrawElements(GL_LINE_STRIP,buffer.indices_size,GL_UNSIGNED_INT, nullptr);
        if(b >= 1.0f || b <= 0.6f)
            inc = -inc;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteProgram(buffer.id);
    glfwTerminate();
    return 0;
}