# GLWall

## How to use
Make sure you have gcc installed and run `make`
Then you can use GLWall like this:
`./GLWall <path_to_shader> [<path_to_texture>]`

This will render the given shader to a window with the settings specified in `config.h`

To use as a wallpaper, you can use [xwinwrap](https://github.com/mmhobi7/xwinwrap) or whatever window manager dependant trickery you want lol

The shader rendering can be paused with `pkill -SIGUSR1 GLWall ` and unpaused with `pkill -SIGUSR2 GLWall` so you can bind that to whatever works with your system (it's advisable to pause rendering when anything is in fullscreen mode or when playing a game in general)