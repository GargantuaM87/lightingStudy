# First OpenGL Project

This is a simulation of Phong lighting in OpenGL, utilizing components such as ambient, diffuse, and specular lighting. This project is tiny. It consists of some floating cubes, an interactive camera, an ImGui interface for manipulating data, two point lights, a directional light, and a spot light. I plan to gradually fix some features and add more as I continue my journey into Computer Graphics. Feel free to build this project and give it a try. If there is anything that piques your interest or you'd like to criticize, please email me at ayindeabrams@gmail.com.

## Required Software
- Git
- A C/C++ Compiler (I use GCC in this case)
- CMake
- GLFW
- Assimp
### Installing GLFW
For a stable install of GLFW on Ubuntu:
```bash
sudo apt update
sudo apt install libglfw3-dev libglfw3
```
This will install the main library and development headers for GLFW.

For installation on macOS:
```bash
brew install glfw
```
And if you don't have the Homebrew package manager, then get it!

### Installing Assimp
For command-line tools and the header files on Ubuntu:
```bash
sudo apt-get install libassimp-dev assimp-utils
```

For installation on macOS:
```bash
brew install assimp
```

## External Libraries
This project uses:
- GLFW
- GLM
- GLAD
- stb
- Dear ImGui
- Assimp

## Installation Steps
1. Clone this repository, then head inside its directory
```bash
git clone https://github.com/GargantuaM87/OpenGL-Light-Simulation.git
cd OpenGL-Light-Simulation
```
2. Create a ./build directory
```bash
mkdir build
cd build
```
3. Configure the project with CMake
```bash
cmake ..
```

4. Finally, build the project!
```bash
cmake --build .
```
Make sure the project is run on ./main.cpp from either the command line or whatever other method you can utilize
**WARNING:** Everything might not build correctly. So it's important that you link the project to the appropriate path of glfw3 when you install it!

## Screenshots
<img width="902" height="938" alt="image" src="https://github.com/user-attachments/assets/e86445fb-ccd5-4100-9050-a3e85718f84e" />


As you can see so far, there's not much to do. As I continue learning, I'll update this project accordingly with new stuff to showcase what I've learned!



