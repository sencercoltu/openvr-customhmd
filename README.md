# openvr-customhmd
DIY attempt for building OpenVR compatible HMD and controllers, and drivers (Windows only). 

I started my VR experience with a custom built HMD by using parts of a disassembled [Sony HMZ-T2](http://www.sony.co.uk/pro/product/semiconductors-head-mounted-display/hmz-t2/overview/), an Arduino based Head Tracker, and [OpenTrack](https://github.com/opentrack/opentrack) software. But only few games were supporting FreeTrack-like protocols. So I decided to write a complete OpenVR driver which hopefully will support newer games, and I hope I will be able to play some VR games with friends who cannot afford ridiculously expensive hardware.  

### Some features:
* Tracking and controller hardware is based on STM32F103.
* Supporting framepacked 3D signal for HMD's with two monitors or 3D TV's. (double horizontal resolution :+1:)
* IVRDirectModeComponent supporting driver for Android phone based orientation tracking and display. (in progress...)
* Lighthouse-like positional tracking, (or sound TimeOfFlight based, still experimenting, also in progress...)
* Expose any USB WebCam (YUY, YUV or RGB) to OpenVR (for Room-View or other camera functions).
* Autodetect monitor position and size in Extended mode for placing the HeadSet Window. 
* .
* ...

### Dependencies:
* ffmpeg from NuGet, for compiling the driver
* dotNet 4.6.2, for the HMD monitor program.
* VC2015 Runtime, for using the driver


_After long resisting to buy one of the expensive but ready VR solutions, I finally bought a Vive (for twice of the actual price :sob: ).
Unlike my older Sony headset's and unlike both my phones screens (G5, Z3), the ones in this expensive HMD are crap; the screen-door effect is annoying, the lenses generate too much flare when there are bright areas in corners etc. Text is unreadable even when doing 2x oversampling. Seems positional tracking is the only thing I paid for :angry:. Shame on me, for buying such an immature product for such a price. I'm going to sacrifice some FOV, FPS and latency for readability, and use my phone as output display for playing games like Elite Dangerous or DCS, which are uncomfortable to play on the Vive because it gives me eye pains. I used to play ED for hours with the orientation-only DIY'ed Sony ..._

