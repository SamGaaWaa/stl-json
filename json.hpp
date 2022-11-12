#ifndef __JSON_H__
#define __JSON_H__

#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>
#include <type_traits>
#include <charconv>
#include <system_error>
#include <functional>
#include <optional>

namespace json {
    namespace _detail {
        struct null_t {};
        template<class T> using vec = std::vector<T>;
        template<class K, class V> using map = std::unordered_map<K, V>;
        template<class T>
        using _value = std::variant<
            std::monostate,
            null_t,
            double,
            std::string,
            vec<T>,
            map<std::string, T>,
            bool>;
        struct _Serializer;
    }
    constexpr _detail::null_t null;


    struct value {
        value() = default;

        value(const value&) = default;

        value& operator=(const value&) = default;

        value(value&&) = default;

        value& operator=(value&&) = default;

        value(const std::string& str) { _val.emplace<3>(str); }

        value& operator=(const std::string& str) {
            _val.emplace<3>(str);
            return *this;
        }

        value(std::string&& str) noexcept { _val.emplace<3>(std::move(str)); }

        value& operator=(std::string&& str) noexcept {
            _val.emplace<3>(std::move(str));
            return *this;
        }

        value(const char* str) { _val.emplace<3>(str); }

        value& operator=(const char* str) {
            _val.emplace<3>(str);
            return *this;
        }

        value(double x) noexcept { _val.emplace<2>(x); }

        value(int x) noexcept { _val.emplace<2>(static_cast<double>(x)); }

        value(long long x) noexcept { _val.emplace<2>(static_cast<double>(x)); }

        value(unsigned int x) noexcept { _val.emplace<2>(static_cast<double>(x)); }

        value(unsigned long long x) noexcept { _val.emplace<2>(static_cast<double>(x)); }

        value(short x) noexcept { _val.emplace<2>(static_cast<double>(x)); }

        value(unsigned short x) noexcept { _val.emplace<2>(static_cast<double>(x)); }

        value& operator=(double x) noexcept {
            _val.emplace<2>(static_cast<double>(x));
            return *this;
        }

        value& operator=(int x) noexcept {
            _val.emplace<2>(static_cast<double>(x));
            return *this;
        }

        value& operator=(long long x) noexcept {
            _val.emplace<2>(static_cast<double>(x));
            return *this;
        }

        value& operator=(unsigned int x) noexcept {
            _val.emplace<2>(static_cast<double>(x));
            return *this;
        }

        value& operator=(unsigned long long x) noexcept {
            _val.emplace<2>(static_cast<double>(x));
            return *this;
        }

        value& operator=(short x) noexcept {
            _val.emplace<2>(static_cast<double>(x));
            return *this;
        }

        value& operator=(unsigned short x) noexcept {
            _val.emplace<2>(static_cast<double>(x));
            return *this;
        }


        value(_detail::null_t) noexcept { _val.emplace<1>(null); }

        value& operator=(_detail::null_t) noexcept {
            _val.emplace<1>(null);
            return *this;
        }

        value(const _detail::vec<value>& vec) { _val.emplace<4>(vec); }

        value& operator=(const _detail::vec<value>& vec) {
            _val.emplace<4>(vec);
            return *this;
        }

        value(_detail::vec<value>&& vec) noexcept { _val.emplace<4>(std::move(vec)); }

        value& operator=(_detail::vec<value>&& vec) noexcept {
            _val.emplace<4>(std::move(vec));
            return *this;
        }

        value(const _detail::map<std::string, value>& map) { _val.emplace<5>(map); }

        value& operator=(const _detail::map<std::string, value>& map) {
            _val.emplace<5>(map);
            return *this;
        }

        value(_detail::map<std::string, value>&& map) noexcept { _val.emplace<5>(std::move(map)); }

        value& operator=(_detail::map<std::string, value>&& map) noexcept {
            _val.emplace<5>(std::move(map));
            return *this;
        }

        value(bool b) noexcept { _val.emplace<6>(b); }

        value& operator=(bool b) noexcept {
            _val.emplace<6>(b);
            return *this;
        }

        int get_int() { return (int)std::get<2>(_val); }

        double get_double() { return std::get<2>(_val); }

        [[nodiscard]] std::string get_string() const { return std::get<3>(_val); }

        std::string& get_string() { return std::get<3>(_val); }

        [[nodiscard]] _detail::vec<value> get_array() const { return std::get<4>(_val); }

        _detail::vec<value>& get_array() { return std::get<4>(_val); }

