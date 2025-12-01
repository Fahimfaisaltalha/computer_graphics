#include <cstdio>
#include <iostream>
#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>
#include <windows.h>
#include <mmsystem.h>
#include <unistd.h>
#include <cmath>
#define PI 3.14159265358979323846
using namespace std;

void myDisplay();
int start_flag = 0;

void *font1 = GLUT_BITMAP_TIMES_ROMAN_24;
void *font2 = GLUT_BITMAP_HELVETICA_18;
void *font3 = GLUT_BITMAP_8_BY_13;

void renderBitmapString(float x, float y, void *font, const char *string)
{
    const char *c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++)
    {
        glutBitmapCharacter(font, *c);
    }
}

// bool isItNight = false;
void night(int value);

// ============================================
// ENHANCEMENT VARIABLES
// ============================================
int dayCounter = 1;     // Tracks number of complete day cycles
bool isPaused = false;  // Pause/resume all animations
float timeSpeed = 1.0f; // Time speed multiplier (0.5x to 5.0x)
float zoomLevel = 1.0f; // Camera zoom level (0.5x to 2.0x)

// ============================================
// ANIMATION VARIABLES
// ============================================
// Windmill rotation
GLfloat angle = 0.0f;

// Truck animation (moves right on top road)
GLfloat truckPosition = 0.0f;
GLfloat truckSpeed = 0.05f;

void updateTruck(int value)
{
    if (!isPaused)
    {
        if (truckPosition > 1.8)
        {
            truckPosition = -1.2f;
        }
        truckPosition += truckSpeed;
    }

    glutPostRedisplay();

    glutTimerFunc(100, updateTruck, 0);
}

// Car animation (moves left on bottom road)
GLfloat carPosition = 0.0f;
GLfloat carSpeed = 0.05f;
void updateCar(int value)
{
    if (!isPaused)
    {
        if (carPosition < -1.5)
        {
            carPosition = 1.2f;
        }
        carPosition -= carSpeed;
    }

    glutPostRedisplay();

    glutTimerFunc(100, updateCar, 0);
}

// Boat animation (sails through river)
GLfloat boatPosition = 0.0f;
GLfloat boatSpeed = 0.005f;

// Cloud animations
GLfloat cloudPosition = 0.0f; // Cloud group 1
GLfloat cloudSpeed = 0.005f;

GLfloat cloud1Position = 0.0f; // Cloud group 2
GLfloat cloud1Speed = 0.007f;

// Moon animation (rises when sun sets)
GLfloat moonPosition = -0.6f; // Start at horizon
GLfloat moonSpeed = 0.003f;   // Synchronized with sun speed
int moonPauseCounter = 0;     // 10-second pause counter at peak

// Wind shear transformation for tree sway effect
GLfloat windShear = 0.0f;
GLfloat windShearSpeed = 0.0005f; // Gentle breeze speed
int windDirection = 1;

// Water wave animation for reflection effect
GLfloat waterWaveOffset = 0.0f;

void updateFan(int value)
{
    if (!isPaused)
    {
        angle += 30.1f;
    }

    glutPostRedisplay();

    glutTimerFunc(1, updateFan, 0);
}

void updateWindShear(int value)
{
    if (!isPaused)
    {
        // Smooth, gentle oscillating wind effect using sine wave for natural movement
        static float windTime = 0.0f;
        windTime += 0.02f;

        // Use sine wave for smooth, natural wind motion
        windShear = sin(windTime) * 0.025f; // Gentler maximum sway
    }

    glutPostRedisplay();
    glutTimerFunc(100, updateWindShear, 0); // Slower update rate for smoother motion
}

void updateWaterWave(int value)
{
    if (!isPaused)
    {
        waterWaveOffset += 0.01f;
        if (waterWaveOffset > 6.28f) // 2*PI
        {
            waterWaveOffset = 0.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(50, updateWaterWave, 0);
}

// ============================================
// COHEN-SUTHERLAND LINE CLIPPING ALGORITHM
// ============================================
// Clips lines to viewport boundaries before drawing
// Uses 4-bit region codes to identify point positions
// Region codes
const int INSIDE = 0; // 0000 - Point inside window
const int LEFT = 1;   // 0001 - Point left of window
const int RIGHT = 2;  // 0010 - Point right of window
const int BOTTOM = 4; // 0100 - Point below window
const int TOP = 8;    // 1000 - Point above window

// Clipping window boundaries
const float X_MIN = -1.0f;
const float X_MAX = 1.0f;
const float Y_MIN = -1.0f;
const float Y_MAX = 1.0f;

// Compute region code for a point (x, y)
int computeRegionCode(float x, float y)
{
    int code = INSIDE;

    if (x < X_MIN)
        code |= LEFT;
    else if (x > X_MAX)
        code |= RIGHT;

    if (y < Y_MIN)
        code |= BOTTOM;
    else if (y > Y_MAX)
        code |= TOP;

    return code;
}

// Cohen-Sutherland line clipping algorithm
bool cohenSutherlandClip(float &x1, float &y1, float &x2, float &y2)
{
    int code1 = computeRegionCode(x1, y1);
    int code2 = computeRegionCode(x2, y2);
    bool accept = false;

    while (true)
    {
        if ((code1 == 0) && (code2 == 0))
        {
            // Both endpoints inside - accept line
            accept = true;
            break;
        }
        else if (code1 & code2)
        {
            // Both endpoints in same outside region - reject line
            break;
        }
        else
        {
            // Line needs clipping
            int codeOut;
            float x, y;

            // Pick point outside clipping rectangle
            codeOut = (code1 != 0) ? code1 : code2;

            // Find intersection point using formulas
            if (codeOut & TOP)
            {
                x = x1 + (x2 - x1) * (Y_MAX - y1) / (y2 - y1);
                y = Y_MAX;
            }
            else if (codeOut & BOTTOM)
            {
                x = x1 + (x2 - x1) * (Y_MIN - y1) / (y2 - y1);
                y = Y_MIN;
            }
            else if (codeOut & RIGHT)
            {
                y = y1 + (y2 - y1) * (X_MAX - x1) / (x2 - x1);
                x = X_MAX;
            }
            else if (codeOut & LEFT)
            {
                y = y1 + (y2 - y1) * (X_MIN - x1) / (x2 - x1);
                x = X_MIN;
            }

            // Replace point outside clipping rectangle with intersection point
            if (codeOut == code1)
            {
                x1 = x;
                y1 = y;
                code1 = computeRegionCode(x1, y1);
            }
            else
            {
                x2 = x;
                y2 = y;
                code2 = computeRegionCode(x2, y2);
            }
        }
    }

    return accept;
}

// ============================================
// DDA (DIGITAL DIFFERENTIAL ANALYZER) LINE DRAWING
// ============================================
// Draws anti-aliased lines using incremental calculations
// Integrated with Cohen-Sutherland clipping for viewport culling
void drawLineDDA(float x1, float y1, float x2, float y2)
{
    // Apply Cohen-Sutherland clipping
    if (!cohenSutherlandClip(x1, y1, x2, y2))
    {
        // Line is completely outside - don't draw
        return;
    }

    // Line is inside or clipped - proceed with DDA
    float dx = x2 - x1;
    float dy = y2 - y1;

    float steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);

    float xIncrement = dx / steps;
    float yIncrement = dy / steps;

    float x = x1;
    float y = y1;

    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++)
    {
        glVertex2f(round(x), round(y));
        x += xIncrement;
        y += yIncrement;
    }
    glEnd();
}

// ============================================
// MIDPOINT CIRCLE ALGORITHM WITH SCANLINE FILL
// ============================================
// Draws filled circles using symmetric properties
// Uses scanline filling for solid interior
void circleSolid(float x, float y, float radius)
{
    // Use scanline fill with midpoint circle algorithm
    float scale = 600.0f;
    int r = (int)(radius * scale);

    int cx = 0;
    int cy = r;
    int p = 1 - r;

    glBegin(GL_POINTS);

    // Draw horizontal lines for each y-coordinate
    auto fillCircleLine = [&](int py)
    {
        int dx = (int)sqrt(r * r - py * py);
        for (int px = -dx; px <= dx; px++)
        {
            float glX = x + (float)px / scale;
            float glY = y + (float)py / scale;
            glVertex2f(glX, glY);
        }
    };

    // Fill from top to bottom
    for (int py = -r; py <= r; py++)
    {
        fillCircleLine(py);
    }

    glEnd();
}

// Sun position and movement
GLfloat sunPosition = 0.0f; // Sun starts at top (0.0 = zenith, -0.9 = set)
GLfloat sunSpeed = 0.003f;  // Movement speed per frame
int dayNightDirection = -1; // -1 for setting, 1 for rising
int sunPauseCounter = 0;    // 10-second pause at peak
void updateMoon(int value);

