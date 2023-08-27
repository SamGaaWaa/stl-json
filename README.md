# stl-json

轻量级json解析器，使用C++23(gcc13、vs2022)，仅依赖标准库

核心数据结构 json::value, 能容纳

- `double`
- `bool`
- `json::null`
- `std::string`
- `std::vector<json::value>`
- `std::map<std::string, json::value>`  

```c++
    json::value val;
    val = 3.14;
    val = true;
    val = json::null;
    val = "hello";
    val = { 3.14, json::object{}, true, "hello"}; // using array = std::vector<value>;
    val = {
        {"key1", 3.14},
        {"key2" , { 123., false } },
        {"key3", false}
    };  // using object = std::map<std::string, value>
```


通过 get_XXX() 获取相应类型引用，类型不符合抛异常, is_XXX()判断类型，emplace_XXX()转换为相应类型

```c++
    val.get_object()["key1"] = 3.1415926;
    val.get_object()["arr"].get_array()[12] = true;

    //或者更简单
    val["key1"] = 3.1415926; 
    val["arr"][12] = true;
    val["key1"].emplace_string() = "hello";
    val["key1"].emplace_number(2.714);
    
    //get_array();
    //get_number();
    //get_string();
    //get_bool();
    //get_null();
    //get_object();

    //emplace_object();
    //emplace_array();
    //emplace_number();
    //emplace_null();
    //emplace_bool();
    //emplace_string();

    //is_object();
    //is_array();
    //is_string();
    //is_number();
    //is_bool();
    //is_null();
```

json::array、json::object 分别是 std::vector\<json::value> 和 std::map\<std::string, json::value>的别名

```c++
    using array = std::vector<value>;
    using object = std::map<std::string, value>;
    using document = std::variant<std::monostate, array, object>;

```
实现为LL1文法的状态机，解析过程可中断。

```c++
    std::expected<json::document, parse_error_t> dom = json::from_file(R"(Big.json)", 4096); //每次只有4KB数据被读入内存。 

```
主要函数
-

```c++
    std::expected<json::document, parse_error_t> parse(const char* data, size_t size, int depth = 19);

    std::expected<json::document, parse_error_t> from_file(const std::string& path, size_t buf_size = 4096, int depth = 19);

    std::expected<std::string, serialize_error_t> to_json(const auto& dom);

```
Benchmark
-

```shell
make test
```