        [[nodiscard]] _detail::map<std::string, value> get_object() const { return std::get<5>(_val); }

        _detail::map<std::string, value>& get_object() { return std::get<5>(_val); }

        bool& get_bool() noexcept { return std::get<6>(_val); }

        bool get_bool() const { return std::get<6>(_val); }

        const char* type() noexcept {
            switch (_val.index()) {
                case 0:
                    return "null";
                case 1:
                    return "null";
                case 2:
                    return "number";
                case 3:
                    return "string";
                case 4:
                    return "array";
                case 5:
                    return "object";
                case 6:
                    return "boolean";
                default:
                    return "";
            }
        }

        private:
        friend struct _detail::_Serializer;
        _detail::_value<value> _val;
    };


    using array = _detail::vec<value>;
    using object = _detail::map<std::string, value>;

    namespace _detail {

        enum class Token {
            Lbrace,
            Rbrace,
            Number,
            String,
            Null,
            LSquare,
            RSquare,
            Comma,  //逗号
            Colon,  //冒号
            True,
            False,
            BUFFER_END,
            ERROR
        };

        class _Tokenizer {
            public:
            void set_buffer(const char* begin, const char* end) {
                _iter = begin;
                _end = end;
            }

            [[nodiscard]] Token peek() const noexcept { return _last_token; }