// ============================================
// SUN ANIMATION UPDATE
// ============================================
// Controls day/night cycle with pause at zenith
void updateSun(int value)
{
    if (!isPaused)
    {
        // If at top position, pause for 10 seconds (100 updates * 100ms)
        if (sunPosition >= 0.0f && dayNightDirection == 1)
        {
            sunPauseCounter++;
            if (sunPauseCounter >= 100)
            {
                sunPauseCounter = 0;
                dayNightDirection = -1; // Start setting
            }
        }
        // If at bottom, don't pause - let moon take over
        else if (sunPosition <= -0.9f && dayNightDirection == -1)
        {
            sunPosition = -0.9f;
            // Sun stays here while moon is up, will rise when moon sets
        }
        else
        {
            // Normal movement with time speed multiplier
            sunPosition += sunSpeed * dayNightDirection * timeSpeed;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(100, updateSun, 0);
}

// ============================================
// MOON ANIMATION UPDATE
// ============================================
// Rises when sun sets, synchronized for continuous day/night
void updateMoon(int value)
{
    if (!isPaused)
    {
        // Moon moves opposite to sun
        int moonDirection = -dayNightDirection;

        // If at top position (0.6 = peak, same height as sun at day), pause for 10 seconds
        if (moonPosition >= 0.6f && moonDirection == 1)
        {
            moonPauseCounter++;
            if (moonPauseCounter >= 100)
            {
                moonPauseCounter = 0;
                dayNightDirection = 1; // Trigger sun to rise (which makes moon set)
                dayCounter++;          // New day starts when moon sets
            }
        }
        // If at bottom, stay there while sun is up
        else if (moonPosition <= -0.6f && moonDirection == -1)
        {
            moonPosition = -0.6f;
        }
        else
        {
            // Normal movement with time speed multiplier
            moonPosition += moonSpeed * moonDirection * timeSpeed;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(100, updateMoon, 0);
}
void updateBoat(int value)
{
    if (!isPaused)
    {
        if (boatPosition > .40)
        {
            boatPosition = -1.0;
        }
        boatPosition += boatSpeed;
    }

    glutPostRedisplay();

    glutTimerFunc(100, updateBoat, 0);
}

void updateCloud(int value)
{
    if (!isPaused)
    {
        if (cloudPosition > 0.9)
        {
            cloudPosition = -1.4;
        }
        cloudPosition += cloudSpeed;
    }

    glutPostRedisplay();

    glutTimerFunc(100, updateCloud, 0);
}

void updateCloud1(int value)
{
    if (!isPaused)
    {
        if (cloud1Position > 1.3)
        {
            cloud1Position = -1.4;
        }

        cloud1Position += cloudSpeed;
    }

    glutPostRedisplay();

    glutTimerFunc(100, updateCloud1, 0);
}

void windmilStick()
{
    // windmill stick
    glColor3ub(255, 255, 255);
    glBegin(GL_POLYGON);
    glVertex2f(0.52f, -0.3f);
    glVertex2f(0.51f, -1.0f);
    glVertex2f(0.55f, -1.0f);
    glVertex2f(0.54f, -0.3f);
    glEnd();
}

void windMills()
{
    float x1, y1, x2, y2;
    float a;
    double radius = 0.027;

    x1 = 0.53, y1 = -0.30;

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(255, 255, 255);
    glVertex2f(x1, y1); // Center point for triangle fan
    for (a = 0.0f; a <= 360.0f; a += 1.0)
    {
        float rad = a * 3.14159f / 180.0f; // Convert to radians
        x2 = x1 + sin(rad) * radius;
        y2 = y1 + cos(rad) * radius;
        glVertex2f(x2, y2);
    }
    glEnd();
}

// ============================================
// TREE DRAWING WITH SHEAR TRANSFORMATION
// ============================================
// Multiple trees with wind sway effect using shear matrix
void tree()
{
    // Tree 1 with wind shear effect
    glPushMatrix();
    // Apply shear transformation matrix for realistic wind sway
    GLfloat shearMatrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        windShear, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};
    glMultMatrixf(shearMatrix);

    glColor3ub(0, 102, 0);
    glPointSize(2.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.9f, -0.7f);
    glVertex2f(-0.85f, -0.7f);
    glVertex2f(-0.85f, -0.35f);
    glEnd();
    glColor3ub(0, 153, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.8f, -0.7f);
    glVertex2f(-0.85f, -0.7f);
    glVertex2f(-0.85f, -0.35f);
    glEnd();
    glPopMatrix();

    // Tree trunk (not affected by wind)
    glColor3ub(102, 53, 0);
    glBegin(GL_QUADS);
    glVertex2f(-0.87f, -0.7f);
    glVertex2f(-0.87f, -0.8f);
    glVertex2f(-0.83f, -0.8f);
    glVertex2f(-0.83f, -0.7f);
    glEnd();

    // Tree 2 with wind shear effect
    glPushMatrix();
    GLfloat shearMatrix2[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        windShear * 0.8f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};
    glMultMatrixf(shearMatrix2);

    glColor3ub(0, 102, 0);
    glPointSize(2.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.0f, -0.3f);
    glVertex2f(-0.0f, -0.7f);
    glVertex2f(-0.08f, -0.7f);
    glEnd();
    glColor3ub(0, 153, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.0f, -0.3f);
    glVertex2f(-0.0f, -0.7f);
    glVertex2f(0.08f, -0.7f);
    glEnd();
    glPopMatrix();

    glColor3ub(102, 53, 0);
    glBegin(GL_QUADS);
    glVertex2f(-0.02f, -0.7f);
    glVertex2f(-0.02f, -0.8f);
    glVertex2f(0.02f, -0.8f);
    glVertex2f(0.02f, -0.7f);
    glEnd();

    // Tree 3 with wind shear effect
    glPushMatrix();
    GLfloat shearMatrix3[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        windShear * 0.9f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};
    glMultMatrixf(shearMatrix3);

    glColor3ub(0, 102, 0);
    glPointSize(2.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.2f, -0.32f);
    glVertex2f(0.2f, -0.65f);
    glVertex2f(0.13f, -0.65f);
    glEnd();
    glColor3ub(0, 153, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.2f, -0.32f);
    glVertex2f(0.2f, -0.65f);
    glVertex2f(0.27f, -0.65f);
    glEnd();
    glPopMatrix();

    glColor3ub(102, 53, 0);
    glBegin(GL_QUADS);
    glVertex2f(0.18f, -0.65f);
    glVertex2f(0.18f, -0.77f);
    glVertex2f(0.22f, -0.77f);
    glVertex2f(0.22f, -0.65f);
    glEnd();

    // Tree
    glColor3ub(0, 102, 0);
    glPointSize(2.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.19f, -0.1f);
    glVertex2f(-0.19f, -0.4f);
    glVertex2f(-0.22f, -0.4f);
    glEnd();
    glColor3ub(0, 153, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.19f, -0.1f);
    glVertex2f(-0.19f, -0.4f);
    glVertex2f(-0.16f, -0.4f);
    glEnd();
    glColor3ub(102, 53, 0);
    glBegin(GL_QUADS);
    glVertex2f(-0.20f, -0.4f);
    glVertex2f(-0.20f, -0.45f);
    glVertex2f(-0.18f, -0.45f);
    glVertex2f(-0.18f, -0.4f);
    glEnd();

    // Tree
    glColor3ub(0, 102, 0);
    glPointSize(2.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.39f, 0.1f);
    glVertex2f(-0.39f, -0.1f);
    glVertex2f(-0.41f, -0.1f);
    glEnd();
    glColor3ub(0, 153, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.39f, 0.1f);
    glVertex2f(-0.39f, -0.1f);
    glVertex2f(-0.37f, -0.1f);
    glEnd();
    glColor3ub(102, 53, 0);
    glBegin(GL_QUADS);
    glVertex2f(-0.398f, -0.1f);
    glVertex2f(-0.398f, -0.13f);
    glVertex2f(-0.388f, -0.13f);
    glVertex2f(-0.388f, -0.1f);
    glEnd();

    // Tree
    glColor3ub(0, 102, 0);
    glPointSize(2.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.4f, 0.1f);
    glVertex2f(0.4f, -0.2f);
    glVertex2f(0.37f, -0.2f);
    glEnd();
    glColor3ub(0, 153, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.4f, 0.1f);
    glVertex2f(0.4f, -0.2f);
    glVertex2f(0.43f, -0.2f);
    glEnd();
    glColor3ub(102, 53, 0);
    glBegin(GL_QUADS);
    glVertex2f(0.39f, -0.2f);
    glVertex2f(0.39f, -0.25f);
    glVertex2f(0.41f, -0.25f);
    glVertex2f(0.41f, -0.2f);
    glEnd();
}

// ============================================
// RIVER WITH WATER REFLECTION
// ============================================
// Uses gradient coloring and simulated sun reflection
void river()
{
    // River with enhanced gradient and shimmer effect
    float shimmer = sin(waterWaveOffset) * 10.0f;

    glBegin(GL_POLYGON);
    glColor3ub(100 + shimmer, 150 + shimmer, 220); // Lighter blue at top with shimmer
    glVertex2f(-1.0f, -0.1f);
    glVertex2f(-0.8f, -0.1f);
    glVertex2f(-0.68f, -0.13f);
    glVertex2f(-0.3f, -0.13f);
    glColor3ub(70 + shimmer * 0.5f, 130 + shimmer * 0.5f, 200);
    glVertex2f(-0.4f, -0.32f);
    glVertex2f(-0.19f, -0.49f);
    glColor3ub(50 + shimmer * 0.3f, 110 + shimmer * 0.3f, 180); // Darker blue for depth
    glVertex2f(-0.25f, -0.67f);
    glVertex2f(-0.04f, -0.79f);
    glVertex2f(0.016f, -0.86f);
    glVertex2f(0.019f, -0.90f);
    glVertex2f(0.021f, -0.905f);
    glVertex2f(0.02f, -0.91f);
    glVertex2f(0.025f, -0.915f);
    glVertex2f(0.023f, -0.920f);
    glVertex2f(0.025f, -0.925f);
    glVertex2f(0.027f, -1.0f);
    glColor3ub(40, 90, 160);
    glVertex2f(-1.0, -1.0f);
    glEnd();

    // Add water reflection of sun (when visible)
    if (sunPosition > -0.75f)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Reflected sun in water with distortion
        float reflectionY = -0.7f - sunPosition;
        float distortion = sin(waterWaveOffset * 2.0f) * 0.02f;

        glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 0.95f, 0.3f, 0.3f); // Semi-transparent yellow
        glVertex2f(-0.8f + distortion, reflectionY);
        for (float a = 0.0f; a < 360.0f; a += 20.0f)
        {
            float rad = a * 3.14159f / 180.0f;
            float x = -0.8f + sin(rad) * 0.08f + distortion;
            float y = reflectionY + cos(rad) * 0.05f;
            glVertex2f(x, y);
        }
        glEnd();

        glDisable(GL_BLEND);
    }
}

void hills()
{
    // Hills 1
    glColor3ub(0, 102, 0);
    glBegin(GL_POLYGON);
    glVertex2f(-0.55f, 0.36f);
    glVertex2f(-0.8f, -0.1f);
    glVertex2f(-0.68f, -0.13f);
    glEnd();
    glColor3ub(3, 182, 10);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.55f, 0.36f);
    glVertex2f(-0.68f, -0.13f);
    glVertex2f(-0.3f, -0.13f);
    glEnd();

    // Hills 2
    glColor3ub(0, 102, 0);
    glBegin(GL_POLYGON);
    glVertex2f(-0.15f, 0.3f);
    glVertex2f(-0.4f, -0.32f);
    glVertex2f(-0.24f, -0.38f);
    glEnd();
    glColor3ub(3, 182, 10);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.15f, 0.3f);
    glVertex2f(-0.24f, -0.38f);
    glVertex2f(0.1f, -0.38f);
    glEnd();

    // HILLS 3
    glColor3ub(0, 102, 0);
    glBegin(GL_POLYGON);
    glVertex2f(-0.85f, 0.12f);
    glVertex2f(-1.0f, -0.25f);
    glVertex2f(-1.0f, -0.55f);
    glEnd();
    glColor3ub(3, 182, 10);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.85f, 0.12f);
    glVertex2f(-1.0f, -0.55f);
    glVertex2f(-0.69f, -0.55f);
    glEnd();

    // Hills 4
    glColor3ub(0, 102, 0);
    glBegin(GL_POLYGON);
    glVertex2f(0.25f, 0.28f);
    glVertex2f(-0.045f, -0.0f);
    glVertex2f(0.0f, -0.3f);
    glVertex2f(0.1f, -0.07f);
    glVertex2f(0.2f, -0.05f);
    glEnd();

    glColor3ub(3, 182, 10);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.15f, 0.3f);
    glVertex2f(-0.24f, -0.38f);
    glVertex2f(0.1f, -0.38f);
    glEnd();
    glColor3ub(3, 182, 10);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.25f, 0.28f);
    glVertex2f(0.2f, -0.05f);
    glVertex2f(0.38f, -0.05f);
    glEnd();

    // Hills 5
    glColor3ub(34, 139, 34);
    glBegin(GL_POLYGON);
    glVertex2f(0.11f, 0.03f);
    glVertex2f(-0.12f, -0.4f);
    glVertex2f(-0.0f, -0.45f);
    glEnd();
    glColor3ub(50, 205, 50);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.11f, 0.03f);
    glVertex2f(0.0f, -0.45f);
    glVertex2f(0.28f, -0.45f);
    glEnd();

    // Hills 6
    glColor3ub(0, 102, 0);
    glBegin(GL_POLYGON);
    glVertex2f(0.31f, -0.13f);
    glVertex2f(-0.0f, -0.55f);
    glVertex2f(0.18f, -0.57f);
    glEnd();
    glColor3ub(3, 182, 10);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.31f, -0.13f);
    glVertex2f(0.18f, -0.57f);
    glVertex2f(0.4f, -0.57f);
    glEnd();

    // Hills 7
    glColor3ub(0, 102, 0);
    glBegin(GL_POLYGON);
    glVertex2f(0.55f, 0.4f);
    glVertex2f(0.3f, 0.0f);
    glVertex2f(0.45f, -0.04f);
    glEnd();
    glColor3ub(3, 182, 10);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.55f, 0.4f);
    glVertex2f(0.45f, -0.04f);
    glVertex2f(0.7f, -0.04f);
    glEnd();
    glColor3ub(3, 182, 10);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.25f, 0.28f);
    glVertex2f(0.2f, -0.05f);
    glVertex2f(0.38f, -0.05f);
    glEnd();

    // Hills 8
    glColor3ub(0, 102, 0);
    glBegin(GL_POLYGON);
    glVertex2f(0.75f, 0.47f);
    glVertex2f(0.55f, -0.04f);
    glVertex2f(0.65f, -0.08f);
    glEnd();
    glColor3ub(3, 182, 10);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.75f, 0.47f);
    glVertex2f(0.65f, -0.08f);
    glVertex2f(0.9f, -0.08f);
    glEnd();

    // Hills 8
    glColor3ub(0, 102, 0);
    glBegin(GL_POLYGON);
    glVertex2f(0.978f, 0.3f);
    glVertex2f(0.8f, -0.08f);
    glVertex2f(0.87f, -0.12f);
    glEnd();
    glColor3ub(3, 182, 10);
    glBegin(GL_POLYGON);
    glVertex2f(0.978f, 0.3f);
    glVertex2f(0.87f, -0.12f);
    glVertex2f(1.0f, -0.16);
    glVertex2f(1.0f, 0.2f);
    glEnd();
}

