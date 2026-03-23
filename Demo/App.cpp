// Demo
//
// Note that this specific subproject is a minimal implementation to showcase the Octree.
// Further expansion would include use of file loaders, multiple header/source files, and proper management of gl functions and errors.

#include <iostream>
#include <Octree.h>
#include <random>
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


const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;

// -- Helper Functions

glm::vec3 toGLMVec3(const Vec3& vec) {
    return { vec.x, vec.y, vec.z };
}

Vec3 toVec3(const glm::vec3& vec) {
    return { vec.x, vec.y, vec.z };
}

// -- GLSL Shader code

const char* vert_cube_shader = "#version 450\n"
"layout (location = 0) uniform mat4 uVP;\n"
"layout (location = 0) in vec3 vPos;\n"
"layout (location = 1) in vec3 iPos;\n"
"layout (location = 2) in vec3 iScale;\n"
"layout (location = 3) in vec3 iColor;\n"
"out gl_PerVertex {\n"
"   vec4 gl_Position;\n"
"};\n"
"out vec3 fColor;\n"
"void main()\n"
"{\n"
"   fColor = iColor;\n"
"   vec3 pos = (vPos * iScale) + iPos;\n"
"   gl_Position = uVP * vec4(pos, 1.0);\n"
"}";

const char * vert_line_shader = "#version 450\n"
"layout (location = 0) uniform mat4 uMVP;\n"
"layout (location = 1) uniform vec3 uColor;\n"
"layout (location = 0) in vec3 vPos;\n"
"out gl_PerVertex {\n"
"   vec4 gl_Position;\n"
"};\n"
"out vec3 fColor;\n"
"void main()\n"
"{\n"
"   fColor = uColor;\n"
"   gl_Position = uMVP * vec4(vPos, 1.0);\n"
"}";

const char * frag_line_shader = "#version 450\n"
"in vec3 fColor;\n"
"out vec4 oColor;\n"
"void main()\n"
"{\n"
"   oColor = vec4(fColor, 1.0);\n"
"}";

const char* vert_point_shader = "#version 450\n"
"layout (location = 0) uniform mat4 uMVP;\n"
"layout (location = 1) uniform vec3 uCameraPos;\n"
"layout (location = 0) in vec3 vPos;\n"
"layout (location = 1) in vec3 vColor;\n"
"out gl_PerVertex {\n"
"   vec4 gl_Position;\n"
"   float gl_PointSize;\n"
"};\n"
"out vec3 fColor;\n"
"void main()\n"
"{\n"
"   fColor = vColor;\n"
"   float dst = distance(uCameraPos, vPos) / 2;"
"   gl_PointSize = clamp(20 - dst, 2, 10);" // scale the point a bit by distance, it gets hard to see the point otherwise
"   gl_Position = uMVP * vec4(vPos, 1.0);\n"
"}";

const char * frag_point_shader = "#version 450\n"
"in vec3 fColor;\n"
"out vec4 oColor;\n"
"void main()\n"
"{\n"
"   oColor = vec4(fColor, 1.0);\n"
"}";

// -- Octree

struct OctreeSettings
{
    float size = 50.0f;
    unsigned min_capacity = 15;
    unsigned max_depth = 5; 
    int point_count = 500;
} current_octree_settings, edit_octree_settings;
Octree<int> octree;

// -- Octree / Ray GL Objects

GLuint pipeline_cube, pipeline_line, pipeline_dot;
GLuint v_cube_shader, v_line_shader, f_line_shader, v_dot_shader, f_dot_shader;

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
GLuint vbo_cube, vbo_instanced_cube, vio_cube, vao_cube;

const int MAX_CUBE_COUNT = 50000;
struct Cube {
    glm::vec3 Position{ 0.0f };
    glm::vec3 Scale{ 1.0f };
    glm::vec3 Color{ 1.0f };
} cubes[MAX_CUBE_COUNT];
int current_cube_count;
const int RENDER_OCTREE_POINT_THRESHOLD = 100000;

// -- Point GL Objects

const int MAX_POINT_COUNT = 1400000;
struct Points {
    glm::vec3 Position;
    glm::vec3 Color;
} points[MAX_POINT_COUNT];
GLuint vbo_points, vao_points;

