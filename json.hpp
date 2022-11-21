#ifndef __JSON_H__
#define __JSON_H__

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include <new>
#include <cstring>

namespace json {
    struct value {
        enum struct Type {
            Number,
            String,
            Null,
            False,
            True,
            Array,
            Object,
            Nothing
        };

        [[nodiscard]] Type type() const noexcept { return _type; }

        value() noexcept: _type{ Type::Nothing } {};

        value(const value& other): _type{ other._type } {
            switch (_type) {
                case Type::Number:
                    _num = other._num;
                    break;
                case Type::String:
                    _str = ::new std::string(
                        *static_cast<std::string*>(other._str)
                    );
                    break;
                case Type::Null:
                    break;
                case Type::True:
                case Type::False:
                    _b = other._b;
                    break;
                case Type::Array:
                    _arr = static_cast<void*>(::new std::vector<value>(
                        *static_cast<std::vector<value> *>(other._arr)
                        ));
                    break;
                case Type::Object:
                    _obj = ::new std::unordered_map<std::string, value>(
                        *static_cast<std::unordered_map<std::string, value> *>(other._obj)
                        );
                    break;
                case Type::Nothing:
                    break;
            }
        }


        value(value&& other) noexcept: _type{ other._type } {
            switch (_type) {
                case Type::Number:
                    _num = other._num;
                    break;
                case Type::String:
                    _str = other._str;
                    other._type = Type::Nothing;
                    break;
                case Type::Null:
                    break;
                case Type::True:
                case Type::False:
                    _b = other._b;
                    break;
                case Type::Array:
                    _arr = other._arr;
                    other._type = Type::Nothing;
                    break;
                case Type::Object:
                    _obj = other._obj;
                    other._type = Type::Nothing;
                    break;
                case Type::Nothing:
                    break;
            }
        }

        value(double x) noexcept: _type{ Type::Number } {
            _num = x;
        }

        value(int x) noexcept: _type{ Type::Number } {
            _num = double{ static_cast<double>(x) };
        }

        value(bool b) noexcept {
            _b = b;
            _type = b ? Type::True : Type::False;
        }

        value(const std::string& str) {
            _str = ::new std::string(str);
            _type = Type::String;
        }

        value(std::string&& str) {
            _str = ::new std::string(std::move(str));
            _type = Type::String;
        }

        value(const char* str) {
            _str = ::new std::string(str);
            _type = Type::String;
        }

        value(const std::vector<value>& array) {
            _arr = ::new std::vector<value>(array);
            _type = Type::Array;
        }

        value(std::vector<value>&& array) {
            _arr = ::new std::vector<value>(std::move(array));
            _type = Type::Array;
        }

        value(const std::unordered_map<std::string, value>& obj) {
            _obj = ::new std::unordered_map<std::string, value>(obj);
            _type = Type::Object;
        }

        value(std::unordered_map<std::string, value>&& obj) {
            _obj = ::new std::unordered_map<std::string, value>(std::move(obj));
            _type = Type::Object;
        }

        value(Type) noexcept: _type{ Type::Null } {}

        friend bool operator==(const value&, const value&);

        friend bool operator!=(const value&, const value&);

        value& operator[](const std::string& key) {
            return get_object()[key];
        }

        const value& operator[](const std::string& key)const {
            return get_object().at(key);
        }

        value& operator=(const value& other) {
            if (this == std::addressof(other))
                return *this;
            if (_type != other._type) {
                _destroy();
                _type = other._type;
            }
            switch (_type) {
                case Type::Number:
                    _num = other._num;
                    break;
                case Type::String:
                    _str = ::new std::string(*static_cast<std::string*>(other._str));
                    break;
                case Type::Null:
                case Type::False:
                case Type::True:
                    break;
                case Type::Array:
                    _arr = ::new std::vector<value>(*static_cast<std::vector<value>*>(other._arr));
                    break;
                case Type::Object:
                    _obj = ::new std::unordered_map<std::string, value>(
                        *static_cast<std::unordered_map<std::string, value>*>(other._obj)
                        );
                    break;
                case Type::Nothing:
                    break;
            }
            return *this;
        }

