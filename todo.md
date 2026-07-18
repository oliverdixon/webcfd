# TODO

## Short Term

1. Send messages to EchoMap class from the ActionControllers without exposing a public member function to service every
   action callback. Use LWTs? (In which case the LWT std::visit monster will need to be refactored, but they can be
   implemented as private member functions.)

2. Restore wave file picker as a JS action.

3. Support manual mapping of externally sourced signals for Wasm targets.

4. Add project loader dialog as a native action.

5. Add a wave file loader as a native action.

6. Merge em-fixes back into master.

## Medium/Long Term

*In no particular order...*

* Use the ImGui DockBuilder API to configure a sane default panel configuration:
  https://github.com/ocornut/imgui/wiki/Docking#programmatically-setting-up-docking-layout-dockbuider-api

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
