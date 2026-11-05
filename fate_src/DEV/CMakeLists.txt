# Game sources originally imported from Game.vcproj (Release|Win32).
cmake_minimum_required(VERSION 3.20)
project(Fate C CXX)

if(NOT WIN32)
    message(FATAL_ERROR "Fate currently uses Win32 and Direct3D 8. On Linux, configure with --preset linux-mingw to build for Wine.")
endif()

set(FATE_SHIMS "${CMAKE_CURRENT_LIST_DIR}/../../fate_tools/shims" CACHE PATH "d3dx8/FSOUND replacement sources")
option(NO_STEAM "Build without the Steam client (needs patch_steam.py applied)" ON)
option(FATE_USE_RC "Compile the .rc resource file" ON)

# --- shims: replace D3DX8 and FMOD 3.75, load d3d8.dll at runtime ---------------------
add_library(fateshims STATIC
    ${FATE_SHIMS}/d3dx8_shim.cpp ${FATE_SHIMS}/fmod_shim.cpp ${FATE_SHIMS}/d3d8_loader.cpp ${FATE_SHIMS}/shim_stb_vorbis.c)
target_include_directories(fateshims PUBLIC ${FATE_SHIMS} ${FATE_SHIMS}/d3d8_headers)
set_target_properties(fateshims PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON C_STANDARD 99)
if(MSVC)
    # d3d8.h / d3d8types.h / d3d8caps.h copied from MinGW-w64 by setup.sh (the Windows SDK has no D3D8)
    target_include_directories(fateshims PUBLIC ${FATE_SHIMS}/d3d8_headers)
    target_compile_options(fateshims PRIVATE /W0 /O2)
    target_compile_definitions(fateshims PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(fateshims PRIVATE -w -O2)
endif()

# --- the game ------------------------------------------------------------------------
add_executable(Fate WIN32
    ANIMATEDMESH/animatedgeometry.cpp
    ANIMATEDMESH/animatedmesh.cpp
    ANIMATEDMESH/animatedmeshsegment.cpp
    ANIMATEDMESH/animatedsubmesh.cpp
    CDA/cdamanager.cpp
    DIRECT3D/d3denumeration.cpp
    DIRECT3D/dx8app.cpp
    DIRECT3D/getdxver.cpp
    GAME/game.cpp
    GAME/gamestatemanager.cpp
    GAMECLIENT/bottommenu.cpp
    GAMECLIENT/buysellmenu.cpp
    GAMECLIENT/confirmmenu.cpp
    GAMECLIENT/enchantmenu.cpp
    GAMECLIENT/fatemenu.cpp
    GAMECLIENT/gameclient.cpp
    GAMECLIENT/gameoptionsmenu.cpp
    GAMECLIENT/gameui.cpp
    GAMECLIENT/helpmenu.cpp
    GAMECLIENT/inventorymenu.cpp
    GAMECLIENT/journalmenu.cpp
    GAMECLIENT/messagemenu.cpp
    GAMECLIENT/petinventorymenu.cpp
    GAMECLIENT/petsellinventorymenu.cpp
    GAMECLIENT/petstatsmenu.cpp
    GAMECLIENT/questdescriptions.cpp
    GAMECLIENT/questmenu.cpp
    GAMECLIENT/skillmenu.cpp
    GAMECLIENT/spellmenu.cpp
    GAMECLIENT/statsmenu.cpp
    GAMECLIENT/templatemanifest.cpp
    GAMECLIENT/tipmenu.cpp
    GAMECLIENT/tips.cpp
    GAMEOBJECTS/blurlayer.cpp
    GAMEOBJECTS/camera.cpp
    GAMEOBJECTS/fillshadow.cpp
    GAMEOBJECTS/genericmodel.cpp
    GAMEOBJECTS/lightmanager.cpp
    GAMEOBJECTS/screenfade.cpp
    GLOWS/glowmanager.cpp
    GLOWS/glowsystem.cpp
    IO/joystick.cpp
    IO/keyhandler.cpp
    IO/mousehandler.cpp
    ITEM/item.cpp
    ITEM/itemblank.cpp
    ITEM/itemgold.cpp
    ITEM/itemtemplate.cpp
    ITEM/spelltemplate.cpp
    LEVEL/level.cpp
    LEVEL/levelcollision.cpp
    LEVEL/levelhistory.cpp
    LEVEL/levelmesh.cpp
    LEVEL/levelmeshpatch.cpp
    LEVEL/levelprop.cpp
    LEVEL/maze.cpp
    LEVEL/modeltemplate.cpp
    LEVEL/template.cpp
    MEDIA/bitmap.cpp
    MEDIA/cubemap.cpp
    MEDIA/material.cpp
    MEDIA/movie.cpp
    MEDIA/refmanager.cpp
    MEDIA/soundmanager.cpp
    MEDIA/texture.cpp
    MENU/SUBMENUS/ancestordifficultymenu.cpp
    MENU/SUBMENUS/ancestormenu.cpp
    MENU/SUBMENUS/ancestorpetmenu.cpp
    MENU/SUBMENUS/continuegamemenu.cpp
    MENU/SUBMENUS/creditsmenu.cpp
    MENU/SUBMENUS/difficultymenu.cpp
    MENU/SUBMENUS/intromenu.cpp
    MENU/SUBMENUS/introquestmenu.cpp
    MENU/SUBMENUS/logomenu.cpp
    MENU/SUBMENUS/mainmenu.cpp
    MENU/SUBMENUS/newgamemenu.cpp
    MENU/SUBMENUS/newpetmenu.cpp
    MENU/SUBMENUS/optionsmenu.cpp
    MENU/SUBMENUS/retiremenu.cpp
    MENU/dropdown.cpp
    MENU/loadingmenu.cpp
    MENU/menu.cpp
    MENU/menumanager.cpp
    MODEL/facelist.cpp
    MODEL/model.cpp
    MODEL/shadowvolume.cpp
    MODEL/submodel.cpp
    MODEL/submodelsegment.cpp
    NAMES/namegenerator.cpp
    OBJECTS/curve.cpp
    OBJECTS/path.cpp
    PARTICLES/particlemanager.cpp
    PARTICLES/particlesystem.cpp
    PATHFINDING/astar.cpp
    POSITIONALAUDIO/positionalaudiomanager.cpp
    SCENE/collisionscene.cpp
    SCENE/scene.cpp
    SCENE/subscene.cpp
    SCENE/subscenesegment.cpp
    SKINNEDMESH/skinnedanimation.cpp
    SKINNEDMESH/skinnedgeometry.cpp
    SKINNEDMESH/skinnedmesh.cpp
    SKINNEDMESH/skinnedmeshsegment.cpp
    SKINNEDMESH/skinnedsubmesh.cpp
    UI/button.cpp
    UI/sprite.cpp
    UI/text.cpp
    UTILITIES/StatsAndAchievements.cpp
    UTILITIES/UserLocaleInfo.cpp
    UTILITIES/capabilities.cpp
    UTILITIES/loadlocalizedstring.cpp
    UTILITIES/math3D.cpp
    UTILITIES/settings.cpp
    UTILITIES/textfile.cpp
    UTILITIES/utilities.cpp
    CHARACTER/character.cpp
    CHARACTER/charactertemplate.cpp
    CHARACTER/gamesave.cpp
    CHARACTER/inventory.cpp
    CHARACTER/monster.cpp
    CHARACTER/player.cpp
    main.cpp
    UTILITIES/SaveFile.cpp
    UTILITIES/Spawn.cpp
    UTILITIES/datafile.cpp
    UTILITIES/randomizer.cpp
    UTILITIES/vistafileutilities.cpp
)
if(FATE_USE_RC)
    enable_language(RC)
    target_include_directories(Fate PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/RESOURCES")
    target_sources(Fate PRIVATE RESOURCES/WinMain.rc)
endif()

target_include_directories(Fate BEFORE PRIVATE ${FATE_SHIMS} ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Pal/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../steam/public/steam
)
target_compile_definitions(Fate PRIVATE WIN32 NDEBUG _WINDOWS _MBCS $<$<BOOL:${NO_STEAM}>:NO_STEAM>)
target_link_libraries(Fate PRIVATE fateshims)

if(MSVC)
    # VS2003 semantics: loop variables leak out of for(), wchar_t is a typedef, static CRT
    target_include_directories(Fate BEFORE PRIVATE ${FATE_SHIMS}/d3d8_headers)
    target_compile_definitions(Fate PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE)
    target_compile_options(Fate PRIVATE /Zc:forScope- /Zc:wchar_t- /EHsc /GR /W0 /O2 /FIstddef.h /FIstdlib.h)
    set_property(TARGET Fate fateshims PROPERTY MSVC_RUNTIME_LIBRARY MultiThreaded)
    target_link_options(Fate PRIVATE /SUBSYSTEM:WINDOWS /IGNORE:4098)
    target_link_libraries(Fate PRIVATE dinput8.lib dxguid.lib winmm.lib odbc32.lib odbccp32.lib vfw32.lib version.lib avifil32.lib msvfw32.lib ole32.lib uuid.lib)
    set(STEAM_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../steam/lib/steam_api.lib")
    if(NOT NO_STEAM AND EXISTS "${STEAM_LIB}")
        target_link_libraries(Fate PRIVATE ${STEAM_LIB})
    endif()
else()
    # MinGW-w64 cross-build for Wine.
    target_compile_options(Fate PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++98 -fpermissive -fno-strict-aliasing -w -O2>)
    target_compile_options(Fate PRIVATE "SHELL:-include stddef.h" "SHELL:-include stdlib.h")
    target_link_options(Fate PRIVATE -static -mwindows)
    foreach(l dinput8 dxguid winmm odbc32 odbccp32 vfw32 version avifil32 msvfw32 ole32 uuid)
        find_library(FATE_LIB_${l} ${l})
        if(FATE_LIB_${l})
            target_link_libraries(Fate PRIVATE ${FATE_LIB_${l}})
        else()
            message(WARNING "system library '${l}' not found in the MinGW sysroot (skipped)")
        endif()
    endforeach()
    set(STEAM_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../steam/lib/steam_api.lib")
    if(NOT NO_STEAM AND EXISTS "${STEAM_LIB}")
        target_link_libraries(Fate PRIVATE ${STEAM_LIB})
    endif()
endif()

set_target_properties(Fate PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
