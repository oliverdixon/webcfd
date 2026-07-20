# TODO

## Short Term

1. Remove Object<T>::id_type specialisations and just use a common type, to reduce excessive includes.

2. Document notification and worker sources and sinks.

3. Use the ImGui DockBuilder API to configure a sane default panel configuration:
   https://github.com/ocornut/imgui/wiki/Docking#programmatically-setting-up-docking-layout-dockbuider-api

4. Harden! Doxygen where necessary, remove incorrect exception specifications, and verify dereferencing behaviour.

5. Add project loader dialog as a native action.

6. Add a wave file loader as a native action.

7. Resolve TODOs in code. 

## Medium/Long Term

*In no particular order...*

* Add an autocorrelation panel. Or maybe just an option on the existing DFT panel and rename it something like
  "Spectrum Analyzer"? These algorithms will be required for the SRP-PHAT anyway.

* Use heuristics to offload FFT work to the GPU via WebGPU for large transforms where the benefits outweigh the costs
  of memory transaction overhead.
  * https://github.com/DTolm/VkFFT could be used, but it doesn't currently have a WebGPU backend. However the API seems
    fairly straightforward, so it may be feasible to add one. https://github.com/DTolm/VkFFT/issues/144

* Investigate SRP-PHAT.
  * Procure example data from European project.
  * Study ODAS (and maybe BeamformIt?) as reference implementations. Implement the time domain variant first, then the
    frequency domain version.

* Add detailed description to README.

* Investigate use of SQLite as an alternative storage format.
  * Used by Audacity (`aup3`) for similar data.
  * There is a well-regarded C++ library: https://github.com/fnc12/sqlite_orm (also on vcpkg)
  * Suited for binary blobs and relational structure.
