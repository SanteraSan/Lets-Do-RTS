#include "platform/GlDraw.hpp"

#include "render/UnitSprite.hpp"

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

const char* kSpriteFragmentShader = R"(#version 330 core
in vec3 vColor;
in vec2 vUv;
uniform sampler2D uTex;
out vec4 fragColor;
void main() {
    vec4 texel = texture(uTex, vUv);
    fragColor = vec4(vColor, 1.0) * texel;
}
)";

const char* kSpriteVertexShader = R"(#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aColor;
uniform vec2 uScreen;
out vec3 vColor;
out vec2 vUv;
void main() {
    float x = aPos.x / uScreen.x * 2.0 - 1.0;
    float y = 1.0 - aPos.y / uScreen.y * 2.0;
    gl_Position = vec4(x, y, 0.0, 1.0);
    vUv = aUv;
    vColor = aColor;
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

    const unsigned spriteVertex = compileShader(GL_VERTEX_SHADER, kSpriteVertexShader);
    const unsigned spriteFragment = compileShader(GL_FRAGMENT_SHADER, kSpriteFragmentShader);
    if (spriteVertex == 0 || spriteFragment == 0) {
        shutdown();
        return false;
    }
    spriteProgram = glCreateProgram();
    glAttachShader(spriteProgram, spriteVertex);
    glAttachShader(spriteProgram, spriteFragment);
    glLinkProgram(spriteProgram);
    glDeleteShader(spriteVertex);
    glDeleteShader(spriteFragment);
    int spriteLinked = 0;
    glGetProgramiv(spriteProgram, GL_LINK_STATUS, &spriteLinked);
    if (spriteLinked == 0) {
        char log[512];
        glGetProgramInfoLog(spriteProgram, 512, nullptr, log);
        SDL_SetError("%s", log);
        shutdown();
        return false;
    }

    const SpriteImage image = makeUnitSprite();
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        image.width,
        image.height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        image.rgba.data());

    glGenVertexArrays(1, &spriteVao);
    glGenBuffers(1, &spriteVbo);
    glBindVertexArray(spriteVao);
    glBindBuffer(GL_ARRAY_BUFFER, spriteVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GlSpriteVertex), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(GlSpriteVertex), reinterpret_cast<void*>(sizeof(float) * 2));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 3, GL_FLOAT, GL_FALSE, sizeof(GlSpriteVertex), reinterpret_cast<void*>(sizeof(float) * 4));

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
    if (spriteVbo != 0) {
        glDeleteBuffers(1, &spriteVbo);
        spriteVbo = 0;
    }
    if (spriteVao != 0) {
        glDeleteVertexArrays(1, &spriteVao);
        spriteVao = 0;
    }
    if (texture != 0) {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
    if (spriteProgram != 0) {
        glDeleteProgram(spriteProgram);
        spriteProgram = 0;
    }
    if (program != 0) {
        glDeleteProgram(program);
        program = 0;
    }
    triangles.clear();
    lines.clear();
    sprites.clear();
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

void GlDraw::sprite(ScreenPoint feet, int pixelWidth, int pixelHeight, int red, int green, int blue) {
    if (pixelWidth <= 0 || pixelHeight <= 0) {
        return;
    }
    const float r = static_cast<float>(red) / 255.0f;
    const float g = static_cast<float>(green) / 255.0f;
    const float b = static_cast<float>(blue) / 255.0f;
    const float left = static_cast<float>(feet.x) - static_cast<float>(pixelWidth) * 0.5f;
    const float right = left + static_cast<float>(pixelWidth);
    const float bottom = static_cast<float>(feet.y);
    const float top = bottom - static_cast<float>(pixelHeight);
    const GlSpriteVertex corners[4] = {
        {left, bottom, 0.0f, 1.0f, r, g, b},
        {right, bottom, 1.0f, 1.0f, r, g, b},
        {right, top, 1.0f, 0.0f, r, g, b},
        {left, top, 0.0f, 0.0f, r, g, b},
    };
    const int order[6] = {0, 1, 2, 0, 2, 3};
    for (int index : order) {
        sprites.push_back(corners[index]);
    }
}

void GlDraw::flush() {
    if (!ready) {
        triangles.clear();
        lines.clear();
        sprites.clear();
        return;
    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    drawBatch(program, width, height, triangles, GL_TRIANGLES);
    if (!sprites.empty() && width > 0 && height > 0 && texture != 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(spriteProgram);
        const int screen = glGetUniformLocation(spriteProgram, "uScreen");
        glUniform2f(screen, static_cast<float>(width), static_cast<float>(height));
        glBindVertexArray(spriteVao);
        glBindBuffer(GL_ARRAY_BUFFER, spriteVbo);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(sprites.size() * sizeof(GlSpriteVertex)),
            sprites.data(),
            GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(sprites.size()));
        glDisable(GL_BLEND);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }
    drawBatch(program, width, height, lines, GL_LINES);
    triangles.clear();
    lines.clear();
    sprites.clear();
}
