// =======================================================
// Team Member: Shehab Eldin Osama
// Task: Planet System Definition & Configuration
// =======================================================
#ifdef _WIN32
#include <windows.h>
#endif

// مكتبة stb_image لقراءة ملفات الصور (يجب تحميلها ووضعها بجانب ملف الـ C++)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/glut.h>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <vector>
#include <iostream>

static const float PI = 3.14159265358979f;
static const float DEG2RAD = PI / 180.0f;

struct Planet {
    const char* name;
    float  radius;
    float  orbitRadius;
    float  orbitSpeed;
    float  spinSpeed;
    float  orbitAngle;
    float  spinAngle;
    const char* texFile;
    GLuint textureID;
    float  specR, specG, specB, shininess;
    bool   hasRings;
};

static Planet planets[] = {
    // name       radius  orbit  orbitSpd spinSpd  orbitAng spinAng  TextureFile                TexID  specR  specG  specB  shine  rings
    {"Sun",       3.0f,  0.0f,  0.0f,   1.5f,   0,    0,    "textures/sun.jpg",       0,     1.0f,  1.0f,  0.5f,  32,    false},
    {"Mercury",   0.5f,  6.5f, 47.9f,   3.0f,   0,    0,    "textures/mercury.jpg",   0,     0.25f, 0.22f, 0.20f, 16,    false},
    {"Venus",     0.9f,  9.5f, 35.0f,   1.5f,  40,    0,    "textures/venus.jpg",     0,     0.40f, 0.38f, 0.25f, 32,    false},
    {"Earth",     1.0f, 13.0f, 29.8f,  15.0f,  80,    0,    "textures/earth.jpg",     0,     0.30f, 0.40f, 0.50f, 64,    false},
    {"Mars",      0.7f, 17.0f, 24.1f,  14.6f, 130,    0,    "textures/mars.jpg",      0,     0.30f, 0.18f, 0.12f, 16,    false},
    {"Jupiter",   2.5f, 24.0f, 13.1f,  36.0f, 200,    0,    "textures/jupiter.jpg",   0,     0.35f, 0.30f, 0.22f, 16,    false},
    {"Saturn",    2.1f, 32.0f,  9.7f,  22.0f, 280,    0,    "textures/saturn.jpg",    0,     0.32f, 0.28f, 0.15f, 16,    true },
    {"Uranus",    1.5f, 40.0f,  6.8f,  12.0f, 320,    0,    "textures/uranus.jpg",    0,     0.30f, 0.40f, 0.42f, 32,    false},
    {"Neptune",   1.4f, 48.0f,  5.4f,  16.0f,  10,    0,    "textures/neptune.jpg",   0,     0.20f, 0.25f, 0.55f, 64,    false},
};

// تعريف خامات إضافية
GLuint texSaturnRing;
GLuint texSkybox;
GLuint texMoon;
GLuint texSunGlow;

// متغيرات القمر
static float moonOrbitAngle = 0.0f;
static float moonSpinAngle = 0.0f;

// متغير التركيز على كوكب معين
static int selectedPlanet = -1;

// =======================================================
// Team Member: Mohamed Hossam
// Task: Camera Controls, Rendering Settings & Utility Functions
// =======================================================
static const int NUM_PLANETS = sizeof(planets) / sizeof(Planet);

static float camYaw = 30.0f;
static float camPitch = 25.0f;
static float camDist = 70.0f;
static int   lastMouseX = 0;
static int   lastMouseY = 0;
static bool  mouseDown = false;

static bool  paused = false;
static float speedMul = 1.0f;
static bool  showOrbits = true;
static int   windowW = 1200;
static int   windowH = 700;
static int   lastTime = 0;