        value& operator=(value&& other)noexcept {
            if (_type != other._type) {
                _destroy();
                _type = other._type;
            }
            other._type = Type::Nothing;
            switch (_type) {
                case Type::Number:
                    _num = other._num;
                    break;
                case Type::String:
                    _str = other._str;
                    break;
                case Type::Null:
                case Type::False:
                case Type::True:
                    break;
                case Type::Array:
                    _arr = other._arr;
                    break;
                case Type::Object:
                    _obj = other._obj;
                    break;
                case Type::Nothing:;
            }
            return *this;
        }

        value& operator=(double x) noexcept {
            _destroy();
            _type = Type::Number;
            _num = x;
            return *this;
        }

        value& operator=(int x) noexcept {
            _destroy();
            _type = Type::Number;
            _num = double{ static_cast<double>(x) };
            return *this;
        }

        value& operator=(bool b) noexcept {
            _destroy();
            _type = b ? Type::True : Type::False;
            _b = b;
            return *this;
        }

        value& operator=(const std::string& str) {
            _destroy();
            _str = ::new std::string(str);
            _type = Type::String;
            return *this;
        }

        value& operator=(std::string&& str) {
            _destroy();
            _str = ::new std::string(std::move(str));
            _type = Type::String;
            return *this;
        }

        value& operator=(const char* str) {
            _destroy();
            _str = ::new std::string(str);
            _type = Type::String;
            return *this;
        }

        value& operator=(const std::vector<value>& array) {
            _destroy();
            _arr = ::new std::vector<value>(array);
            _type = Type::Array;
            return *this;
        }

        value& operator=(std::vector<value>&& array) {
            _destroy();
            _arr = ::new std::vector<value>(std::move(array));
            _type = Type::Array;
            return *this;
        }

        value& operator=(const std::unordered_map<std::string, value>& obj) {
            _destroy();
            _obj = ::new std::unordered_map<std::string, value>(obj);
            _type = Type::Object;
            return *this;
        }

        value& operator=(std::unordered_map<std::string, value>&& obj) {
            _destroy();
            _obj = ::new std::unordered_map<std::string, value>(std::move(obj));
            _type = Type::Object;
            return *this;
        }

        value& operator=(Type) noexcept {
            _destroy();
            _type = Type::Null;
            return *this;
        }

        [[nodiscard]] bool is_number()const noexcept { return _type == Type::Number; }
        [[nodiscard]] bool is_bool()const noexcept { return _type == Type::True || _type == Type::False; }
        [[nodiscard]] bool is_string()const noexcept { return _type == Type::String; }
        [[nodiscard]] bool is_array()const noexcept { return _type == Type::Array; }
        [[nodiscard]] bool is_object()const noexcept { return _type == Type::Object; }

        [[nodiscard]] double get_number() const {
            if (_type != Type::Number)
                throw std::runtime_error{ "The value isn't a number." };
            return _num;
        }

        [[nodiscard]] double& get_number() {
            if (_type != Type::Number)
                throw std::runtime_error{ "The value isn't a number." };
            return _num;
        }

        [[nodiscard]] bool get_bool() const {
            if (_type != Type::True && _type != Type::False)
                throw std::runtime_error{ "The value isn't a Boolean." };
            return _b;
        }

        [[nodiscard]] bool& get_bool() {
            if (_type != Type::True && _type != Type::False)
                throw std::runtime_error{ "The value isn't a Boolean." };
            return _b;
        }

        [[nodiscard]] const std::string& get_string() const {
            if (_type != Type::String)
                throw std::runtime_error{ "The value isn't a string." };
            return *static_cast<std::string*>(_str);
        }

        [[nodiscard]] std::string& get_string() {
            if (_type != Type::String)
                throw std::runtime_error{ "The value isn't a string." };
            return *static_cast<std::string*>(_str);
        }

        [[nodiscard]] const std::vector<value>& get_array() const {
            if (_type != Type::Array)
                throw std::runtime_error{ "The value isn't an array." };
            return *static_cast<std::vector<value> *>(_arr);
        }

        [[nodiscard]] std::vector<value>& get_array() {
            if (_type != Type::Array)
                throw std::runtime_error{ "The value isn't an array." };
            return *static_cast<std::vector<value> *>(_arr);
        }

        [[nodiscard]] const std::unordered_map<std::string, value>& get_object() const {
            if (_type != Type::Object)
                throw std::runtime_error{ "The value isn't an object." };
            return *static_cast<std::unordered_map<std::string, value> *>(_obj);
        }

