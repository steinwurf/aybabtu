#! /usr/bin/env python
# encoding: utf-8

from waflib.Build import BuildContext
import os

APPNAME = "aybabtu"
VERSION = "5.0.0"


def options(ctx):
    ctx.load("cmake")

def configure(ctx):
    ctx.load("cmake")

    if ctx.is_toplevel():
        ctx.cmake_configure()


def build(ctx):
    ctx.load("cmake")

    if ctx.is_toplevel():
        ctx.cmake_build()


class ReleaseContext(BuildContext):
    cmd = "prepare_release"
    fun = "prepare_release"


def prepare_release(ctx):
    """Prepare a release."""

    # Rewrite versions
    with ctx.rewrite_file(filename="src/aybabtu/version.hpp") as f:
        pattern = r"#define STEINWURF_AYBABTU_VERSION v\d+_\d+_\d+"
        replacement = "#define STEINWURF_AYBABTU_VERSION v{}".format(
            VERSION.replace(".", "_")
        )

        f.regex_replace(pattern=pattern, replacement=replacement)

    with ctx.rewrite_file(filename="src/aybabtu/version.cpp") as f:
        pattern = r'return "\d+\.\d+\.\d+"'
        replacement = 'return "{}"'.format(VERSION)

        f.regex_replace(pattern=pattern, replacement=replacement)
