#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cstring>

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 2.8f, 6.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float fov = 45.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Lighting
glm::vec3 lightPos(2.0f, 5.0f, 2.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 woodColor(0.65f, 0.45f, 0.25f);
glm::vec3 roofColor(0.8f, 0.2f, 0.2f);

// ---------- Shader Class ----------
class Shader {
public:
    unsigned int ID;
    Shader(const char* vertexSource, const char* fragmentSource) {
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        checkCompileErrors(vertexShader, "VERTEX");
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        checkCompileErrors(fragmentShader, "FRAGMENT");
        ID = glCreateProgram();
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    void use() { glUseProgram(ID); }
    void setMat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
    void setVec3(const std::string& name, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }
    void setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
private:
    void checkCompileErrors(unsigned int shader, std::string type) {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "Shader error (" << type << "): " << infoLog << std::endl;
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "Program linking error: " << infoLog << std::endl;
            }
        }
    }
};

// ---------- Shader Sources ----------
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 FragPos;
out vec3 Normal;
void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
in vec3 FragPos;
in vec3 Normal;
void main() {
    float ambientStrength = 0.3f;
    vec3 ambient = ambientStrength * lightColor;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    float specularStrength = 0.5f;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

// Unlit grid shader
const char* gridVertexSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";
const char* gridFragmentSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
}
)";

// Unlit light cube shader
const char* lightCubeVertexSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";
const char* lightCubeFragmentSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
}
)";

// Textured signboard shader
const char* texVertexSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec2 TexCoord;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";
const char* texFragmentSource = R"(
#version 330 core
out vec4 FragColor;
uniform sampler2D ourTexture;
in vec2 TexCoord;
void main() {
    FragColor = texture(ourTexture, TexCoord);
}
)";

// ---------- Geometry Data ----------
float cubeVertices[] = {
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f
};
unsigned int cubeIndices[] = {
    0,1,2, 2,3,0, 4,5,6, 6,7,4,
    8,9,10, 10,11,8, 12,13,14, 14,15,12,
    16,17,18, 18,19,16, 20,21,22, 22,23,20
};

// Quad for signboard
float quadVertices[] = {
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
};
unsigned int quadIndices[] = { 0,1,2, 2,3,0 };

// ---------- Utility Functions ----------
unsigned int createCubeVAO() {
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return VAO;
}

void drawCube(unsigned int VAO, Shader& shader, glm::mat4 model) {
    shader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

// Table (centered)
void drawTable(Shader& shader, unsigned int VAO, glm::vec3 pos) {
    glm::mat4 model;
    model = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 0.7f, 0.0f));
    model = glm::scale(model, glm::vec3(1.5f, 0.1f, 1.2f));
    drawCube(VAO, shader, model);
    glm::vec3 legOffsets[] = {{-0.65f,0.3f,-0.55f},{0.65f,0.3f,-0.55f},{-0.65f,0.3f,0.55f},{0.65f,0.3f,0.55f}};
    for(auto& off : legOffsets) {
        model = glm::translate(glm::mat4(1.0f), pos + off);
        model = glm::scale(model, glm::vec3(0.12f, 0.7f, 0.12f));
        drawCube(VAO, shader, model);
    }
}

// Chair (left/right)
void drawChair(Shader& shader, unsigned int VAO, glm::vec3 pos, float angle) {
    glm::mat4 base = glm::translate(glm::mat4(1.0f), pos);
    base = glm::rotate(base, glm::radians(angle), glm::vec3(0,1,0));
    glm::mat4 model;
    model = base * glm::translate(glm::mat4(1.0f), glm::vec3(0,0.2f,0));
    model = glm::scale(model, glm::vec3(0.6f,0.1f,0.6f));
    drawCube(VAO, shader, model);
    model = base * glm::translate(glm::mat4(1.0f), glm::vec3(0,0.55f,-0.28f));
    model = glm::scale(model, glm::vec3(0.6f,0.4f,0.08f));
    drawCube(VAO, shader, model);
    glm::vec3 legOffsets[] = {{-0.25f,0.05f,-0.25f},{0.25f,0.05f,-0.25f},{-0.25f,0.05f,0.25f},{0.25f,0.05f,0.25f}};
    for(auto& off : legOffsets) {
        model = base * glm::translate(glm::mat4(1.0f), off);
        model = glm::scale(model, glm::vec3(0.08f,0.2f,0.08f));
        drawCube(VAO, shader, model);
    }
    model = base * glm::translate(glm::mat4(1.0f), glm::vec3(-0.35f,0.4f,0));
    model = glm::scale(model, glm::vec3(0.08f,0.08f,0.6f));
    drawCube(VAO, shader, model);
    model = base * glm::translate(glm::mat4(1.0f), glm::vec3(0.35f,0.4f,0));
    model = glm::scale(model, glm::vec3(0.08f,0.08f,0.6f));
    drawCube(VAO, shader, model);
}

