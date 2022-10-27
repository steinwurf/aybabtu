=======
aybabtu
=======

|Linux make-specs| |Windows make-specs| |MacOS make-specs| |Linux CMake| |Windows CMake| |MacOS CMake| |Raspberry Pi| |Valgrind| |No Assertions| |Clang Format| |Cppcheck|

.. |Linux make-specs| image:: https://github.com/steinwurf/aybabtu/actions/workflows/linux_mkspecs.yml/badge.svg
   :target: https://github.com/steinwurf/aybabtu/actions/workflows/linux_mkspecs.yml

.. |Windows make-specs| image:: https://github.com/steinwurf/aybabtu/actions/workflows/windows_mkspecs.yml/badge.svg
   :target: https://github.com/steinwurf/aybabtu/actions/workflows/windows_mkspecs.yml

.. |MacOS make-specs| image:: https://github.com/steinwurf/aybabtu/actions/workflows/macos_mkspecs.yml/badge.svg
   :target: https://github.com/steinwurf/aybabtu/actions/workflows/macos_mkspecs.yml

.. |Linux CMake| image:: https://github.com/steinwurf/aybabtu/actions/workflows/linux_cmake.yml/badge.svg
   :target: https://github.com/steinwurf/aybabtu/actions/workflows/linux_cmake.yml

.. |Windows CMake| image:: https://github.com/steinwurf/aybabtu/actions/workflows/windows_cmake.yml/badge.svg
   :target: https://github.com/steinwurf/aybabtu/actions/workflows/windows_cmake.yml

.. |MacOS CMake| image:: https://github.com/steinwurf/aybabtu/actions/workflows/macos_cmake.yml/badge.svg
   :target: https://github.com/steinwurf/aybabtu/actions/workflows/macos_cmake.yml

.. |Raspberry Pi| image:: https://github.com/steinwurf/aybabtu/actions/workflows/raspberry_pi.yml/badge.svg
   :target: https://github.com/steinwurf/aybabtu/actions/workflows/raspberry_pi.yml

.. |Clang Format| image:: https://github.com/steinwurf/aybabtu/actions/workflows/clang-format.yml/badge.svg
   :target: https://github.com/steinwurf/aybabtu/actions/workflows/clang-format.yml

.. |No Assertions| image:: https://github.com/steinwurf/aybabtu/actions/workflows/nodebug.yml/badge.svg
   :target: https://github.com/steinwurf/aybabtu/actions/workflows/nodebug.yml

.. |Valgrind| image:: https://github.com/steinwurf/aybabtu/actions/workflows/valgrind.yml/badge.svg
   :target: https://github.com/steinwurf/aybabtu/actions/workflows/valgrind.yml

.. |Cppcheck| image:: https://github.com/steinwurf/aybabtu/actions/workflows/cppcheck.yml/badge.svg
   :target: https://github.com/steinwurf/aybabtu/actions/workflows/cppcheck.yml

.. image:: http://www.allyourbasearebelongtous.com/gif/allyourbase.gif
   :target: https://en.wikipedia.org/wiki/All_your_base_are_belong_to_us

aybabtu is a tiny lib containing functions to encode and decode base64 strings.

.. contents:: Table of Contents:
   :local:

Usage
=====

See the ``example.cpp`` for how to use the library.

Use as Dependency in CMake
==========================

To depend on this project when using the CMake build system, add the following
in your CMake build script:

::

   add_subdirectory("/path/to/aybabtu" aybabtu)
   target_link_libraries(<my_target> steinwurf::aybabtu)

Where ``<my_target>`` is replaced by your target.