        [[nodiscard]] std::unordered_map<std::string, value>& get_object() {
            if (_type != Type::Object)
                throw std::runtime_error{ "The value isn't an object." };
            return *static_cast<std::unordered_map<std::string, value> *>(_obj);
        }


        ~value() {
            _destroy();
        }

        private:
        void _destroy() noexcept {
            switch (_type) {
                case Type::Number:
                case Type::True:
                case Type::False:
                case Type::Null:
                case Type::Nothing:
                    break;
                case Type::String:
                    delete static_cast<std::string*>(_str);
                    break;
                case Type::Array:
                    delete static_cast<std::vector<value>*>(_arr);
                    break;
                case Type::Object:
                    delete static_cast<std::unordered_map<std::string, value>*>(_obj);
            }
            _type = Type::Nothing;
        }

        private:
        Type _type = Type::Nothing;
        union {
            double _num;
            bool _b;
            void* _arr;
            void* _obj;
            void* _str;
        };

    };

    bool operator==(const value& x, const value& y) {
        if (x._type == y._type) {
            switch (x._type) {
                case value::Type::Number:
                    return x._num == y._num;
                case value::Type::String:
                    return *(static_cast<std::string*>(x._str)) == *(static_cast<std::string*>(y._str));
                case value::Type::Null:
                case value::Type::False:
                case value::Type::True:
                case value::Type::Nothing:
                    return true;
                case value::Type::Array:
                    return *(static_cast<std::vector<value> *>(x._arr)) == *(static_cast<std::vector<value> *>(y._arr));
                case value::Type::Object:
                    return *(static_cast<std::unordered_map<std::string, value> *>(x._obj)) ==
                        *(static_cast<std::unordered_map<std::string, value> *>(y._obj));
            }
        }
        return false;
    }

    bool operator!=(const value& x, const value& y) {
        return !(x == y);
    }


    using object = std::unordered_map<std::string, value>;
    using array = std::vector<value>;

    constexpr value::Type null = value::Type::Null;

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

            Token get_token() {
                if (_last_token == Token::Number && !_double.empty())
                    return _last_token;
                if (_last_token == Token::String && !_string.empty())
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
                                    case 'e':
                                        _state = State::P_NUM_DOT_NUM_E;
                                        _double.push_back(c);
                                        ++_iter;
                                        continue;
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
                                        while (_iter != _end && *_iter == c)
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

                _last_token = Token::BUFFER_END;
                return Token::BUFFER_END;
            }

            std::string get_string() {
                std::string res;
                _string.swap(res);
                _state = State::NORMAL;
                return res;
            }

            double get_number() {
                std::string res;
                _double.swap(res);
                _state = State::NORMAL;
                return std::atof(res.data());
            }

