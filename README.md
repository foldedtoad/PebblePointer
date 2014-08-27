
OVERVIEW
========
General instructions for installing, building and running
the PebblePointer apps.

These directions are written with an Linux-as-dev-system perspective.
This code was developed and tested on a Nexus-7 2nd Gen.

It is assumed that you have installed --
    the Pebble SDK, and
    the Android SDK with Eclipse installed


PREPARE A WORKSPACE FOR PEBBLEPOINTER
=====================================

1) Create new workspace (optional)  assume: ~/workspace
    mkdir ~/workspace
    cd ~/workspace

2) Checkout PebblePointer from github
    git clone https://github.com/foldedtoad/PebblePointer.git


SETTING UP THE ANDROID PROJECT
==============================

3) Start Eclipse and "Switch Workspace" to new workspace

    [File]-->[Switch Workspace]-->[other]  { navigate to your workspace } [OK]

4) Import PEBBLE_KIT project into Eclipse.

    [File]-->[Import]-->[Android]-->[Existing Android Code Into Workspace]-->[Next]
    [Browse] { pebble-dev/PebbleSDK-2.2/PebbleKit-Android/PebbleKit } [OK]
    
    Be sure "Project to Import" is checked for PEBBLE_KIT.
    
    [Finish]

    You should now see "PEBBLE_KIT" listed in Package Explorer in Eclipse.

5) Import the Android app project.

    [File]-->[Import]-->[Android]-->[Existing Android Code Into Workspace]-->[Next]
    [Browse] { ~/workspace/PebblePointer/android-app } [OK]

    Be sure "Project to Import" is checked for MainActivity.

    [Finish]

    You should now see "MainActivity" listed in Package Explorer in Eclipse.

 6) If you see an red X by the src directory, then do the following

        In the "Package Explorer", select MainActivity so it is hi-lited.
        [Project]-->[Properties]-->[Android]
        If you see a red X in the Reference box, then
            Select and [Remove] it.
            [Add], select "PEBBLE_KIT", then [OK]
            [Apply] then [OK]
    
    Else skip this step.

7) You should now be able to build and load (debug) the Android app on your
   target system.  


BUILDING THE PEBBLE WATCH APP
==============================

8) From a command shell, navigate to { ~/workspace/Pebble/watch-app }

9) Review the "make.sh" file

10) Run `PEBBLE_IP=196.168.1.2 ./make.sh`, which will do a build, load and go sequence
    for the watch app. Set `PEBBLE_IP` to the IP address of your target Android
    device which is bound to your Pebble watch.