glm::vec3 line[] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
GLuint vbo_line, vao_line;

// -- Camera & Ray Settings

glm::mat4 view = glm::lookAt(glm::vec3{ 0.0f, 0.0f, 75.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
glm::mat4 projection = glm::perspective(45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.001f, 1000.0f);

glm::vec3 ray_origin = glm::vec3{ 10.0f, 10.0f, -10.0f };
glm::vec3 ray_rotation_euler = glm::vec3{ 15.0f, 330.0f, 0.0f };
glm::quat ray_rotation{ glm::vec3{0.0f} };
float ray_tolerance = 1.0f;

int ray_nearest = 20;

glm::vec3 camera_pos = { 0.0f, 0.0f, -75.0f };
glm::vec3 camera_rotation = { 0.0f, 0.0f, 0.0f };
glm::quat camera_quat;
float movement_speed = 50.0f;
glm::vec2 camera_rotation_speed{ 10.0f };
bool ray_follows_camera = false;
bool show_octree = true;


// -- Caching values

bool should_update_casting = true;

void setup_gl()
{
    glEnable(GL_PROGRAM_POINT_SIZE);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
   
    // Shader Programs & Pipelines

    v_cube_shader = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vert_cube_shader);
    f_line_shader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &frag_line_shader);
    glCreateProgramPipelines(1, &pipeline_cube);
    glUseProgramStages(pipeline_cube, GL_VERTEX_SHADER_BIT, v_cube_shader);
    glUseProgramStages(pipeline_cube, GL_FRAGMENT_SHADER_BIT, f_line_shader);

    v_line_shader = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vert_line_shader);
    glCreateProgramPipelines(1, &pipeline_line);
    glUseProgramStages(pipeline_line, GL_VERTEX_SHADER_BIT, v_line_shader);
    glUseProgramStages(pipeline_line, GL_FRAGMENT_SHADER_BIT, f_line_shader);

    v_dot_shader = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vert_point_shader);
    f_dot_shader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &frag_point_shader);
    glCreateProgramPipelines(1, &pipeline_dot);
    glUseProgramStages(pipeline_dot, GL_VERTEX_SHADER_BIT, v_dot_shader);
    glUseProgramStages(pipeline_dot, GL_FRAGMENT_SHADER_BIT, f_dot_shader);

    // Cube

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

    glCreateBuffers(1, &vbo_instanced_cube);
    glNamedBufferStorage(vbo_instanced_cube, sizeof(cubes), cubes, GL_DYNAMIC_STORAGE_BIT);
    glVertexArrayVertexBuffer(vao_cube, 1, vbo_instanced_cube, 0, sizeof(Cube));
    glVertexArrayBindingDivisor(vao_cube, 1, 1);

    glEnableVertexArrayAttrib(vao_cube, 1);
    glVertexArrayAttribFormat(vao_cube, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Cube, Position));
    glVertexArrayAttribBinding(vao_cube, 1, 1);

    glEnableVertexArrayAttrib(vao_cube, 2);
    glVertexArrayAttribFormat(vao_cube, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Cube, Scale));
    glVertexArrayAttribBinding(vao_cube, 2, 1);

    glEnableVertexArrayAttrib(vao_cube, 3);
    glVertexArrayAttribFormat(vao_cube, 3, 3, GL_FLOAT, GL_FALSE, offsetof(Cube, Color));
    glVertexArrayAttribBinding(vao_cube, 3, 1);

    // Line

    glCreateBuffers(1, &vbo_line);
    glNamedBufferStorage(vbo_line, sizeof(line), line, GL_DYNAMIC_STORAGE_BIT);

    glCreateVertexArrays(1, &vao_line);
    glVertexArrayVertexBuffer(vao_line, 0, vbo_line, 0, sizeof(glm::vec3));
    glEnableVertexArrayAttrib(vao_line, 0);
    glVertexArrayAttribFormat(vao_line, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao_line, 0, 0);

    // Points

    glCreateBuffers(1, &vbo_points);
    glNamedBufferStorage(vbo_points, sizeof(points), points, GL_DYNAMIC_STORAGE_BIT);

    glCreateVertexArrays(1, &vao_points);

    glVertexArrayVertexBuffer(vao_points, 0, vbo_points, 0, sizeof(Points));
    glVertexArrayBindingDivisor(vao_points, 0, 1);

    glEnableVertexArrayAttrib(vao_points, 0);
    glVertexArrayAttribFormat(vao_points, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Points, Position));
    glVertexArrayAttribBinding(vao_points, 0, 0);

    glEnableVertexArrayAttrib(vao_points, 1);
    glVertexArrayAttribFormat(vao_points, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Points, Color));
    glVertexArrayAttribBinding(vao_points, 1, 0);

}