            private:
            const char* _iter;
            const char* _end;
            enum struct State {
                P_STR,
                P_NUM,
                P_NUM_DOT,
                P_NUM_DOT_NUM,
                P_NUM_DOT_NUM_E,
                P_NUM_DOT_NUM_E_NUM,
                NORMAL,
                T,
                TR,
                TRU,
                F,
                FA,
                FAL,
                FALS,
                N,
                NU,
                NUL
            };
            State _state = State::NORMAL;
            std::string _string;
            std::string _double;
            Token _last_token = Token::ERROR;
        };

        class _Parser {
            public:
            _Parser() {
                _stack.reserve(128);
            }

            enum struct result {
                normal,
                error,
                buffer_end,
                finished
            };

            void set_buffer(const char* begin, const char* end) noexcept {
                _tokenizer.set_buffer(begin, end);
                if (_state != State::begin) {
                    _state = State::normal;
                    _tokenizer.get_token();
                }
            }

            json::object get_result() noexcept {
                json::object obj;
                std::swap(obj, _object);
                return { std::move(obj) };

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
                                    0,
                                    (void*)&_object, //json::object*
                                    Getter::Type::Object
                                });
                            _state = State::normal;
                            continue;
                        default:
                            return result::error;
                    }
                }
            }

            private:
            struct Getter;
            using Stack = std::vector<Getter>;

            struct Getter {
                enum struct Type: char {
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

                Getter(char state, void* data, Getter::Type type) noexcept: state{ state }, data{ data }, type{ type } {}

                result operator()(_Tokenizer* tkp, void* stkp) {
                    Stack& stack = *((Stack*)stkp);

                    const auto token = tkp->peek();
                    if (token == Token::ERROR)
                        return result::error;


                    switch (type) {
                        case Type::Object:
                            if (state == 0) { // wait a {
                                if (token == Token::Lbrace) {
                                    tkp->get_token();
                                    state = 1;
                                    stack.push_back(Getter{
                                            0,
                                            this->data, //json::object*
                                            Type::A
                                        });
                                }
                                else if (token == Token::BUFFER_END) {
                                    return result::buffer_end;
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
                                            0,
                                            this->data,     //json::object*
                                            Type::Members
                                        });
                                    return result::normal;
                                }
                                else if (token == Token::Rbrace) {
                                    tkp->get_token();
                                    stack.pop_back();
                                    return result::normal;
                                }
                                else if (token == Token::BUFFER_END) {
                                    return result::buffer_end;
                                }
                                return result::error;
                            }
                            else {
                                if (token == Token::Rbrace) {
                                    tkp->get_token();
                                    stack.pop_back();
                                    return result::normal;
                                }
                                else if (token == Token::BUFFER_END) {
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
                                            0,
                                            this->data, //json::object*
                                            Type::Pair
                                        });
                                    return result::normal;
                                }
                                else if (token == Token::BUFFER_END) {
                                    return result::buffer_end;
                                }
                                else {
                                    return result::error;
                                }
                            }
                            else if (state == 1) {
                                if (token == Token::BUFFER_END) {
                                    return result::buffer_end;
                                }
                                state = 2;
                                stack.push_back(Getter{
                                        0,
                                        this->data, //json::object*
                                        Type::B
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
                                            0,
                                            this->data, //json::object*
                                            Type::Members
                                        });
                                    return result::normal;
                                }
                                else if (token == Token::BUFFER_END) {
                                    return result::buffer_end;
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
                                            auto pair = obj->insert({ tkp->get_string(), {} });

                                            if (!pair.second)
                                                return result::error;
                                            tkp->get_token();
                                            data = (void*)&(pair.first->second); //json::value*
                                            state = 1;
                                            return result::normal;
                                        }
                                        else if (token == Token::BUFFER_END) {
                                            return result::buffer_end;
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
                                        else if (token == Token::BUFFER_END) {
                                            return result::buffer_end;
                                        }
                                        else return result::error;
                                    }
                                case 2: // wait a Value
                                    {
                                        if (token == Token::BUFFER_END) {
                                            return result::buffer_end;
                                        }
                                        state = 3;
                                        stack.push_back(Getter{
                                                0,
                                                this->data, //json::value*
                                                Type::Value
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
                                                0,
                                                this->data, //json::array*
                                                Type::C
                                            });
                                        return result::normal;
                                    }
                                    else if (token == Token::BUFFER_END) {
                                        return result::buffer_end;
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
                                                        0,
                                                        this->data, //json::array*
                                                        Type::Elements
                                                    });
                                                return result::normal;
                                            }
                                        case Token::BUFFER_END:
                                            return result::buffer_end;
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
                                    else if (token == Token::BUFFER_END) {
                                        return result::buffer_end;
                                    }
                                    else return result::error;
                                }
                            }
                        case Type::Elements:
                            {
                                if (state == 0) {
                                    if (token == Token::BUFFER_END) {
                                        return result::buffer_end;
                                    }
                                    auto arr = (json::array*)data;
                                    arr->push_back(json::value{});
                                    state = 1;
                                    stack.push_back(Getter{
                                            0,
                                            (void*)&(arr->back()), //json::value*
                                            Type::Value
                                        });
                                    return result::normal;
                                }
                                else if (state == 1) {
                                    if (token == Token::BUFFER_END) {
                                        return result::buffer_end;
                                    }
                                    state = 2;
                                    stack.push_back(Getter{
                                            0,
                                            this->data, //json::array*
                                            Type::D
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
                                                0,
                                                this->data, //json::array*
                                                Type::Elements
                                            });
                                        return result::normal;
                                    }
                                    else if (token == Token::BUFFER_END) {
                                        return result::buffer_end;
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
                                                    0,
                                                    (void*)&(val->get_object()), //json::object*
                                                    Type::Object
                                                });
                                            return result::normal;
                                        }
                                    case Token::Number:
                                        {
                                            auto val = (json::value*)data;
                                            *val = tkp->get_number();
                                            tkp->get_token();
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
                                                    0,
                                                    (void*)&(val->get_array()), //json::array*
                                                    Type::Array
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
                                    case Token::BUFFER_END:
                                        return result::buffer_end;
                                }
                            }
                            else {
                                stack.pop_back();
                                return result::normal;
                            }
                    }
                    return result::error;
                }

                char state = 0;
                void* data;

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
            std::string get_result() noexcept {
                std::string res;
                std::swap(res, _buffer);
                return res;
            }

            void operator()(const json::array& arr) {
                _buffer.push_back('[');
                for (const auto& e : arr) {
                    switch (e.type()) {
                        case value::Type::Number:
                            _buffer.append(std::to_string(e.get_number()));
                            break;
                        case value::Type::String:
                            _buffer.push_back('\"');
                            _buffer.append(e.get_string());
                            _buffer.push_back('\"');
                            break;
                        case value::Type::Null:
                            _buffer.append("null");
                            break;
                        case value::Type::False:
                            _buffer.append("false");
                            break;
                        case value::Type::True:
                            _buffer.append("true");
                            break;
                        case value::Type::Array:
                            this->operator()(e.get_array());
                            break;
                        case value::Type::Object:
                            this->operator()(e.get_object());
                            break;
                        case value::Type::Nothing:
                            break;
                    }
                    _buffer.push_back(',');
                }
                _buffer.back() = ']';
            }

            void operator()(const json::object& obj) {
                _buffer.push_back('{');
                for (const auto& p : obj) {
                    _buffer.push_back('\"');
                    _buffer.append(p.first.begin(), p.first.end());
                    _buffer.push_back('\"');
                    _buffer.push_back(':');
                    switch (p.second.type()) {
                        case value::Type::Number:
                            _buffer.append(std::to_string(p.second.get_number()));
                            break;
                        case value::Type::String:
                            _buffer.push_back('\"');
                            _buffer.append(p.second.get_string());
                            _buffer.push_back('\"');
                            break;
                        case value::Type::Null:
                            _buffer.append("null");
                            break;
                        case value::Type::False:
                            _buffer.append("false");
                            break;
                        case value::Type::True:
                            _buffer.append("true");
                            break;
                        case value::Type::Array:
                            this->operator()(p.second.get_array());
                            break;
                        case value::Type::Object:
                            this->operator()(p.second.get_object());
                            break;
                        case value::Type::Nothing:
                            break;
                    }
                    _buffer.push_back(',');
                }
                _buffer.back() = '}';
            }

            private:
            std::string _buffer;
        };
    }


    json::object parse(const char* first, const char* last) {
        json::_detail::_Parser parser;
        parser.set_buffer(first, last);
        auto res = parser.parse();
        if (res != _detail::_Parser::result::finished)
            throw std::invalid_argument{ "Invalid format." };
        return parser.get_result();
    }

    json::object from_string(const char* str) {
        return parse(str, str + strlen(str));
    }

    json::object from_string(const std::string& str) {
        return parse(str.data(), str.data() + str.size());
    }

    json::object from_file(const char* path, size_t buff_s = 4096) {
        std::string buffer;
        _detail::_Parser parser;
        size_t res = 0;

        auto file = std::fopen(path, "r");
        if (!file)
            throw std::runtime_error{ "Open file failed." };

        buffer.resize(buff_s, '\0');
        while ((res = std::fread((void*)buffer.data(), 1, buffer.size(), file)) == buffer.size()) {
            parser.set_buffer(buffer.data(), buffer.size() + buffer.data());
            auto r = parser.parse();
            if (r == _detail::_Parser::result::error)
                throw std::runtime_error{ "Invalid format1." };
            if (r != _detail::_Parser::result::buffer_end)
                return parser.get_result();
        }
        if (std::feof(file)) {
            parser.set_buffer(buffer.data(), res + buffer.data());
            auto r = parser.parse();
            if (r == _detail::_Parser::result::error)
                throw std::runtime_error{ "Invalid format2." };
            if (r != _detail::_Parser::result::buffer_end)
                return parser.get_result();
        }
        throw std::runtime_error{ "Invalid format2." };
    }

    std::string to_json(const json::object& obj) {
        _detail::_Serializer serializer;
        serializer(obj);
        return serializer.get_result();
    }

}


#endif



