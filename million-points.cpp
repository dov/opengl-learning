//======================================================================
//
//  Test generating one hundred million points and showing them as circles
//  through the geometry shader.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//
//  License: MIT
// 
//  2023-12-08 Fri
//----------------------------------------------------------------------

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "linmath.h"
#include <fmt/core.h> 
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <string>
#include <unistd.h>
#include <random>
#include <glm/vec2.hpp>

using namespace std;

vector<glm::vec2> vertices;

static const struct
{
  float x, y;
} xvertices[3] =
  {
    { -0.6f, -0.4f },
    {  0.6f, -0.4f },
    {   0.f,  0.6f }
  };

int gWidth = 640;
int gHeight = 480;
bool need_redraw = true;

static const char* vertex_shader_text =
  "#version 430 \n"
  "layout (location = 0) in vec2 vPos;\n"
  "uniform mat4 MVP;\n"
  "void main()\n"
  "{\n"
  "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
  "}\n";

// A geometry shader that turns points into circles
static const char* geom_shader_text =
  "#version 430\n"
  "layout(points) in;\n"
  "layout(triangle_strip, max_vertices=96) out;\n"
  "uniform ivec2 Resolution;\n"
  "#define M_PI 3.1415926535897932384626433832795\n"
  "void main()\n"
  "{\n"
  "  float d = 0.01; // 0.01;\n"
  "  int n=8; // octogons\n" 
  "  float xs = 1.0*Resolution.y/Resolution.x;\n"
  "  for (int i=0; i<gl_in.length(); ++i)\n"
  "  {\n"
  "    for (int j=0; j<n+2; j++)\n"
  "    {\n"
  "      if (j%2==0)\n"
  "      {\n"
  "        gl_Position = gl_in[i].gl_Position;\n"
  "        EmitVertex();\n"
  "      }\n"
  "      float theta = j*(2*M_PI/n);\n"
  "      float st = sin(theta);\n"
  "      float ct = cos(theta);\n"
  "      gl_Position = gl_in[i].gl_Position + vec4(d*ct*xs, d*st, 0,0);\n"
  "      EmitVertex();\n"
  "    }\n"
  "    EndPrimitive();\n"
  "  }\n"
  "}\n";

static const char* fragment_shader_text =
  "#version 430\n"
  "out vec4 fragColor;\n"
  "void main()\n"
  "{\n"
  "    vec3 color = vec3(1,0,0);\n"
  "    fragColor = vec4(color, 0.05);\n"
  "}\n";


static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}
 
static void window_size_callback(GLFWwindow* window, int width, int height)
{
  gWidth = width;
  gHeight = height;
  need_redraw=true;
}

void window_refresh_callback(GLFWwindow* window)
{
  glfwSwapBuffers(window);
  need_redraw=true;
}

static void die(const char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt); 
    
  vfprintf(stderr, fmt, ap);
  exit(-1);
}

GLuint compile_shader(GLuint shader_type, const string& source)
{
  GLuint shader = glCreateShader(shader_type);

  GLchar const* files[] = { source.c_str() };
  GLint lengths[] = { (int)source.size() };

  glShaderSource(shader, 1, files, lengths);
  glCompileShader(shader);

  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

  if(isCompiled == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
  
    // The maxLength includes the NULL character
    string shader_type_str;
    switch(shader_type) {
    case GL_VERTEX_SHADER:
      shader_type_str = "vertex";
      break;
    case GL_FRAGMENT_SHADER:
      shader_type_str = "fragment";
      break;
    default:
      shader_type_str = "unknown type";
      break;
    }
  	string errorLog(maxLength, ' ');
  	glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
    //    print("Failed compilation of {} shader: {}\n", shader_type_str, errorLog);
    die("%s: %s\n", shader_type_str.c_str(), errorLog.c_str());
  }
  
  return shader;
}

double urand(void)
{
  return 1.0*rand()/RAND_MAX;
}

void generate_vertices()
{
  default_random_engine generator;
  double middle = 0.5;
  double std = 0.08;
  normal_distribution<double> distribution(middle,std);

  printf("Generating points\n");
  int n = 100000000;
  vertices.resize(n);
  for (int i=0; i<(int)vertices.size(); i++)
  {
    if (i%1000==0)
      printf("%d\r",i);
    double r = distribution(generator);
    double theta = 2*M_PI*urand();
    double x = r*cos(theta);
    double y = r*sin(theta);
      
    vertices[i] ={x,y};
  }

  printf("Done generating %d points\n", vertices.size());
}

int main(void)
{
  GLFWwindow* window;
  GLuint vertex_buffer, vertex_shader, geom_shader, fragment_shader, program;
 
  generate_vertices();

  glfwSetErrorCallback(error_callback);
 
  if (!glfwInit())
    exit(EXIT_FAILURE);
 
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
 
  window = glfwCreateWindow(1024, 768, "One Hundred Million Points", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
 
  glfwSetKeyCallback(window, key_callback);
  glfwSetWindowSizeCallback(window, window_size_callback);
  glfwSetWindowRefreshCallback(window, window_refresh_callback); 
  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  glfwSwapInterval(1);
 
  // NOTE: OpenGL error checks have been omitted for brevity
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);
 
  vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_text);
  geom_shader = compile_shader(GL_GEOMETRY_SHADER, geom_shader_text);
  fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_text);
 
  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, geom_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
 
  GLint mvp_location = glGetUniformLocation(program, "MVP");
  GLint vpos_location = glGetAttribLocation(program, "vPos");
  GLint res_location = glGetUniformLocation(program, "Resolution");
 
  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void*) 0);

  int frameCount = 0;
  double previousTime = glfwGetTime();

  while (!glfwWindowShouldClose(window))
  {
    float ratio;
    int width, height;
    mat4x4 m, p, mvp;

    if (need_redraw)
    {
      glfwGetFramebufferSize(window, &width, &height);
      ratio = width / (float) height;
   
      glViewport(0, 0, width, height);
      glClear(GL_COLOR_BUFFER_BIT);
   
      mat4x4_identity(m);
      mat4x4_rotate_Z(m, m, (float) glfwGetTime());
      mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
      mat4x4_mul(mvp, p, m);
   
      glUseProgram(program);
      glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
      glUniform2i(res_location, gWidth, gHeight);
      glDrawArrays(GL_POINTS, 0, vertices.size());
   
      glfwSwapBuffers(window);
  
      double currentTime = glfwGetTime();
      if ( currentTime - previousTime >= 1.0 )
      {
        printf("FPS=%d\r", frameCount);
        fflush(stdout);
        frameCount = 0;
        previousTime = currentTime;
      }
      frameCount++;
      //      need_redraw = false;
      need_redraw= true;
    }
    else
      usleep(1000);
    glfwPollEvents();

  }
 
  glfwDestroyWindow(window);
 
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
 
