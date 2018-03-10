# Assets, Graphics and Animation (2018-01-10)
Following the theme of do-it-yourself, I have written an image file reader and writer. It operates on Portable Arbitrary Map (`.pam`) files. The format is very simple, with a short plain-text header and uncompressed contents. As this game does not rely on high resolution graphics, it is perfectly reasonable to use uncompressed data for graphical assets.

Now that I can easily import (and export) pixel data, I have fleshed out the rendering system. At its core, my renderer has an internal pixel buffer. This buffer is where everything is rendered. Finally, this entire buffer is displayed on screen. This step can done by sending the pixel data to the OS, or by copying the data to a streaming texture in video memory and displaying with the GPU.

#### Animation
Rendering still images is very useful, but I would like to produce animations that are made up of multiple frames. In order to achieve this, I first need to consider how I will deliver this data to the program.

Bitmap rendering in this project is often concerned with raw pixels. The formula `x + y * width` is employed throughout the renderer to access, set, and copy pixels. `width` is actually not the most correct way to describe this formula; `pitch` is more correct than `width`. The difference is that while you might want to draw an image of width `20px`, The pixels of that image might have come from a much larger image. This is a very common feature of renderers often called a texture atlas (or sprite sheet): an image that holds many images packed together.

I noticed that I could lean on my previous image rendering code if I made my `pitch` and `width` equal. This can be done by packing in images vertically; each consecutive frame is underneath the previous. I would simply have to pass in a pointer to the top-left-most pixel of the frame that I want. It is a simple equation to calculate this:

```C
// To render frame 2 of an animation:
int animation_frame = 2;
int pixels_per_frame = width * height;
int pixel_offset_to_current_frame = pixels_per_frame * animation_frame;
u32 * final_pointer = pointer_to_start_of_image + pixel_offset_to_current_frame;
```
