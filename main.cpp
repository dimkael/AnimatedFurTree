#include <cmath>
#include <glut.h>
#include <glaux.h>
#define PI 3.1415926

GLuint textures[3];
float cam_angle = 0.0f;
float t = 0.0f;
float wind = 2.0f;

struct Vec3f {
    Vec3f() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    float x;
    float y;
    float z;
};

using Point3f = Vec3f;

Vec3f operator-(const Point3f& a, const Point3f& b) {
    return Vec3f(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3f cross(const Vec3f& u, const Vec3f& v) {
    return Vec3f(
        u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x
    );
}

Vec3f triangleNormal(const Point3f& a, const Point3f& b, const Point3f& c) {
    Vec3f u = b - a;
    Vec3f v = c - a;
    return cross(u, v);
}

void delta(float& x, float k) {
    x += k * sin(t);
}

void drawCone(Vec3f point, float radius, float h, float k1, float k2) {
    glLoadIdentity();
    glTranslatef(point.x, point.y, point.z);

    const int n = 128;
    const float cone_angle = float(PI) * 2 / n;

    Vec3f normal;
    Point3f top(point.x, point.y + h, point.z);
    Point3f next;
    Point3f prev(
        radius * sin(cone_angle * 0) + point.x,
        point.y,
        -radius * cos(cone_angle * 0) + point.z
    );
    delta(prev.x, k1);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    for (int i = 1; i <= n; i++) {
        top.x = point.x;
        next.x = radius * sin(cone_angle * i) + point.x;
        next.y = point.y;
        next.z = -radius * cos(cone_angle * i) + point.z;

        delta(next.x, k1);
        delta(top.x, k2);

        normal = triangleNormal(prev, top, next);

        glColor3ub(200, 255, 255);
        glBegin(GL_TRIANGLES);
        glNormal3f(normal.x, normal.y, normal.z);
        glTexCoord2f((i + 1) / n, 0.0f);
        glVertex3f(next.x, next.y, next.z);
        glTexCoord2f(i / n, 0.0f);
        glVertex3f(prev.x, prev.y, prev.z);
        glTexCoord2f((i + 0.5f) / n, 1.0f);
        glVertex3f(top.x, top.y, top.z);
        glEnd();

        prev = next;
    }
}

void drawCylinder(Vec3f point, float radius, float h, float k) {
    glLoadIdentity();
    glTranslatef(point.x, point.y, point.z);
    const int n = 64;
    const float cone_angle = float(PI) * 2 / n;

    Vec3f normal;
    Point3f next_top;
    Point3f next_bottom;
    Point3f prev_top(
        radius * sin(cone_angle * 0) + point.x,
        point.y + h,
        -radius * cos(cone_angle * 0) + point.z
    );
    Point3f prev_bottom(
        radius * sin(cone_angle * 0) + point.x,
        point.y,
        -radius * cos(cone_angle * 0) + point.z
    );
    delta(prev_top.x, k);

    for (int i = 1; i <= n; i++) {
        next_top.x = radius * sin(cone_angle * i) + point.x;
        next_top.y = point.y;
        next_top.z = -radius * cos(cone_angle * i) + point.z;

        next_bottom = next_top;
        next_top.y += h;

        delta(next_top.x, k);

        normal = triangleNormal(prev_bottom, next_bottom, next_top);

        glBindTexture(GL_TEXTURE_2D, textures[1]);
        glColor3ub(255, 255, 255);
        glBegin(GL_QUADS);
        glNormal3f(normal.x, normal.y, normal.z);
        glVertex3f(next_top.x, next_top.y, next_top.z);
        glVertex3f(prev_top.x, prev_top.y, prev_top.z);
        glVertex3f(prev_bottom.x, prev_bottom.y, prev_bottom.z);
        glVertex3f(next_bottom.x, next_bottom.y, next_bottom.z);
        glEnd();

        prev_top = next_top;
        prev_bottom = next_bottom;
    }
}

void drawPlane(Vec3f point, float w, float h) {
    glLoadIdentity();
    glTranslatef(point.x, point.y, point.z);

    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glColor3ub(100, 100, 100);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 2.0f);
    glVertex3f(point.x - w / 2, point.y, point.z + h / 2);
    glTexCoord2f(2.0f, 2.0f);
    glVertex3f(point.x + w / 2, point.y, point.z + h / 2);
    glTexCoord2f(2.0f, 0.0f);
    glVertex3f(point.x + w / 2, point.y, point.z - h / 2);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(point.x - w / 2, point.y, point.z - h / 2);
    glEnd();
}

void setLight(GLenum pname, GLfloat position[], GLfloat diffuse[], float attenuation) {
    glEnable(pname);
    glLightfv(pname, GL_DIFFUSE, position);
    glLightfv(pname, GL_POSITION, diffuse);
    glLightf(pname, GL_QUADRATIC_ATTENUATION, attenuation);
}

GLvoid LoadGLTexture(int index, LPCWSTR path)
{
    AUX_RGBImageRec* texture;
    texture = auxDIBImageLoad(path);
    glGenTextures(1, &textures[index]);
    glBindTexture(GL_TEXTURE_2D, textures[index]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, texture->sizeX, texture->sizeY,
        0, GL_RGB, GL_UNSIGNED_BYTE, texture->data);
}

void initGL() {
    LoadGLTexture(0, L"needles.bmp");
    LoadGLTexture(1, L"wood.bmp");
    LoadGLTexture(2, L"grass.bmp");
    glEnable(GL_TEXTURE_2D);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLfloat light1_diffuse[] = { 1.0, 1.0, 1.0 };
    GLfloat light1_position[] = { 0.0, 10.0, 10.0, 1.0 };
    setLight(GL_LIGHT1, light1_diffuse, light1_position, 0.0f);

    drawCone(Point3f(0.0f, 2.8f, 0.0f), 0.9f, 1.8f, wind * 0.8f, wind * 1.4f);
    drawCone(Point3f(0.0f, 1.8f, 0.0f), 1.4f, 2.8f, wind * 0.4f, wind * 1.0f);
    drawCone(Point3f(0.0f, 0.5f, 0.0f), 2.0f, 4.0f, wind / 10.0f, wind * 0.6f);
    drawCylinder(Point3f(0.0f, 0.0f, 0.0f), 0.4f, 0.5f, wind / 10.0f);
    drawPlane(Point3f(0.0f, 0.0f, 0.0f), 20.0f, 20.0f);

    glutSwapBuffers();
}

void reshape(GLsizei width, GLsizei height) {
    if (height == 0) height = 1;
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, aspect, 0.1f, 100.0f);
    gluLookAt(15.0 * sin(cam_angle), 6.0f, 15.0 * cos(cam_angle), 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 244) {
        cam_angle += 6.0f;
    }
    else if (key == 226) {
        cam_angle -= 6.0f;
    }
}

void timer(int value) {
    glutPostRedisplay();
    reshape(GLsizei(640), GLsizei(480));
    t += 0.05f;
    glutTimerFunc(1000 / 60, timer, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Animated fir tree");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    initGL();
    glutKeyboardFunc(keyboard);
    glutTimerFunc(1000 / 60, timer, 0);
    glutMainLoop();

    return 0;
}