static void drawOrbitRing(float radius) {
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 128; ++i) {
        float a = 2.0f * PI * i / 128.0f;
        glVertex3f(radius * cosf(a), 0.0f, radius * sinf(a));
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

// =======================================================
// Team Member: Ziad Essam
// Task: Ring Rendering & Material/Lighting Setup
// =======================================================
static void drawRings(float inner, float outer) {
    int steps = 64;
    glBegin(GL_QUAD_STRIP); // QUAD_STRIP أفضل للحلقات لتجنب التقطيع
    for (int i = 0; i <= steps; ++i) {
        float a = 2.0f * PI * i / steps;
        float c = cosf(a), s = sinf(a);
        float u = (float)i / steps;

        glNormal3f(0, 1, 0);
        glTexCoord2f(u, 1.0f);
        glVertex3f(outer * c, 0.0f, outer * s);

        glTexCoord2f(u, 0.0f);
        glVertex3f(inner * c, 0.0f, inner * s);
    }
    glEnd();
}

static void setMaterial(const Planet& p, bool isSun) {
    float white[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    if (isSun) {
        glMaterialfv(GL_FRONT, GL_EMISSION, white);
        glMaterialfv(GL_FRONT, GL_AMBIENT, white);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, white);
    }
    else {
        float zero[] = { 0, 0, 0, 1 };
        float amb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        float spec[] = { p.specR,  p.specG,  p.specB,  1.0f };

        glMaterialfv(GL_FRONT, GL_EMISSION, zero);
        glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, white);
        glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
        glMaterialf(GL_FRONT, GL_SHININESS, p.shininess);
    }
}

// =======================================================
// TEAM MEMBER : Mohamed Hossam & Ziad Essam
// TASK : Texture UV Mapping & Sphere Generation
// =======================================================
static void drawPlanetSphere(float r) {
    // تم زيادة الجودة إلى 64 لحل مشكلة الـ Pinch عند القطبين
    const int slices = 64;
    const int stacks = 64;

    for (int j = 0; j < stacks; ++j) {
        float phi0 = PI * (-0.5f + (float)j / stacks);
        float phi1 = PI * (-0.5f + (float)(j + 1) / stacks);

        float t0 = (float)j / stacks;
        float t1 = (float)(j + 1) / stacks;

        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float theta = 2.0f * PI * i / slices;
            float s = (float)i / slices;

            float ct = cosf(theta);
            float st = sinf(theta);

            float cp0 = cosf(phi0), sp0 = sinf(phi0);
            float cp1 = cosf(phi1), sp1 = sinf(phi1);

            glNormal3f(cp0 * ct, sp0, cp0 * st);
            glTexCoord2f(s, t0);
            glVertex3f(r * cp0 * ct, r * sp0, r * cp0 * st);

            glNormal3f(cp1 * ct, sp1, cp1 * st);
            glTexCoord2f(s, t1);
            glVertex3f(r * cp1 * ct, r * sp1, r * cp1 * st);
        }
        glEnd();
    }
}

// =======================================================
// Team Member: Mazen Mohamed
// Task: Labels Rendering & Space Background (Stars/Skybox)
// =======================================================
static void drawLabel(const char* text, float x, float y, float z) {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 0.8f);
    glRasterPos3f(x, y, z);
    for (const char* c = text; *c; ++c)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

