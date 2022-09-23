# IPC specification

Here lies my design notes for shared-memory IPC between plugin and GUI.

## Contents of SHM block

Shared fields should include:
- an atomic `uint32_t` containing the current inputs
- a `CONTROL[4]`, separate from the one given from `InitiateControllers`
  - a mutex controlling access to this struct
  - an atomic flag, set when the controller has new data.
- an atomic `bool` describing who has control over the inputs
  - `false` if the GUI is sending inputs
  - `true` if the plugin is sending inputs