void setup_octree(OctreeSettings settings)
{
    std::random_device rand_device;
    std::default_random_engine rand_engine { rand_device()};
    std::uniform_real_distribution<float> rand_dist{ -settings.size / 2.0f, settings.size / 2.0f };

    float time = static_cast<float>(glfwGetTime());
    Bounds bounds { {0.0f, 0.0f, 0.0f}, {settings.size, settings.size, settings.size} };
    octree = Octree<int>{ bounds, settings.min_capacity, settings.max_depth };
    for (int i = 0; i < settings.point_count; i++)
    {
        points[i].Position = glm::vec3{ rand_dist(rand_engine), rand_dist(rand_engine), rand_dist(rand_engine) };
        points[i].Color = glm::vec3{ 1.0f };
        octree.add(toVec3(points[i].Position), i);
    }

    float finish_time = static_cast<float>(glfwGetTime());
    current_octree_settings = settings;
    std::cout << "Generated " << settings.point_count << " points in " << (finish_time - time) << " seconds." << std::endl;
}

void prepare_cube(glm::vec3 position, glm::vec3 scale, glm::vec3 color = glm::vec3{ 1.0f })
{
    cubes[current_cube_count].Position = position;
    cubes[current_cube_count].Scale = scale;
    cubes[current_cube_count].Color = color;

    current_cube_count++;
}

void draw_cubes()
{
    glm::mat4 mvp = projection * view;

    glBindProgramPipeline(pipeline_cube);

    glNamedBufferSubData(vbo_instanced_cube, 0, sizeof(Cube) * current_cube_count, cubes);

    glLineWidth(1.0f);
    glBindVertexArray(vao_cube);

    glProgramUniformMatrix4fv(v_cube_shader, 0, 1, GL_FALSE, glm::value_ptr(mvp));
    glDrawElementsInstanced(GL_LINES, sizeof(cube_indices) / sizeof(unsigned), GL_UNSIGNED_INT, 0, current_cube_count);

    glBindProgramPipeline(0);
    current_cube_count = 0;
}

void prepare_octant(const Octree<int>* octant, const glm::vec3* color = nullptr)
{
    Bounds bounds = octant->get_bounds();
    glm::vec3 pos = toGLMVec3(bounds.center);
    glm::vec3 size = toGLMVec3(bounds.size);
    glm::vec3 draw_color;
    if (color)
    {
        draw_color = *color;
    }
    else
    {
        const glm::vec3 CUBE_BASE_COLOR = glm::vec3{ 0.9f };
        float color_mult = 0.1f;
        draw_color = CUBE_BASE_COLOR - octant->get_depth_level() * color_mult;
    }

    prepare_cube(pos, size, draw_color);
}

void draw_points()
{
    glm::mat4 t = glm::translate(glm::mat4{ 1.0f }, glm::vec3{0.0f});
    glm::mat4 mvp = projection * view * t;

    glBindProgramPipeline(pipeline_dot);

    glNamedBufferSubData(vbo_points, 0, sizeof(Points) * current_octree_settings.point_count, points);

    glBindVertexArray(vao_points);

    glProgramUniformMatrix4fv(v_dot_shader, 0, 1, GL_FALSE, glm::value_ptr(mvp));
    glProgramUniform3fv(v_dot_shader, 1, 1, glm::value_ptr(camera_pos));
    glDrawArraysInstanced(GL_POINTS, 0, 1, current_octree_settings.point_count);

    glBindProgramPipeline(0);
}

