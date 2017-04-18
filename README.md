# openvr-customhmd
DIY attempt for building OpenVR compatible HMD and controllers. 

I started my VR experience with a custom built HMD (by using internal parts of a Sony HMZ-T2 and Arduino based HT), but only few games were supporting FreeTrack-like protocols. So I decided to write a complete OpenVR driver which hopefully will support newer games.

### Some features:
* Tracking and controller hardware is based on STM32F103.
* Supporting framepacked 3D signal for HMD's with two monitors or 3D TV's. (double horizontal resolution :)
* DirectMode supporting driver for Android phone based orientation tracking and display. (in progress...)
* Lighthouse-like positional tracking, (or sound TimeOfFlight based, still experimenting...)


_After long resisting to buy one of the expensive but ready VR solutions, I finally bought a Vive (for twice of the actual price :( ).
Unlike my older Sony headset, and unlike my phones screens, the ones in this expensive HMD are crap; the screen-door effect is annoying, the lenses generate too much flare when there are bright areas in corners etc. Seems positional tracking is the only thing I paid for :( Shame on you htc (or on me, for buying such an immature product). I'm going to sacrifice FPS for readability, and use my phone as display, after finishing the positional tracking part._