void sun()
{
    glPushMatrix();
    glTranslatef(0.0f, sunPosition, 0.0f);

    float x1, y1, x2, y2;
    float a;
    double radius = 0.1;

    x1 = -0.8, y1 = 0.7;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Outer glow layer 1 (large)
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 0.9f, 0.3f, 0.15f);
    glVertex2f(x1, y1);
    glColor4f(1.0f, 0.85f, 0.2f, 0.0f);
    for (a = 0.0f; a <= 360.0f; a += 2.0)
    {
        float rad = a * 3.14159f / 180.0f;
        x2 = x1 + sin(rad) * radius * 2.5;
        y2 = y1 + cos(rad) * radius * 2.5;
        glVertex2f(x2, y2);
    }
    glEnd();

    // Outer glow layer 2 (medium)
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 0.95f, 0.3f, 0.25f);
    glVertex2f(x1, y1);
    glColor4f(1.0f, 0.9f, 0.2f, 0.0f);
    for (a = 0.0f; a <= 360.0f; a += 2.0)
    {
        float rad = a * 3.14159f / 180.0f;
        x2 = x1 + sin(rad) * radius * 1.8;
        y2 = y1 + cos(rad) * radius * 1.8;
        glVertex2f(x2, y2);
    }
    glEnd();

    glDisable(GL_BLEND);

    // Main sun body with gradient
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 1.0f, 0.9f); // Bright white-yellow center
    glVertex2f(x1, y1);
    glColor3f(1.0f, 0.9f, 0.1f); // Yellow
    for (a = 0.0f; a <= 360.0f; a += 2.0)
    {
        float rad = a * 3.14159f / 180.0f;
        x2 = x1 + sin(rad) * radius * 0.7;
        y2 = y1 + cos(rad) * radius * 0.7;
        glVertex2f(x2, y2);
    }
    glEnd();

    // Outer sun ring
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 0.9f, 0.1f);
    glVertex2f(x1, y1);
    glColor3f(1.0f, 0.8f, 0.0f); // Orange-yellow edge
    for (a = 0.0f; a <= 360.0f; a += 2.0)
    {
        float rad = a * 3.14159f / 180.0f;
        x2 = x1 + sin(rad) * radius;
        y2 = y1 + cos(rad) * radius;
        glVertex2f(x2, y2);
    }
    glEnd();

    glPopMatrix();
}

