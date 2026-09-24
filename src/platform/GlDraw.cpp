#include "platform/GlDraw.hpp"

#include <SDL.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

namespace {

const char* kVertexShader = R"(#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
uniform vec2 uScreen;
out vec3 vColor;
void main() {
    float x = aPos.x / uScreen.x * 2.0 - 1.0;
    float y = 1.0 - aPos.y / uScreen.y * 2.0;
    gl_Position = vec4(x, y, 0.0, 1.0);
    vColor = aColor;
}
)";

const char* kFragmentShader = R"(#version 330 core
in vec3 vColor;
out vec4 fragColor;
void main() {
    fragColor = vec4(vColor, 1.0);
}
)";

unsigned compileShader(unsigned type, const char* source) {
    const unsigned shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == 0) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        SDL_SetError("%s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GlVertex vertex(ScreenPoint point, int red, int green, int blue) {
    return GlVertex{
        static_cast<float>(point.x),
        static_cast<float>(point.y),
        static_cast<float>(red) / 255.0f,
        static_cast<float>(green) / 255.0f,
        static_cast<float>(blue) / 255.0f};
}

void drawBatch(unsigned program, int width, int height, const std::vector<GlVertex>& vertices, unsigned mode) {
    if (vertices.empty() || width <= 0 || height <= 0) {
        return;
    }
    glUseProgram(program);
    const int screen = glGetUniformLocation(program, "uScreen");
    glUniform2f(screen, static_cast<float>(width), static_cast<float>(height));
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(GlVertex)),
        vertices.data(),
        GL_DYNAMIC_DRAW);
    glDrawArrays(mode, 0, static_cast<int>(vertices.size()));
}

} // namespace

bool GlDraw::init() {
    shutdown();
    const unsigned vertex = compileShader(GL_VERTEX_SHADER, kVertexShader);
    const unsigned fragment = compileShader(GL_FRAGMENT_SHADER, kFragmentShader);
    if (vertex == 0 || fragment == 0) {
        if (vertex != 0) {
            glDeleteShader(vertex);
        }
        if (fragment != 0) {
            glDeleteShader(fragment);
        }
        return false;
    }

    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    int linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == 0) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        SDL_SetError("%s", log);
        glDeleteProgram(program);
        program = 0;
        return false;
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GlVertex), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(GlVertex), reinterpret_cast<void*>(sizeof(float) * 2));
    ready = true;
    return true;
}

void GlDraw::shutdown() {
    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    if (program != 0) {
        glDeleteProgram(program);
        program = 0;
    }
    triangles.clear();
    lines.clear();
    ready = false;
}

void GlDraw::clear(float red, float green, float blue) {
    if (width > 0 && height > 0) {
        glViewport(0, 0, width, height);
    }
    glClearColor(red, green, blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GlDraw::triangle(ScreenPoint a, ScreenPoint b, ScreenPoint c, int red, int green, int blue) {
    triangles.push_back(vertex(a, red, green, blue));
    triangles.push_back(vertex(b, red, green, blue));
    triangles.push_back(vertex(c, red, green, blue));
}

void GlDraw::line(ScreenPoint a, ScreenPoint b, int red, int green, int blue) {
    lines.push_back(vertex(a, red, green, blue));
    lines.push_back(vertex(b, red, green, blue));
}

void GlDraw::flush() {
    if (!ready) {
        triangles.clear();
        lines.clear();
        return;
    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    drawBatch(program, width, height, triangles, GL_TRIANGLES);
    drawBatch(program, width, height, lines, GL_LINES);
    triangles.clear();
    lines.clear();
}
