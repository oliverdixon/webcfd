# TODO

*In no particular order...*

* Use the ImGui DockBuilder API to configure a sane default panel configuration:
  https://github.com/ocornut/imgui/wiki/Docking#programmatically-setting-up-docking-layout-dockbuider-api

* Use a proper publisher-subscriber model for asynchronous results and tasks.

* Use heuristics to offload FFT work to the GPU via WebGPU for large transforms where the benefits outweigh the costs
  of memory transaction overhead.

* Investigate SRP-PHAT.
  * Procure example data from European project.
  * Study ODAS (and maybe BeamformIt?) as reference implementations. Implement the time domain variant first, then the
    frequency domain version.

* Add detailed description to README.

* Add human-readable descriptions to the JSON schema(s).

* Investigate use of SQLite as an alternative storage format.
  * Used by Audacity (`aup3`) for similar data.
  * There is a well-regarded C++ library: https://github.com/fnc12/sqlite_orm (also on vcpkg)
  * Suited for binary blobs and relational structure.