void moon()
{
    glPushMatrix();
    glTranslatef(0.0f, moonPosition, 0.0f);
    float x1, y1, x2, y2;
    float a;
    double radius = 0.08;

    x1 = -0.8, y1 = 0.0; // Base position at center, will move with moonPosition

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Moon glow - soft light glow only
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.95f, 0.95f, 1.0f, 0.15f);
    glVertex2f(x1, y1);
    glColor4f(0.9f, 0.9f, 1.0f, 0.0f);
    for (a = 0.0f; a <= 360.0f; a += 2.0)
    {
        float rad = a * 3.14159f / 180.0f;
        x2 = x1 + sin(rad) * radius * 1.8;
        y2 = y1 + cos(rad) * radius * 1.8;
        glVertex2f(x2, y2);
    }
    glEnd();

    glDisable(GL_BLEND);

    // Main moon body with slight gradient
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 1.0f, 1.0f); // Bright white center
    glVertex2f(x1, y1);
    glColor3f(0.95f, 0.95f, 0.98f); // Slightly blue-tinted edge
    for (a = 0.0f; a <= 360.0f; a += 2.0)
    {
        float rad = a * 3.14159f / 180.0f;
        x2 = x1 + sin(rad) * radius;
        y2 = y1 + cos(rad) * radius;
        glVertex2f(x2, y2);
    }
    glEnd();

    // Add moon craters for realism - visible gray texture
    glColor3f(0.85f, 0.85f, 0.88f);
    circleSolid(x1 + 0.03f, y1 + 0.02f, 0.015);
    circleSolid(x1 - 0.02f, y1 + 0.01f, 0.012);
    circleSolid(x1 - 0.04f, y1 - 0.02f, 0.01);
    circleSolid(x1 + 0.02f, y1 - 0.03f, 0.008);

    glPopMatrix();
}

void clouds1()
{
    glPushMatrix();
    glTranslatef(cloud1Position, 0.0f, 0.0f);
    float x1, y1, x2, y2;
    float x3, y3, x4, y4;
    float x5, y5, x6, y6;
    float x7, y7, x8, y8;
    float x9, y9, x0, y0;
    float x11, y11, x12, y12;
    float x13, y13, x14, y14;
    float a;
    double radius = 0.07;

    x1 = -0.4, y1 = 0.65;

    // Enhanced cloud rendering with white/light gray color
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f); // White with slight transparency
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x2 = x1 + sin(a) * radius;
        y2 = y1 + cos(a) * radius;
        glVertex2f(x2, y2);
    }
    glEnd();

    x3 = -0.35, y3 = 0.56;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.98f, 0.98f, 1.0f, 0.88f);
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x4 = x3 + sin(a) * radius;
        y4 = y3 + cos(a) * radius;
        glVertex2f(x4, y4);
    }
    glEnd();

    x5 = -0.35, y5 = 0.7;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x6 = x5 + sin(a) * radius;
        y6 = y5 + cos(a) * radius;
        glVertex2f(x6, y6);
    }
    glEnd();

    x7 = -0.26, y7 = 0.56;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.98f, 0.98f, 1.0f, 0.88f);
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x8 = x7 + sin(a) * radius;
        y8 = y7 + cos(a) * radius;
        glVertex2f(x8, y8);
    }
    glEnd();

    x9 = -0.26, y9 = 0.7;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x0 = x9 + sin(a) * radius;
        y0 = y9 + cos(a) * radius;
        glVertex2f(x0, y0);
    }
    glEnd();

    x11 = -0.18, y11 = 0.63;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.98f, 0.98f, 1.0f, 0.88f);
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x12 = x11 + sin(a) * radius;
        y12 = y11 + cos(a) * radius;
        glVertex2f(x12, y12);
    }
    glEnd();

    x13 = -0.3, y13 = 0.63;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x14 = x13 + sin(a) * radius;
        y14 = y13 + cos(a) * radius;
        glVertex2f(x14, y14);
    }
    glEnd();

    glDisable(GL_BLEND);

    glPopMatrix();
}

void clouds2()
{
    glPushMatrix();
    glTranslatef(cloudPosition, 0.0f, 0.0f);
    float x1, y1, x2, y2;
    float x3, y3, x4, y4;
    float x5, y5, x6, y6;
    float x7, y7, x8, y8;
    float x9, y9, x0, y0;
    float x11, y11, x12, y12;
    float x13, y13, x14, y14;
    float a;
    double radius = 0.07;

    x1 = 0.5, y1 = 0.65;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x2 = x1 + sin(a) * radius;
        y2 = y1 + cos(a) * radius;
        glVertex2f(x2, y2);
    }
    glEnd();

    x3 = 0.45, y3 = 0.56;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.98f, 0.98f, 1.0f, 0.88f);
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x4 = x3 + sin(a) * radius;
        y4 = y3 + cos(a) * radius;
        glVertex2f(x4, y4);
    }
    glEnd();

    x5 = 0.45, y5 = 0.7;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x6 = x5 + sin(a) * radius;
        y6 = y5 + cos(a) * radius;
        glVertex2f(x6, y6);
    }
    glEnd();

    x7 = 0.36, y7 = 0.56;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.98f, 0.98f, 1.0f, 0.88f);
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x8 = x7 + sin(a) * radius;
        y8 = y7 + cos(a) * radius;
        glVertex2f(x8, y8);
    }
    glEnd();

    x9 = 0.36, y9 = 0.7;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x0 = x9 + sin(a) * radius;
        y0 = y9 + cos(a) * radius;
        glVertex2f(x0, y0);
    }
    glEnd();

    x11 = 0.28, y11 = 0.63;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.98f, 0.98f, 1.0f, 0.88f);
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x12 = x11 + sin(a) * radius;
        y12 = y11 + cos(a) * radius;
        glVertex2f(x12, y12);
    }
    glEnd();

    x13 = 0.4, y13 = 0.63;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    for (a = 0.0f; a < 360.0f; a += 0.2)
    {
        x14 = x13 + sin(a) * radius;
        y14 = y13 + cos(a) * radius;
        glVertex2f(x14, y14);
    }
    glEnd();

    glDisable(GL_BLEND);

    glPopMatrix();
}
// ============================================
// SKY RENDERING (DAY)
// ============================================
// Dynamic gradient based on sun position for realistic transitions
void sky(int value)
{
    // Dynamic gradient sky based on sun position
    float dayIntensity = (sunPosition + 0.75f) / 1.5f; // 0.0 to 1.0

    glBegin(GL_POLYGON); // Sky
    // Top color - deep blue to orange gradient
    glColor3f(0.1f + dayIntensity * 0.5f, 0.3f + dayIntensity * 0.4f, 0.8f);
    glVertex2f(-1, 1);
    glVertex2f(1, 1);
    // Bottom color - lighter with sunset tones
    glColor3f(0.5f + dayIntensity * 0.4f, 0.6f + dayIntensity * 0.2f, 0.9f);
    glVertex2f(1, -.1);
    glVertex2f(-1, -.1);
    glEnd();

    glutPostRedisplay();
    glutTimerFunc(5, sky, 0);
}

// ============================================
// NIGHT SKY WITH STARS
// ============================================
// Midnight blue gradient with twinkling star points
void night(int value)
{
    // Night sky gradient - extends to bottom of screen
    glBegin(GL_POLYGON);
    glColor3f(0.02f, 0.02f, 0.15f); // Deep midnight blue top
    glVertex2f(-1, 1);
    glVertex2f(1, 1);
    glColor3f(0.1f, 0.1f, 0.3f); // Lighter blue at horizon
    glVertex2f(1, -1);
    glVertex2f(-1, -1);
    glEnd();

    // Add twinkling stars
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(-0.9f, 0.8f);
    glVertex2f(-0.6f, 0.9f);
    glVertex2f(-0.3f, 0.85f);
    glVertex2f(0.1f, 0.95f);
    glVertex2f(0.5f, 0.8f);
    glVertex2f(0.8f, 0.9f);
    glVertex2f(-0.75f, 0.6f);
    glVertex2f(0.3f, 0.7f);
    glVertex2f(0.65f, 0.65f);
    glVertex2f(-0.4f, 0.75f);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3ub(0, 40, 0); // Darker green for night grass
    glVertex2f(0.0f, 0.0f);
    glColor3ub(0, 50, 0);
    glVertex2f(-0.4f, -0.33f);
    glColor3ub(0, 45, 0);
    glVertex2f(-0.4f, -1.0);
    glVertex2f(1.0f, -1.0);
    glVertex2f(1.0f, 0.05f);
    glEnd();

    glutPostRedisplay();
}

