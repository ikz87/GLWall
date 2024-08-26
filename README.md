# GLWall

## How to use
You'll have to install the following C libraries:
- [glfw](https://archlinux.org/packages/extra/x86_64/glfw/)
- [stb](https://archlinux.org/packages/extra/any/stb/)
- [glew](https://archlinux.org/packages/extra/x86_64/glew/)

Then, make sure you have gcc installed and run `make`

Once compiled, you can use GLWall like this:

`./GLWall <path_to_shader>` for shaders in `regular_shaders/`

`./GLWall <path_to_shader> <path_to_texture>` for shaders in `textured_shaders/`, where the texture path leads to an image file (I've tested jpegs and pngs)

This will render the given shader to a window with the settings specified in `config.h`

To use as a wallpaper, you can use [xwinwrap](https://github.com/mmhobi7/xwinwrap) or whatever window manager dependant trickery you want lol

The shader rendering can be paused with `pkill -SIGUSR1 GLWall` and unpaused with `pkill -SIGUSR2 GLWall` so you can bind that to whatever works with your system (it's advisable to pause rendering when anything is in fullscreen mode or when playing a game in general)
