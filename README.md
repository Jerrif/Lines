# Lines: Image to Abstract Art Generator

A for-fun image processing tool written in C. This started as my attempt at a programming challenge I found online with the following rules:
* Load an image
* Create a new blank image
* Draw a line of a random color at a random location on the new image
* Compare the new image to the source image
* If the new image is more similar to the source after drawing the line, then keep the changes, otherwise discard them
* Repeat many thousands of times until you end up with an image resembling the source image (but cooler!)


https://github.com/user-attachments/assets/9fc13ee5-512e-49fb-b378-dbbbd964b9e1


Note: the intention is not to perfectly recreate the source image; the random sampling is the fun part!

I ended up really enjoying making it so I kept adding features as a way to learn as much about C as I could.

## Features

- **Loads of Customizable Parameters**:
  - Control line length and iteration count
  - Specify line orientation
  - Choose between an even or weighted color distribution
  - Set an RNG seed
- **Multi-threaded Processing**: Leverages parallel processing for optimized performance on multi-core systems (one image per thread)
* **Animation Mode**: Creates ~150 images at various points in the process, which can be combined to create a flashy video
- **Multiple Input Formats**: Supports PNG and JPG image inputs via integrated image libraries
- **Built-in Documentation**: Use the `-h` flag to give a detailed description of all command line options

![iterations: 100,000, line size: 200px](https://github.com/Jerrif/Lines/blob/45b0dd1518d0a827f1ff872bf9765dd3a99be808/output/eva%20i%3D0.1m%20s%3D200.png?raw=true "iterations: 100,000, line size: 200px")

## Technical Implementation

### Architecture
- **Language**: C (C11 standard)
- **Threading**: Windows multi-threading (via `process.h`)
- **Image Processing**: 
  - PNG encoding/decoding via lodepng
  - JPEG support via stb_image
  - Efficient color space calculations using RGB distances

### Key Components
- `lines_threaded.c`: Core algorithm and main processing engine
- `rgb_struct.h`: RGB color structure definitions
- `hash_tables.h`: Hash table implementation for color tracking optimization

## Performance Considerations

- Multi-threaded processing distributes workload across available CPU cores
- Customizable thread count for hardware optimization
- Iteration count directly affects quality vs. computation time trade-off
- Larger line sizes give a more abstract look but require more computation for the same iteration count

## Example Outputs

The `output/` directory contains sample renderings demonstrating the project's capabilities:
- Various artistic interpretations with different iteration and size parameters
- Animated sequences showing progressive reconstruction

![iterations: 1,000,000, line size: 200px](https://github.com/Jerrif/Lines/blob/45b0dd1518d0a827f1ff872bf9765dd3a99be808/output/mononoke%20i%3D1m%20s%3D100%20(no%20sqrt).png?raw=true "iterations: 1,000,000, line size: 200px")

## Building and Running

### Prerequisites
- GCC compiler
- Windows environment

### Compilation
Just run `make`

Then simply invoke the executable:
```bash
lines.exe -i <iterations> -s <size> [options]
```
