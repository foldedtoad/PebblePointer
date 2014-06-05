=============================
PebblePointer  June 4, 2014
=============================

BEFORE STARTING
---------------

Review the instructions at 
developer.getpebble.com/2/mobile-app-guide/android-guide.html

    Install Android SDK, DDMS, and Eclipse support, if you have not already.
    Be sure to your ADB link to your target system is functional.
    Start Eclipse

These instructions assume a workspace directory at ~/workspace: adjust these instructions accordingly.


INSTALLING Pebble_Kit INTO YOUR WORKSPACE
------------------------------------------

Below is a customized version of those instructions.

To load it into Eclipse, follow these simple steps:

    Click File -> New Project…
    Select "Android Project from Existing Code" - Press "Next"
    Select ~/pebble-dev/PebbleSDK-2.2/PebbleKit-Android/PebbleKit as the root directory (via the browse button)
    Click Finish

This loads a new project called "Pebble_Kit".
Ensure there are no errors on this project.
PebblePointer will have a dependency on Pebble-Kit.


INSTALLING PebblePointer INTO YOUR WORKSPACE
------------------------------------------

If you clone the PebblePointer project into the same workspace as the above Pebble-Kit, then
you should be able to open the project and built/run it.
You might want to do a Project->Clean before building and loading.


CREDITS
-------
I would like to credit the AndroidPlot developers, but I don't see any file containing authors, et.al.
see https://bitbucket.org/androidplot/androidplot