void ship()
{
    // base
    glPushMatrix();
    glTranslatef(boatPosition, 0.0f, 0.0f);
    glColor3ub(255, 110, 0);
    glBegin(GL_POLYGON);
    glVertex2f(-0.69f, -0.30f);
    glVertex2f(-0.62f, -0.30f);
    glVertex2f(-0.58f, -0.24f);
    glVertex2f(-0.73f, -0.24f);
    glEnd();

    // stick
    glColor3ub(160, 82, 35);
    glPointSize(3);
    drawLineDDA(-0.65, -0.24, -0.65, -0.1);

    glColor3ub(255, 255, 255);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.655, -0.13);
    glVertex2f(-0.655, -0.24);
    glVertex2f(-0.71, -0.24);
    glEnd();

    glColor3ub(255, 255, 255);
    glBegin(GL_POLYGON);
    glVertex2f(-0.648, -0.08);
    glVertex2f(-0.648, -0.24);
    glVertex2f(-0.59, -0.24);
    glVertex2f(-0.59, -0.18);
    glVertex2f(-0.6, -0.19);
    glVertex2f(-0.61, -0.16);
    glVertex2f(-0.63, -0.12);
    glEnd();

    glPopMatrix();

    // Hill 2 for hiding the ship
    glColor3ub(0, 102, 0);
    glBegin(GL_POLYGON);
    glVertex2f(-0.15f, 0.3f);
    glVertex2f(-0.4f, -0.32f);
    glVertex2f(-0.24f, -0.38f);
    glEnd();

    glColor3ub(3, 182, 10);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.15f, 0.3f);
    glVertex2f(-0.24f, -0.38f);
    glVertex2f(0.1f, -0.38f);
    glEnd();

    glColor3ub(0, 102, 0);
    glBegin(GL_POLYGON);
    glVertex2f(0.11f, 0.03f);
    glVertex2f(-0.12f, -0.4f);
    glVertex2f(-0.0f, -0.45f);
    glEnd();

    glColor3ub(3, 182, 10);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.11f, 0.03f);
    glVertex2f(0.0f, -0.45f);
    glVertex2f(0.28f, -0.45f);
    glEnd();

    glColor3ub(0, 102, 0);
    glPointSize(2.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.19f, -0.1f);
    glVertex2f(-0.19f, -0.4f);
    glVertex2f(-0.22f, -0.4f);
    glEnd();

    glColor3ub(0, 153, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.19f, -0.1f);
    glVertex2f(-0.19f, -0.4f);
    glVertex2f(-0.16f, -0.4f);
    glEnd();

    glColor3ub(102, 53, 0);
    glBegin(GL_QUADS);
    glVertex2f(-0.20f, -0.4f);
    glVertex2f(-0.20f, -0.45f);
    glVertex2f(-0.18f, -0.45f);
    glVertex2f(-0.18f, -0.4f);
    glEnd();

    // Hills 6
    glColor3ub(0, 102, 0);
    glBegin(GL_POLYGON);
    glVertex2f(0.31f, -0.13f);
    glVertex2f(-0.0f, -0.55f);
    glVertex2f(0.18f, -0.57f);
    glEnd();

    glColor3ub(3, 182, 10);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.31f, -0.13f);
    glVertex2f(0.18f, -0.57f);
    glVertex2f(0.4f, -0.57f);
    glEnd();

    // Tree
    glColor3ub(0, 102, 0);
    glPointSize(2.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.0f, -0.3f);
    glVertex2f(-0.0f, -0.7f);
    glVertex2f(-0.08f, -0.7f);
    glEnd();

    glColor3ub(0, 153, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.0f, -0.3f);
    glVertex2f(-0.0f, -0.7f);
    glVertex2f(0.08f, -0.7f);
    glEnd();

    glColor3ub(102, 53, 0);
    glBegin(GL_QUADS);
    glVertex2f(-0.02f, -0.7f);
    glVertex2f(-0.02f, -0.8f);
    glVertex2f(0.02f, -0.8f);
    glVertex2f(0.02f, -0.7f);
    glEnd();

    // Tree
    glColor3ub(0, 102, 0);
    glPointSize(2.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.2f, -0.32f);
    glVertex2f(0.2f, -0.65f);
    glVertex2f(0.13f, -0.65f);
    glEnd();

    glColor3ub(0, 153, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.2f, -0.32f);
    glVertex2f(0.2f, -0.65f);
    glVertex2f(0.27f, -0.65f);
    glEnd();

    glColor3ub(102, 53, 0);
    glBegin(GL_QUADS);
    glVertex2f(0.18f, -0.65f);
    glVertex2f(0.18f, -0.77f);
    glVertex2f(0.22f, -0.77f);
    glVertex2f(0.22f, -0.65f);
    glEnd();

    // HILLS 3
    glColor3ub(0, 102, 0);
    glBegin(GL_POLYGON);
    glVertex2f(-0.85f, 0.12f);
    glVertex2f(-1.0f, -0.25f);
    glVertex2f(-1.0f, -0.55f);
    glEnd();

    glColor3ub(3, 182, 10);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.85f, 0.12f);
    glVertex2f(-1.0f, -0.55f);
    glVertex2f(-0.69f, -0.55f);
    glEnd();

    // Tree
    glColor3ub(0, 102, 0);
    glPointSize(2.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.9f, -0.7f);
    glVertex2f(-0.85f, -0.7f);
    glVertex2f(-0.85f, -0.35f);
    glEnd();

    glColor3ub(0, 153, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.8f, -0.7f);
    glVertex2f(-0.85f, -0.7f);
    glVertex2f(-0.85f, -0.35f);
    glEnd();

    glColor3ub(102, 53, 0);
    glBegin(GL_QUADS);
    glVertex2f(-0.87f, -0.7f);
    glVertex2f(-0.87f, -0.8f);
    glVertex2f(-0.83f, -0.8f);
    glVertex2f(-0.83f, -0.7f);
    glEnd();
}

// ============================================
// VEHICLE HEADLIGHTS (NIGHT ONLY)
// ============================================
// Renders triangular light cones from vehicles
// Only active when sunPosition <= -0.7 (night time)
void drawVehicleHeadlights()
{
    if (sunPosition > -0.7)
        return; // Only at night when sun is below horizon

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Truck headlights
    glPushMatrix();
    glTranslatef(truckPosition, 0.0f, 0.0f);
    glBegin(GL_TRIANGLES);
    glColor4f(1.0f, 0.95f, 0.6f, 0.4f);
    glVertex2f(-0.58f, -0.80f);
    glColor4f(1.0f, 0.95f, 0.6f, 0.0f);
    glVertex2f(-0.45f, -0.85f);
    glVertex2f(-0.45f, -0.75f);
    glEnd();
    glPopMatrix();

    // Car headlights (car moves left, so headlight on left side)
    glPushMatrix();
    glTranslatef(carPosition, 0.0f, 0.0f);
    glBegin(GL_TRIANGLES);
    glColor4f(1.0f, 0.95f, 0.6f, 0.4f);
    glVertex2f(0.0f, -0.90f);
    glColor4f(1.0f, 0.95f, 0.6f, 0.0f);
    glVertex2f(-0.08f, -0.95f);
    glVertex2f(-0.08f, -0.85f);
    glEnd();
    glPopMatrix();

    glDisable(GL_BLEND);
}

// ============================================
// DYNAMIC SHADOWS (DAY ONLY)
// ============================================
// Casts shadows from houses and trees based on sun position
// Shadow intensity varies with sun height
void drawShadows()
{
    if (sunPosition <= -0.5)
        return; // No shadows at night

    float shadowIntensity = (sunPosition + 0.5f) / 1.2f;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // House 1 shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.2f * shadowIntensity);
    glBegin(GL_QUADS);
    glVertex2f(0.54f, -0.42f);
    glVertex2f(0.60f, -0.47f);
    glVertex2f(0.81f, -0.47f);
    glVertex2f(0.75f, -0.42f);
    glEnd();

    // House 2 shadow
    glBegin(GL_QUADS);
    glVertex2f(-0.79f, -0.72f);
    glVertex2f(-0.74f, -0.76f);
    glVertex2f(-0.57f, -0.76f);
    glVertex2f(-0.62f, -0.72f);
    glEnd();

    // Tree shadows (simplified)
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.85f, -0.80f);
    glVertex2f(-0.88f, -0.82f);
    glVertex2f(-0.82f, -0.82f);
    glEnd();

    glDisable(GL_BLEND);
}