static void drawSkybox() {
    // رسم Skybox ككرة ضخمة تحيط بالمشهد
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // التعديل هنا: تعطيل الـ Culling عشان الأوجه الداخلية للكرة تبان
    glDisable(GL_CULL_FACE);

    glBindTexture(GL_TEXTURE_2D, texSkybox);
    drawPlanetSphere(500.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

static void drawStars() {
    static bool ready = false;
    static float stars[600];
    if (!ready) {
        unsigned int seed = 12345;
        for (int i = 0; i < 600; ++i) {
            seed = seed * 1664525u + 1013904223u;
            stars[i] = ((float)(seed & 0xFFFF) / 32767.5f - 1.0f) * 300.0f;
        }
        ready = true;
    }
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glPointSize(1.5f);
    glColor3f(1, 1, 1);
    glBegin(GL_POINTS);
    for (int i = 0; i < 200; ++i)
        glVertex3f(stars[i * 3], stars[i * 3 + 1], stars[i * 3 + 2]);
    glEnd();
    glPointSize(1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

// =======================================================
// Team Member: Shehab Amgad
// Task: On-Screen UI Overlay & Simulation Info
// =======================================================
static void drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowW, 0, windowH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    glColor4f(0, 0, 0, 0.6f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // شريط سفلي للمعلومات
    glBegin(GL_QUADS);
    glVertex2i(0, windowH - 80);
    glVertex2i(windowW, windowH - 80);
    glVertex2i(windowW, windowH);
    glVertex2i(0, windowH);
    glEnd();
    glDisable(GL_BLEND);

    char buf[256];
    glColor3f(1.0f, 0.9f, 0.3f);
    snprintf(buf, sizeof(buf), "3D Solar System Simulation");
    glRasterPos2i(10, windowH - 20);
    for (char* c = buf; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    glColor3f(0.8f, 0.8f, 0.8f);
    snprintf(buf, sizeof(buf),
        "Speed: %.2fx  |  %s  |  Orbits: %s  |  [SPACE] Pause  [+/-] Speed  [O] Orbits  [R] Reset  [1-9] Zoom Planet  [0] View All",
        speedMul, paused ? "PAUSED" : "Running", showOrbits ? "ON" : "OFF");
    glRasterPos2i(10, windowH - 45);
    for (char* c = buf; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);

    // إذا كان هناك كوكب محدد، اعرض معلوماته
    if (selectedPlanet >= 0 && selectedPlanet < NUM_PLANETS) {
        Planet& p = planets[selectedPlanet];
        glColor3f(0.4f, 0.8f, 1.0f);
        snprintf(buf, sizeof(buf), ">> Target: %s  |  Radius: %.1f  |  Orbit Dist: %.1f  |  Orbit Speed: %.1f km/s",
            p.name, p.radius, p.orbitRadius, p.orbitSpeed);
        glRasterPos2i(10, windowH - 65);
        for (char* c = buf; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// =======================================================
// Team Member: Ziad Wael
// Task: Main Rendering Function (Camera, Lighting & Scene Setup)
// =======================================================
static void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float yawRad = camYaw * DEG2RAD;
    float pitchRad = camPitch * DEG2RAD;

    float targetX = 0.0f, targetY = 0.0f, targetZ = 0.0f;

    // حساب موقع الكاميرا والهدف في حالة الـ Zoom
    if (selectedPlanet >= 0 && selectedPlanet < NUM_PLANETS) {
        Planet& p = planets[selectedPlanet];
        targetX = p.orbitRadius * cosf(p.orbitAngle * DEG2RAD);
        targetZ = -p.orbitRadius * sinf(p.orbitAngle * DEG2RAD);

        float distFromPlanet = p.radius * 4.0f + 5.0f; // مسافة الكاميرا عن الكوكب
        float cx = targetX + distFromPlanet * cosf(pitchRad) * sinf(yawRad);
        float cy = targetY + distFromPlanet * sinf(pitchRad);
        float cz = targetZ + distFromPlanet * cosf(pitchRad) * cosf(yawRad);
        gluLookAt(cx, cy, cz, targetX, targetY, targetZ, 0, 1, 0);
    }
    else {
        float cx = camDist * cosf(pitchRad) * sinf(yawRad);
        float cy = camDist * sinf(pitchRad);
        float cz = camDist * cosf(pitchRad) * cosf(yawRad);
        gluLookAt(cx, cy, cz, 0, 0, 0, 0, 1, 0);
    }

    // رسم الخلفيات أولاً
    drawSkybox();
    drawStars();

    float lightPos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float lightDiff[] = { 1.0f, 0.95f, 0.8f, 1.0f };
    float lightAmb[] = { 0.1f, 0.1f, 0.15f, 1.0f };
    float lightSpec[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.005f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0001f);

    // =======================================================
    // Team Member: Mohamed Abdelaziz
    // Task: Scene Rendering: Planets, Orbits, Rings & HUD
    // =======================================================
    if (showOrbits) {
        glDisable(GL_LIGHTING);
        glColor3f(0.25f, 0.35f, 0.55f);
        for (int i = 1; i < NUM_PLANETS; ++i)
            if (planets[i].orbitRadius > 0.0f)
                drawOrbitRing(planets[i].orbitRadius);
        glEnable(GL_LIGHTING);
    }

    for (int i = 0; i < NUM_PLANETS; ++i) {
        Planet& p = planets[i];
        bool isSun = (i == 0);

        glPushMatrix();
        glRotatef(p.orbitAngle, 0, 1, 0);
        glTranslatef(p.orbitRadius, 0, 0);
        //__________________________________________________sun glow ______________________
        
        if (isSun) {
            glDisable(GL_LIGHTING);

            
        }

        glRotatef(-90.0f, 1, 0, 0);
        glRotatef(p.spinAngle, 0, 0, 1);

        glRotatef(-90.0f, 1, 0, 0);
        glRotatef(p.spinAngle, 0, 0, 1);

        setMaterial(p, isSun);

        glBindTexture(GL_TEXTURE_2D, p.textureID);
        drawPlanetSphere(p.radius);
//____________________________________________________________________________
        if (isSun) {
            glEnable(GL_LIGHTING); // إعادة تفعيل الإضاءة لباقي الكواكب
        }

        // رسم القمر التابع للأرض (Nested Transformations)
        if (strcmp(p.name, "Earth") == 0) {
            glPushMatrix();
            // دوران القمر حول الأرض
            glRotatef(moonOrbitAngle, 0, 0, 1);
            glTranslatef(p.radius + 1.0f, 0, 0); // مسافة القمر عن الأرض
            glRotatef(moonSpinAngle, 0, 0, 1);   // دوران القمر حول نفسه

            float moonAmb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
            float moonDiff[] = { 0.8f, 0.8f, 0.8f, 1.0f };
            glMaterialfv(GL_FRONT, GL_AMBIENT, moonAmb);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, moonDiff);

            glBindTexture(GL_TEXTURE_2D, texMoon);
            drawPlanetSphere(0.3f); // حجم القمر
            glPopMatrix();
        }

        // رسم حلقات الكواكب (زحل)
        if (p.hasRings) {
            glBindTexture(GL_TEXTURE_2D, texSaturnRing);
            float ring[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glMaterialfv(GL_FRONT, GL_DIFFUSE, ring);
            glMaterialfv(GL_FRONT, GL_AMBIENT, ring);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_CULL_FACE);

            drawRings(p.radius * 1.4f, p.radius * 2.3f);

            // تم إزالة تفعيل glEnable(GL_CULL_FACE) من هنا لأنها كانت بتخفي الخلفية
            glDisable(GL_BLEND);
        }

        glPopMatrix();

        // رسم اسم الكوكب
        glPushMatrix();
        glRotatef(p.orbitAngle, 0, 1, 0);
        glTranslatef(p.orbitRadius, 0, 0);
        drawLabel(p.name, 0, p.radius + 0.6f, 0);
        glPopMatrix();
    }

    drawHUD();
    glutSwapBuffers();
}

// =======================================================
// Team Member: Mohamed Emad
// Task: Window Resizing & Animation Timing Update
// =======================================================
static void reshape(int w, int h) {
    if (h == 0) h = 1;
    windowW = w; windowH = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)w / h, 0.5f, 600.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void timer(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;

    if (!paused) {
        for (int i = 1; i < NUM_PLANETS; ++i) {
            Planet& p = planets[i];
            p.orbitAngle += p.orbitSpeed * speedMul * dt;
            p.spinAngle += p.spinSpeed * speedMul * dt;
            if (p.orbitAngle >= 360.0f) p.orbitAngle -= 360.0f;
            if (p.spinAngle >= 360.0f) p.spinAngle -= 360.0f;
        }
        planets[0].spinAngle += planets[0].spinSpeed * speedMul * dt;

        // تحديث حركة القمر
        moonOrbitAngle += 60.0f * speedMul * dt;
        moonSpinAngle += 20.0f * speedMul * dt;
        if (moonOrbitAngle >= 360.0f) moonOrbitAngle -= 360.0f;
        if (moonSpinAngle >= 360.0f) moonSpinAngle -= 360.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// =======================================================
// Team Member: Hazem Ahmed
// Task: User Input Handling (Keyboard & Mouse Controls)
// =======================================================
static void keyboard(unsigned char key, int, int) {
    switch (key) {
    case 27:  exit(0);           break;
    case ' ': paused = !paused;  break;
    case '+': case '=':
        speedMul += 0.25f;
        if (speedMul > 10.0f) speedMul = 10.0f;
        break;
    case '-': case '_':
        speedMul -= 0.25f;
        if (speedMul < 0.25f) speedMul = 0.25f;
        break;
    case 'o': case 'O': showOrbits = !showOrbits; break;
    case 'r': case 'R':
        camYaw = 30.0f; camPitch = 25.0f; camDist = 70.0f; selectedPlanet = -1;
        break;
        // تحديد الكواكب من 1 إلى 9
    case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        selectedPlanet = (key - '1') + 1;
        if (selectedPlanet >= NUM_PLANETS) selectedPlanet = NUM_PLANETS - 1;
        break;
    case '0':
        selectedPlanet = -1; // العودة للرؤية الحرة (الشمس كمركز للكل)
        break;
    }
}

static void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        mouseDown = (state == GLUT_DOWN);
        lastMouseX = x; lastMouseY = y;
    }
    if (button == 3) { camDist -= 2.5f; if (camDist < 5.0f)   camDist = 5.0f;   glutPostRedisplay(); }
    if (button == 4) { camDist += 2.5f; if (camDist > 250.0f) camDist = 250.0f; glutPostRedisplay(); }
}

static void mouseMotion(int x, int y) {
    if (!mouseDown) return;
    float dx = (float)(x - lastMouseX);
    float dy = (float)(y - lastMouseY);
    lastMouseX = x; lastMouseY = y;

    camYaw += dx * 0.4f;
    camPitch -= dy * 0.4f;
    if (camPitch > 89.0f) camPitch = 89.0f;
    if (camPitch < -89.0f) camPitch = -89.0f;

    glutPostRedisplay();
}

GLuint loadTexture(const char* filename, bool useAlpha = false) {
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, useAlpha ? 4 : 3);

    if (data) {
        GLenum format = useAlpha ? GL_RGBA : GL_RGB;
        gluBuild2DMipmaps(GL_TEXTURE_2D, useAlpha ? 4 : 3, width, height, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    else {
        std::cout << "Failed to load texture: " << filename << std::endl;
    }
    return texID;
}

// =======================================================
// Team Member: Shahd Ahmed
// Task: OpenGL Initialization & Application Entry Point
// =======================================================
static void initGL() {
    glClearColor(0.0f, 0.0f, 0.03f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    glEnable(GL_TEXTURE_2D);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    lastTime = glutGet(GLUT_ELAPSED_TIME);

    for (int i = 0; i < NUM_PLANETS; ++i) {
        planets[i].textureID = loadTexture(planets[i].texFile);
    }
    texSaturnRing = loadTexture("textures/saturn_ring.png", true);

    // تحميل الخامات الجديدة (تأكد من وضع هذه الصور في مجلد textures)
    texSkybox = loadTexture("textures/skybox.jpg"); // خريطة فضاء عالية الدقة
    texMoon = loadTexture("textures/moon.jpg");
    texSunGlow = loadTexture("textures/sun_glow.png", true); // صورة Halo شفافة
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowW, windowH);
    glutInitWindowPosition(100, 80);
    glutCreateWindow("3D Solar System");

    initGL();

    reshape(windowW, windowH);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}