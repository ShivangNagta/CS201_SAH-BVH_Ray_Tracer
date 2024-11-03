# CS201_SAH-BVH_Ray_Tracer

This is the final project submission for the CS201 (Data Structure) Course.

This project aims to improve the efficiency of ray intersection tests in ray tracing by building upon a bounding volume hierarchy (BVH) structure for storing the scene primitives. Further Surface Area Heuristics (SAH) is used for efficient building of the BVH. The raytracer is just using CPU as of now. A naive implementation of microfacet model is used for material properties. Only diffuse materials are added for now with predefined values. Other material properties are not included for simplicity.


## Dependencies
- **SDL2**: Cross platform header only Multimedia library for rendering the calculated frames and event handling features like user input
- **Gnuplot**: Graph plotting library in c

# Setup Instructions 

## Requirements

### macOS

- **Homebrew**: Package manager for macOS, used to install dependencies.
- **SDL2** and **Gnuplot**: SDL2 for rendering, and Gnuplot for benchmarking plots.

### Windows

- **Gnuplot**: Required for benchmark data plot.
- **SDL2**: SDL2 headers and DLLs are included in the project, so no additional setup is required.

## Setup and Installation

### macOS

1. **Install Homebrew** if you haven’t already:
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```
2. Install SDL2 and Gnuplot via Homebrew:
   ```bash
   brew install sdl2 sdl2_image gnuplot
   ```
3. Clone the Repository:
   ```bash
   git clone https://github.com/ShivangNagta/CS201_SAH-BVH_Ray_Tracer.git
   ```
   ```bash
   cd CS201_SAH-BVH_Ray_Tracer
   ```
4. Build the project (make should be pre-installed with Xcode, if not do install it first)
   ```bash
   make
   ```
5. Run the Raytracer
   ```bash
   ./raytracer
   ```

### Windows

1. Install **Gnuplot**:<br>
   Download the Gnuplot installer from [SourceForge](https://sourceforge.net/projects/gnuplot/) and ensure that the option to add Gnuplot to your system path is selected during installation.

2. Install **MinGW**:
   Install MinGW from the [MinGW website](http://www.mingw.org/) if not installed. During installation, select `mingw32-make` and add it to your system path.

2. Clone the Repository:
   ```bash
   git clone https://github.com/ShivangNagta/CS201_SAH-BVH_Ray_Tracer.git
   ```
   ```bash
   cd CS201_SAH-BVH_Ray_Tracer
   ```
4. Build the project
   ```bash
   make
   ```
5. Run the Raytracer
   ```bash
   .\raytracer
   ```

### Linux

#### Not tested

# How to Use the Application
Upon launching, the program will display options to choose the desired mode:<br>

**Press '1' for benchmark testing with graph plot.**<br>
**Press '2' for real-time CPU ray tracing.**<br>

Option 1: Benchmark Testing with Graph Plot

- Type 1 and press Enter:
  The program will run a benchmark test to measure the intersection performance of rays with randomly generated spheres, both with and without a bounding volume hierarchy (BVH). Results are plotted using gnuplot, saved as   benchmark_results.png, and displayed in an SDL window. To close the plot, press the ESC key or click the close button on the SDL window. The instructions for testing are set in benchmark.c file.

Option 2: Real-Time Ray Tracing
- Type 2 and press Enter:
  A real-time ray tracing window will open, allowing you to explore the scene with keyboard and mouse inputs.
Controls:


| Key                    | Action                                             |
|------------------------|----------------------------------------------------|
| **W**                  | Move camera forward                                |
| **S**                  | Move camera backward                               |
| **A**                  | Move camera left                                   |
| **D**                  | Move camera right                                  |
| **Space**              | Move camera up                                     |
| **Left Shift**         | Move camera down                                   |
| **B**                  | Toggle BVH on/off                                  |
| **O**                  | Toggle BVH visualization                           |
| **Mouse** (hold left-click) | Rotate the camera view by moving the mouse     |
| **ESC**                | Close the application window.                      |


## Screenshots and Results

<p align="center">
  <img width="796" alt="Ray Tracer Output" src="https://github.com/user-attachments/assets/65a276f8-f0ca-49e8-b37c-e9b7317ee6b4">
</p>

<p align="center">
  <img width="987" alt="Complexity Analysis" src="https://github.com/user-attachments/assets/76585d93-f58d-4939-9c68-02093f8adb07">
</p>

<p align="center">
  <img width="987" alt="BVH Intersection Test" src="https://github.com/user-attachments/assets/9351f1c2-5951-4e6f-b4a1-96ed9a8cdf4a">
</p>

<p align="center">
  <img width="796" alt="Additional Result 1" src="https://github.com/user-attachments/assets/9e7bf9f7-b1ce-4c90-a051-772a6d6651e2">
</p>

<p align="center">
  <img width="793" alt="Additional Result 2" src="https://github.com/user-attachments/assets/cf9e8c17-80bf-4319-9279-8720fa968389">
</p>
