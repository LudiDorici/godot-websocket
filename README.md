# WebSocket module for Godot Engine 3.0

This module allows for easy creation of **WebSocket Client and Server** using [libwebsockets](https://libwebsockets.org/) as a thirdparty library. Libwebsockets is released as LGPLv2.1 + static linkning exception. You can find that license in `thirdparty/LICENSE.txt`

A small demo project is available in `lws_demo` to show how to use the module.

This module is still a work in progress.

Tested on:

 * Linux
 * Windows (mingw build)

Compiles on (untested but should work):

 * Android
 * iOS
 * OSX

### Build instruction

Simply copy (or link) `modules/lws` inside Godot `modules` directory. Also copy `thirdparty/lws` inside Godot `thirdparty` directory.

Compile Godot Engine as you would normally do.
