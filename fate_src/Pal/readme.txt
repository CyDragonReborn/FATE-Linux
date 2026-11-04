You will find more detailed docs on in the doc directory. This readme is meant to give an overview.

Requirements: The pal and wire libs are created with vc6 libs and stlport.  Your project can be linked to vc7 libs and use the ms stl.  You may need to link to stlport_vc6_static.lib in order to resolve all symbols.  

The first thing to do is to add the include and lib dirs to your project settings.  Add pal_stub.lib (or pal_stubd.lib for debug builds) to your linker input.  You will need to copy the files in the html directory and the following dlls into your game directory:
WireControl.dll
pal_development.dll
(you can also put the debug versions in there if you are trying to run those builds)

You need to register the WireControl.dll with regsvr32.exe (in windows/system32) in order for th game to run. 

The html files that are provided here are placeholders. They will be replaced with real pages later on. Once I get some real pages from another game,  I will forward them on to you as they will probably be named differently. 

once you have all of your compiler settings ready and the files in the correct place, you can start adding the code to your game.
See the main.cpp for an example of how to integrate the pal calls.  You can then use PAL_Unlocked() throughout your game code to dertemine if the game is unlocked or not.  You do not need to implement any unlock screen in your code, it will all be handled by the wire web pages.

