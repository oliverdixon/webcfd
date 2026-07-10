# TODO

*In no particular order...*

1. Use the ImGui DockBuilder API to configure a sane default panel configuration:
   https://github.com/ocornut/imgui/wiki/Docking#programmatically-setting-up-docking-layout-dockbuider-api

2. Set up multithreaded architecture with worker and render threads
    * Qt example: https://doc.qt.io/qt-6/qtcore-threads-mandelbrot-example.html
    * Qt documentation (same theory applies): https://doc.qt.io/qt-6/threads-qobject.html

3. Use FFTW3 to compute and optionally display Fourier transforms of source and downsampled signals.

4. Use heuristics to offload FFT work to the GPU via WebGPU for large transforms where the benefits outweigh the costs
   of memory transaction overhead.

5. Investigate SRP-PHAT.
    * Procure example data from European project.
    * Study ODAS (and maybe BeamformIt?) as reference implementations. Implement the time domain variant first, then the
      frequency domain version.

6. Add detailed description to README.

7. Add human-readable descriptions to the JSON schema(s).

8. Investigate use of SQLite as an alternative storage format.
   * Used by Audacity (`aup3`) for similar data.
   * There is a well-regarded C++ library: https://github.com/fnc12/sqlite_orm (also on vcpkg)
   * Suited for binary blobs and relational structure.