void draw_ray()
{
    glm::mat4 t = glm::translate(glm::mat4{ 1.0f }, ray_origin);
    glm::mat4 r = glm::mat4_cast(ray_rotation);
    glm::mat4 s = glm::scale(glm::mat4{ 1.0f }, glm::vec3{ 1000.0f });
    glm::mat4 mvp = projection * view * t * r * s;

    glm::vec3 color{ 1.0f, 0.0f, 0.0f };
    glBindProgramPipeline(pipeline_line);

    glLineWidth(1.0f);
    glBindVertexArray(vao_line);

    glProgramUniformMatrix4fv(v_line_shader, 0, 1, GL_FALSE, glm::value_ptr(mvp));
    glProgramUniform3fv(v_line_shader, 1, 1, glm::value_ptr(color));
    glDrawArrays(GL_LINES, 0, 2);

    glBindProgramPipeline(0);
}

void handle_raycast()
{
    static std::vector<const Octree<int>*> nodes;
    static std::vector<std::pair<Vec3, int>> nearest_points;

    glm::vec3 rot_vec = ray_rotation * glm::vec3{ 0.0f, 0.0f, 1.0f };
    Vec3 origin = toVec3(ray_origin);
    Vec3 direction = toVec3(rot_vec);
    const glm::vec3 OCTANT_CAST_COLOR = glm::vec3{ 1.0f, 0.4f, 0.4f };

    if (should_update_casting || ray_follows_camera)
    {
        // cache casting and nearest neighbors
        nodes = octree.intersects_nodes(origin, direction);
        if (ray_nearest > 0)
        {
            nearest_points = octree.nearest(origin, ray_nearest);
        }
        else
        {
            nearest_points.clear();
        }
        should_update_casting = false;
    }

    for (auto point : nearest_points)
    {
        points[point.second].Color = glm::vec3{ 0.0f, 0.0f, 1.0f };
    }

    for (auto node : nodes)
    {
        auto node_points = node->get_points();
        for (std::pair<Vec3, int> point : node_points)
        {
            if (MathUtility::ray_point_intersects(origin, direction, point.first, ray_tolerance))
            {
                points[point.second].Color = glm::vec3{ 0.0f, 1.0f, 0.0f };
            }
        }
        if (show_octree)
            prepare_octant(node, &OCTANT_CAST_COLOR);
    }
}

void update_camera(GLFWwindow* window, float dt)
{
    static bool esc_was_down = false;
    static bool can_look = true;
    static glm::vec2 prev_mouse_pos;

    bool esc_down_this_frame = false;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        if (!esc_was_down)
        {
            can_look = !can_look;
        }
        esc_was_down = true;
    }
    else
    {
        esc_was_down = false;
    }

    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    glm::vec2 mouse_pos = glm::vec2{ mouse_x, mouse_y };
    glm::vec2 mouse_dt = mouse_pos - prev_mouse_pos;
    prev_mouse_pos = mouse_pos;

    if (can_look)
    {
        glm::vec2 mouse_movement = mouse_dt * camera_rotation_speed * dt;
        camera_rotation.x += mouse_movement.y;
        camera_rotation.x = glm::clamp(camera_rotation.x, -89.99f, 89.99f);
        camera_rotation.y -= mouse_movement.x;
    }

    camera_quat = glm::quat{ glm::radians(camera_rotation) };

    glm::vec3 relative_movement{ 0.0f };
    glm::vec3 absolute_movement{ 0.0f };
    glm::vec3 rotation = camera_rotation;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        relative_movement.z += movement_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        relative_movement.z -= movement_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        relative_movement.x += movement_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        relative_movement.x -= movement_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        absolute_movement.y += movement_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        absolute_movement.y -= movement_speed;
    }
    camera_pos += camera_quat * relative_movement * dt;
    camera_pos += absolute_movement * dt;
}

void run()
{
    for (int i = 0; i < current_octree_settings.point_count; i++)
    {
        points[i].Color = glm::vec3{ 1.0f };
    }
    
    glm::vec3 look_dir = camera_quat * glm::vec3{ 0.0f, 0.0f, 1.0f };
    view = glm::lookAt(camera_pos, camera_pos + look_dir, glm::vec3{ 0.0f, 1.0f, 0.0f });

    if (show_octree)
    {
        for (auto octant : octree)
        {
            prepare_octant(octant);
        }
    }

    ray_rotation = glm::quat{ glm::radians(ray_rotation_euler) };
    if (ray_follows_camera)
    {
        ray_origin = camera_pos;
        ray_rotation_euler = camera_rotation;
    }

    handle_raycast();
}

