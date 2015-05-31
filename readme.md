# Matrixman

Matrixman is low-resolution take on the classic game of Pac-Man. I've been calling this "1-Pixel Pac-Man" because the player is just
1-pixel in size. I wrote all of the code for the game, following along with the rules of the original as are described by the 
[Pac-Man Dossier](http://home.comcast.net/~jpittman2/pacman/pacmandossier.html).

![Smartmatrix playing 1-Pixel-Pacman](https://cdn.hackaday.io/images/3899461432957388260.JPG)

The display used in this project is a Smartmatrix. It's a 32x32 RGB LED panel driven by a Teensy 3.1 Microcontroller. I soldered a DB9
connector to the Teensy 3.1 in order to interface an Atari joystick as the user input.

Full build information can be found on [my Hackaday.io project page](https://hackaday.io/project/6036-1-pixel-pac-man).

## Main Branch

The main branch is designed for Teensy 3.1 and can be compiled using the Arduino IDE and a few libraries:

* [Arduino IDE](http://www.arduino.cc/en/Main/Software)
* [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html)
* [Smartmatrix Library](https://github.com/pixelmatix/SmartMatrix)

## Hardware Emulation Branch

I actually coded most of this without the hardware in hand. To do so I wrote an emulator using SDL2. It's simply a grid of
32x32 boxes that serves as a stand-in for the SmartMatrix. The game code itself is rather complex and this really sped up
development time. Because of this I've kept the code in this repository abstracted so that game changes can easily be merged
between branches.

### To run the game emulator

Install the SDL2 library:

```sudo apt-get install libsdl2-dev```

Launch the emulator by typing `make`

### To Merge Changes Between Branches

The only file that should be different between the 'master' (teensy 3.1) and the hardware_emulator (SDL2 based) branches
is the matrixman.c which includes all of the hardware specific functions. If additional functions are needed these should
be implemented in both branches (even if the function is blank) and added to the matrixman.h headerfile. All other files
should be merged between branches to keep the repo current.
