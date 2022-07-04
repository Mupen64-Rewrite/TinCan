TASInput-Qt
===========

A rewrite of TASInput from the ground up. I originally `used
gtkmm <https://github.com/jgcodes2020/tas-input-v2>`__ to do this, but
it turns out GTK is a huge pain on anything besides Linux.

.. contents::

Building on Linux
-----------------

You should have CMake and make/ninja before building. You should also
install Qt 6, Boost, and Protobuf:

+-------------------------+--------------------------------------------------------------------+
| Distribution            | Install command                                                    |
+=========================+====================================================================+
| Ubuntu/Debian Testing   | ``sudo apt install qt6-base-dev libboost-all-dev libprotobuf-dev`` |
+-------------------------+--------------------------------------------------------------------+
| Fedora/RHEL/CentOS      | ``sudo dnf install qt6-qtbase-devel boost-devel protobuf-devel``   |
+-------------------------+--------------------------------------------------------------------+
| Arch/Manjaro            | ``sudo pacman -S qt6-base boost protobuf``                         |
+-------------------------+--------------------------------------------------------------------+

From there:

.. code:: bash

   mkdir -p <project root>/build && cd <project root>/build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake --build .

Building on Windows using vcpkg
-------------------------------

You should have Visual Studio 2022 and CMake. CMake should be installed
separately, as the one bundled with VS tends to be very old.

Then, setup `vcpkg <https://github.com/microsoft/vcpkg>`__ and clone
this repository. Keep note of where vcpkg is installed, as it’ll be
important later.

Install Qt 6, Boost, and Protobuf:

.. code:: powershell

   cd <vcpkg root>
   .\vcpkg install qtbase:x64-windows boost:x64-windows protobuf:x64-windows

This can take a **very** long time, as vcpkg currently compiles
everything from source, Qt included.

Once that is done, go into ``CMakePresets.json``, and change this line:

.. code:: json

         "name": "win-vcpkg",
         "displayName": "Visual Studio + vcpkg",
         "description": "Windows build using VS2022 and vcpkg",
         "binaryDir": "${sourceDir}/build-vs",
         "cacheVariables": {
           // this one
           "CMAKE_TOOLCHAIN_FILE": "C:\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake"
         },

to this:

.. code:: json

         "cacheVariables": {
           "CMAKE_TOOLCHAIN_FILE": "<vcpkg root>\\scripts\\buildsystems\\vcpkg.cmake"
         },

Open the project in Visual Studio, and you should be able to build it as
normal.
