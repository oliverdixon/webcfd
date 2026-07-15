# EchoMap Documentation

Currently, "documentation" refers to generated HTML from Doxygen.

The HTML is fully reproducible from the source, so anybody wishing the review the docs can either:

* view the `/** ... */` Doxygen comments in the source directly;
* clone the repo and run `doxygen` in the root; or
* clone the repo and use the CMake preset:
  ```shell
  $ cmake --preset documentation
  $ cmake --build docs/out/ --target EchoMapHTMLDocumentation
  $ docs/open.sh
  ```
