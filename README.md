buld instructions: you may create all the build files by running from root dir: python3/py scripts/scan_build_project.py

you could also build the project using cmake extension hotkeys for wsl/windows
hotkeys: ctrl+shift+p select configure preset / run  without debugging

HW8 file notes (AUTHOR: JUNSEOK LEE)
- source/Demo/DemoDepthPost.hpp: HW8 demo state declaration for depth sorting, MSAA render targets, and post-processing controls.
- source/Demo/DemoDepthPost.cpp: HW8 demo implementation, draw order logic, scene setup, MSAA resolve, and post-processing passes.
- Assets/shaders/HW8/sprite.vert: Sprite vertex shader with per-instance depth output.
- Assets/shaders/HW8/sprite.frag: Sprite fragment shader with tinting.
- Assets/shaders/HW8/fullscreen.vert: Fullscreen triangle vertex shader for post-processing.
- Assets/shaders/HW8/chromatic.frag: Chromatic aberration post effect.
- Assets/shaders/HW8/vignette.frag: Vignette post effect.
- Assets/shaders/HW8/grain.frag: Film grain + scanline post effect.
- Assets/shaders/HW8/gamma.frag: Gamma correction post effect.
