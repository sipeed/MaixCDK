## How to Write C++ Code and Map it to Python Functions Using `@maixpy` Annotations

### 1. Creating a C++ Header File
- You can create a C++ header file (.hpp) at any location within the component's include path. For example, you might create a file named `maix_py_example.hpp`.
- Ensure that this file includes necessary headers and does not redundantly include the same content (such as `<vector>`).

```cpp
#pragma once

#include <string>
#include <vector>
#include <map>
#include "maix_type.hpp"
```

### 2. Defining APIs
- All APIs should be defined within the `maix` namespace. This can include classes, functions, enums, etc.
- APIs should have clear names and be as simple as possible, avoiding complex C++ syntax and hierarchies.

```cpp
namespace maix {
    namespace example {
        // Define classes, functions, enums, etc.
    }
}
```

### 3. Adding Comments
- Use Doxygen-style comments to document each API. Provide detailed explanations for each parameter and return value, including value ranges or important notes.
- These comments not only help developers understand the API but also serve as the basis for automatically generating Python APIs.

```cpp
/**
 * @brief Example function
 * @param[in] name A string representing a name, with no restrictions on its value range.
 * @return Returns a greeting string.
 */
std::string hello(std::string name);
```

### 4. Using `@maixpy` Annotations
- In the comments, use the `@maixpy` keyword to specify how the API should be mapped to Python. This tells the generator which APIs need to be converted into Python APIs.
- For class member functions, specify their method names in Python (usually `__init__` for constructors).

```cpp
/**
 * @brief Constructor
 * @maixpy maix.example.Test.__init__
 */
Test();
```

### 5. Precautions
- **Module Definition**: Each header file should define only one module. This means that multiple different classes or function collections should not be defined in a single header file.
- **Complex Syntax**: Avoid using complex C++ syntax, as it may not map correctly to Python.
- **Tested Syntax**: Only use C++ syntax that has been tested to ensure proper mapping.
- **Memory Management**: When returning dynamically allocated objects from C++, ensure that Python can correctly manage their lifecycle. For example, when returning `std::vector<int>*`, Python will automatically handle memory release.
- **Parameter Passing**: For composite types like lists and dictionaries, use `std::vector` and `std::map` instead of direct C/C++ arrays.
- **Callback Functions**: Support passing Python callback functions to C++. Note that the signature of the callback function must match what C++ expects.

### Example Code Analysis

- **Example Class `Test`**: Demonstrates how to define a simple class and add `@maixpy` annotations to its constructor.

```cpp
class Test {
public:
    /**
     * @brief Test constructor
     * @maixpy maix.example.Test.__init__
     */
    Test() {
        data = new int[1024 * 1024];
        printf("Test() new data: %p, size: %d\n", data, 1024 * 1024);
    }

    ~Test() {
        printf("~Test() delete data: %p\n", data);
        delete[] (int *)data;
    }

private:
    void *data;
};
```

- **Enum Definition**: It is recommended to use `enum class` to define enums, which avoids naming conflicts and makes it easier to map to Python.

```cpp
/**
 * @brief Example enum class
 * @maixpy maix.example.Kind2
 */
enum class Kind2 {
    NONE = 0,
    DOG,
    CAT,
    BIRD,
    MAX
};
```

- **Function `hello`**: Shows a simple function that accepts a string parameter and returns a string.

```cpp
/**
 * @brief Say hello
 * @param name The name of the person to greet
 * @return Greeting message
 * @maixpy maix.example.hello
 */
std::string hello(std::string name) {
    return "hello " + name + ", " + std::to_string(test_var);
}
```

- **Class `Example`**: Demonstrates how to define a more complex class, including member variables, member functions, and static members.

```cpp
class Example {
public:
    Example(std::string &name, int age = 18, example::Kind pet = example::KIND_NONE);

    std::string get_name();
    int get_age();
    void set_name(std::string name);
    void set_age(int age);
    void set_pet(example::Kind pet);
    example::Kind get_pet();

    static std::string hello_str;

    std::string name;
    int age;
};
```

- **Static Members and Constants**: Shows how to define static members and constants and map them to Python.

```cpp
const std::string var1 = "Sipeed";
static std::string hello_str = "hello ";
```