            auto get_token() {
                if (_last_token == Token::Number and !_double.empty())
                    return _last_token;
                if (_last_token == Token::String and !_string.empty())
                    return _last_token;

                while (_iter != _end) {
                    const char c = *_iter;
                    switch (_state) {
                        case State::P_STR:
                            if (c != '\"') {
                                ++_iter;
                                _string.push_back(c);
                                continue;
                            }
                            else {
                                ++_iter;
                                _last_token = Token::String;
                                return Token::String;
                            }
                        case State::P_NUM:
                            {
                                switch (c) {
                                    case ' ':
                                    case ',':
                                    case ']':
                                    case '}':
                                    case '\n':
                                    case '\t':
                                        _last_token = Token::Number;
                                        return Token::Number;
                                    case '0':
                                    case '1':
                                    case '2':
                                    case '3':
                                    case '4':
                                    case '5':
                                    case '6':
                                    case '7':
                                    case '8':
                                    case '9':
                                        _double.push_back(c);
                                        ++_iter;
                                        continue;
                                    case '.':
                                        _double.push_back(c);
                                        _state = State::P_NUM_DOT;
                                        ++_iter;
                                        continue;
                                    default:
                                        _last_token = Token::ERROR;
                                        return Token::ERROR;
                                }
                            }
                        case State::P_NUM_DOT:
                            {
                                switch (c) {
                                    case '0':
                                    case '1':
                                    case '2':
                                    case '3':
                                    case '4':
                                    case '5':
                                    case '6':
                                    case '7':
                                    case '8':
                                    case '9':
                                        _double.push_back(c);
                                        ++_iter;
                                        _state = State::P_NUM_DOT_NUM;
                                        continue;
                                    default:
                                        _last_token = Token::ERROR;
                                        return Token::ERROR;
                                }
                            }
                        case State::P_NUM_DOT_NUM:
                            {
                                switch (c) {
                                    case '0':
                                    case '1':
                                    case '2':
                                    case '3':
                                    case '4':
                                    case '5':
                                    case '6':
                                    case '7':
                                    case '8':
                                    case '9':
                                        _double.push_back(c);
                                        ++_iter;
                                        continue;
                                    case ' ':
                                    case ',':
                                    case ']':
                                    case '}':
                                    case '\n':
                                    case '\t':
                                        _last_token = Token::Number;
                                        return Token::Number;
                                    case 'e':
                                    case 'E':
                                        _state = State::P_NUM_DOT_NUM_E;
                                        _double.push_back(c);
                                        ++_iter;
                                        continue;
                                    default:
                                        _last_token = Token::ERROR;
                                        return Token::ERROR;
                                }
                            }
                        case State::P_NUM_DOT_NUM_E:
                            {
                                switch (c) {
                                    case '0':
                                    case '1':
                                    case '2':
                                    case '3':
                                    case '4':
                                    case '5':
                                    case '6':
                                    case '7':
                                    case '8':
                                    case '9':
                                        _double.push_back(c);
                                        ++_iter;
                                        _state = State::P_NUM_DOT_NUM_E_NUM;
                                        continue;
                                    default:
                                        _last_token = Token::ERROR;
                                        return Token::ERROR;
                                }
                            }
                        case State::P_NUM_DOT_NUM_E_NUM:
                            {
                                switch (c) {
                                    case '0':
                                    case '1':
                                    case '2':
                                    case '3':
                                    case '4':
                                    case '5':
                                    case '6':
                                    case '7':
                                    case '8':
                                    case '9':
                                        _double.push_back(c);
                                        ++_iter;
                                        continue;
                                    case ' ':
                                    case ',':
                                    case ']':
                                    case '}':
                                    case '\n':
                                    case '\t':
                                        _last_token = Token::Number;
                                        return Token::Number;
                                    default:
                                        _last_token = Token::ERROR;
                                        return Token::ERROR;
                                }
                            }
                        case State::NORMAL:
                            {
                                switch (c) {
                                    case '0':
                                    case '1':
                                    case '2':
                                    case '3':
                                    case '4':
                                    case '5':
                                    case '6':
                                    case '7':
                                    case '8':
                                    case '9':
                                    case '+':
                                    case '-':
                                        _state = State::P_NUM;
                                        _double.push_back(c);
                                        ++_iter;
                                        continue;
                                    case '{':
                                        ++_iter;
                                        _last_token = Token::Lbrace;
                                        return Token::Lbrace;
                                    case '}':
                                        ++_iter;
                                        _last_token = Token::Rbrace;
                                        return Token::Rbrace;
                                    case '[':
                                        ++_iter;
                                        _last_token = Token::LSquare;
                                        return Token::LSquare;
                                    case ']':
                                        ++_iter;
                                        _last_token = Token::RSquare;
                                        return Token::RSquare;
                                    case ':':
                                        ++_iter;
                                        _last_token = Token::Colon;
                                        return Token::Colon;
                                    case ',':
                                        ++_iter;
                                        _last_token = Token::Comma;
                                        return Token::Comma;
                                    case ' ':
                                    case '\n':
                                    case '\t':
                                        while (_iter != _end and *_iter == c)
                                            ++_iter;
                                        continue;
                                    case '\"':
                                        _state = State::P_STR;
                                        ++_iter;
                                        continue;
                                    case 't':
                                        _state = State::T;
                                        ++_iter;
                                        continue;
                                    case 'f':
                                        _state = State::F;
                                        ++_iter;
                                        continue;
                                    case 'n':
                                        _state = State::N;
                                        ++_iter;
                                        continue;
                                    default:
                                        ++_iter;
                                        continue;
                                }
                            }
                        case State::T:
                            if (c == 'r')
                                _state = State::TR;
                            else {
                                _last_token = Token::ERROR;
                                return Token::ERROR;
                            }
                            ++_iter;
                            continue;
                        case State::TR:
                            if (c == 'u')
                                _state = State::TRU;
                            else {
                                _last_token = Token::ERROR;
                                return Token::ERROR;
                            }
                            ++_iter;
                            continue;
                        case State::TRU:
                            if (c == 'e')
                                _state = State::NORMAL;
                            else {
                                _last_token = Token::ERROR;
                                return Token::ERROR;
                            }
                            ++_iter;
                            _last_token = Token::True;
                            return Token::True;
                        case State::F:
                            if (c == 'a')
                                _state = State::FA;
                            else {
                                _last_token = Token::ERROR;
                                return Token::ERROR;
                            }
                            ++_iter;
                            continue;
                        case State::FA:
                            if (c == 'l')
                                _state = State::FAL;
                            else {
                                _last_token = Token::ERROR;
                                return Token::ERROR;
                            }
                            ++_iter;
                            continue;
                        case State::FAL:
                            if (c == 's')
                                _state = State::FALS;
                            else {
                                _last_token = Token::ERROR;
                                return Token::ERROR;
                            }
                            ++_iter;
                            continue;
                        case State::FALS:
                            if (c == 'e')
                                _state = State::NORMAL;
                            else {
                                _last_token = Token::ERROR;
                                return Token::ERROR;
                            }
                            ++_iter;
                            _last_token = Token::False;
                            return Token::False;
                        case State::N:
                            if (c == 'u')
                                _state = State::NU;
                            else {
                                _last_token = Token::ERROR;
                                return Token::ERROR;
                            }
                            ++_iter;
                            continue;
                        case State::NU:
                            if (c == 'l')
                                _state = State::NUL;
                            else {
                                _last_token = Token::ERROR;
                                return Token::ERROR;
                            }
                            ++_iter;
                            continue;
                        case State::NUL:
                            if (c == 'l')
                                _state = State::NORMAL;
                            else {
                                _last_token = Token::ERROR;
                                return Token::ERROR;
                            }
                            ++_iter;
                            _last_token = Token::Null;
                            return Token::Null;
                        default:
                            return Token::ERROR;
                    }
                }

                return Token::BUFFER_END;
            }

