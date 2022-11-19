#include <iostream>
#include <fstream>
#include <cstring>

#include "json.hpp"

void c_str_test() {
    const char* json = R"({
    "face_num": 1,
    "face_list": [
null]})";

    auto obj = json::parse(json, json + strlen(json));
    obj["version"] = { 1.1 };
    auto str = json::to_json(obj);

    std::cout << "json: \n" << str << '\n';
}

void file_test() {
    try {
        const char* path = R"(./test/face.json)";
        auto obj = json::parse(path, 1);        //缓冲区设置为1，逐个字符解析
        std::cout << "size=" << obj.size() << '\n';
        std::cout << "json:\n" << json::to_json(obj) << '\n';
        std::ofstream os{ R"(./test/test.json)" };

        obj.insert({ "array", json::array{{ {123}, {"123"} }} });

        os << json::to_json(obj);
        os.close();

    }
    catch (const std::exception& e) {
        std::cout << e.what() << '\n';
    }
}

int main() {
    c_str_test();
    file_test();
}
