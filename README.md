# 3D-Food-Stall
A fully interactive 3D scene built with **OpenGL**, **GLFW**, **GLAD**, and **GLM**.   The scene features a wooden food stall with a large **"FOOD STALL"** signboard, a central table, and two chairs placed on either side – perfect for a street food visualisation.
## 📌 Overview

This project demonstrates a complete 3D environment with:

- A **wooden table** (centered) with four legs.
- Two **chairs** (left & right of the table) with armrests and backrests.
- A **food stall** consisting of a counter, back wall, red roof, side posts, and a prominent **signboard** on top.
- Dynamic **Phong lighting** (ambient, diffuse, specular) with a movable light source.
- A **ground grid** for spatial reference.
- **First‑person camera** (WASD + mouse + scroll).

All 3D models are built from **cubes** (no external models needed). The signboard texture is generated **procedurally** (no external images required).

## ✨ Features

- ✅ Fully self‑contained – no external textures or models  
- ✅ Real‑time lighting with specular highlights  
- ✅ Smooth first‑person camera (mouse look, WASD movement, zoom)  
- ✅ Procedurally generated signboard with large “FOOD STALL” text  
- ✅ Clean, organised code with a reusable `Shader` class  
- ✅ Cross‑platform (Windows / Linux / macOS with appropriate build setup)

## 📦 Dependencies

Make sure you have the following installed:

| Library | Purpose | Installation (Ubuntu / Debian) |
|---------|---------|-------------------------------|
| [GLFW](https://www.glfw.org/) | Window & input management | `sudo apt install libglfw3-dev` |
| [GLAD](https://glad.dav1d.de/) | OpenGL function loading | Included via `glad.h` (source provided) |
| [GLM](https://github.com/g-truc/glm) | Mathematics for 3D | `sudo apt install libglm-dev` |
| OpenGL | Graphics API | Usually pre‑installed |

> **Note:** On Windows, link against `glfw3.lib`, `opengl32.lib`, and `glad.lib`.  
> On macOS, use `-framework OpenGL -lglfw`.

## 🚀 Compilation & Execution

Clone the repository and compile with **g++**:

```bash
git clone https://github.com/yourusername/3d-food-stall.git
cd 3d-food-stall
g++ -o foodstall main.cpp -lglfw -lGL -ldl -lglm
./foodstall