// ============================================
// USER INTERFACE OVERLAY
// ============================================
// Displays day counter, speed, pause status, and controls
void drawUI()
{
    // Day counter
    glColor3f(1.0f, 1.0f, 1.0f);
    char dayStr[30];
    sprintf(dayStr, "DAY: %d", dayCounter);
    renderBitmapString(-0.98f, 0.95f, font3, dayStr);

    // Pause indicator
    if (isPaused)
    {
        glColor3f(1.0f, 1.0f, 0.0f);
        renderBitmapString(-0.15f, 0.0f, font1, "PAUSED");
    }

    // Speed indicator
    if (timeSpeed != 1.0f)
    {
        glColor3f(0.8f, 1.0f, 0.8f);
        char speedStr[30];
        sprintf(speedStr, "Speed: %.1fx", timeSpeed);
        renderBitmapString(-0.98f, 0.85f, font3, speedStr);
    }

    // Controls help (small text at bottom)
    glColor3f(0.9f, 0.9f, 0.9f);
    renderBitmapString(-0.98f, -0.95f, font3, "[P]Pause [+/-]Speed [Z]Zoom");
}

void grass1()
{
    glBegin(GL_POLYGON);
    glColor3ub(60, 170, 60);
    glVertex2f(-0.69f, -0.55f);
    glVertex2f(-1.0f, -0.55f);
    glColor3ub(34, 139, 34);
    glVertex2f(-1.0, -1.0);
    glVertex2f(-0.57, -1.0);
    glColor3ub(50, 160, 50);
    glVertex2f(-0.57, -0.72);
    glVertex2f(-0.55, -0.69);
    glVertex2f(-0.53, -0.67);
    glVertex2f(-0.51, -0.65);
    glVertex2f(-0.55, -0.63);
    glVertex2f(-0.59, -0.6);
    glEnd();
}

void road1()
{
    glColor3ub(32, 32, 32);
    glBegin(GL_POLYGON);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(-0.57f, -1.0f);
    glVertex2f(-0.57f, -0.82);
    glVertex2f(-1.0f, -0.82);
    glEnd();

    glColor3ub(255, 255, 255);
    glPointSize(5);
    drawLineDDA(-0.98, -0.91, -0.92, -0.91);
    drawLineDDA(-0.85, -0.91, -0.79, -0.91);
    drawLineDDA(-0.73, -0.91, -0.67, -0.91);
}
void grass2()
{
    // Enhanced grass with gradient and texture effect
    glBegin(GL_POLYGON);
    glColor3ub(85, 180, 85); // Lighter green at top
    glVertex2f(0.0f, 0.0f);
    glColor3ub(60, 170, 60);
    glVertex2f(-0.4f, -0.33f);
    glColor3ub(34, 139, 34); // Darker green at bottom for depth
    glVertex2f(-0.4f, -1.0);
    glVertex2f(1.0f, -1.0);
    glColor3ub(50, 160, 50);
    glVertex2f(1.0f, 0.05f);
    glEnd();

    // Add some grass texture details
    glColor3ub(70, 150, 70);
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (float x = 0.1f; x < 0.9f; x += 0.08f)
    {
        for (float y = -0.7f; y < -0.3f; y += 0.05f)
        {
            float offset = sin(x * 10.0f + waterWaveOffset) * 0.01f;
            glVertex2f(x + offset, y);
        }
    }
    glEnd();
}

void road2()
{
    glColor3ub(32, 32, 32);
    glBegin(GL_POLYGON);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, -0.82f);
    glVertex2f(-0.17f, -0.82f);
    glVertex2f(-0.17f, -1.0f);
    glEnd();

    glColor3ub(255, 255, 255);
    glPointSize(5);
    drawLineDDA(0.28, -0.91, 0.34, -0.91);
    drawLineDDA(0.4, -0.91, 0.46, -0.91);
    drawLineDDA(0.52, -0.91, 0.58, -0.91);
    drawLineDDA(0.64, -0.91, 0.7, -0.91);
    drawLineDDA(0.76, -0.91, 0.82, -0.91);
    drawLineDDA(0.88, -0.91, 0.94, -0.91);
    drawLineDDA(0.22, -0.91, 0.16, -0.91);
    drawLineDDA(0.1, -0.91, 0.04, -0.91);
    drawLineDDA(-0.02, -0.91, -0.08, -0.91);
}

void bridge()
{
    glColor3ub(255, 255, 255);
    glBegin(GL_POLYGON);
    glVertex2f(-0.57f, -0.82);
    glVertex2f(-0.57f, -1.0f);
    glVertex2f(-0.17f, -1.0f);
    glVertex2f(-0.17f, -0.82f);
    glEnd();

    // some grass
    glColor3ub(50, 205, 50);
    glBegin(GL_POLYGON);
    glVertex2f(-0.17f, -0.82f);
    glVertex2f(-0.0f, -0.82f);
    glVertex2f(-0.07f, -0.65f);
    glEnd();

    glColor3ub(0, 0, 0);
    glPointSize(5);
    drawLineDDA(-0.54, -0.91, -0.48, -0.91);
    drawLineDDA(-0.42, -0.91, -0.36, -0.91);
    drawLineDDA(-0.30, -0.91, -0.24, -0.91);
}

void hut()
{
    // Main house wall - warm brick color
    glColor3ub(210, 140, 90);
    glBegin(GL_QUADS);
    glVertex2f(0.54f, -0.42f);
    glVertex2f(0.75f, -0.42f);
    glVertex2f(0.75f, -0.14f);
    glVertex2f(0.54f, -0.14f);
    glEnd();

    // Roof - darker brown/red tiles
    glColor3ub(160, 60, 40);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.52f, -0.14f);
    glVertex2f(0.77f, -0.14f);
    glVertex2f(0.645f, 0.08f);
    glEnd();

    // Roof edge highlight
    glColor3ub(140, 50, 30);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.645f, 0.08f);
    glVertex2f(0.77f, -0.14f);
    glVertex2f(0.75f, -0.14f);
    glEnd();

    // Side extension wall
    glColor3ub(195, 130, 80);
    glBegin(GL_POLYGON);
    glVertex2f(0.75f, -0.42f);
    glVertex2f(0.88f, -0.42f);
    glVertex2f(0.88f, -0.25f);
    glVertex2f(0.75f, -0.14f);
    glEnd();

    // Door - wooden brown
    glColor3ub(101, 67, 33);
    glBegin(GL_QUADS);
    glVertex2f(0.58f, -0.42f);
    glVertex2f(0.64f, -0.42f);
    glVertex2f(0.64f, -0.22f);
    glVertex2f(0.58f, -0.22f);
    glEnd();

    // Door frame
    glColor3ub(80, 50, 25);
    glPointSize(2.5);
    drawLineDDA(0.58f, -0.22f, 0.64f, -0.22f);
    drawLineDDA(0.58f, -0.22f, 0.58f, -0.42f);
    drawLineDDA(0.64f, -0.22f, 0.64f, -0.42f);

    // Door handle
    glColor3ub(255, 215, 0);
    circleSolid(0.625f, -0.32f, 0.008f);

    // Window 1 - front (on left side of door, no overlap)
    glColor3ub(135, 206, 250);
    glBegin(GL_QUADS);
    glVertex2f(0.545f, -0.36f);
    glVertex2f(0.575f, -0.36f);
    glVertex2f(0.575f, -0.25f);
    glVertex2f(0.545f, -0.25f);
    glEnd();

    // Window frame 1
    glColor3ub(80, 50, 25);
    glPointSize(1.5);
    drawLineDDA(0.545f, -0.25f, 0.575f, -0.25f);
    drawLineDDA(0.545f, -0.36f, 0.575f, -0.36f);
    drawLineDDA(0.545f, -0.25f, 0.545f, -0.36f);
    drawLineDDA(0.575f, -0.25f, 0.575f, -0.36f);
    drawLineDDA(0.56f, -0.25f, 0.56f, -0.36f);
    drawLineDDA(0.545f, -0.305f, 0.575f, -0.305f);

    // Window 2 - side extension
    glColor3ub(135, 206, 250);
    glBegin(GL_QUADS);
    glVertex2f(0.79f, -0.36f);
    glVertex2f(0.84f, -0.36f);
    glVertex2f(0.84f, -0.29f);
    glVertex2f(0.79f, -0.29f);
    glEnd();

    // Window frame 2
    glColor3ub(80, 50, 25);
    glPointSize(1.5);
    drawLineDDA(0.79f, -0.29f, 0.84f, -0.29f);
    drawLineDDA(0.79f, -0.36f, 0.84f, -0.36f);
    drawLineDDA(0.79f, -0.29f, 0.79f, -0.36f);
    drawLineDDA(0.84f, -0.29f, 0.84f, -0.36f);
    drawLineDDA(0.815f, -0.29f, 0.815f, -0.36f);
    drawLineDDA(0.79f, -0.325f, 0.84f, -0.325f);

    // Small decorative pine tree - left of house
    glColor3ub(34, 139, 34);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.48f, -0.52f);
    glVertex2f(0.52f, -0.52f);
    glVertex2f(0.50f, -0.46f);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(0.485f, -0.48f);
    glVertex2f(0.515f, -0.48f);
    glVertex2f(0.50f, -0.43f);
    glEnd();
    // Tree trunk
    glColor3ub(101, 67, 33);
    glBegin(GL_QUADS);
    glVertex2f(0.495f, -0.52f);
    glVertex2f(0.505f, -0.52f);
    glVertex2f(0.505f, -0.48f);
    glVertex2f(0.495f, -0.48f);
    glEnd();

    // Small decorative pine tree - right of house
    glColor3ub(34, 139, 34);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.89f, -0.52f);
    glVertex2f(0.93f, -0.52f);
    glVertex2f(0.91f, -0.46f);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(0.895f, -0.48f);
    glVertex2f(0.925f, -0.48f);
    glVertex2f(0.91f, -0.43f);
    glEnd();
    // Tree trunk
    glColor3ub(101, 67, 33);
    glBegin(GL_QUADS);
    glVertex2f(0.905f, -0.52f);
    glVertex2f(0.915f, -0.52f);
    glVertex2f(0.915f, -0.48f);
    glVertex2f(0.905f, -0.48f);
    glEnd();
}
void vehicle() // This is the truck
{

    glPushMatrix();
    glTranslatef(truckPosition, 0.0f, 0.0f);

    // body
    glColor3ub(0, 255, 255);
    glBegin(GL_POLYGON);
    glVertex2f(-0.9f, -0.86f);
    glVertex2f(-0.68f, -0.86f);
    glVertex2f(-0.68f, -0.67f);
    glVertex2f(-0.9f, -0.67f);
    glEnd();

    // base of the truck
    glColor3ub(0, 0, 0);
    glBegin(GL_POLYGON);
    glVertex2f(-0.9f, -0.86f);
    glVertex2f(-0.58f, -0.86f);
    glVertex2f(-0.58f, -0.9f);
    glVertex2f(-0.9f, -0.9f);
    glEnd();

    // front
    glColor3ub(240, 0, 0);
    glBegin(GL_POLYGON);
    glVertex2f(-0.67f, -0.86f);
    glVertex2f(-0.58f, -0.86f);
    glVertex2f(-0.58f, -0.79f);
    glVertex2f(-0.61f, -0.735f);
    glVertex2f(-0.67f, -0.735f);
    glEnd();

    // wheels
    glColor3ub(20, 20, 120);
    circleSolid(-0.84f, -0.88f, 0.034);
    circleSolid(-0.64f, -0.88f, 0.034);
    glColor3ub(130, 130, 130);
    circleSolid(-0.84f, -0.88f, 0.02);
    circleSolid(-0.64f, -0.88f, 0.02);

    // door
    glColor3ub(0, 0, 0);
    glPointSize(5.0);
    drawLineDDA(-.59619, -0.815, -0.61, -0.815);

    // window
    glColor3ub(153, 206, 250);
    glBegin(GL_POLYGON);
    glVertex2f(-0.66, -0.797);
    glVertex2f(-0.596, -0.797);
    glVertex2f(-0.596, -0.781);
    glColor3ub(100, 149, 237);
    glVertex2f(-0.615, -0.747);
    glVertex2f(-0.66, -0.747);
    glEnd();
    glPopMatrix();
}

