TASInput-Qt
===========

A rewrite of TASInput from the ground up. I originally `used
gtkmm <https://github.com/jgcodes2020/tas-input-v2>`__ to do this, but
it turns out GTK is a huge pain on anything besides Linux.

.. contents::

Building on Linux
-----------------

You should have CMake and make/ninja before building. You will also need the 
dependencies listed below. This handy table should tell you what packages to 
install.

+-------------+------------------------+-----------------------+---------------+
| Dependency  | Ubuntu/Debian Testing  | Fedora/RHEL/CentOS    | Arch/Manjaro  |
+=============+========================+=======================+===============+
| Qt 6        | ``qt6-base-dev``       | ``qt6-qtbase-devel``  | ``qt6-base``  |
+-------------+------------------------+-----------------------+---------------+
| Boost       | ``libboost-dev``       | ``boost-devel``       | ``boost``     |
+-------------+------------------------+-----------------------+---------------+
| Protobuf    | ``libprotoc-dev``     | ``protobuf-devel``    | ``protobuf``  |
+-------------+------------------------+-----------------------+---------------+
| fmt         | ``libfmt-dev``         | ``fmt-devel``         | ``fmt``       |
+-------------+------------------------+-----------------------+---------------+
.. note::
  libprotoc-dev depends on libprotobuf-dev, and should provide the needed headers.

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

Install the dependencies. Ensure you get the *static* version, because
it will save you from having a bunch of libraries in the output folder.

.. code:: powershell

   cd <vcpkg root>
   .\vcpkg install qtbase:x64-windows-static
   .\vcpkg install boost:x64-windows-static
   .\vcpkg install protobuf:x64-windows-static
   .\vcpkg install fmt:x64-windows-static

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

.. note::
  There is currently a bug in Protobuf that results in a C2127 error (illegal
  initialization of ``constinit`` entity with a non-constant expression).
  
  Here's the manual way of patching it, but there's probably a better way to patch Protobuf.
  Open ``<vcpkg root>\installed\<triple>\include\google\protobuf\port_def.inc``, then go to 
  line 641 and change this line:
  
  .. code:: cpp
    #if defined(__cpp_constinit)
    #define PROTOBUF_CONSTINIT constinit
    #define PROTOBUF_CONSTEXPR constexpr

  to this:
  
  .. code:: cpp
    #if defined(__cpp_constinit) && !defined(_MSC_VER)
    #define PROTOBUF_CONSTINIT constinit
    #define PROTOBUF_CONSTEXPR constexpr

  Protobuf will now no longer bother you.