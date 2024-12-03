## 如何编写C++代码，并通过`@maixpy`标注映射为Python函数



#### 1. 创建C++头文件
- 您可以在组件的包含路径中的任何位置创建一个C++头文件（.hpp）。例如，可以创建名为`maix_py_example.hpp`的文件。
- 确保该文件包含了必要的头文件，并且没有重复包含相同的内容（如`<vector>`）。

```cpp
#pragma once

#include <string>
#include <vector>
#include <map>
#include "maix_type.hpp"
```

#### 2. 定义API
- 所有的API应该被定义在`maix`命名空间内。这可以是类、函数、枚举等。
- API应当有清晰的命名，并且应该尽可能简单，避免复杂的C++语法和层次结构。

```cpp
namespace maix {
    namespace example {
        // 定义类、函数、枚举等
    }
}
```

#### 3. 添加注释
- 使用Doxygen风格的注释为每个API添加文档。对于每个参数和返回值，必须提供详细的说明，包括值的范围或注意事项。
- 这些注释不仅帮助开发者理解API，而且也是自动生成Python API的基础。

```cpp
/**
 * @brief 示例函数
 * @param[in] name 名字字符串，值范围无限制
 * @return 返回问候语字符串
 */
std::string hello(std::string name);
```

#### 4. 使用`@maixpy`标注
- 在注释中使用`@maixpy`关键字来指定API应该如何映射到Python。这告诉生成器哪些API需要转换为Python API。
- 对于类成员函数，还需要指定它们在Python中的方法名（通常是`__init__`表示构造函数）。

```cpp
/**
 * @brief 构造函数
 * @maixpy maix.example.Test.__init__
 */
Test();
```

#### 5. 注意事项
- **模块定义**：每个头文件应该只定义一个模块。这意味着在一个头文件中不应定义多个不同的类或函数集合。
- **复杂语法**：避免使用复杂的C++语法，因为这些可能不会被正确地映射到Python。
- **测试过的语法**：仅使用经过测试的C++语法，以确保API能够正确映射。
- **内存管理**：当从C++返回动态分配的对象时，确保Python端能够正确处理这些对象的生命周期。例如，返回`std::vector<int>*`时，Python会自动管理其内存释放。
- **参数传递**：对于列表、字典等复合类型，使用`std::vector`和`std::map`而不是直接使用C/C++数组。
- **回调函数**：支持传递Python回调函数给C++，但需要注意回调函数的签名要匹配。

#### 示例代码解析

- **示例类`Test`**：展示了如何定义一个简单的类，并为其构造函数添加`@maixpy`标注。

```cpp
class Test {
public:
    /**
     * @brief 测试构造函数
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

- **枚举定义**：推荐使用`enum class`来定义枚举，这样可以避免命名冲突，并且更容易映射到Python。

```cpp
enum class Kind2 {
    NONE = 0,
    DOG,
    CAT,
    BIRD,
    MAX
};
```

- **函数`hello`**：展示了一个简单的函数，它接受一个字符串参数并返回一个字符串。

```cpp
/**
 * @brief 说你好
 * @param name 要问候的人的名字
 * @return 问候语
 * @maixpy maix.example.hello
 */
std::string hello(std::string name) {
    return "hello " + name + ", " + std::to_string(test_var);
}
```

- **类`Example`**：展示了如何定义一个更复杂的类，包括成员变量、成员函数和静态成员。

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

- **静态成员和常量**：展示了如何定义静态成员和常量，并将其映射到Python。

```cpp
const std::string var1 = "Sipeed";
static std::string hello_str = "hello ";
```

- **返回动态分配的对象**：展示了如何返回动态分配的对象，并确保Python能够正确管理其生命周期。

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

- **回调函数**：展示了如何接受Python回调函数并在C++中调用它们。

```cpp
static int callback(std::function<int(int, int)> cb) {
    return cb(1, 2);
}
```

- **字典参数**：展示了如何处理字典类型的参数。

```cpp
static std::map<std::string, int> *hello_dict(std::map<std::string, int> *dict) {
    for (auto i : *dict) {
        printf("%s: %d\n", i.first.c_str(), i.second);
    }
    (*dict)["a"] = 100;
    return dict;
}
```

通过以上步骤和示例，您可以编写出能够被正确映射到Python的C++代码。

好的，接下来我们将继续深入探讨一些更具体的细节，并提供更多的示例和最佳实践，以确保您能够顺利地将C++代码映射为Python函数。

### 6. **处理复杂数据类型**

#### 6.1 列表（`std::vector`）
在C++中使用`std::vector`来表示列表，这可以确保与Python的`list`类型兼容。当从C++返回`std::vector`时，Python会自动将其转换为`list`。同样，当Python传递`list`或`tuple`给C++函数时，它们会被自动转换为`std::vector`。

**示例：返回一个包含整数的列表**

```cpp
/**
 * @brief 获取一个包含整数的列表
 * @param in 输入列表，元素为整数类型
 * @return 返回一个新的列表，内容是 [1, 2, 3] + in
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

#### 6.2 字典（`std::map`）
使用`std::map<std::string, int>`来表示字典，其中键为字符串，值为整数。Python中的`dict`对象会被自动转换为`std::map`，反之亦然。

**示例：处理字典参数**

```cpp
/**
 * @brief 处理字典参数
 * @param in 输入字典，键为字符串类型，值为整数类型
 * @return 返回一个新的字典，内容是 {"a": 1} + in
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

#### 6.3 字节序列（`Bytes`）
对于字节序列，使用自定义的`Bytes`类来表示。Python中的`bytes`对象会被自动转换为`Bytes`类的实例。注意，修改`Bytes`对象不会影响原始的Python `bytes`对象。

**示例：处理字节序列**

```cpp
/**
 * @brief 处理字节序列
 * @param bytes 字节序列参数
 * @return 返回一个新的字节序列，内容是修改后的字节序列
 * @maixpy maix.example.Example.hello_bytes
 */
