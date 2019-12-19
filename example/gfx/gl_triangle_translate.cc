#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>

#include "math/mat.h"

const char* vertex_shader = R"(
  #version 410
  layout (location = 0) in vec3 vertex_position;
  layout (location = 1) in vec3 vertex_color;
  uniform mat4 matrix;
  out vec3 color; 
  void main() {
    color = vertex_color; 
    gl_Position = matrix * vec4(vertex_position, 1.0);
  }
)";

const char* fragment_shader = R"(
  #version 410
  in vec3 color;
	out vec4 frag_color;
  void main() {
   frag_color = vec4(color, 1.0);
  }
)";

int
main()
{
  if (!glfwInit()) {
    std::cerr << "Cound not start GLFW3" << std::endl;
    return 1;
  }
  // Only for mac I need this?
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(640, 480, "Hello Triangle", NULL, NULL);
  if (!window) {
    std::cout << "Failed to open window with GLFW3" << std::endl;
    return 1;
  }
  glfwMakeContextCurrent(window);  // glad must be called after this.
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize OpenGL context" << std::endl;
    return 1;
  }
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  std::cout << renderer << std::endl;
  std::cout << version << std::endl;
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glEnable(GL_CULL_FACE);  // cull face
  glCullFace(GL_BACK);     // cull back face
  glFrontFace(GL_CW);      // GL_CCW for counter clock-wise

  GLfloat points[] = {
      0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f,
  };
  GLfloat colors[] = {
      1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
  };
  math::Mat4f matrix{
      1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
  };

  // Create points VBO.
  GLuint points_vbo = 0;
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), points, GL_STATIC_DRAW);

  // Create colors VBO.
  GLuint colors_vbo = 0;
  glGenBuffers(1, &colors_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  // This needs to be done for each bound vao
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  // Create and compile vertex shader.
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  glCompileShader(vs);
  // Create and compile fragment shader.
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  glCompileShader(fs);
  // Link them together.
  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, fs);
  glAttachShader(shader_program, vs);
  glLinkProgram(shader_program);

  int matrix_location = -1;
  matrix_location = glGetUniformLocation(shader_program, "matrix");
  std::cout << "matrix_location: " << matrix_location << std::endl;

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Set the shader program.
    glUseProgram(shader_program);
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, &matrix[0]);
    // Bind the vertex array object.
    glBindVertexArray(vao);
    // glPolygonMode(GL_FRONT, GL_LINE);
    // Draw them points.
    glDrawArrays(GL_TRIANGLES, 0, 3);  // Draws the triangle
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwTerminate();
  return 0;
}
