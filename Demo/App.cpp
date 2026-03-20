// Demo
//
// Note that this specific subproject is a minimal implementation to showcase the Octree.
// Further expansion would include use of file loaders, multiple header/source files, and proper management of gl functions and errors.

#include "App.h"
#include <array>
#include <Octree.h>
#include <random>
#include <string>
#include <Vec3.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

glm::vec3 toGLMVec3(const Vec3& vec) {
    return { vec.x, vec.y, vec.z };
}

Vec3 toVec3(const glm::vec3& vec) {
    return { vec.x, vec.y, vec.z };
}

void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
{
    auto const src_str = [source]() {
        switch (source)
        {
        case GL_DEBUG_SOURCE_API: return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
        case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
        case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
        case GL_DEBUG_SOURCE_OTHER: return "OTHER";
        }
        }();

    auto const type_str = [type]() {
        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR: return "ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
        case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
        case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
        case GL_DEBUG_TYPE_MARKER: return "MARKER";
        case GL_DEBUG_TYPE_OTHER: return "OTHER";
        }
        }();

    auto const severity_str = [severity]() {
        switch (severity) {
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
        case GL_DEBUG_SEVERITY_LOW: return "LOW";
        case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
        case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
        }
        }();
    std::cout << src_str << ", " << type_str << ", " << severity_str << ", " << id << ": " << message << '\n';
}

const char * vert_shader = "#version 450\n"
"layout (location = 0) uniform mat4 uMVP;\n"
"layout (location = 0) in vec3 vPos;\n"
"out gl_PerVertex {\n"
"   vec4 gl_Position;\n"
"};\n"
"void main()\n"
"{\n"
"   gl_Position = uMVP * vec4(vPos, 1.0);\n"
"}";

const char * frag_line_shader = "#version 450\n"
"layout (location = 0) uniform vec3 uColor = vec3(1.0);"
"out vec4 oColor;\n"
"void main()\n"
"{\n"
"   oColor = vec4(uColor, 1.0);\n"
"}";

const char * frag_point_shader = "#version 450\n"
"layout (location = 0) uniform vec3 uColor = vec3(1.0);"
"out vec4 oColor;\n"
"void main()\n"
"{\n"
"   oColor = vec4(uColor, 1.0);\n"
"}";

int width = 800, height = 600;
float octree_size = 50.0f;
Bounds octree_bounds{ {0.0f, 0.0f, 0.0f}, {octree_size, octree_size, octree_size} };
Octree octree{ octree_bounds };

GLuint p_cube;

GLuint v_shader, f_cube_shader, f_dot_shader;

glm::vec3 cube_vertices[] = {
    { -0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f},
    { -0.5f, -0.5f, 0.5f }, { 0.5f, -0.5f, 0.5f},
    { -0.5f, 0.5f, -0.5f }, { 0.5f, 0.5f, -0.5f},
    { -0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f, -0.5f}
};
// rendered as lines, not triangles.
unsigned cube_indices[] = {
    0, 1, 1, 3, 3, 2, 2, 0, // front
    5, 4, 4, 6, 6, 7, 7, 5, // back
    4, 0, 6, 2, 1, 5, 7, 3 // sides
};
GLuint vbo_cube, vio_cube, vao_cube;

const int POINT_COUNT = 500;
glm::vec3 points[POINT_COUNT] = {};
GLuint vbo_points, vao_points;

glm::vec3 line[] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
GLuint vbo_line, vao_line;

