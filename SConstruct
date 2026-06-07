#!/usr/bin/env python
"""Builds the Propwash GDExtension (libpropwash) into godot/bin/.

Usage:
  scons platform=linux target=template_debug
  scons platform=windows target=template_debug

Delegates compiler/platform setup to godot-cpp's SConstruct, then adds our
sources. See https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/
"""
import os

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

libname = "propwash{}{}".format(env["suffix"], env["SHLIBSUFFIX"])
library = env.SharedLibrary(target="godot/bin/{}".format(libname), source=sources)

Default(library)
