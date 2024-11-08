---
title: MaixCDK Development Guidelines and Guidance
---


## General Guidelines

* **Simplicity and Ease of Use**: Assume minimal prior experience from users. Minimize required actions and provide simple APIs with comprehensive documentation.
> For example, instead of asking users to choose and download a toolchain, it is better to automatically select and download the toolchain based on the chosen platform, reducing the entry barrier.

* **Universality**: APIs should be designed to be platform-agnostic and provide consistent abstraction. If this cannot be achieved, reconsider whether the feature should be included.

* **Consistency**: Maintain consistent API styles and coding standards.

* **Extensibility**: While ensuring core functionality, provide extension interfaces to allow users to add features easily.

* **Depth**: Keep APIs simple but document the underlying principles. Open-source as much code as possible to facilitate deeper exploration by users.

## Source Code and Binary Files

* **Avoid Using Git Submodules**: This facilitates faster downloads for users in China. Even if the main repository is mirrored, submodules still need to be fetched from GitHub, which can be slow for Chinese users.

* **Do Not Store Large Files or Binaries in the Source Code Repository**: Storing these will significantly increase the Git repository size. Git is optimized for text files; handle binary files as described below.

* **Automatically Download Third-Party Libraries and Binaries Before Compilation**: Define necessary files for download in [component.py](https://github.com/sipeed/MaixCDK/blob/main/components/3rd_party/asio/component.py). During compilation, they will be downloaded to `dl/pkgs` and extracted to `dl/extracted`, allowing direct inclusion in `CMakeLists.txt`, e.g., `list(APPEND ADD_INCLUDE "${DL_EXTRACTED_PATH}/sunxi_mpp/sunxi-mpp-1.0.0/include")`.
> The list of files to be downloaded is stored in `dl/pkgs_info.json` during each build. Users with slow internet connections can manually download these files from official sources or third-party services and place them in `dl/pkgs`.

* **Do Not Modify Third-Party Libraries**: Avoid modifying the source code of third-party libraries to facilitate upgrades. If modifications are necessary, consider applying patches automatically after extraction.

## Overview of MaixCDK Architecture

For general application developers, key concepts include:

* MaixCDK consists of two main parts: `MaixCDK` (library storage) and `project` (application code). Official examples and apps are located in `MaixCDK/examples` and `MaixCDK/projects`, respectively. You can also create your own projects in `MaixCDK/projects`.

Alternatively, you can separate the two and set an environment variable `MAIXCDK_PATH` pointing to the `MaixCDK` directory, e.g., `export MAIXCDK_PATH=/home/xxx/MaixCDK`. This allows projects to reside in other locations, such as `/home/xxx/projects`.

* **Components**: Each functional module can be encapsulated as a component, making it easy to include in different applications.
  * For example, the `main` component in [examples/hello_world](./examples/hello_world) and various components in [components](./components). You can also add custom components, like `hello_world/component1`.
  * Use the environment variable `MAIXCDK_EXTRA_COMPONENTS_PATH` to specify additional component paths.
  * Each component includes a `CMakeLists.txt` file describing its contents, e.g., `list(APPEND ADD_INCLUDE "include")` for header files, `list(APPEND ADD_SRCS "src/hello.c")` for source files, and `list(APPEND ADD_REQUIREMENTS basic)` for dependencies.

* **Kconfig**: Provides terminal-based configuration options. Each component can include a `Kconfig` file for setting options, accessible via `maixcdk menuconfig`. The configuration is saved in `build/config`, generating `global_config.cmake` and `global_config.h` for use in `CMakeLists.txt` and C/C++ files.

* **Third-Party Library Integration**: There are two recommended methods:
  * **Method 1**: Specify third-party libraries in the component’s `CMakeLists.txt` for automatic download during compilation. This is preferred for libraries integrated into `MaixCDK`.
  * **Method 2**: Package and publish the library as a Python package on [pypi.org](https://pypi.org) with the naming convention `maixcdk-xxx`. Users can then install it using `pip install maixcdk-xxx`.

* **Documentation**: Located in the [docs](./docs/) directory. API documentation is auto-generated from code; do not edit it manually. The application documentation serves as a user guide.

## Coding Style

To ensure consistency across MaixCDK and MaixPy APIs, follow these coding style guidelines:

* **Function Names**: Use lowercase with underscores, e.g., `get_name`.
* **Variable Names**: Use lowercase with underscores, e.g., `file_path`.
* **Class Names**: Use CamelCase, with common abbreviations in uppercase, e.g., `Person`, `YOLOv2`, `UART`.
* **Macros**: Use uppercase with underscores, e.g., `MAIXPY_VERSION`.
* **Class Member Variables**: Use lowercase with underscores without `m_` prefix, e.g., `name`.
* **Private Class Member Variables**: Prefix with `_`, e.g., `_name`.
* **Using Class Member Variables**: Explicitly use `this->` for clarity, e.g., `this->name`.
* **Namespace**: Use the `maix` namespace for all official APIs, with sub-namespaces for different features, e.g., `maix::thread`, `maix::peripheral`.
* **Source File Naming**: Use lowercase with underscores, e.g., `maix_peripheral_uart.cpp`. Use `.hpp` for C++ header files.
* **Comments**: Follow Doxygen style for API documentation, as shown in [components/basic/include/maix_api_example.hpp](../../../components/basic/include/maix_api_example.hpp).

### API Documentation Guidelines

Include the following keywords in your API comments:
  * **brief**: A brief description of the functionality.
  * **param**: Detailed parameter descriptions, including value requirements and direction (e.g., `param[in] a`).
  * **return**: Detailed return value descriptions.
  * **retval**: Use this for specific return value explanations, e.g., `@retval 0 success`.
  * **attention**: Special considerations for using the API.
  * **maixpy**: Indicates a `MaixPy` API, including the package and variable name, e.g., `maix.example`.
  * **maixcdk**: Indicates a `MaixCDK` API.

Follow the guidelines strictly, especially when defining types and default values, to ensure accurate `MaixPy API` and documentation generation.

## API Standardization Suggestions

* Prefer MaixCDK APIs over direct third-party APIs for better cross-platform compatibility, e.g., use `maix_fs.hpp` for file operations.
* Use logging APIs from `maix_log.hpp` for consistent logging and to manage log levels.
* Use error handling from `maix_err.hpp` for consistent error codes and exception handling.
* For common APIs across modules, define a base class and inherit it in specific implementations, enhancing code reuse and consistency across platforms.