// euler angles in radians
glm::vec3 global_rotation{ 0.0f };
float global_scale = 1.0f;
glm::mat4 global_transform;
glm::mat4 view = glm::lookAt(glm::vec3{ 0.0f, 0.0f, -75.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
glm::mat4 projection = glm::perspective(45.0f, (float)width / (float)height, 0.001f, 1000.0f);

glm::vec3 ray_origin = glm::vec3{ 20.0f, 10.0f, -20.0f };
glm::vec3 ray_rotation = glm::vec3{ 20.0f, 310.0f, 0.0f };

void setup_gl()
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, nullptr);

    glViewport(0, 0, width, height);
   
    v_shader = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vert_shader);
    f_cube_shader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &frag_line_shader);

    glCreateProgramPipelines(1, &p_cube);
    glUseProgramStages(p_cube, GL_VERTEX_SHADER_BIT, v_shader);
    glUseProgramStages(p_cube, GL_FRAGMENT_SHADER_BIT, f_cube_shader);
    // glBindProgramPipeline(p_cube);

    glCreateBuffers(1, &vbo_cube);
    glNamedBufferStorage(vbo_cube, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_STORAGE_BIT);
    
    glCreateBuffers(1, &vio_cube);
    glNamedBufferStorage(vio_cube, sizeof(cube_indices), cube_indices, GL_DYNAMIC_STORAGE_BIT);

    glCreateVertexArrays(1, &vao_cube);
    glVertexArrayVertexBuffer(vao_cube, 0, vbo_cube, 0, sizeof(glm::vec3));
    glVertexArrayElementBuffer(vao_cube, vio_cube);
    glEnableVertexArrayAttrib(vao_cube, 0);
    glVertexArrayAttribFormat(vao_cube, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao_cube, 0, 0);

    glCreateBuffers(1, &vbo_points);
    glNamedBufferStorage(vbo_points, sizeof(points), points, GL_DYNAMIC_STORAGE_BIT);

    glCreateVertexArrays(1, &vao_points);
    glVertexArrayVertexBuffer(vao_points, 0, vbo_points, 0, sizeof(glm::vec3));
    glEnableVertexArrayAttrib(vao_points, 0);
    glVertexArrayAttribFormat(vao_points, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao_points, 0, 0);

    glCreateBuffers(1, &vbo_line);
    glNamedBufferStorage(vbo_line, sizeof(line), line, GL_DYNAMIC_STORAGE_BIT);

    glCreateVertexArrays(1, &vao_line);
    glVertexArrayVertexBuffer(vao_line, 0, vbo_line, 0, sizeof(glm::vec3));
    glEnableVertexArrayAttrib(vao_line, 0);
    glVertexArrayAttribFormat(vao_line, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao_line, 0, 0);

}

void setup_octree()
{
    std::random_device rand_device;
    std::default_random_engine rand_engine { rand_device()};
    std::uniform_real_distribution<float> rand_dist{ -octree_size / 2.0f, octree_size / 2.0f };

    for (int i = 0; i < POINT_COUNT; i++)
    {
        points[i] = glm::vec3{ rand_dist(rand_engine), rand_dist(rand_engine), rand_dist(rand_engine) };
        octree.add(toVec3(points[i]));
    }
    std::cout << "Generated " << POINT_COUNT << " points." << std::endl;
}

void draw_cube(glm::vec3 position, glm::vec3 scale, glm::vec3 color = glm::vec3{1.0f})
{
    glm::mat4 t = glm::translate(glm::mat4{ 1.0f }, position);
    glm::mat4 s = glm::scale(glm::mat4{ 1.0f }, scale);
    glm::mat4 m = global_transform * t * s;

    glm::mat4 mvp = projection * view * m;

    glBindProgramPipeline(p_cube);
    glLineWidth(1.0f);
    glBindVertexArray(vao_cube);

    glProgramUniformMatrix4fv(v_shader, 0, 1, GL_FALSE, glm::value_ptr(mvp));
    glProgramUniform3fv(f_cube_shader, 0, 1, glm::value_ptr(color));
    glDrawElements(GL_LINES, sizeof(cube_indices) / sizeof(unsigned), GL_UNSIGNED_INT, 0);

    glBindProgramPipeline(0);
}

void draw_octant(const Octree* octant)
{
    Bounds bounds = octant->get_bounds();
    glm::vec3 pos = toGLMVec3(bounds.center);
    glm::vec3 size = toGLMVec3(bounds.size);
    glm::vec3 base_color = glm::vec3{ 0.9f };
    float color_mult = 0.1f;
    draw_cube(pos, size, base_color - octant->get_depth_level() * color_mult);

    //glBindProgramPipeline(p_cube);
    //glPointSize(1.0f);

    //glLineWidth(1.0f);
    //glBindVertexArray(vao_cube);
    //glDrawElements(GL_LINES, sizeof(cube_indices) / sizeof(unsigned), GL_UNSIGNED_INT, 0);
    //glBindProgramPipeline(0);
}

