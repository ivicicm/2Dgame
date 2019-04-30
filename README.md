# 2Dgame

The repository contains a solution for visual studio. It uses SDL dlls for graphics - all required 
files to make it work (libraries, header files) are there. The properties for visual studio project
are in the project file, but, at least for me, visual studio uses default properties, so the paths
to SDL have to be added manually. 

Here is how:
1. In visual studio go to project->properties.
2. Insert '$(SolutionDir)\include;' into VC++ Directories -> Include Directories.
3. Insert '$(SolutionDir)\lib\x86;' into VC++ Directories -> Library Directories.
4. Set Linker -> System -> SubSystem to Windows.
5. Insert 'SDL2.lib;SDL2main.lib;SDL2_image.lib;SDL2_ttf.lib;' into Linker -> Input -> Additionl Dependencies.
6. Make sure the Solution Platform in the top bar next to Debug/Release is set to x86.

