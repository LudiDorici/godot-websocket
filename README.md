# NOTE: This module was merged in Godot Engine master branch via pull request https://github.com/godotengine/godot/pull/14888 and will be available on official Godot Engine releases starting from version 3.1. This repository is no longer mantained and only left for historical purpose. All further development will be done inside the Godot Engine repository ( https://github.com/godotengine/godot ). Thank you for your support.
# Demos are available for newer versions of Godot in the official [demo repository](https://github.com/godotengine/godot-demo-projects/tree/master/networking).

# If you like this, and want to support us, [check out our videogame](http://store.steampowered.com/app/679100/Aequitas_Orbis) and follow us on [twitter](https://twitter.com/aequitasorbis)



-----------

# WebSocket module for Godot Engine 3.0

This module allows for easy creation of **WebSocket Client and Server** using [libwebsockets](https://libwebsockets.org/) as a thirdparty library. Libwebsockets is released as LGPLv2.1 + static linkning exception. You can find that license in `thirdparty/LICENSE.txt`

The client API is also available in project exported to HTML5 using native Javascript code.

A small demo project is available in `websocket_chat_demo` to show how to use the module.

This module is still a work in progress.

Tested on:

 * Linux
 * Windows (mingw build)
 * Javascript/HTML5

Compiles on (untested but should work):

 * Android
 * iOS
 * OSX

The module supports writing data in both TEXT and BINARY mode.

### NOTE

You can run the demo server from a godot instance and connect to it with the demo client exported to Javascipt/HTML5, checkout the screenshot:

![LWS Module Screenshot](https://github.com/LudiDorici/godot-lws/raw/master/screenshot.png)

### Build instruction

Simply copy (or link) `modules/lws` inside Godot `modules` directory. Also copy `thirdparty/lws` inside Godot `thirdparty` directory.

Compile Godot Engine as you would normally do.

#### Support us

If you like this, and want to support us, [check out our videogame](http://store.steampowered.com/app/679100/Aequitas_Orbis) and follow us on [twitter](https://twitter.com/aequitasorbis)