static Bytes *hello_bytes(Bytes &bytes) {
    printf("hello_bytes: %ld\n", bytes.size());
    for (auto i : bytes) {
        printf("%02x ", i);
    }
    printf("\n");

    // 修改字节序列不会影响原始的Python bytes对象
    bytes.data[0] = 0x11;
    bytes.data[1] = 0x22;
    return new Bytes(bytes.data, bytes.size(), true, true);
}
```

### 7. **回调函数**

C++支持通过`std::function`来接受Python回调函数。Python回调函数可以传递给C++函数，并在C++中调用。回调函数的签名必须与C++函数期望的签名匹配。

**示例：接受并调用回调函数**

```cpp
/**
 * @brief 回调函数示例
 * @param cb 回调函数，参数为两个整数，返回值为整数
 * @return 返回回调函数的返回值
 * @maixpy maix.example.Example.callback
 */
static int callback(std::function<int(int, int)> cb) {
    return cb(1, 2);
}

/**
 * @brief 回调函数示例（带列表参数）
 * @param cb 回调函数，参数为一个整数列表和一个整数，返回值为整数
 * @return 返回回调函数的返回值
 * @maixpy maix.example.Example.callback2
 */
static int callback2(std::function<int(std::vector<int>, int)> cb) {
    std::vector<int> a;
    a.push_back(1);
    return cb(a, 2);
}
```

### 8. **静态成员和常量**

静态成员和常量可以直接映射到Python。静态成员可以通过类名访问，而常量则可以在Python中作为只读属性访问。

**示例：静态成员和常量**

```cpp
/**
 * @brief 静态成员变量
 * @maixpy maix.example.Example.hello_str
 */
static std::string hello_str = "hello ";

/**
 * @brief 只读变量
 * @maixpy maix.example.Example.var1
 */
const std::string var1 = "Example.var1";

/**
 * @brief 只读变量（明确标注为只读）
 * @maixpy maix.example.Example.var2
 * :readonly
 */
std::string var2 = "Example.var2";
```

### 9. **内存管理**

当从C++返回动态分配的对象时，Python会自动管理这些对象的生命周期。这意味着Python会在适当的时候释放这些对象的内存。为了确保这一点，返回的对象应该使用指针（如`std::vector<int>*`）而不是直接返回对象本身。

**示例：返回动态分配的对象**

```cpp
/**
 * @brief 返回一个包含Test对象的字典
 * @return 返回一个字典，键为字符串，值为Test对象
 * @maixpy maix.example.Example.dict_test
 */
static std::map<std::string, example::Test *> *dict_test() {
    std::map<std::string, example::Test *> *dict = new std::map<std::string, example::Test *>;
    (*dict)["a"] = new Test();
    return dict;
}
```

### 10. **更改参数名称**

有时您可能希望在Python中使用不同的参数名称，而不改变C++代码中的名称。您可以使用`@maixpy`标注来指定Python中的参数名称。

**示例：更改参数名称**

```cpp
/**
 * @brief 更改参数名称示例
 * @param e Example对象
 * @return 返回相同的对象
 * @maixpy maix.example.change_arg_name
 */
example::Example *change_arg_name(example::Example *e) {
    e->name = "changed_name";
    return e;
}

/**
 * @brief 更改参数名称示例（引用传递）
 * @param e Example对象
 * @maixpy maix.example.change_arg_name2
 */
void change_arg_name2(example::Example &e) {
    e.name = "changed_name2";
}
```

### 11. **枚举类**

推荐使用`enum class`来定义枚举类型，因为它们提供了更好的命名空间管理和类型安全。`enum class`可以被正确地映射到Python中的枚举类型。

**示例：枚举类**

```cpp
/**
 * @brief 示例枚举类
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

### 12. **错误处理**

在C++中抛出的异常会被自动转换为Python中的异常。因此，您可以使用标准的C++异常机制来处理错误，并确保这些错误能够在Python中被捕获。

**示例：错误处理**

```cpp
/**
 * @brief 构造函数
 * @param name 名字字符串
 * @param age 年龄，范围为 [0, 100]
 * @param pet 宠物类型，默认为 NONE
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

### 13. **总结**

通过以上步骤和示例，您可以编写出能够被正确映射到Python的C++代码。以下是几个关键点的总结：

- **命名空间**：所有API应位于`maix`命名空间内。
- **注释**：使用Doxygen风格的注释，并确保每个参数和返回值都有详细的说明。
- **`@maixpy`标注**：使用`@maixpy`标注来指定API如何映射到Python。
- **复杂数据类型**：使用`std::vector`、`std::map`等标准库类型来处理列表和字典。
- **回调函数**：使用`std::function`来接受Python回调函数。
- **静态成员和常量**：静态成员和常量可以直接映射到Python。
- **内存管理**：返回动态分配的对象时，确保Python能够正确管理其生命周期。
- **错误处理**：使用C++异常机制来处理错误，并确保这些错误能够在Python中被捕获。

