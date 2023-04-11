# TinCan
(waiting for someone to come up with a clever backronym)

The successor to the original TASInput. Powered by wxWidgets to appear native on all target platforms.
Note: the GUI had to be moved to a separate process to keep it portable.

## Building

This project uses vcpkg on Windows and system libraries on Linux.

For Windows, install `wxwidgets:x64-windows-static`. This ensures that the entire wxWidgets toolkit will be
linked statically. This is a necessary evil, as it ensures that the user does not have to add any additional
library files to use TINCAN.