void hut2()
{
    // Main wall - red brick
    glColor3ub(180, 80, 70);
    glBegin(GL_QUADS);
    glVertex2f(-0.79f, -0.72f);
    glVertex2f(-0.62f, -0.72f);
    glVertex2f(-0.62f, -0.50f);
    glVertex2f(-0.79f, -0.50f);
    glEnd();

    // Roof - dark red
    glColor3ub(140, 50, 50);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.81f, -0.50f);
    glVertex2f(-0.60f, -0.50f);
    glVertex2f(-0.705f, -0.28f);
    glEnd();

    // Roof highlight
    glColor3ub(120, 40, 40);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.705f, -0.28f);
    glVertex2f(-0.60f, -0.50f);
    glVertex2f(-0.62f, -0.50f);
    glEnd();

    // Door - wooden
    glColor3ub(101, 67, 33);
    glBegin(GL_QUADS);
    glVertex2f(-0.73f, -0.72f);
    glVertex2f(-0.68f, -0.72f);
    glVertex2f(-0.68f, -0.56f);
    glVertex2f(-0.73f, -0.56f);
    glEnd();

    // Door frame
    glColor3ub(70, 45, 20);
    glPointSize(2.0);
    drawLineDDA(-0.73f, -0.56f, -0.68f, -0.56f);
    drawLineDDA(-0.73f, -0.56f, -0.73f, -0.72f);
    drawLineDDA(-0.68f, -0.56f, -0.68f, -0.72f);

    // Door handle
    glColor3ub(255, 215, 0);
    circleSolid(-0.69f, -0.64f, 0.008f);

    // Window - moved to left side of door
    glColor3ub(135, 206, 250);
    glBegin(GL_QUADS);
    glVertex2f(-0.78f, -0.63f);
    glVertex2f(-0.745f, -0.63f);
    glVertex2f(-0.745f, -0.58f);
    glVertex2f(-0.78f, -0.58f);
    glEnd();

    // Window frame
    glColor3ub(70, 45, 20);
    glPointSize(1.5);
    drawLineDDA(-0.78f, -0.58f, -0.745f, -0.58f);
    drawLineDDA(-0.78f, -0.63f, -0.745f, -0.63f);
    drawLineDDA(-0.78f, -0.58f, -0.78f, -0.63f);
    drawLineDDA(-0.745f, -0.58f, -0.745f, -0.63f);
    drawLineDDA(-0.7625f, -0.58f, -0.7625f, -0.63f);
    drawLineDDA(-0.78f, -0.605f, -0.745f, -0.605f);

    // Small decorative pine tree - left of house
    glColor3ub(34, 139, 34);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.82f, -0.78f);
    glVertex2f(-0.78f, -0.78f);
    glVertex2f(-0.80f, -0.74f);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.815f, -0.76f);
    glVertex2f(-0.785f, -0.76f);
    glVertex2f(-0.80f, -0.72f);
    glEnd();
    // Tree trunk
    glColor3ub(101, 67, 33);
    glBegin(GL_QUADS);
    glVertex2f(-0.805f, -0.78f);
    glVertex2f(-0.795f, -0.78f);
    glVertex2f(-0.795f, -0.76f);
    glVertex2f(-0.805f, -0.76f);
    glEnd();

    // Small decorative pine tree - right of house
    glColor3ub(34, 139, 34);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.62f, -0.78f);
    glVertex2f(-0.58f, -0.78f);
    glVertex2f(-0.60f, -0.74f);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.615f, -0.76f);
    glVertex2f(-0.585f, -0.76f);
    glVertex2f(-0.60f, -0.72f);
    glEnd();
    // Tree trunk
    glColor3ub(101, 67, 33);
    glBegin(GL_QUADS);
    glVertex2f(-0.605f, -0.78f);
    glVertex2f(-0.595f, -0.78f);
    glVertex2f(-0.595f, -0.76f);
    glVertex2f(-0.605f, -0.76f);
    glEnd();
}

// ============================================
// WINDMILL FAN WITH ROTATION TRANSFORMATION
// ============================================
// Three-blade windmill using accumulated rotation
void drawWindmill()
{
    int i;

    glTranslatef(0.53, -0.3, 0);
    glRotated(angle * (180.0 / 46), 0, 0, 1); // Apply rotation transformation
    glColor3f(255.0f, 255.0f, 255.0f);
    for (i = 0; i < 3; i++)
    {
        glRotated(120, 0, 0, 1); // Note: These rotations accumulate.
        glBegin(GL_POLYGON);
        glVertex2f(0.035, 0);
        glVertex2f(0.15f, -0.03f);
        glVertex2f(0.3f, 0.0f);
        glVertex2f(0.15f, 0.03f);
        glEnd();
    }
}

