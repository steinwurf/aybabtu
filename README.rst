=======
aybabtu
=======

.. image:: https://travis-ci.org/steinwurf/aybabtu.svg?branch=master
    :target: https://travis-ci.org/steinwurf/aybabtu

.. image:: http://www.allyourbasearebelongtous.com/gif/allyourbase.gif
   :target: https://en.wikipedia.org/wiki/All_your_base_are_belong_to_us

aybabtu is a tiny lib containing functions to encode and decode base64 strings.
The implememtation is inspired greatly by code presented on
`wikipedia's Base64 page <https://en.wikipedia.org/wiki/Base64>`_.

.. contents:: Table of Contents:
   :local:

Usage
=====

See the ``example.cpp`` for an example on how to use the library.

Use as Dependency in CMake
==========================

To depend on this project when using the CMake build system, add the following
in your CMake build script:

::

   add_subdirectory("/path/to/aybabtu" aybabtu)
   target_link_libraries(<my_target> steinwurf::aybabtu)

Where ``<my_target>`` is replaced by your target.
