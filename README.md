# 🌄 Scenic Landscape Simulation

A comprehensive **Computer Graphics Project** featuring a dynamic day/night cycle, animated vehicles, and realistic environmental effects using **OpenGL/GLUT** in C++.

![OpenGL](https://img.shields.io/badge/OpenGL-FFFFFF?style=for-the-badge&logo=opengl)
![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![GLUT](https://img.shields.io/badge/GLUT-FFA500?style=for-the-badge)

---

## 📋 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [Core Algorithms](#-core-algorithms)
- [Transformations](#-transformations)
- [Controls](#-controls)
- [Installation](#-installation)
- [Building & Running](#-building--running)
- [Project Structure](#-project-structure)
- [Technical Details](#-technical-details)
- [Author](#-author)

---

## 🎯 Overview

This project demonstrates fundamental **Computer Graphics** concepts including line drawing algorithms, circle rendering, geometric transformations, and complex animations. The scene depicts a scenic landscape with houses, trees, a river, roads, vehicles, and a windmill, featuring a realistic day/night cycle with dynamic lighting effects.

### Key Highlights

- ✅ **All 5 transformation types** implemented
- ✅ **3 core algorithms**: DDA Line Drawing, Midpoint Circle, Cohen-Sutherland Clipping
- ✅ **8+ animations** running simultaneously
- ✅ **Dynamic lighting**: Vehicle headlights at night, dynamic shadows during day
- ✅ **Background music**: Continuous ambient soundtrack throughout the experience
- ✅ **Interactive controls**: Pause, zoom, time speed control
- ✅ **Professional polish**: Gradients, reflections, shimmer effects

---

## ✨ Features

### 🌅 Day/Night Cycle

- **Automatic sun and moon movement** with synchronized transitions
- **10-second pause** at zenith/nadir for extended viewing
- **Dynamic sky gradients** transitioning from blue to midnight
- **Twinkling stars** visible during night
- **Day counter** tracking complete cycles

### 🚗 Animated Objects

| Object         | Animation Type | Description                          |
| -------------- | -------------- | ------------------------------------ |
| 🚚 Truck       | Translation    | Moves right on upper road            |
| 🚙 Car         | Translation    | Moves left on lower road             |
| ⛵ Boat        | Translation    | Sails through river                  |
| ☁️ Clouds (2x) | Translation    | Drift across sky at different speeds |
| 🌀 Windmill    | Rotation       | Continuous blade rotation            |
| 🌳 Trees       | Shear          | Gentle wind sway effect              |
| 🌊 Water       | Reflection     | Shimmer and sun reflection           |

### 💡 Lighting & Effects

- **Vehicle Headlights**: Triangular light cones appear automatically at night (when `sunPosition ≤ -0.7`)
- **Dynamic Shadows**: Houses and trees cast shadows during daytime based on sun position
- **Water Reflection**: Sun reflection with wave distortion in river
- **Wind Effect**: Trees sway gently using shear transformation
- **Background Music**: Continuous ambient soundtrack playing throughout the simulation using Windows Multimedia API

### 🎮 Interactive Controls

- **Pause System**: Freeze all animations while maintaining scene state
- **Time Control**: Speed up or slow down day/night cycle (0.5x - 5.0x)
- **Camera Zoom**: Zoom in/out of the scene (0.5x - 2.0x)
- **Vehicle Speed**: Independent control for truck and car speeds
- **Real-time UI**: Display day counter, speed multiplier, pause status, and controls

---

## 🔧 Core Algorithms

### 1. **DDA (Digital Differential Analyzer) Line Drawing**

```cpp
void drawLineDDA(float x1, float y1, float x2, float y2)
```

- **Purpose**: Draws anti-aliased lines using incremental calculations
- **Used for**: Window frames, door frames, road markings, windmill stick
- **Features**: Integrated with Cohen-Sutherland clipping for viewport culling

### 2. **Midpoint Circle Algorithm with Scanline Fill**

```cpp
void circleSolid(float x, float y, float radius)
```

- **Purpose**: Renders filled circles using symmetric properties
- **Used for**: Sun, moon, vehicle wheels, door handles, moon craters
- **Technique**: Scanline filling for solid interior (not just outline)

### 3. **Cohen-Sutherland Line Clipping**

```cpp
bool cohenSutherlandClip(float &x1, float &y1, float &x2, float &y2)
```

- **Purpose**: Clips lines to viewport boundaries before rendering
- **Method**: Uses 4-bit region codes (INSIDE, LEFT, RIGHT, TOP, BOTTOM)
- **Optimization**: Prevents off-screen drawing for performance

---

## 🔄 Transformations

All 5 transformation types are implemented:

| Transformation  | Implementation   | Used For                            |
| --------------- | ---------------- | ----------------------------------- |
| **Translation** | `glTranslatef()` | Vehicles, sun, moon, clouds, boat   |
| **Rotation**    | `glRotated()`    | Windmill blades (120° increments)   |
| **Scaling**     | `glScalef()`     | Camera zoom functionality           |
| **Shear**       | Custom matrix    | Wind effect on trees (sine wave)    |
| **Reflection**  | Water surface    | Sun reflection with wave distortion |

### Shear Matrix Example

```cpp
GLfloat shearMatrix[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    windShear, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};
glMultMatrixf(shearMatrix);
```

---

## 🎮 Controls

### Vehicle Controls

| Key | Action          |
| --- | --------------- |
| `A` | Slow down truck |
| `S` | Speed up truck  |
| `D` | Slow down car   |
| `F` | Speed up car    |

### Scene Controls

| Key       | Action                           |
| --------- | -------------------------------- |
| `P`       | Pause/Resume all animations      |
| `+` / `=` | Speed up time (faster day/night) |
| `-` / `_` | Slow down time                   |
| `Z`       | Zoom in                          |
| `X`       | Zoom out                         |
| `C`       | Reset zoom to default            |
| `SPACE`   | Toggle to thank you screen       |
| `ENTER`   | Exit program                     |

---

## 🛠️ Installation

### Prerequisites

- **MinGW-w64** (with GCC/G++ compiler)
- **OpenGL** libraries
- **GLUT** (FreeGLUT recommended)
- **Windows OS** (for mmsystem.h - sound support ready)

### Setup Steps

1. **Install MinGW-w64**

   ```bash
   # Download from: https://www.msys2.org/
   # Install to: C:\msys64
   ```

2. **Install Required Libraries**

   ```bash
   pacman -S mingw-w64-x86_64-freeglut
   pacman -S mingw-w64-x86_64-gcc
   ```

3. **Clone Repository**
   ```bash
   git clone https://github.com/Fahimfaisaltalha/computer_graphics.git
   cd computer_graphics
   ```

---

## 🚀 Building & Running

### Method 1: Using VS Code Task (Recommended)

```bash
# Open in VS Code
code .

# Press Ctrl+Shift+B (Run Build Task)
# Or manually run:
```

### Method 2: Command Line

```bash
g++ -fdiagnostics-color=always -g main.cpp -o main.exe -lopengl32 -lglu32 -lfreeglut -lwinmm
./main.exe
```

### Method 3: Using VS Code Tasks

The project includes a pre-configured `tasks.json`:

```json
{
  "label": "C/C++: g++.exe build active file with OpenGL",
  "command": "C:\\msys64\\mingw64\\bin\\g++.exe",
  "args": [
    "-fdiagnostics-color=always",
    "-g",
    "${file}",
    "-o",
    "${fileDirname}\\${fileBasenameNoExtension}.exe",
    "-lopengl32",
    "-lglu32",
    "-lfreeglut",
    "-lwinmm"
  ]
}
```

---

## 📁 Project Structure

```
computer_graphics/
│
├── main.cpp                 # Main source file (2300+ lines)
│   ├── Algorithm Implementations
│   │   ├── drawLineDDA()           # DDA line drawing
│   │   ├── circleSolid()           # Midpoint circle with fill
│   │   └── cohenSutherlandClip()   # Line clipping
│   │
│   ├── Scene Components
│   │   ├── sky() / night()         # Sky rendering
│   │   ├── sun() / moon()          # Celestial bodies
│   │   ├── hills()                 # Background hills
│   │   ├── river()                 # Water with reflection
│   │   ├── tree()                  # Trees with wind effect
│   │   ├── hut() / hut2()          # Houses
│   │   ├── vehicle() / vehicle2()  # Truck and car
│   │   ├── ship()                  # Boat
│   │   └── drawWindmill()          # Windmill with rotation
│   │
│   ├── Effects & Enhancements
│   │   ├── drawVehicleHeadlights() # Night lighting
│   │   ├── drawShadows()           # Dynamic shadows
│   │   ├── drawUI()                # User interface overlay
│   │   └── updateWindShear()       # Wind animation
│   │
│   └── Core Functions
│       ├── myDisplay()             # Main render loop
│       ├── myKeyboard()            # Input handling
│       └── main()                  # Initialization
│
├── README.md                # This file
└── .vscode/
    └── tasks.json           # Build configuration
```

---

## 🔬 Technical Details

### Graphics Pipeline

1. **Clear buffer** → `glClear(GL_COLOR_BUFFER_BIT)`
2. **Apply zoom** → `glScalef(zoomLevel, zoomLevel, 1.0f)`
3. **Render sky** (day or night based on `sunPosition`)
4. **Draw background** (hills, river, grass)
5. **Render shadows** (if daytime)
6. **Draw objects** (houses, trees, bridge)
7. **Apply vehicle headlights** (if nighttime)
8. **Draw animated objects** (vehicles, windmill)
9. **Reset transformation** → `glLoadIdentity()`
10. **Render UI overlay**
11. **Flush buffer** → `glFlush()`

### Animation Timing

- **Most animations**: 100ms interval (`glutTimerFunc(100, ...)`)
- **Windmill**: 1ms interval (smooth rotation)
- **Water waves**: 50ms interval (ripple effect)
- **Wind shear**: 100ms with sine wave calculation

### Color Schemes

- **Day Sky**: RGB gradient from `(0.6, 0.7, 0.8)` to `(0.9, 0.8, 0.9)`
- **Night Sky**: RGB gradient from `(0.02, 0.02, 0.15)` to `(0.1, 0.1, 0.3)`
- **Sun**: Bright yellow `(1.0, 1.0, 0.9)` with orange glow
- **Moon**: White `(1.0, 1.0, 1.0)` with gray craters
- **Grass**: Green gradient `(34, 139, 34)` to `(85, 180, 85)`
- **River**: Blue gradient `(50, 110, 180)` to `(100, 150, 220)`

### Performance

- **Frame Rate**: ~60 FPS (GLUT_SINGLE buffer mode)
- **Resolution**: 1200×680 pixels
- **Viewport**: Normalized coordinates (-1.0 to 1.0)

---

## 🎓 Learning Outcomes

This project demonstrates proficiency in:

- ✅ **Graphics Algorithms**: DDA, Midpoint Circle, Cohen-Sutherland
- ✅ **Geometric Transformations**: Translation, Rotation, Scaling, Shear, Reflection
- ✅ **Animation Techniques**: Timer callbacks, state management, interpolation
- ✅ **OpenGL/GLUT**: Primitives, buffers, matrices, blending
- ✅ **Scene Composition**: Layering, depth ordering, visual hierarchy
- ✅ **User Interaction**: Keyboard input, real-time controls
- ✅ **Code Organization**: Modular functions, clear naming, comprehensive comments

---

## 🐛 Known Issues & Future Enhancements

### Current Limitations

- Single buffer mode (no double buffering)
- Fixed window size (no resize handling)
- Windows-only (uses `windows.h` and `mmsystem.h`)

### Planned Features

- 🌧️ **Weather System**: Rain animation (removed due to circular motion bug)
- 🦋 **Additional Wildlife**: Birds, fireflies (previously implemented, removed per design choice)
- 🏠 **More Buildings**: Village expansion with varied architecture
- 🚶 **Walking People**: Pedestrian animations on bridge and roads
- 🔊 **Sound Effects**: Additional sound effects for vehicles and environment

---

## 📝 Requirements Checklist

### Core Requirements (100%)

- [x] **Primitives**: GL_POLYGON, GL_QUADS, GL_TRIANGLES, GL_TRIANGLE_FAN, GL_POINTS
- [x] **Line Drawing**: DDA Algorithm with Cohen-Sutherland Clipping
- [x] **Circle Drawing**: Midpoint Circle Algorithm with Scanline Fill
- [x] **Transformations**: Translation, Rotation, Scaling, Shear, Reflection
- [x] **Animations**: 8+ objects (sun, moon, truck, car, boat, clouds×2, windmill, wind, water)
- [x] **Theme**: Cohesive scenic landscape

### Enhancements (Bonus Features)

- [x] Dynamic lighting (headlights, shadows)
- [x] Interactive controls (pause, zoom, speed)
- [x] Real-time UI overlay
- [x] Background music (continuous ambient soundtrack)
- [x] Advanced effects (gradients, blending, reflections)
- [x] Multiple animation types coordinated
- [x] Comprehensive documentation

---

## 👨‍💻 Author

**Fahim Faisal Talha**

- GitHub: [@Fahimfaisaltalha](https://github.com/Fahimfaisaltalha)
- Repository: [computer_graphics](https://github.com/Fahimfaisaltalha/computer_graphics)

---

## 📄 License

This project is created for educational purposes as part of a Computer Graphics course. Feel free to use it as a learning resource.

---

## 🙏 Acknowledgments

- **OpenGL** - Industry-standard graphics API
- **GLUT/FreeGLUT** - Windowing and input handling
- **MinGW-w64** - Windows GCC compiler toolchain
- Course instructors and Computer Graphics community

---

## 📚 References

- [OpenGL Documentation](https://www.opengl.org/documentation/)
- [FreeGLUT Documentation](http://freeglut.sourceforge.net/docs/api.php)
- [Computer Graphics Principles and Practice](https://www.amazon.com/Computer-Graphics-Principles-Practice-3rd/dp/0321399528)
- Bresenham's and DDA Line Drawing Algorithms
- Midpoint Circle Algorithm (Bresenham's Circle)

---

<div align="center">

### ⭐ If you found this project helpful, please star the repository!

**Made with ❤️ using OpenGL/GLUT**

</div>
