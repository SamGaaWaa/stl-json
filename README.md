# stl-json

轻量级json解析器，使用C++11标准，仅依赖标准库

核心数据结构 json::value, 能容纳

- double、int
- bool
- json::null
- std::string
- std::vector\<json::value>
- std::unordered_map<std::string, json::value>  

---
    json::value val;
    val = 3.14;
    val = 123;
    val = true;
    val = json::null;
    val = "hello";
    val = std::vector<json::value>{ {3.14}, {123}, {true}, {"hello"}};
    val = std::unordered_map<std::string, json::value>{
        {"key1", {3.14}},
        {"key2" , { std::vector<json::value>({ {123}, {false} }) }}
    };

通过 get_XXX() 获取相应类型引用，类型不符合抛异常

    val.get_object()["key1"] = 3.1415926;
    
    //get_array();
    //get_number();
    //get_string();
    //get_bool();

json::array、json::object 分别是 std::vector\<json::value> 和 std::unorder_map\<std::string, json::value>的别名

    using array = std::vector<value>;
    using object = std::unorder_map<std::string, value>;

实现为LL1文法的状态机，解析过程可中断。

    json::object obj = json::from_file(R"(Big.json)", 4096); //每次只有4KB数据被读入内存。 

主要函数
-

    json::object parse(const char *first, const char *last);

    json::object from_string(const char*);
    json::object from_string(const std::string&);
    json::object from_file(const char *path, size_t buffer_size = 4096);
    json::object from_file(const std::string& path, size_t buffer_size = 4096);

    std::string to_json(const json::object&);