            std::string get_string() {
                std::string res;
                _string.swap(res);
                _state = State::NORMAL;
                return res;
            }

            std::variant<double, std::errc> get_number() {
                std::string res;
                _double.swap(res);

                _state = State::NORMAL;

                double num;
                auto [r, err] = std::from_chars(res.data(), res.data() + res.size(), num);
                if (err == std::errc::invalid_argument or err == std::errc::result_out_of_range) {
                    return std::variant<double, std::errc>{ err };
                }
                return { num };
            }

            private:
            const char* _iter;
            const char* _end;
            enum struct State {
                P_STR, P_NUM, P_NUM_DOT, P_NUM_DOT_NUM, P_NUM_DOT_NUM_E, P_NUM_DOT_NUM_E_NUM, NORMAL, T, TR, TRU, F, FA, FAL, FALS, N, NU, NUL
            };
            State _state = State::NORMAL;
            std::string _string;
            std::string _double;
            Token _last_token = Token::ERROR;
        };

        class _Parser {
            public:
            enum struct result {
                normal,
                error,
                buffer_end,
                finished
            };

            void set_buffer(const char* begin, const char* end) noexcept { _tokenizer.set_buffer(begin, end); }

            std::optional<json::object> get_result() noexcept {
                if (_state == State::finished) {
                    json::object obj;
                    std::swap(obj, _object);
                    return { std::move(obj) };
                }
                else return std::nullopt;
            }

            result parse() {
                while (true) {
                    switch (_state) {
                        case State::normal:
                            {
                                while (!_stack.empty()) {
                                    auto res = _stack.back()(&_tokenizer, (void*)&_stack);
                                    if (res == result::buffer_end) {
                                        _state = State::buffer_end;
                                        return res;
                                    }
                                    if (res == result::error) {
                                        _state = State::error;
                                        return res;
                                    }
                                }
                                _state = State::finished;
                                return result::finished;
                            }
                        case State::error:
                            return result::error;
                        case State::buffer_end:
                            return result::buffer_end;
                        case State::finished:
                            return result::finished;
                        case State::begin:
                            _tokenizer.get_token();
                            _stack.push_back(Getter{
                                    .state = 0,
                                    .data = (void*)&_object, //json::object*
                                    .type = Getter::Type::Object
                                });
                            _state = State::normal;
                            continue;
                        default:
                            return result::error;
                    }
                }
            }

            private:
            using Stack = std::vector<std::function<result(_Tokenizer*, void*)>>;