// ---------- Food Stall with BIG Signboard ----------
// Create texture with large "FOOD STALL" letters (24x24 per character)
unsigned int createSignTexture() {
    int width = 512, height = 256;  // taller to fit big letters
    unsigned char* pixels = new unsigned char[width * height * 4];
    // Bright yellow background
    for(int i = 0; i < width * height; i++) {
        pixels[i*4+0] = 255;
        pixels[i*4+1] = 220;
        pixels[i*4+2] = 100;
        pixels[i*4+3] = 255;
    }
    // Simple 8x8 font pattern (we'll scale it 3x to get 24x24)
    const unsigned char font[][8] = {
        {0x7E,0x08,0x08,0x08,0x08,0x7E,0x00,0x00}, // F
        {0x3C,0x42,0x81,0x81,0x81,0x42,0x3C,0x00}, // O
        {0x3C,0x42,0x81,0x81,0x81,0x42,0x3C,0x00}, // O
        {0x7C,0x42,0x81,0x81,0x81,0x42,0x7C,0x00}, // D
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // space
        {0x3C,0x42,0x40,0x3C,0x02,0x42,0x3C,0x00}, // S
        {0x7E,0x08,0x08,0x08,0x08,0x08,0x08,0x00}, // T
        {0x3C,0x42,0x81,0x81,0xFF,0x81,0x81,0x00}, // A
        {0x40,0x40,0x40,0x40,0x40,0x40,0x7E,0x00}  // L
    };
    const char* text = "FOOD STALL";
    int scale = 3;  // scale factor: 8x8 -> 24x24
    int charWidth = 8 * scale;
    int charHeight = 8 * scale;
    int startX = 80;
    int startY = (height - charHeight) / 2;  // vertically centered
    for(int i = 0; text[i]; i++) {
        int idx = -1;
        char c = text[i];
        if(c == 'F') idx = 0;
        else if(c == 'O') idx = 1;
        else if(c == 'D') idx = 3;
        else if(c == ' ') idx = 4;
        else if(c == 'S') idx = 5;
        else if(c == 'T') idx = 6;
        else if(c == 'A') idx = 7;
        else if(c == 'L') idx = 8;
        if(idx >= 0) {
            for(int row = 0; row < 8; row++) {
                unsigned char bits = font[idx][row];
                for(int col = 0; col < 8; col++) {
                    if(bits & (1 << (7-col))) {
                        // draw a scaled block
                        int baseX = startX + i * (charWidth + 8) + col * scale;
                        int baseY = startY + row * scale;
                        for(int dy = 0; dy < scale; dy++) {
                            for(int dx = 0; dx < scale; dx++) {
                                int px = baseX + dx;
                                int py = baseY + dy;
                                if(px >=0 && px < width && py >=0 && py < height) {
                                    pixels[(py*width + px)*4+0] = 0;
                                    pixels[(py*width + px)*4+1] = 0;
                                    pixels[(py*width + px)*4+2] = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    delete[] pixels;
    return texture;
}

unsigned int createQuadVAO() {
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    return VAO;
}

void drawSignboard(Shader& shader, unsigned int quadVAO, unsigned int texture, glm::vec3 pos, glm::vec3 size) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
    model = glm::scale(model, size);
    shader.use();
    shader.setMat4("model", model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.setInt("ourTexture", 0);
    glBindVertexArray(quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void drawFoodStall(Shader& litShader, Shader& texShader, unsigned int cubeVAO, unsigned int quadVAO, unsigned int signTex) {
    // Main counter
    glm::mat4 model;
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.4f, -2.2f));
    model = glm::scale(model, glm::vec3(2.2f, 0.6f, 1.5f));
    drawCube(cubeVAO, litShader, model);
    // Back wall
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.1f, -3.0f));
    model = glm::scale(model, glm::vec3(2.2f, 1.2f, 0.1f));
    drawCube(cubeVAO, litShader, model);
    // Roof (red)
    litShader.setVec3("objectColor", roofColor);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.7f, -2.2f));
    model = glm::scale(model, glm::vec3(2.5f, 0.1f, 2.0f));
    drawCube(cubeVAO, litShader, model);
    // Side posts
    litShader.setVec3("objectColor", woodColor);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.1f, 0.9f, -3.0f));
    model = glm::scale(model, glm::vec3(0.1f, 1.4f, 0.1f));
    drawCube(cubeVAO, litShader, model);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(1.1f, 0.9f, -3.0f));
    model = glm::scale(model, glm::vec3(0.1f, 1.4f, 0.1f));
    drawCube(cubeVAO, litShader, model);
    
    // SIGNBOARD on top (facing front) – large and prominent
    drawSignboard(texShader, quadVAO, signTex, glm::vec3(0.0f, 2.3f, -1.9f), glm::vec3(2.2f, 0.8f, 0.05f));
}

// ---------- Ground Grid ----------
void createGridVAO(unsigned int& gridVAO, unsigned int& gridVBO, int& gridVertexCount) {
    std::vector<float> verts;
    for(float i = -5.0f; i <= 5.0f; i += 0.5f) {
        verts.push_back(i); verts.push_back(-0.1f); verts.push_back(-5.0f);
        verts.push_back(i); verts.push_back(-0.1f); verts.push_back(5.0f);
        verts.push_back(-5.0f); verts.push_back(-0.1f); verts.push_back(i);
        verts.push_back(5.0f); verts.push_back(-0.1f); verts.push_back(i);
    }
    gridVertexCount = verts.size() / 3;
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}
void drawGround(Shader& shader, unsigned int gridVAO, int count) {
    shader.setVec3("color", glm::vec3(0.6f,0.6f,0.6f));
    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, count);
}

