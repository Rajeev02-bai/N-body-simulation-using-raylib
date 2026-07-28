# N-Body Gravity Simulation
 
A real-time N-body gravitational simulation written in C using [raylib](https://www.raylib.com/) for rendering.
 
## Overview
 
This project simulates gravitational interaction between multiple objects (bodies) in a 2D space, using Newtonian gravity to compute attraction between each pair of objects. Each body tracks its position, velocity, acceleration, mass, and radius, and leaves behind a fading trail showing its recent path.
 
## Features
 
- Real-time gravity simulation using Newton's law of universal gravitation
- Per-object motion trails (configurable length)
- 2D camera system (pan/zoom over a world larger than the screen)
- Dynamic object array (grows as needed, no fixed object cap)
- Simple velocity damping to control simulation stability
## Dependencies
 
- GCC (or any C compiler)
- [raylib](https://www.raylib.com/) — install and build from source, or via your package manager
- Linux libraries: `libGL`, `libpthread`, `libdl`, `librt`, `libX11`
On Kali/Debian-based systems:
```bash
sudo apt install libgl1-mesa-dev libx11-dev
```
 
## Building
 
Run the included build script:
 
```bash
chmod +x build.bash
./build.bash
```
 
This compiles `Nbody.c` into an executable named `out` using:
 
```bash
gcc Nbody.c -o out -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```
 
> **Note:** Scripts edited on Windows may end up with CRLF line endings, which breaks execution on Linux (`bad interpreter` or `$'\r': command not found` errors). If that happens, run `sed -i 's/\r$//' build.bash` to fix it.
 
## Running
 
```bash
./out
```
 
## Configuration
 
Key constants can be tuned at the top of `Nbody.c`:
 
| Constant | Description |
|---|---|
| `WORLD_WIDTH` / `WORLD_HEIGHT` | Size of the simulated world |
| `WIDTH` / `HEIGHT` | Window resolution |
| `G` | Gravitational constant |
| `DAMPING` | Velocity damping factor |
| `TRAIL_LENGTH` | Number of points kept per object's trail |
 
## Controls
 
1.Left Click to place object.
2.To increase mass and radius use your scroll wheel while cursor is still on the object.
3.(Left/Right)Ctrl + scroll to zoom in and out.
 
## License
 
Add a license of your choice here.