- **Returning Dynamically Allocated Objects**: Demonstrates how to return dynamically allocated objects and ensure Python manages their lifecycle correctly.

```cpp
std::vector<int> *get_list(std::vector<int> in) {
    std::vector<int> *final = new std::vector<int>;
    final->push_back(1);
    final->push_back(2);
    final->push_back(3);
    for (auto i : in) {
        final->push_back(i);
    }
    return final;
}
```

- **Callback Functions**: Shows how to accept and call Python callback functions in C++.

```cpp
static int callback(std::function<int(int, int)> cb) {
    return cb(1, 2);
}
```

- **Dictionary Parameters**: Demonstrates how to handle dictionary parameters.

```cpp
static std::map<std::string, int> *hello_dict(std::map<std::string, int> *dict) {
    for (auto i : *dict) {
        printf("%s: %d\n", i.first.c_str(), i.second);
    }
    (*dict)["a"] = 100;
    return dict;
}
```

### 6. Handling Complex Data Types

#### 6.1 Lists (`std::vector`)
Use `std::vector` in C++ to represent lists, ensuring compatibility with Python's `list` type. When returning a `std::vector` from C++, Python will automatically convert it to a `list`. Similarly, when Python passes a `list` or `tuple` to a C++ function, they are automatically converted to `std::vector`.

**Example: Returning a List of Integers**

```cpp
/**
 * @brief Get a list of integers
 * @param in Input list of integers
 * @return Returns a new list containing [1, 2, 3] + in
 * @maixpy maix.example.Example.get_list
 */
std::vector<int> *get_list(std::vector<int> in) {
    std::vector<int> *final = new std::vector<int>;
    final->push_back(1);
    final->push_back(2);
    final->push_back(3);
    for (auto i : in) {
        final->push_back(i);
    }
    return final;
}
```

#### 6.2 Dictionaries (`std::map`)
Use `std::map<std::string, int>` to represent dictionaries, where keys are strings and values are integers. Python's `dict` objects are automatically converted to `std::map`, and vice versa.

**Example: Handling Dictionary Parameters**

```cpp
/**
 * @brief Handle dictionary parameters
 * @param in Input dictionary with string keys and integer values
 * @return Returns a new dictionary containing {"a": 1} + in
 * @maixpy maix.example.Example.get_dict
 */
std::map<std::string, int> get_dict(std::map<std::string, int> &in) {
    std::map<std::string, int> final = {{"a", 1}};
    for (auto i : in) {
        final[i.first] = i.second;
    }
    return final;
}
```

#### 6.3 Byte Sequences (`Bytes`)
For byte sequences, use a custom `Bytes` class. Python's `bytes` objects are automatically converted to instances of the `Bytes` class. Note that modifying the `Bytes` object does not affect the original Python `bytes` object.

**Example: Handling Byte Sequences**

```cpp
/**
 * @brief Handle byte sequences
 * @param bytes Byte sequence parameter
 * @return Returns a new byte sequence with modified content
 * @maixpy maix.example.Example.hello_bytes
 */
static Bytes *hello_bytes(Bytes &bytes) {
    printf("hello_bytes: %ld\n", bytes.size());
    for (auto i : bytes) {
        printf("%02x ", i);
    }
    printf("\n");

    // Modifying the byte sequence does not affect the original Python bytes object
    bytes.data[0] = 0x11;
    bytes.data[1] = 0x22;
    return new Bytes(bytes.data, bytes.size(), true, true);
}
```

### 7. Callback Functions

C++ supports accepting Python callback functions via `std::function`. Python callback functions can be passed to C++ functions and called within C++. The signature of the callback function must match what C++ expects.

**Example: Accepting and Calling Callback Functions**

```cpp
/**
 * @brief Callback function example
 * @param cb Callback function taking two integers and returning an integer
 * @return Returns the result of the callback function
 * @maixpy maix.example.Example.callback
 */
static int callback(std::function<int(int, int)> cb) {
    return cb(1, 2);
}

/**
 * @brief Callback function example (with list parameter)
 * @param cb Callback function taking a list of integers and an integer, returning an integer
 * @return Returns the result of the callback function
 * @maixpy maix.example.Example.callback2
 */
static int callback2(std::function<int(std::vector<int>, int)> cb) {
    std::vector<int> a;
    a.push_back(1);
    return cb(a, 2);
}
```

