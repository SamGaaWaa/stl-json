#include "json.hpp"

#include <iostream>
#include "stdio.h"

int main() {
    auto path = R"(test/face.json)";

    auto fd = fopen(path, "rt");
    std::string str;
    std::array<char, 1024> buffer;

    unsigned int ret;
    while ((ret = fread(buffer.data(), 1, buffer.size(), fd)) == buffer.size())
        str.append(buffer.data(), buffer.data() + buffer.size());
    if (feof(fd)) {
        str.append(buffer.data(), buffer.data() + ret);
        std::cout << str << '\n';
        fclose(fd);
    }
    else if (ferror(fd)) {
        std::cerr << "Read file failed.\n";
        fclose(fd);
        return -1;
    }

    json::_detail::_Parser parser;
    parser.set_buffer(str.data(), str.data() + str.size());

    auto res = parser.parse();
    if (res == json::_detail::_Parser::result::finished) {
        std::optional<json::object> objp = parser.get_result();
        if (objp) {
            json::object obj = std::move(*objp);

            obj["version"] = 1.0;

            auto serializer = json::_detail::_Serializer{};
            serializer(obj);
            std::string json_str = serializer.get_result();

            std::cout << json_str << '\n';

            fd = fopen(R"(test/test.json)", "w");

            ret = fwrite(json_str.data(), 1, json_str.size(), fd);
            if (ret < json_str.size()) {
                std::cerr << "write file error.\n";
            }
        }
    }
    else
        std::cout << "error occurred.\n" << "res ="
        << (res == json::_detail::_Parser::result::error ? "error" : "buffer end") << '\n';

}