            struct Getter {
                result operator()(_Tokenizer* tkp, void* stkp) {
                    const auto token = tkp->peek();
                    if (token == Token::ERROR)
                        return result::error;
                    if (token == Token::BUFFER_END)
                        return result::buffer_end;

                    Stack& stack = *((Stack*)stkp);
                    switch (type) {
                        case Type::Object:
                            if (state == 0) { // wait a {
                                if (token == Token::Lbrace) {
                                    tkp->get_token();
                                    state = 1;
                                    stack.push_back(Getter{
                                            .state = 0,
                                            .data = this->data,
                                            .type = Type::A
                                        });
                                }
                                else {
                                    return result::error;
                                }
                            }
                            else {
                                stack.pop_back();
                            }
                            return result::normal;
                        case Type::A:
                            if (state == 0) {
                                if (token == Token::String) {
                                    state = 1;
                                    stack.push_back(Getter{
                                            .state = 0,
                                            .data = this->data,
                                            .type = Type::Members
                                        });
                                    return result::normal;
                                }
                                else if (token == Token::Rbrace) {
                                    tkp->get_token();
                                    stack.pop_back();
                                    return result::normal;
                                }
                                else return result::error;
                            }
                            else {
                                if (token == Token::Rbrace) {
                                    tkp->get_token();
                                    stack.pop_back();
                                    return result::normal;
                                }
                                else {
                                    return result::error;
                                }
                            }
                        case Type::Members:
                            if (state == 0) {
                                if (token == Token::String) {
                                    state = 1;
                                    stack.push_back(Getter{
                                            .state = 0,
                                            .data = this->data, //json::object*
                                            .type = Type::Pair
                                        });
                                    return result::normal;
                                }
                                else {
                                    return result::error;
                                }
                            }
                            else if (state == 1) {
                                state = 2;
                                stack.push_back(Getter{
                                        .state = 0,
                                        .data = this->data, //json::object*
                                        .type = Type::B
                                    });
                                return result::normal;
                            }
                            else {
                                stack.pop_back();
                                return result::normal;
                            }
                        case Type::B:
                            if (state == 0) {
                                if (token == Token::Rbrace) {
                                    stack.pop_back();
                                    return result::normal;
                                }
                                else if (token == Token::Comma) {
                                    tkp->get_token();
                                    state = 1;
                                    stack.push_back(Getter{
                                            .state = 0,
                                            .data = this->data, //json::object*
                                            .type = Type::Members
                                        });
                                    return result::normal;
                                }
                                else return result::error;
                            }
                            else {
                                stack.pop_back();
                                return result::normal;
                            }
                        case Type::Pair:
                            switch (state) {
                                case 0: // wait a String
                                    {
                                        if (token == Token::String) {
                                            auto obj = (json::object*)data;
                                            auto [it, b] = obj->insert({ tkp->get_string(), {} });
                                            if (!b)
                                                return result::error;
                                            tkp->get_token();
                                            data = (void*)&(it->second); //json::value*
                                            state = 1;
                                            return result::normal;
                                        }
                                        else {
                                            return result::error;
                                        }
                                    }
                                case 1: // wait :
                                    {
                                        if (token == Token::Colon) {
                                            state = 2;
                                            tkp->get_token();
                                            return result::normal;
                                        }
                                        else return result::error;
                                    }
                                case 2: // wait a Value
                                    {
                                        state = 3;
                                        stack.push_back(Getter{
                                                .state = 0,
                                                .data = this->data, //json::value*
                                                .type = Type::Value
                                            });
                                        return result::normal;
                                    }
                                case 3:
                                    stack.pop_back();
                                    return result::normal;
                            }
                        case Type::Array:
                            {
                                if (state == 0) {
                                    if (token == Token::LSquare) {
                                        tkp->get_token();
                                        state = 1;
                                        stack.push_back(Getter{
                                                .state = 0,
                                                .data = this->data, //json::array*
                                                .type = Type::C
                                            });
                                        return result::normal;
                                    }
                                    else {
                                        return result::error;
                                    }
                                }
                                else {
                                    stack.pop_back();
                                    return result::normal;
                                }
                            }
                        case Type::C:
                            {
                                if (state == 0) {
                                    switch (token) {
                                        case Token::Lbrace:
                                        case Token::LSquare:
                                        case Token::Number:
                                        case Token::True:
                                        case Token::False:
                                        case Token::String:
                                        case Token::Null:
                                            {
                                                state = 1;
                                                stack.push_back(Getter{
                                                        .state = 0,
                                                        .data = this->data, //json::array*
                                                        .type = Type::Elements
                                                    });
                                                return result::normal;
                                            }
                                        default:
                                            {
                                                if (token == Token::RSquare) {
                                                    tkp->get_token();
                                                    stack.pop_back();
                                                    return result::normal;
                                                }
                                                else return result::error;
                                            }
                                    }
                                }
                                else {
                                    if (token == Token::RSquare) {
                                        tkp->get_token();
                                        stack.pop_back();
                                        return result::normal;
                                    }
                                    else return result::error;
                                }
                            }
                        case Type::Elements:
                            {
                                if (state == 0) {
                                    auto arr = (json::array*)data;
                                    arr->push_back(json::value{});
                                    state = 1;
                                    stack.push_back(Getter{
                                            .state = 0,
                                            .data = (void*)&(arr->back()), //json::value*
                                            .type = Type::Value
                                        });
                                    return result::normal;
                                }
                                else if (state == 1) {
                                    state = 2;
                                    stack.push_back(Getter{
                                            .state = 0,
                                            .data = this->data, //json::array*
                                            .type = Type::D
                                        });
                                    return result::normal;
                                }
                                else {
                                    stack.pop_back();
                                    return result::normal;
                                }
                            }
                        case Type::D:
                            {
                                if (state == 0) {
                                    if (token == Token::RSquare) {
                                        stack.pop_back();
                                        return result::normal;
                                    }
                                    else if (token == Token::Comma) {
                                        tkp->get_token();
                                        state = 1;
                                        stack.push_back(Getter{
                                                .state = 0,
                                                .data = this->data, //json::array*
                                                .type = Type::Elements
                                            });
                                        return result::normal;
                                    }
                                    else return result::error;
                                }
                                else {
                                    stack.pop_back();
                                    return result::normal;
                                }
                            }
                        case Type::Value:
                            if (state == 0) {
                                switch (token) {
                                    case Token::Lbrace: // wait an object
                                        {
                                            auto val = (json::value*)data;
                                            *val = json::object{};
                                            state = 1;
                                            stack.push_back(Getter{
                                                    .state = 0,
                                                    .data = (void*)&(val->get_object()), //json::object*
                                                    .type = Type::Object
                                                });
                                            return result::normal;
                                        }
                                    case Token::Number:
                                        {
                                            auto vnum = tkp->get_number();
                                            tkp->get_token();
                                            if (vnum.index() == 1)
                                                return result::error;
                                            auto val = (json::value*)data;
                                            *val = std::get<0>(vnum);
                                            stack.pop_back();
                                            return result::normal;
                                        }
                                    case Token::String:
                                        {
                                            auto val = (json::value*)data;
                                            *val = tkp->get_string();
                                            tkp->get_token();
                                            stack.pop_back();
                                            return result::normal;
                                        }
                                    case Token::Null:
                                        {
                                            auto val = (json::value*)data;
                                            *val = json::null;
                                            tkp->get_token();
                                            stack.pop_back();
                                            return result::normal;
                                        }
                                    case Token::LSquare: // wait an Array
                                        {
                                            auto val = (json::value*)data;
                                            *val = json::array{};
                                            state = 1;
                                            stack.push_back(Getter{
                                                    .state = 0,
                                                    .data = (void*)&(val->get_array()), //json::array*
                                                    .type = Type::Array
                                                });
                                            return result::normal;
                                        }
                                    case Token::True:
                                        {
                                            auto val = (json::value*)data;
                                            *val = true;
                                            tkp->get_token();
                                            stack.pop_back();
                                            return result::normal;
                                        }
                                    case Token::False:
                                        {
                                            auto val = (json::value*)data;
                                            *val = false;
                                            tkp->get_token();
                                            stack.pop_back();
                                            return result::normal;
                                        }
                                }
                            }
                            else {
                                stack.pop_back();
                                return result::normal;
                            }
                    }
                }