void vehicle2() // This is the car
{
    glPushMatrix();
    glTranslatef(carPosition, 0.0f, 0.0f);
    // body
    glColor3ub(255, 255, 0);
    glBegin(GL_POLYGON);
    glVertex2f(0.0f, -0.94f);
    glVertex2f(0.275f, -0.94f);
    glVertex2f(0.275f, -0.859f);
    glVertex2f(0.22f, -0.842f);
    glVertex2f(0.18f, -0.758f);
    glVertex2f(0.1f, -0.758f);
    glVertex2f(0.08f, -0.842f);
    glVertex2f(0.0f, -0.856f);
    glEnd();
    // wheels
    glColor3ub(233, 0, 0);
    circleSolid(0.07, -0.938, 0.035);
    circleSolid(0.2, -0.938, 0.035);
    //
    glColor3ub(0, 0, 0);
    glPointSize(1.1);
    drawLineDDA(0.141f, -0.758f, 0.141f, -0.842f);
    // window
    glColor3ub(153, 206, 250);
    glBegin(GL_POLYGON);
    glVertex2f(0.071, -0.842);
    glVertex2f(0.132, -0.842);
    glColor3ub(100, 149, 237);
    glVertex2f(0.132, -0.77);
    glVertex2f(0.106, -0.77);
    glEnd();
    glColor3ub(153, 206, 250);
    glBegin(GL_POLYGON);
    glVertex2f(0.148, -0.842);
    glVertex2f(0.206, -0.842);
    glColor3ub(100, 149, 237);
    glVertex2f(0.173, -0.77);
    glVertex2f(0.148, -0.77);
    glEnd();
    // door
    glColor3ub(0, 0, 0);
    glPointSize(5.0);
    drawLineDDA(0.11, -0.863, 0.13, -0.863);
    drawLineDDA(0.186, -0.863, 0.206, -0.863);
    glColor3ub(255, 255, 255);
    glPopMatrix();
}

void LastDesign()
{
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    // menu
    glColor3ub(105, 105, 105);
    glBegin(GL_POLYGON);
    glVertex2f(-.28, -.5);
    glVertex2f(.28, -0.5);
    glVertex2f(0.28, 0.5);
    glVertex2f(-0.28, 0.5);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(-.28, -.5);
    glVertex2f(0.28, 0.5);
    glVertex2f(-0.38, 0.15);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(-.28, -.5);
    glVertex2f(0.28, 0.5);
    glVertex2f(0.38, 0.05);
    glEnd();

    glColor3ub(20, 222, 100);
    renderBitmapString(-.11, 0.06, font1, "Thank You !");
}

// ============================================
// MAIN DISPLAY FUNCTION
// ============================================
// Renders complete scene with all objects and effects
// Order: Sky -> Background -> Shadows -> Objects -> Headlights -> UI
void myDisplay(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Apply zoom transformation
    glScalef(zoomLevel, zoomLevel, 1.0f);

    glPointSize(2.0);
    /*if(start_flag ==0 )
        {

        }*/
    if (start_flag == 0)
    {
        // Render appropriate sky based on sun position
        if (sunPosition <= -0.7)
        {
            night(1);
        }
        else
        {
            sky(1);
        }

        // Draw sun only when it's above horizon (day time)
        if (sunPosition > -0.7)
        {
            sun();
        }

        // Draw moon after sun sets - always visible during night
        if (sunPosition <= -0.7)
        {
            moon();
        }

        grass2();

        river();

        hills();

        grass1();

        road1();

        road2();

        clouds1();

        clouds2();

        // Draw shadows before objects
        drawShadows();

        hut();

        bridge();

        tree();

        ship();

        hut2();

        // Vehicle headlights before vehicles
        drawVehicleHeadlights();

        vehicle();

        vehicle2();

        windMills();

        windmilStick();

        drawWindmill();

        // UI overlay (reset scale for UI)
        glLoadIdentity();
        drawUI();
    }

    else if (start_flag == 1)
    {
        LastDesign();
    }

    glFlush();
}

// ============================================
// KEYBOARD INTERACTION HANDLER
// ============================================
// Controls: A/S (truck), D/F (car), P (pause), +/- (speed),
//           Z/X/C (zoom), SPACE (toggle), ENTER (exit)
void myKeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {

    case 'a': // press a to slow truck
    case 'A':
        if (truckSpeed > 0.01f)
            truckSpeed -= 0.02f;
        break;

    case 's': // press s to fast truck
    case 'S':
        if (truckSpeed < 0.15f)
            truckSpeed += 0.02f;
        break;

    case 'd': // press d to slow car
    case 'D':
        if (carSpeed > 0.01f)
            carSpeed -= 0.02f;
        break;

    case 'f': // press f to fast car
    case 'F':
        if (carSpeed < 0.15f)
            carSpeed += 0.02f;
        break;

    case 'p': // Pause/Resume
    case 'P':
        isPaused = !isPaused;
        break;

    case '+': // Speed up time
    case '=':
        if (timeSpeed < 5.0f)
            timeSpeed += 0.5f;
        break;

    case '-': // Slow down time
    case '_':
        if (timeSpeed > 0.5f)
            timeSpeed -= 0.5f;
        break;

    case 'z': // Zoom in
    case 'Z':
        if (zoomLevel < 2.0f)
            zoomLevel += 0.1f;
        break;

    case 'x': // Zoom out
    case 'X':
        if (zoomLevel > 0.5f)
            zoomLevel -= 0.1f;
        break;

    case 'c': // Reset zoom
    case 'C':
        zoomLevel = 1.0f;
        break;

    case ' ':
        if (start_flag == 0)
        {
            start_flag = 1;
        }
        else if (start_flag == 1)
        {
            start_flag = 2;
        }
        break;

    case 10:
        exit(0);
        break;

    default:
        break;

        glutPostRedisplay();
    }
}
void myInit(void)
{
    glClearColor(0.5f, 0.5f, 0.5f, 1);
}

// ============================================
// MAIN PROGRAM ENTRY POINT
// ============================================
// Initializes OpenGL/GLUT and starts animation timers
int main(int argc, char **argv)
{

    cout << "\n========================================" << endl;
    cout << "   SCENIC LANDSCAPE SIMULATION" << endl;
    cout << "   Computer Graphics Project" << endl;
    cout << "   ENHANCED VERSION with Effects" << endl;
    cout << "========================================\n"
         << endl;

    cout << "CORE FEATURES:" << endl;
    cout << "  - Day/Night Cycle with Sun & Moon" << endl;
    cout << "  - Animated Vehicles & Windmill" << endl;
    cout << "  - DDA Line Drawing Algorithm" << endl;
    cout << "  - Midpoint Circle Algorithm" << endl;
    cout << "  - Cohen-Sutherland Clipping" << endl;
    cout << "  - Shear Transformation (Wind Effect)" << endl;
    cout << "  - Reflection (Water Surface)\n"
         << endl;

    cout << "NEW ENHANCEMENTS:" << endl;
    cout << "  💡 Vehicle Headlights" << endl;
    cout << "  🎬 Pause & Time Speed Control" << endl;
    cout << "  🔍 Zoom In/Out" << endl;
    cout << "  📊 Real-Time UI (Time, Day, FPS)" << endl;
    cout << "  🌑 Dynamic Shadows" << endl;
    cout << "  🎵 Background Music\n"
         << endl;

    cout << "VEHICLE CONTROLS:" << endl;
    cout << "  [A] - Slow down Truck" << endl;
    cout << "  [S] - Speed up Truck" << endl;
    cout << "  [D] - Slow down Car" << endl;
    cout << "  [F] - Speed up Car\n"
         << endl;

    cout << "SCENE CONTROLS:" << endl;
    cout << "  [P] - Pause/Resume Animation" << endl;
    cout << "  [+] - Speed up Time (faster day/night)" << endl;
    cout << "  [-] - Slow down Time" << endl;
    cout << "  [Z] - Zoom In" << endl;
    cout << "  [X] - Zoom Out" << endl;
    cout << "  [C] - Reset Zoom" << endl;
    cout << "  [SPACE] - Toggle Screen" << endl;
    cout << "  [ENTER] - Exit Program\n"
         << endl;

    cout << "Watch the UI at top of screen for live info!" << endl;
    cout << "Enjoy the enhanced scenic view! 🌄✨\n"
         << endl;

    // ============================================
    // START BACKGROUND MUSIC
    // ============================================
    // Play Music.mp3 in loop mode using MCI (Media Control Interface)
    // mciSendString supports MP3 format unlike PlaySound which only supports WAV
    mciSendString(TEXT("open \"Music.mp3\" type mpegvideo alias mp3"), NULL, 0, NULL);
    mciSendString(TEXT("play mp3 repeat"), NULL, 0, NULL);
    cout << "🎵 Background music started...\n"
         << endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(1200, 680);
    glutInitWindowPosition(50, 10);
    glutCreateWindow("Enhanced Scenic Landscape | Computer Graphics Project");

    myInit();

    glutDisplayFunc(myDisplay);

    glutKeyboardFunc(myKeyboard);

    glutTimerFunc(1000, updateBoat, 0);
    glutTimerFunc(1000, updateCloud, 0);
    glutTimerFunc(1000, updateCloud1, 0);
    glutTimerFunc(1000, updateFan, 0);
    glutTimerFunc(1000, updateTruck, 0);
    glutTimerFunc(1000, updateCar, 0);
    glutTimerFunc(1000, updateSun, 0);
    glutTimerFunc(1000, updateMoon, 0);
    glutTimerFunc(50, updateWindShear, 0); // Wind shear effect
    glutTimerFunc(50, updateWaterWave, 0); // Water wave effect

    glutMainLoop();
}
