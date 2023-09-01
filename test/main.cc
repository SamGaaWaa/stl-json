#include "../json.hpp"

#include <format>
#include <fstream>
#include <chrono>
#include <filesystem>

#include "cJSON.h"


std::string read_file(const std::string& path)noexcept {
    std::ifstream file(path);
    std::string content;
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            content += line + "\n";
        }
        file.close();
    }
    else {
        std::cerr << "Can't open file.\n";
        return "";
    }
    if (!content.empty())
        content.pop_back();
    return content;
}

void parse_test() {
    auto dir = R"(./test/json/)";
    for (auto& p : std::filesystem::directory_iterator(dir)) {
        auto file_name = p.path().filename().string();
        auto js = read_file(p.path().string());
        auto ret = json::parse(js.data(), js.size());
        if (file_name.starts_with("fail")) {
            if (!ret.has_value())
                std::cout << std::format("Test {:32}: OK!\n", file_name);
            else std::cout << std::format("Test {:32}: FAILED!\n", file_name);
        }
        else {
            if (ret.has_value()) {
                std::cout << std::format("Test {:32}: OK!\n", file_name);
            }
            else std::cout << std::format("Test {:32}: FAILED!\n", file_name);
        }
    }
}

void from_file_test() {
    auto dir = R"(./test/json/)";

    constexpr auto parse_again = [ ](const json::document& dom)noexcept->bool {
        const auto js = json::to_json(dom);
        if (!js.has_value()) {
            std::cout << std::format("Serialize error:{}\n", json::error_string(js.error()));
            return false;
        }
        if (auto ret = json::parse(js->data(), js->size()); !ret.has_value()) {
            std::cout << std::format("Parse error:{}\n", json::error_string(ret.error()));
            return false;
        }
        return true;
        };

    for (auto& p : std::filesystem::directory_iterator(dir)) {
        auto file_name = p.path().filename().string();
        auto ret = json::from_file(p.path().string(), 4096 * 4);
        if (file_name.starts_with("fail")) {
            if (!ret.has_value())
                std::cout << std::format("Test {:32}: OK!\n", file_name);
            else std::cout << std::format("Test {:32}: FAILED!\n", file_name);
        }
        else {
            if (ret.has_value() and parse_again(*ret)) {
                std::cout << std::format("Test {:32}: OK!\n", file_name);
            }
            else std::cout << std::format("Test {:32}: FAILED! {}\n", file_name, ret.has_value() ? "" : json::error_string(ret.error()));
        }
    }
}

void single_file_test() {
    auto path = R"(./test/json/big.json)";
    auto dom = json::from_file(path, 4096, 1000);
    if (!dom.has_value()) {
        std::cout << std::format("Error:{}\n", json::error_string(dom.error()));
    }
}

void make_big_file() {
    auto dir = R"(./test/json/)";
    json::array arr;
    for (auto& p : std::filesystem::directory_iterator(dir)) {
        auto file_name = p.path().filename().string();
        auto ret = json::from_file(p.path().string(), 4096 * 4);
        if (ret.has_value()) {
            if (std::holds_alternative<json::array>(*ret))
                arr.emplace_back(std::move(std::get<json::array>(*ret)));
            else if (std::holds_alternative<json::object>(*ret))
                arr.emplace_back(std::move(std::get<json::object>(*ret)));
            else std::cout << std::format("Error while parsing {}\n", file_name);
        }
    }
    const auto js = json::to_json(arr).value();
    //std::cout << js << '\n';
    //if (auto ret = json::parse(js.data(), js.size()); !ret.has_value()) {
    //    std::cout << json::error_string(ret.error()) << '\n';
    //}
    std::ofstream file{ R"(./test/json/big.json)" };
    if (file.is_open()) {
        file << js;
        file.close();
    }
}

void benchmark_from_file() {
    std::cout << "Benchmark from_file...\n";

    auto path = R"(./test/json/big.json)";
    const auto js = read_file(path);
    std::cout << js.size() << '\n';

    const int n = 100;

    auto size = 0;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < n; ++i) {
        auto dom = json::from_file(path, 4096, 100);
        if (dom and std::holds_alternative<json::array>(*dom))
            size += std::get<json::array>(*dom).size();
    }
    auto end = std::chrono::steady_clock::now();

    std::cout << std::format("result:{}\n", size);

    double per_parsing = std::chrono::duration<double>(end - start).count() / n;
    std::cout << std::format("{} MB/s\n\n", (js.size() >> 20) / per_parsing);
}

void benchmark_parse() {
    std::cout << "Benchmark parse...\n";

    auto path = R"(./test/json/big.json)";
    const auto js = read_file(path);
    std::cout << js.size() << '\n';

    const int n = 100;

    auto size = 0;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < n; ++i) {
        auto dom = json::parse(js.data(), js.size(), 100);
        if (dom and std::holds_alternative<json::array>(*dom))
            size += std::get<json::array>(*dom).size();
    }
    auto end = std::chrono::steady_clock::now();

    std::cout << std::format("result:{}\n", size);

    double per_parsing = std::chrono::duration<double>(end - start).count() / n;
    std::cout << std::format("{} MB/s\n\n", (js.size() >> 20) / per_parsing);
}

void benchmark_cJSON_parse() {
    std::cout << "Benchmark cJSON parse...\n";

    auto path = R"(./test/json/big.json)";
    const auto js = read_file(path);
    std::cout << js.size() << '\n';

    const int n = 100;

    auto size = 0;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < n; ++i) {
        auto dom = cJSON_Parse(js.data());
        if (dom and dom->type == cJSON_Array) {
            size += cJSON_GetArraySize(dom);
        }
        cJSON_Delete(dom);
    }
    auto end = std::chrono::steady_clock::now();

    std::cout << std::format("result:{}\n", size);

    double per_parsing = std::chrono::duration<double>(end - start).count() / n;
    std::cout << std::format("{} MB/s\n\n", (js.size() >> 20) / per_parsing);
}

void benchmark_pure_parse() {
    std::cout << "Benchmark pure parse...\n";

    auto path = R"(./test/json/big.json)";
    const auto js = read_file(path);
    std::cout << js.size() << '\n';

    const int n = 100;

    auto size = 0;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < n; ++i) {
        auto dom = json::parse<json::detail::document_printer>(js.data(), js.size(), 100);
        if (dom)
            size += *dom;
    }
    auto end = std::chrono::steady_clock::now();

    std::cout << std::format("result:{}\n", size);

    double per_parsing = std::chrono::duration<double>(end - start).count() / n;
    std::cout << std::format("{} MB/s\n\n", (js.size() >> 20) / per_parsing);
}

int main() {
    parse_test();
    from_file_test();
    //single_file_test();
    make_big_file();
    benchmark_from_file();
    benchmark_parse();
    benchmark_cJSON_parse();
    benchmark_pure_parse();
    return 0;
}