// ---------- Callbacks ----------
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if(firstMouse) { lastX=xpos; lastY=ypos; firstMouse=false; }
    float xoff = xpos - lastX, yoff = lastY - ypos;
    lastX=xpos; lastY=ypos;
    float sens=0.1f;
    xoff*=sens; yoff*=sens;
    yaw+=xoff; pitch+=yoff;
    if(pitch>89.0f) pitch=89.0f;
    if(pitch<-89.0f) pitch=-89.0f;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw))*cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw))*cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if(fov<1.0f) fov=1.0f;
    if(fov>45.0f) fov=45.0f;
}
void processInput(GLFWwindow* window) {
    float speed = 2.5f * deltaTime;
    if(glfwGetKey(window, GLFW_KEY_W)==GLFW_PRESS) cameraPos += speed * cameraFront;
    if(glfwGetKey(window, GLFW_KEY_S)==GLFW_PRESS) cameraPos -= speed * cameraFront;
    if(glfwGetKey(window, GLFW_KEY_A)==GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if(glfwGetKey(window, GLFW_KEY_D)==GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(window, true);
}

// ---------- Main ----------
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Food Stall with Seating Area", NULL, NULL);
    if(!window) {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to init GLAD\n";
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    Shader mainShader(vertexShaderSource, fragmentShaderSource);
    Shader gridShader(gridVertexSource, gridFragmentSource);
    Shader lightCubeShader(lightCubeVertexSource, lightCubeFragmentSource);
    Shader texShader(texVertexSource, texFragmentSource);

    unsigned int cubeVAO = createCubeVAO();
    unsigned int quadVAO = createQuadVAO();
    unsigned int signTexture = createSignTexture();

    unsigned int gridVAO, gridVBO;
    int gridVertexCount;
    createGridVAO(gridVAO, gridVBO, gridVertexCount);

    mainShader.use();
    mainShader.setVec3("objectColor", woodColor);
    mainShader.setVec3("lightColor", lightColor);
    mainShader.setVec3("lightPos", lightPos);

    while(!glfwWindowShouldClose(window)) {
        float cur = glfwGetTime();
        deltaTime = cur - lastFrame;
        lastFrame = cur;
        processInput(window);
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 proj = glm::perspective(glm::radians(fov), (float)SCR_WIDTH/SCR_HEIGHT, 0.1f, 100.0f);

        mainShader.use();
        mainShader.setMat4("view", view);
        mainShader.setMat4("projection", proj);
        mainShader.setVec3("viewPos", cameraPos);
        mainShader.setVec3("lightPos", lightPos);
        mainShader.setVec3("objectColor", woodColor);

        // Table center, chairs left & right
        drawTable(mainShader, cubeVAO, glm::vec3(0.0f, 0.0f, 0.0f));
        drawChair(mainShader, cubeVAO, glm::vec3(1.4f, 0.0f, 0.0f), -90.0f);
        drawChair(mainShader, cubeVAO, glm::vec3(-1.4f, 0.0f, 0.0f), 90.0f);

        // Food stall with big signboard on top
        drawFoodStall(mainShader, texShader, cubeVAO, quadVAO, signTexture);

        // Ground grid
        gridShader.use();
        gridShader.setMat4("view", view);
        gridShader.setMat4("projection", proj);
        drawGround(gridShader, gridVAO, gridVertexCount);

        // Light source marker
        lightCubeShader.use();
        lightCubeShader.setMat4("view", view);
        lightCubeShader.setMat4("projection", proj);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), lightPos);
        model = glm::scale(model, glm::vec3(0.2f));
        lightCubeShader.setMat4("model", model);
        lightCubeShader.setVec3("color", glm::vec3(1.0f, 1.0f, 0.8f));
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &gridVBO);
    glDeleteTextures(1, &signTexture);
    glfwTerminate();
    return 0;
}