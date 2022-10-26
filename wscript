#! /usr/bin/env python
# encoding: utf-8

from waflib.Build import BuildContext
import os

APPNAME = "aybabtu"
VERSION = "2.2.0"


def build(bld):

    bld.recurse("src/aybabtu")

    if bld.is_toplevel():

        # Only build tests when executed from the top-level wscript,
        # i.e. not when included as a dependency
        bld.recurse("test")
        bld.recurse("examples")
        bld.recurse("benchmarks")


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


class PlotContext(BuildContext):
    cmd = "plot_benchmarks"
    fun = "plot_benchmarks"


def plot_benchmarks(ctx):

    venv = ctx.create_virtualenv(name="virtualenv-plots", overwrite=False)

    if not os.path.isfile("benchmarks/requirements.txt"):
        venv.run("python -m pip install pip-tools")
        venv.run("pip-compile benchmarks/requirements.in")

    venv.run("python -m pip install -r benchmarks/requirements.txt")

    if not os.path.exists("benchmark_results"):
        os.mkdir("benchmark_results")

    # Block benchmark
    venv.run(
        "build_current/benchmarks/throughput --benchmark_out='benchmark_results/throughput.json' --benchmark_counters_tabular=true"
    )
    venv.run("python benchmarks/plot.py -i benchmark_results/throughput.json")
