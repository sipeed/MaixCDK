---
title: Add API for MaixCDK / MaixPy
---

## Code Guidelines

Please refer to [Code Guidelines](./README.md) first.

## How to Add an API

At the end of the [Quick Start](../README.md), we briefly mentioned adding a new API to MaixPy using annotations. It looks straightforward – you only need to add a comment to the API function, for example:

```cpp
namespace maix::example
{
    /**
     * @brief Say hello to someone
     * @param[in] name The name of the person, of string type
     * @return A string containing "hello" + name
     * @maixpy maix.example.hello
     */
    std::string hello(std::string name);
}
```

Then you can call this in MaixPy:

```python
from maix import example

result = example.hello("Bob")
print(result)
```

To ensure that the APIs we add are **usable** for users, we need to follow these guidelines:

* Design the API names and parameters to be reasonable, general, and highly cross-platform.
* Ensure the API is annotated (documentation will be automatically generated during compilation).
* The API should come with usage documentation, a tutorial, and example code.

Here is a more detailed process and guidelines:

1. **Confirm the functionality and add a usage document and example code in the [MaixPy Documentation Source](https://github.com/sipeed/MaixPy/tree/main/docs/doc)**. This acts as a design document, helping to avoid frequent API changes due to incomplete considerations during coding. It also serves as documentation. **(This is very important!)**
2. You can add an **application document** under [docs/doc/application](https://github.com/sipeed/MaixCDK/tree/main/docs/doc/application) to record development details. Use lowercase filenames with underscores, e.g., `peripheral/uart.md` or `ai/yolov2.md`.
3. Mention the sources or open-source projects referenced for API design in the development document to facilitate the review process and improve the chances of passing the review quickly.
4. Refer to [components/basic/include/maix_api_example.hpp] and add the API to an appropriate `component`. If it's a new component, consider discussing its rationality first in [issues](https://github.com/sipeed/MaixCDK/issues).

> Note: The `API` is identified through comments, which helps automatically generate documentation and `MaixPy` source code. Pay close attention to the annotation guidelines and refer to `maix_api_example.hpp` for specifics.
> Additionally, because `MaixCDK` does not include definitions related to `Python.h` or `Pybind11.h`, language-native types are automatically converted by `pybind11`. For instance, `void hello(std::string a, std::vector<int> b)` is equivalent to `def hello(a: str, b: list)` in `MaixPy`.
> Common conversions include `std::vector` to `list`, `std::map` to `dict`, `std::valarray` (accepts `list` and `bytes` as input, returns `list`), and `maix::Bytes` (both input and return values are `bytes`). `std::function` maps to a function in MaixPy. For more details, refer to the [pybind11 documentation](https://pybind11.readthedocs.io/en/stable/advanced/cast/overview.html#conversion-table).

5. Add a C++ example to the `examples` directory and ensure it compiles and runs successfully.
6. The documentation will be automatically generated during compilation. Check the generated files under `docs/doc/api` for any errors and correct them in the code if needed.
7. Test the updated `MaixPy` project with the new `MaixCDK` to ensure it compiles successfully and the generated documentation is correct. Fix any errors if they occur.
8. Submit your code to your own GitHub repository and wait for the `action` to automatically build and test it. Correct any errors promptly.
9. Once all online tests pass, submit a `PR` (Pull Request) and request to merge it into the `dev` branch on [GitHub](https://github.com/sipeed/MaixCDK).

## Manually Adding a MaixPy API

The above method can automatically generate a MaixPy API, but in certain scenarios, manual addition might be necessary. For example, when the parameter is a specific type, such as `numpy.array`:

* Add the header file and code under `components/maix/include` in the `MaixPy` project. You can use the same `namespace` as in `MaixCDK`. For instance, the function `maix.image.cv2image` converts a `numpy` array to an `image.Image` object. Refer to the definitions in the `convert_image.hpp` file.

More references:

* [Building MaixPy](https://wiki.sipeed.com/maixpy/doc/zh/source_code/build.html)
* [Adding a C/C++ Module to MaixPy for MaixCAM](https://wiki.sipeed.com/maixpy/doc/zh/source_code/add_c_module.html)