### 8. Static Members and Constants

Static members and constants can be directly mapped to Python. Static members can be accessed via the class name, while constants can be accessed as read-only attributes in Python.

**Example: Static Members and Constants**

```cpp
/**
 * @brief Static member variable
 * @maixpy maix.example.Example.hello_str
 */
static std::string hello_str = "hello ";

/**
 * @brief Read-only variable
 * @maixpy maix.example.Example.var1
 */
const std::string var1 = "Example.var1";

/**
 * @brief Read-only variable (explicitly marked as read-only)
 * @maixpy maix.example.Example.var2
 * :readonly
 */
std::string var2 = "Example.var2";
```

### 9. Memory Management

When returning dynamically allocated objects from C++, Python will automatically manage their lifecycle. This means Python will release the memory of these objects when appropriate. To ensure this, returned objects should be pointers (e.g., `std::vector<int>*`) rather than the objects themselves.

**Example: Returning Dynamically Allocated Objects**

```cpp
/**
 * @brief Return a dictionary containing Test objects
 * @return Returns a dictionary with string keys and Test object values
 * @maixpy maix.example.Example.dict_test
 */
static std::map<std::string, example::Test *> *dict_test() {
    std::map<std::string, example::Test *> *dict = new std::map<std::string, example::Test *>;
    (*dict)["a"] = new Test();
    return dict;
}
```

### 10. Changing Parameter Names

Sometimes you may want to use different parameter names in Python without changing the C++ code. You can use the `@maixpy` annotation to specify the parameter names in Python.

**Example: Changing Parameter Names**

```cpp
/**
 * @brief Change parameter name example
 * @param e Example object
 * @return Returns the same object
 * @maixpy maix.example.change_arg_name
 */
example::Example *change_arg_name(example::Example *e) {
    e->name = "changed_name";
    return e;
}

/**
 * @brief Change parameter name example (pass by reference)
 * @param e Example object
 * @maixpy maix.example.change_arg_name2
 */
void change_arg_name2(example::Example &e) {
    e.name = "changed_name2";
}
```

### 11. Enum Classes

It is recommended to use `enum class` to define enum types, as they provide better namespace management and type safety. `enum class` can be correctly mapped to Python's enum types.

**Example: Enum Class**

```cpp
/**
 * @brief Example enum class
 * @maixpy maix.example.Kind2
 */
enum class Kind2 {
    NONE = 0,
    DOG,
    CAT,
    BIRD,
    MAX
};
```

### 12. Error Handling

Exceptions thrown in C++ are automatically converted to Python exceptions. Therefore, you can use standard C++ exception handling mechanisms to manage errors, ensuring that these errors can be caught in Python.

**Example: Error Handling**

```cpp
/**
 * @brief Constructor
 * @param name String representing a name
 * @param age Age, with a valid range of [0, 100]
 * @param pet Pet type, default is NONE
 * @maixpy maix.example.Example.__init__
 */
Example(std::string &name, int age = 18, example::Kind pet = example::KIND_NONE) {
    if (age < 0 || age > 100) {
        throw std::invalid_argument("age should be in [0, 100]");
    }
    this->name = name;
    this->age = age;
    this->_pet = pet;
}
```

### 13. Summary

By following the steps and examples provided, you can write C++ code that can be correctly mapped to Python. Here are some key points to summarize:

- **Namespace**: All APIs should be defined within the `maix` namespace.
- **Comments**: Use Doxygen-style comments and ensure that each parameter and return value is thoroughly documented.
- **`@maixpy` Annotations**: Use `@maixpy` annotations to specify how APIs should be mapped to Python.
- **Complex Data Types**: Use `std::vector`, `std::map`, and other standard library types to handle lists and dictionaries.
- **Callback Functions**: Use `std::function` to accept Python callback functions.
- **Static Members and Constants**: Static members and constants can be directly mapped to Python.
- **Memory Management**: Ensure that Python can correctly manage the lifecycle of dynamically allocated objects returned from C++.
- **Error Handling**: Use C++ exception handling mechanisms to manage errors, ensuring that these errors can be caught in Python.