void draw()
{
    if (show_octree)
        draw_cubes();
    draw_points();


    if (!ray_follows_camera)
    {
        draw_ray();
    }
}

void gui(float dt)
{
    ImGui::Begin("Instructions");
    ImGui::Text("WASD / LShift / Space - Move");
    ImGui::Text("Esc - Lock/Unlock Camera Rotation (Look)");
    ImGui::Text("Intersected nodes - red");
    ImGui::Text("Intersected points - green");
    ImGui::Text("Points near ray origin - blue");
    ImGui::End();

    ImGui::Begin("Settings");

    float fps = 1.0f / dt;
    ImGui::LabelText("FPS", "%.f", fps);

    if (ImGui::TreeNode("Octree"))
    {
        ImGui::BeginDisabled(current_octree_settings.point_count > RENDER_OCTREE_POINT_THRESHOLD);
        ImGui::Checkbox("Show Octree Grid (Affects Performance)", &show_octree);
        ImGui::EndDisabled();
        ImGui::SliderFloat("Grid Size", &edit_octree_settings.size, 50.0f, 1000.0f);

        int depth = edit_octree_settings.max_depth;
        if (ImGui::SliderInt("Max Depth", &depth, 0, 10))
            edit_octree_settings.max_depth = depth;

        ImGui::SliderInt("Num Points", &edit_octree_settings.point_count, 0, MAX_POINT_COUNT);
        if (ImGui::Button("Remake Octree"))
        {
            if (edit_octree_settings.point_count > RENDER_OCTREE_POINT_THRESHOLD)
            {
                show_octree = false;
            }
            setup_octree(edit_octree_settings);
            should_update_casting = true;
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Ray"))
    {
        if (ImGui::InputFloat3("Origin", glm::value_ptr(ray_origin)))
        {
            should_update_casting = true;
        }

        if (ImGui::SliderFloat3("Rotation", glm::value_ptr(ray_rotation_euler), 0.0f, 360.0f))
        {
            should_update_casting = true;
        }
        if (ImGui::Button("Orient Ray to Camera"))
        {
            ray_origin = camera_pos;
            ray_rotation_euler = camera_rotation;
            should_update_casting = true;
        }

        ImGui::Checkbox("Lock Ray to Camera", &ray_follows_camera);

        if (ImGui::InputFloat("Cast Tolerance", &ray_tolerance))
        {
            should_update_casting = true;
        }
        int max_point_count = std::min(edit_octree_settings.point_count, 5000);
        if (ImGui::SliderInt("Num Nearest Points (Affects Performance)", &ray_nearest, 0, max_point_count))
        {
            should_update_casting = true;
        }
        ImGui::TreePop();
    }
    ImGui::End();
}

void unload()
{
    glDeleteBuffers(1, &vbo_cube);
    glDeleteBuffers(1, &vbo_instanced_cube);
    glDeleteBuffers(1, &vio_cube);
    glDeleteVertexArrays(1, &vao_cube);

    glDeleteBuffers(1, &vbo_line);
    glDeleteVertexArrays(1, &vao_line);

    glDeleteVertexArrays(1, &vao_points);
    glDeleteBuffers(1, &vbo_points);

    glDeleteProgramPipelines(1, &pipeline_line);
    glDeleteProgramPipelines(1, &pipeline_dot);

    glDeleteShader(v_line_shader);
    glDeleteShader(f_line_shader);
    glDeleteShader(v_dot_shader);
    glDeleteShader(f_dot_shader);
}

int main()
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Octree Demo", nullptr, nullptr);
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
    
    setup_octree(current_octree_settings);
    setup_gl();

    static float prev_time = static_cast<float>(glfwGetTime());
    while (!glfwWindowShouldClose(window))
    {
        float current_time = static_cast<float>(glfwGetTime());
        float dt = current_time - prev_time;
        prev_time = current_time;

        glClearColor(0.3f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        update_camera(window, dt);
        run();
        draw();
        gui(dt);

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
