Digital Elevation Model Viewer
------------------------------
Written by Thomas Spriggs in 2007. For contact with author use thomas.spriggs@gmail.com

Keys
----
F1	display about box
F2      toggle displaying of the opposite side of the object
F3      load file
F5      wireframe render
F6      shaded render
alt-F6  shaded render with 4x antialiasing
ctrl-F6 shaded render with 16x antialiasing
F9      refresh preview render
F11     cycle colouring modes
q       set light vector to the camera direction vector
l       display current geographic coordinate

mouse control
-------------
drag left mouse button                     slide camera
drag right mouse button                    rotate camera
drag left and right mouse button           rotate object
drag middle mouse button                   slide camera position in/out
                                           and third rotation axis

.drs files are dem renderer script files. They can be edited with a text editor. Comment lines start with a #. All script file contents after "stop" is ignored. The script interpreter fails silently when failure occours. All unrecognised input is ignored. Refer to examples for syntax and commands.

known issues
------------
* Control system malfunctions when numlock is on.
* Mouse buttons get stuck sometimes.

disclamer
---------
Digital Elevation Model Viewer is provided as is. Use at your own risk. 