#! /usr/bin/env python
# encoding: utf-8

from waflib.Build import BuildContext
import os

APPNAME = "aybabtu"
VERSION = "3.1.1"


def configure(conf):

    _set_simd_flags(conf)


def _set_simd_flags(conf):
    """
    Sets flags used to compile in SIMD mode
    """
    CXX = conf.env.get_flat("CXX")
    flags = []
    # DEST_CPU should be set explicitly for clang cross-compilers
    cpu = conf.env["DEST_CPU"]

    # Matches both g++ and clang++
    if "g++" in CXX or "clang" in CXX:
        # Test different compiler flags based on the target CPU
        if cpu == "x86" or cpu == "x86_64":
            flags += conf.mkspec_try_flags(
                "cxxflags", ["-msse2", "-mssse3", "-msse4.2", "-mavx2"]
            )
        elif cpu == "arm" or cpu == "thumb":
            flags += conf.mkspec_try_flags("cxxflags", ["-mfpu=neon"])

    elif "CL.exe" in CXX or "cl.exe" in CXX:
        if cpu == "x86" or cpu == "x86_64" or cpu == "amd64":
            flags += conf.mkspec_try_flags("cxxflags", ["/arch:AVX2"])

    elif "em++" in CXX:
        flags = []

    else:
        conf.fatal("Unknown compiler - no SIMD flags specified")

    conf.env["CFLAGS_AYBABTU_SIMD"] = flags
    conf.env["CXXFLAGS_AYBABTU_SIMD"] = flags


def build(bld):

    optimized_sources = {
        "detail/base64_avx2": ["-mavx2", "/arch:AVX2"],
        "detail/base64_neon": ["-mfpu=neon"],
        "detail/base64_ssse3": ["-mssse3"],
    }

    libs = ["cpuid", "platform_includes"]

    cxxflags = []
    compiler_binary = bld.env.get_flat("CXX").lower()

    if "g++" in compiler_binary and "clang" not in compiler_binary:
        cxxflags += ["-funroll-loops"]

    for source, flags in optimized_sources.items():

        simd_flags = [f for f in flags if f in bld.env["CXXFLAGS_AYBABTU_SIMD"]]

        if "-mavx2" in simd_flags and "clang" not in compiler_binary:
            simd_flags += [
                "-mno-avx256-split-unaligned-load",
                "-mno-avx256-split-unaligned-store",
            ]

        bld.objects(
            source="src/aybabtu/" + source + ".cpp",
            target=source,
            cxxflags=cxxflags + simd_flags,
            use=libs,
        )

    source_files = bld.path.ant_glob(
        "src/aybabtu/**/*.cpp",
        excl=["src/aybabtu/" + s + ".cpp" for s in optimized_sources],
    )

    # Build static library if this is top-level, otherwise just .o files
    features = ["cxx"]
    if bld.is_toplevel():
        features += ["cxxstlib"]

    bld(
        features=features,
        source=source_files,
        target="aybabtu",
        cxxflags=cxxflags,
        use=libs + list(optimized_sources.keys()),
        install_path="${PREFIX}/lib",
        export_includes=["src"],
    )

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