void draw_points()
{
    glm::mat4 t = glm::translate(glm::mat4{ 1.0f }, glm::vec3{0.0f});
    glm::mat4 mvp = projection * view * global_transform * t;

    glm::vec3 color{ 1.0f};
    glBindProgramPipeline(p_cube);

    glPointSize(2.0f);
    glBindVertexArray(vao_points);

    glProgramUniformMatrix4fv(v_shader, 0, 1, GL_FALSE, glm::value_ptr(mvp));
    glProgramUniform3fv(f_cube_shader, 0, 1, glm::value_ptr(color));
    glDrawArrays(GL_POINTS, 0, POINT_COUNT);

    glBindProgramPipeline(0);
}

void draw_ray()
{
    glm::mat4 t = glm::translate(glm::mat4{ 1.0f }, ray_origin);
    glm::mat4 r = glm::mat4_cast(glm::quat{ glm::radians(ray_rotation) });
    glm::mat4 s = glm::scale(glm::mat4{ 1.0f }, glm::vec3{ 1000.0f });
    glm::mat4 mvp = projection * view * global_transform * t * r * s;

    glm::vec3 color{ 1.0f, 0.0f, 0.0f };
    glBindProgramPipeline(p_cube);

    glLineWidth(2.0f);
    glBindVertexArray(vao_line);

    glProgramUniformMatrix4fv(v_shader, 0, 1, GL_FALSE, glm::value_ptr(mvp));
    glProgramUniform3fv(f_cube_shader, 0, 1, glm::value_ptr(color));
    glDrawArrays(GL_LINES, 0, 2);

    glBindProgramPipeline(0);
}

void run()
{
    glm::mat4 g_r = glm::mat4_cast(glm::quat{ glm::radians(global_rotation) });
    glm::mat4 g_s = glm::scale(glm::mat4{ 1.0f }, glm::vec3{ global_scale });
    global_transform = g_r * g_s;

    for (auto octant : octree)
    {
        draw_octant(octant);
    }
    draw_points();
    draw_ray();

    ImGui::Begin("Settings");
    ImGui::SliderFloat3("Rotation", glm::value_ptr(global_rotation), 0.0f, 360.0f);
    ImGui::SliderFloat("Scale", &global_scale, 0.25f, 2.0f);

    if (ImGui::TreeNode("Ray"))
    {
        ImGui::InputFloat3("Origin", glm::value_ptr(ray_origin));
        ImGui::SliderFloat3("Rotation", glm::value_ptr(ray_rotation), 0.0f, 360.0f);

        ImGui::TreePop();
    }

    // ImGui::InputFloat3("Ray Origin")
    ImGui::End();

}

void unload()
{

}


int main()
{
    //std::random_device rand_device;
    //std::default_random_engine rand_engine { rand_device()};
    //std::uniform_real_distribution<float> rand_dist{ -50.0f, 50.0f };
    //
    //std::array<Vec3, 5000> points;
    //std::uniform_int_distribution<> rand_int { 0, points.size() - 1};

    //Bounds b{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    //Octree octree{ b, 10, 5 };
    //for (int i = 0; i < points.size(); i++)
    //{
    //    points[i] = Vec3{ rand_dist(rand_engine), rand_dist(rand_engine), rand_dist(rand_engine) };
    //    octree.add(points[i]);
    //}
    //std::cout << "Generated " << points.size() << " points." << std::endl;

    //std::cout << static_cast<std::string>(octree) << std::endl;

    //bool has_point = octree.has(points[rand_int(rand_engine)]);
    //std::cout << ((has_point) ? "Found point" : "Does not contain point") << std::endl;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(width, height, "Octree Demo", nullptr, nullptr);
    if (!window)
    {
        const char* error;
        glfwGetError(&error);
        std::cout << error << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    
    setup_octree();
    setup_gl();


    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.3f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        run();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    unload();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}