                char8_t state = 0;
                void* data;
                enum struct Type: char8_t {
                    Object,
                    A,
                    Members,
                    B,
                    Pair,
                    Array,
                    C,
                    Elements,
                    D,
                    Value
                };
                const Type type;
            };


            Stack _stack;
            _Tokenizer _tokenizer;
            json::object _object;

            enum struct State {
                normal,
                error,
                buffer_end,
                finished,
                begin
            };
            State _state = State::begin;
        };

        struct _Serializer {
            auto get_result() noexcept {
                std::string res;
                std::swap(res, _buffer);
                return res;
            }

            void operator()(const std::string& str) {
                _buffer.push_back('\"');
                _buffer.append(str.begin(), str.end());
                _buffer.push_back('\"');
            }

            void operator()(double db) {
                char buf[64];
                auto [ptr, err] = std::to_chars(buf, buf + 64, db);
                *ptr = '\0';
                _buffer.append(buf);
            }

            void operator()(bool b) {
                if (b)
                    _buffer.append("true");
                else _buffer.append("false");
            }

            void operator()(null_t) { _buffer.append("null"); }

            void operator()(const json::array& arr) {
                _buffer.push_back('[');
                for (const auto& e : arr) {
                    std::visit(*this, e._val);
                    _buffer.push_back(',');
                }
                _buffer.back() = ']';
            }

            void operator()(const json::object& obj) {
                _buffer.push_back('{');
                for (const auto& [k, v] : obj) {
                    _buffer.push_back('\"');
                    _buffer.append(k.begin(), k.end());
                    _buffer.push_back('\"');
                    _buffer.push_back(':');
                    std::visit(*this, v._val);
                    _buffer.push_back(',');
                }
                _buffer.back() = '}';
            }

            void operator()(std::monostate) {
                throw std::invalid_argument{ "Not a json object." };
            }

            private:
            std::string _buffer;
        };
    }

    json::object parse(const char* begin, const char* end) {
        json::_detail::_Parser parser;
        parser.set_buffer(begin, end);
        auto res = parser.parse();
        if (res != _detail::_Parser::result::finished)
            throw std::invalid_argument{ "Invalid format." };
        return parser.get_result().value();
    }

    std::string to_json(const json::object& obj) {
        _detail::_Serializer serializer;
        serializer(obj);
        return serializer.get_result();
    }
}


#endif
