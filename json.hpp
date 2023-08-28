/*
  Copyright (c) 2023 SamGaaWaa

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
#pragma once

#include <utility>
#include <concepts>
#include <optional>
#include <string_view>
#include <cstring>
#include <bit>
#include <cctype>
#include <cstring>
#include <iostream>
#include <format>
#include <cstdint>
#include <variant>
#include <any>
#include <vector>
#include <map>
#include <algorithm>
#include <charconv>
#include <string_view>
#include <cassert>
#include <expected>
#include <stdio.h>
#include <functional>
#include <memory>

namespace json {

    enum struct parse_error_t : uint8_t {
        error,
        happy_ending,
        unknown_string_character,
        unknown_space,
        unknown_ESC,
        unknown_hex_character,
        unknown_utf8_bytes,
        unknown_number_character,
        unknown_boolean_character,
        unknown_null_character,

        early_EOF,
        read_file_error,
        extra_content,
        too_deep,
        duplicate_key
    };

    enum struct serialize_error_t : uint8_t {
        error,
        unknown_utf8_bytes
    };

    constexpr const char* error_string(const parse_error_t err)noexcept {
        switch (err) {
            case parse_error_t::error:
                return "Error.";
            case parse_error_t::happy_ending:
                return "Happy ending.";
            case parse_error_t::unknown_string_character:
                return "Unknown string character.";
            case parse_error_t::unknown_space:
                return "Unknown space character.";
            case parse_error_t::unknown_ESC:
                return "Unknown ESC.";
            case parse_error_t::unknown_hex_character:
                return "Unknown hex character.";
            case parse_error_t::unknown_utf8_bytes:
                return "Unknown utf8 bytes.";
            case parse_error_t::unknown_number_character:
                return "Unknown number character.";
            case parse_error_t::unknown_boolean_character:
                return "Unknown boolean character.";
            case parse_error_t::unknown_null_character:
                return "Unknown null character.";
            case parse_error_t::early_EOF:
                return "Early EOF.";
            case parse_error_t::read_file_error:
                return "Read file error.";
            case parse_error_t::extra_content:
                return "Extra content.";
            case parse_error_t::too_deep:
                return "Too deep.";
            case parse_error_t::duplicate_key:
                return "Duplicate key.";
            default:
                std::unreachable();
        }
        std::unreachable();
    }

    const char* error_string(const serialize_error_t err)noexcept {
        switch (err) {
            case serialize_error_t::error:
                return "Error.";
            case serialize_error_t::unknown_utf8_bytes:
                return "Unknown utf8 bytes.";
            default:
                std::unreachable();
        }
        std::unreachable();
    }

    template<class P>
    concept Parser = requires(P p, char c) {
        { p.on_null() }->std::same_as<std::optional<parse_error_t>>;
        { p.on_true() }->std::same_as<std::optional<parse_error_t>>;
        { p.on_false() }->std::same_as<std::optional<parse_error_t>>;
        { p.on_string(c) }->std::same_as<std::optional<parse_error_t>>;
        { p.on_string_begin() }->std::same_as<std::optional<parse_error_t>>;
        { p.on_string_end() }->std::same_as<std::optional<parse_error_t>>;
        { p.on_comma() }->std::same_as<std::optional<parse_error_t>>;
        { p.on_colon() }->std::same_as<std::optional<parse_error_t>>;
        { p.on_number(c) }->std::same_as<std::optional<parse_error_t>>;
        { p.on_number_begin(c) }->std::same_as<std::optional<parse_error_t>>;
        { p.on_number_end() }->std::same_as<std::optional<parse_error_t>>;
        { p.on_left_square_bracket() }->std::same_as<std::optional<parse_error_t>>;
        { p.on_right_square_bracket() }->std::same_as<std::optional<parse_error_t>>;
        { p.on_left_brace() }->std::same_as<std::optional<parse_error_t>>;
        { p.on_right_brace() }->std::same_as<std::optional<parse_error_t>>;
    };

    template<class B>
    concept Builder = requires(B b, std::string str, std::string num, bool bb) {
        { b.on_document_begin() }->std::same_as<std::optional<parse_error_t>>;
        { b.on_document_end() }->std::same_as<std::optional<parse_error_t>>;
        { b.on_object_begin() }->std::same_as<std::optional<parse_error_t>>;
        { b.on_object_end() }->std::same_as<std::optional<parse_error_t>>;
        { b.on_array_begin() }->std::same_as<std::optional<parse_error_t>>;
        { b.on_array_end() }->std::same_as<std::optional<parse_error_t>>;
        { b.on_key(std::move(str)) }->std::same_as<std::optional<parse_error_t>>;
        { b.on_string(std::move(str)) }->std::same_as<std::optional<parse_error_t>>;
        { b.on_number(std::move(num)) }->std::same_as<std::optional<parse_error_t>>;
        { b.on_bool(bb) }->std::same_as<std::optional<parse_error_t>>;
        { b.on_null() }->std::same_as<std::optional<parse_error_t>>;
        { b.get() };
    };


    namespace detail {
        template<Parser P>
        struct lexer {
            constexpr explicit lexer(P* p)noexcept :_parser{ p } {}

            constexpr std::optional<parse_error_t> operator()(const char* data, const size_t size)noexcept {
                const char* iter = data;
                const char* const end = iter + size;
                std::optional<parse_error_t> err;
                bytes = 0;

                while (iter < end) {
                    switch (_state) {
                        case state_t::normal:
                            {
                                iter = skip_space(iter, end);
                                if (iter == end) {
                                    bytes += size;
                                    return {};
                                }
                                const char c = *iter;
                                switch (c) {
                                    case '{':
                                        err = _parser->on_left_brace();
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        ++iter;
                                        continue;
                                    case '}':
                                        err = _parser->on_right_brace();
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        ++iter;
                                        continue;
                                    case '[':
                                        err = _parser->on_left_square_bracket();
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        ++iter;
                                        continue;
                                    case ']':
                                        err = _parser->on_right_square_bracket();
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        ++iter;
                                        continue;
                                    case 'f':
                                        _state = state_t::f;
                                        ++iter;
                                        continue;
                                    case 't':
                                        _state = state_t::t;
                                        ++iter;
                                        continue;
                                    case 'n':
                                        _state = state_t::n;
                                        ++iter;
                                        continue;
                                    case '-':
                                        err = _parser->on_number_begin(c);
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        _state = state_t::after_optional_negative;
                                        ++iter;
                                        continue;
                                    case '0':
                                        err = _parser->on_number_begin(c);
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        _state = state_t::after_zero;
                                        ++iter;
                                        continue;
                                    case '1':
                                    case '2':
                                    case '3':
                                    case '4':
                                    case '5':
                                    case '6':
                                    case '7':
                                    case '8':
                                    case '9':
                                        err = _parser->on_number_begin(c);
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        _state = state_t::after_one_to_nine;
                                        ++iter;
                                        continue;
                                    case '\"':
                                        err = _parser->on_string_begin();
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        _state = state_t::after_quotation_mark;
                                        ++iter;
                                        continue;
                                    case ':':
                                        err = _parser->on_colon();
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        ++iter;
                                        continue;
                                    case ',':
                                        err = _parser->on_comma();
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        ++iter;
                                        continue;
                                    default:
                                        {
                                            bytes += iter - data + 1;
                                            return parse_error_t::unknown_space;
                                        }
                                }
                            }

                            // string
                        case state_t::after_quotation_mark:
                            {
                                const char c = *iter;
                                switch (std::countl_one((uint8_t)c)) {
                                    case 0:
                                        {   //ASCII
                                            if (std::iscntrl(c))
                                                return parse_error_t{};
                                            switch (c) {
                                                case '\"':
                                                    err = _parser->on_string_end();
                                                    if (err) {
                                                        bytes += iter - data + 1;
                                                        return err;
                                                    }
                                                    _state = state_t::normal;
                                                    ++iter;
                                                    continue;
                                                case '\\':
                                                    _state = state_t::after_reverse_solidus;
                                                    ++iter;
                                                    continue;
                                                default:
                                                    _parser->on_string(c);
                                                    ++iter;
                                                    continue;
                                            }
                                        }
                                    case 1:
                                        return parse_error_t::unknown_string_character;
                                    case 2:
                                        _parser->on_string(c);
                                        _state = state_t::wait_1_utf8_bytes;
                                        ++iter;
                                        continue;
                                    case 3:
                                        _parser->on_string(c);
                                        _state = state_t::wait_2_utf8_bytes;
                                        ++iter;
                                        continue;
                                    case 4:
                                        _parser->on_string(c);
                                        _state = state_t::wait_3_utf8_bytes;
                                        ++iter;
                                        continue;
                                    default:
                                        bytes += iter - data + 1;
                                        return parse_error_t::unknown_string_character;
                                }
                            }
                        case state_t::after_reverse_solidus:
                            {
                                const char c = *iter;
                                switch (c) {
                                    case '\"':
                                    case '\\':
                                    case '/':
                                        _parser->on_string(c);
                                        _state = state_t::after_quotation_mark;
                                        ++iter;
                                        continue;
                                    case 'b':
                                        _parser->on_string('\b');
                                        _state = state_t::after_quotation_mark;
                                        ++iter;
                                        continue;
                                    case 'f':
                                        _parser->on_string('\f');
                                        _state = state_t::after_quotation_mark;
                                        ++iter;
                                        continue;
                                    case 'n':
                                        _parser->on_string('\n');
                                        _state = state_t::after_quotation_mark;
                                        ++iter;
                                        continue;
                                    case 'r':
                                        _parser->on_string('\r');
                                        _state = state_t::after_quotation_mark;
                                        ++iter;
                                        continue;
                                    case 't':
                                        _parser->on_string('\t');
                                        _state = state_t::after_quotation_mark;
                                        ++iter;
                                        continue;
                                    case 'u':
                                        _state = state_t::wait_4_hex;
                                        ++iter;
                                        continue;
                                    default:
                                        bytes += iter - data + 1;
                                        return parse_error_t::unknown_ESC;
                                }
                            }
                        case state_t::wait_4_hex:
                            {
                                const char c = *iter;
                                if (auto op = hex_to_int(c); op) {
                                    unicode_high |= ((*op) << 12);
                                    _state = state_t::wait_3_hex;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_hex_character;
                                }
                            }
                        case state_t::wait_3_hex:
                            {
                                const char c = *iter;
                                if (auto op = hex_to_int(c); op) {
                                    unicode_high |= ((*op) << 8);
                                    _state = state_t::wait_2_hex;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_hex_character;
                                }
                            }
                        case state_t::wait_2_hex:
                            {
                                const char c = *iter;
                                if (auto op = hex_to_int(c); op) {
                                    unicode_high |= ((*op) << 4);
                                    _state = state_t::wait_1_hex;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_hex_character;
                                }
                            }
                        case state_t::wait_1_hex:
                            {
                                const char c = *iter;
                                if (auto op = hex_to_int(c); op) {
                                    unicode_high |= *op;
                                    if (unicode_high >= 0xD800 && unicode_high <= 0xDBFF) {
                                        _state = state_t::wait_low_reverse_solidus;
                                        ++iter;
                                        continue;
                                    }
                                    const auto utf8 = unicode_to_utf8(unicode_high);
                                    for (const auto c : utf8)
                                        _parser->on_string(c);
                                    unicode_high = 0;
                                    _state = state_t::after_quotation_mark;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_hex_character;
                                }
                            }
                        case state_t::wait_low_reverse_solidus:
                            {
                                const char c = *iter;
                                if (c == '\\') {
                                    _state = state_t::wait_low_u;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_hex_character;
                                }
                            }
                        case state_t::wait_low_u:
                            {
                                const char c = *iter;
                                if (c == 'u') {
                                    _state = state_t::wait_low_4_hex;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_hex_character;
                                }
                            }
                        case state_t::wait_low_4_hex:
                            {
                                const char c = *iter;
                                if (auto op = hex_to_int(c); op) {
                                    unicode_low |= ((*op) << 12);
                                    _state = state_t::wait_low_3_hex;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_hex_character;
                                }
                            }
                        case state_t::wait_low_3_hex:
                            {
                                const char c = *iter;
                                if (auto op = hex_to_int(c); op) {
                                    unicode_low |= ((*op) << 8);
                                    _state = state_t::wait_low_2_hex;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_hex_character;
                                }
                            }
                        case state_t::wait_low_2_hex:
                            {
                                const char c = *iter;
                                if (auto op = hex_to_int(c); op) {
                                    unicode_low |= ((*op) << 4);
                                    _state = state_t::wait_low_1_hex;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_hex_character;
                                }
                            }
                        case state_t::wait_low_1_hex:
                            {
                                const char c = *iter;
                                if (auto op = hex_to_int(c); op) {
                                    unicode_low |= *op;
                                    const uint32_t uc = 0x10000 + ((unicode_high - 0xD800) << 10) + (unicode_low - 0xDC00);
                                    const auto utf8 = unicode_to_utf8(uc);
                                    for (const auto c : utf8)
                                        _parser->on_string(c);
                                    unicode_high = unicode_low = 0;
                                    _state = state_t::after_quotation_mark;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_hex_character;
                                }
                            }
                        case state_t::wait_3_utf8_bytes:
                            if (std::countl_one((uint8_t)*iter) != 1) {
                                bytes += iter - data + 1;
                                return parse_error_t::unknown_utf8_bytes;
                            }
                            _parser->on_string(*iter);
                            _state = state_t::wait_2_utf8_bytes;
                            ++iter;
                            continue;
                        case state_t::wait_2_utf8_bytes:
                            if (std::countl_one((uint8_t)*iter) != 1) {
                                bytes += iter - data + 1;
                                return parse_error_t::unknown_utf8_bytes;
                            }
                            _parser->on_string(*iter);
                            _state = state_t::wait_1_utf8_bytes;
                            ++iter;
                            continue;
                        case state_t::wait_1_utf8_bytes:
                            if (std::countl_one((uint8_t)*iter) != 1) {
                                bytes += iter - data + 1;
                                return parse_error_t::unknown_utf8_bytes;
                            }
                            _parser->on_string(*iter);
                            _state = state_t::after_quotation_mark;
                            ++iter;
                            continue;

                            // number
                        case state_t::after_optional_negative:
                            {
                                const char c = *iter;
                                switch (c) {
                                    case '0':
                                        _parser->on_number(c);
                                        _state = state_t::after_zero;
                                        ++iter;
                                        continue;
                                    case '1':
                                    case '2':
                                    case '3':
                                    case '4':
                                    case '5':
                                    case '6':
                                    case '7':
                                    case '8':
                                    case '9':
                                        _parser->on_number(c);
                                        _state = state_t::after_one_to_nine;
                                        ++iter;
                                        continue;
                                    default:
                                        bytes += iter - data + 1;
                                        return parse_error_t::unknown_number_character;
                                }
                            }
                        case state_t::after_point:
                            {
                                const char c = *iter;
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
                                        _parser->on_number(c);
                                        _state = state_t::fractional_part;
                                        ++iter;
                                        continue;
                                    default:
                                        bytes += iter - data + 1;
                                        return parse_error_t::unknown_number_character;
                                }
                            }
                        case state_t::after_e:
                            {
                                const char c = *iter;
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
                                        _parser->on_number(c);
                                        _state = state_t::parsing_exponent;
                                        ++iter;
                                        continue;
                                    case '+':
                                    case '-':
                                        _parser->on_number(c);
                                        _state = state_t::after_exponent_sign;
                                        ++iter;
                                        continue;
                                    default:
                                        bytes += iter - data + 1;
                                        return parse_error_t::unknown_number_character;
                                }
                            }
                        case state_t::after_exponent_sign:
                            {
                                const char c = *iter;
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
                                        _parser->on_number(c);
                                        _state = state_t::parsing_exponent;
                                        ++iter;
                                        continue;
                                    default:
                                        bytes += iter - data + 1;
                                        return parse_error_t::unknown_number_character;
                                }
                            }

                            // number acceptable state
                        case state_t::after_zero:
                            {
                                const char c = *iter;
                                switch (c) {
                                    case '.':
                                        _parser->on_number(c);
                                        _state = state_t::after_point;
                                        ++iter;
                                        continue;
                                    case 'e':
                                    case 'E':
                                        _parser->on_number(c);
                                        _state = state_t::after_e;
                                        ++iter;
                                        continue;
                                    default:
                                        err = _parser->on_number_end();
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        _state = state_t::normal;
                                        continue;
                                }
                            }
                        case state_t::after_one_to_nine:
                            {
                                const char c = *iter;
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
                                        _parser->on_number(c);
                                        _state = state_t::parsing_digital;
                                        ++iter;
                                        continue;
                                    case '.':
                                        _parser->on_number(c);
                                        _state = state_t::after_point;
                                        ++iter;
                                        continue;
                                    case 'e':
                                    case 'E':
                                        _parser->on_number(c);
                                        _state = state_t::after_e;
                                        ++iter;
                                        continue;
                                    default:
                                        err = _parser->on_number_end();
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        _state = state_t::normal;
                                        continue;
                                }
                            }
                        case state_t::parsing_digital:
                            {
                                const char c = *iter;
                                switch (c) {
                                    case '.':
                                        _parser->on_number(c);
                                        _state = state_t::after_point;
                                        ++iter;
                                        continue;
                                    case 'e':
                                    case 'E':
                                        _parser->on_number(c);
                                        _state = state_t::after_e;
                                        ++iter;
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
                                        _parser->on_number(c);
                                        ++iter;
                                        continue;
                                    default:
                                        err = _parser->on_number_end();
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        _state = state_t::normal;
                                        continue;
                                }
                            }
                        case state_t::parsing_exponent:
                            {
                                const char c = *iter;
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
                                        _parser->on_number(c);
                                        ++iter;
                                        continue;
                                    default:
                                        err = _parser->on_number_end();
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        _state = state_t::normal;
                                        continue;
                                }
                            }
                        case state_t::fractional_part:
                            {
                                const char c = *iter;
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
                                        _parser->on_number(c);
                                        ++iter;
                                        continue;
                                    case 'e':
                                    case 'E':
                                        _parser->on_number(c);
                                        _state = state_t::after_e;
                                        ++iter;
                                        continue;
                                    default:
                                        err = _parser->on_number_end();
                                        if (err) {
                                            bytes += iter - data + 1;
                                            return err;
                                        }
                                        _state = state_t::normal;
                                        continue;
                                }
                            }

                            // false
                        case state_t::f:
                            {
                                if (*iter == 'a') {
                                    _state = state_t::fa;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_boolean_character;
                                }
                            }
                        case state_t::fa:
                            {
                                if (*iter == 'l') {
                                    _state = state_t::fal;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_boolean_character;
                                }
                            }
                        case state_t::fal:
                            {
                                if (*iter == 's') {
                                    _state = state_t::fals;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_boolean_character;
                                }
                            }
                        case state_t::fals:
                            {
                                if (*iter == 'e') {
                                    err = _parser->on_false();
                                    if (err) {
                                        bytes += iter - data + 1;
                                        return err;
                                    }
                                    _state = state_t::normal;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_boolean_character;
                                }
                            }

                            //true
                        case state_t::t:
                            {
                                if (*iter == 'r') {
                                    _state = state_t::tr;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_boolean_character;
                                }
                            }
                        case state_t::tr:
                            {
                                if (*iter == 'u') {
                                    _state = state_t::tru;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_boolean_character;
                                }
                            }
                        case state_t::tru:
                            {
                                if (*iter == 'e') {
                                    err = _parser->on_true();
                                    if (err) {
                                        bytes += iter - data + 1;
                                        return err;
                                    }
                                    _state = state_t::normal;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_boolean_character;
                                }
                            }

                            //null
                        case state_t::n:
                            {
                                if (*iter == 'u') {
                                    _state = state_t::nu;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_null_character;
                                }
                            }
                        case state_t::nu:
                            {
                                if (*iter == 'l') {
                                    _state = state_t::nul;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_null_character;
                                }
                            }
                        case state_t::nul:
                            {
                                if (*iter == 'l') {
                                    err = _parser->on_null();
                                    if (err) {
                                        bytes += iter - data + 1;
                                        return err;
                                    }
                                    _state = state_t::normal;
                                    ++iter;
                                    continue;
                                }
                                else {
                                    bytes += iter - data + 1;
                                    return parse_error_t::unknown_null_character;
                                }
                            }
                        default:
                            std::unreachable();
                    }
                }
                bytes += size;
                return {};
            }

            static constexpr bool is_space(const char c)noexcept {
                switch (c) {
                    case ' ':
                    case '\n':
                    case '\t':
                    case '\r':
                        return true;
                    default:
                        return false;
                }
            }

            static constexpr const char* skip_space(const char* iter, const char* const end)noexcept {
                while (iter != end and is_space(*iter)) {
                    ++iter;
                }
                return iter;
            }

            static constexpr std::optional<uint32_t> hex_to_int(const char c)noexcept {
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
                        return uint32_t(c - '0');
                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                    case 'e':
                    case 'f':
                        return uint32_t(c - 'a' + 10);
                    case 'A':
                    case 'B':
                    case 'C':
                    case 'D':
                    case 'E':
                    case 'F':
                        return uint32_t(c - 'A' + 10);
                    default:
                        return std::nullopt;
                }
            }

            constexpr static std::string_view unicode_to_utf8(const uint32_t uc)noexcept {
                static char buf[4];
                size_t n = 0;

                if (uc <= 0x7F) {
                    // 单字节 UTF-8 编码
                    n = 1;
                    buf[0] = static_cast<char>(uc);
                }
                else if (uc <= 0x7FF) {
                    // 双字节 UTF-8 编码
                    n = 2;
                    buf[0] = static_cast<char>(0xC0 | (uc >> 6));
                    buf[1] = static_cast<char>(0x80 | (uc & 0x3F));
                }
                else if (uc <= 0xFFFF) {
                    // 三字节 UTF-8 编码
                    n = 3;
                    buf[0] = static_cast<char>(0xE0 | (uc >> 12));
                    buf[1] = static_cast<char>(0x80 | ((uc >> 6) & 0x3F));
                    buf[2] = static_cast<char>(0x80 | (uc & 0x3F));
                }
                else if (uc <= 0x10FFFF) {
                    // 四字节 UTF-8 编码
                    n = 4;
                    buf[0] = static_cast<char>(0xF0 | (uc >> 18));
                    buf[1] = static_cast<char>(0x80 | ((uc >> 12) & 0x3F));
                    buf[2] = static_cast<char>(0x80 | ((uc >> 6) & 0x3F));
                    buf[3] = static_cast<char>(0x80 | (uc & 0x3F));
                }

                return { buf, n };
            }

            P* _parser;
            enum struct state_t {
                normal,

                // string
                after_quotation_mark,
                after_reverse_solidus,
                wait_4_hex,
                wait_3_hex,
                wait_2_hex,
                wait_1_hex,

                wait_low_reverse_solidus,
                wait_low_u,
                wait_low_4_hex,
                wait_low_3_hex,
                wait_low_2_hex,
                wait_low_1_hex,

                wait_3_utf8_bytes,
                wait_2_utf8_bytes,
                wait_1_utf8_bytes,

                // number
                after_optional_negative,
                after_zero,         //end
                after_one_to_nine,  //end
                after_point,
                parsing_digital,    //end
                after_e,
                parsing_exponent,   //end
                after_exponent_sign,
                fractional_part,    //end

                // false
                f,
                fa,
                fal,
                fals,

                //true
                t,
                tr,
                tru,

                //null
                n,
                nu,
                nul,

                error
            };
            state_t _state = state_t::normal;
            uint8_t _hex = 0;
            size_t bytes = 0;
            uint32_t unicode_high = 0;
            uint32_t unicode_low = 0;
        };

        template<Builder B>
        struct parser {
            explicit parser(B* b) :builder{ b } {
                _stack.emplace_back(json_parser{ {}, state_t{}, this });
            }

            std::optional<parse_error_t> on_null()noexcept {
                assert(!_stack.empty());
                return std::visit([ ](auto& p) {
                    return p.on_null();
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_true()noexcept {
                assert(!_stack.empty());
                return std::visit([ ](auto& p) {
                    return p.on_true();
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_false()noexcept {
                assert(!_stack.empty());
                return std::visit([ ](auto& p) {
                    return p.on_false();
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_string(const char c)noexcept {
                assert(!_stack.empty());
                return std::visit([=](auto& p) {
                    return p.on_string(c);
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_string_begin()noexcept {
                assert(!_stack.empty());
                return std::visit([ ](auto& p) {
                    return p.on_string_begin();
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_string_end()noexcept {
                assert(!_stack.empty());
                return std::visit([ ](auto& p) {
                    return p.on_string_end();
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_comma()noexcept {
                assert(!_stack.empty());
                return std::visit([ ](auto& p) {
                    return p.on_comma();
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_colon()noexcept {
                assert(!_stack.empty());
                return std::visit([ ](auto& p) {
                    return p.on_colon();
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_number(const char c)noexcept {
                assert(!_stack.empty());
                return std::visit([=](auto& p) {
                    return p.on_number(c);
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_number_begin(const char c)noexcept {
                assert(!_stack.empty());
                return std::visit([=](auto& p) {
                    return p.on_number_begin(c);
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_number_end()noexcept {
                assert(!_stack.empty());
                return std::visit([ ](auto& p) {
                    return p.on_number_end();
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_left_square_bracket()noexcept {
                assert(!_stack.empty());
                return std::visit([ ](auto& p) {
                    return p.on_left_square_bracket();
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_right_square_bracket()noexcept {
                assert(!_stack.empty());
                return std::visit([ ](auto& p) {
                    return p.on_right_square_bracket();
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_left_brace()noexcept {
                assert(!_stack.empty());
                return std::visit([ ](auto& p) {
                    return p.on_left_brace();
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_right_brace()noexcept {
                assert(!_stack.empty());
                return std::visit([ ](auto& p) {
                    return p.on_right_brace();
                    }, _stack.back());
            }

            B* builder;

            private:
            enum struct state_t {
                start,

                parsing_object,

                parsing_A,
                parsing_A_after_B,

                parsing_B,

                parsing_pair,
                parsing_pair_after_string,

                parsing_array,

                parsing_C,

                parsing_elements,

                parsing_D,

                parsing_value,
            };

            struct parse_functor_base {
                std::optional<parse_error_t> on_null()noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_true()noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_false()noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_string(const char c)noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_string_begin()noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_string_end()noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_comma()noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_colon()noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_number(const char c)noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_number_begin(const char c)noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_number_end()noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_left_square_bracket()noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_right_square_bracket()noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_left_brace()noexcept {
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_right_brace()noexcept {
                    return parse_error_t{};
                }
            };

            struct json_parser : parse_functor_base {
                std::optional<parse_error_t> on_left_brace()noexcept {
                    auto p = parent;
                    auto err = p->builder->on_document_begin();
                    if (err)
                        return err;
                    p->_stack.back() = object_parser{ {}, state_t::parsing_object, p };
                    return p->on_left_brace();
                }

                std::optional<parse_error_t> on_left_square_bracket()noexcept {
                    auto p = parent;
                    auto err = p->builder->on_document_begin();
                    if (err)
                        return err;
                    p->_stack.back() = array_parser{ {}, state_t::parsing_array, p };
                    return p->on_left_square_bracket();
                }

                state_t state;
                parser* parent;
            };

            struct object_parser : parse_functor_base {
                std::optional<parse_error_t> on_left_brace()noexcept {
                    auto err = parent->builder->on_object_begin();
                    auto p = parent;
                    p->_stack.back() = A_parser{ {}, state_t::parsing_A, p };
                    return err;
                }

                state_t state;
                parser* parent;
            };

            struct A_parser : parse_functor_base {
                std::optional<parse_error_t> on_right_brace()noexcept {
                    if (state == state_t::parsing_A or state == state_t::parsing_A_after_B) {
                        auto p = parent;
                        auto err = p->builder->on_object_end();
                        if (err)
                            return err;
                        p->_stack.pop_back();
                        if (p->_stack.empty()) {
                            p->builder->on_document_end();
                            return parse_error_t::happy_ending;
                        }
                        return {};
                    }
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_string_begin()noexcept {
                    if (state == state_t::parsing_A) {
                        state = state_t::parsing_A_after_B;
                        auto p = parent;
                        p->_stack.emplace_back(B_parser{ {}, state_t::parsing_B, p });
                        p->_stack.emplace_back(pair_parser{ {}, state_t::parsing_pair, p });
                        return p->on_string_begin();
                    }
                    return parse_error_t{};
                }

                state_t state;
                parser* parent;
            };

            struct B_parser : parse_functor_base {
                std::optional<parse_error_t> on_right_brace()noexcept {
                    if (state == state_t::parsing_B) {
                        auto p = parent;
                        p->_stack.pop_back();
                        assert(!p->_stack.empty());
                        return p->on_right_brace();
                    }
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_comma()noexcept {
                    auto p = parent;
                    p->_stack.emplace_back(pair_parser{ {}, state_t::parsing_pair,p });
                    return {};
                }

                state_t state;
                parser* parent;
            };

            struct pair_parser : parse_functor_base {
                std::optional<parse_error_t> on_string_begin()noexcept {
                    if (state == state_t::parsing_pair) {
                        return {};
                    }
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_string(const char c)noexcept {
                    key.push_back(c);
                    return {};
                }

                std::optional<parse_error_t> on_string_end()noexcept {
                    std::string str;
                    str.swap(key);
                    state = state_t::parsing_pair_after_string;
                    return parent->builder->on_key(std::move(str));
                }

                std::optional<parse_error_t> on_colon()noexcept {
                    if (state == state_t::parsing_pair_after_string) {
                        auto p = parent;
                        p->_stack.back() = value_parser{ {}, state_t::parsing_value, p };
                        return {};
                    }
                    return parse_error_t{};
                }

                state_t state;
                parser* parent;
                std::string key;
            };

            struct value_parser : parse_functor_base {
                std::optional<parse_error_t> on_left_brace()noexcept {
                    auto p = parent;
                    p->_stack.back() = object_parser{ {}, state_t::parsing_object, p };
                    return p->on_left_brace();
                }

                std::optional<parse_error_t> on_left_square_bracket()noexcept {
                    auto p = parent;
                    p->_stack.back() = array_parser{ {}, state_t::parsing_array, p };
                    return p->on_left_square_bracket();
                }

                std::optional<parse_error_t> on_number(const char c)noexcept {
                    str.push_back(c);
                    return {};
                }

                std::optional<parse_error_t> on_number_begin(const char c)noexcept {
                    str.push_back(c);
                    return {};
                }

                std::optional<parse_error_t> on_number_end()noexcept {
                    std::string num;
                    str.swap(num);
                    auto err = parent->builder->on_number(std::move(num));
                    parent->_stack.pop_back();
                    return err;
                }

                std::optional<parse_error_t> on_true()noexcept {
                    auto err = parent->builder->on_bool(true);
                    parent->_stack.pop_back();
                    return err;
                }

                std::optional<parse_error_t> on_false()noexcept {
                    auto err = parent->builder->on_bool(false);
                    parent->_stack.pop_back();
                    return err;
                }

                std::optional<parse_error_t> on_string(const char c)noexcept {
                    str.push_back(c);
                    return {};
                }

                std::optional<parse_error_t> on_string_begin()noexcept {
                    return {};
                }

                std::optional<parse_error_t> on_string_end()noexcept {
                    std::string s;
                    s.swap(str);
                    auto err = parent->builder->on_string(std::move(s));
                    parent->_stack.pop_back();
                    return err;
                }

                std::optional<parse_error_t> on_null()noexcept {
                    auto err = parent->builder->on_null();
                    parent->_stack.pop_back();
                    return err;
                }

                state_t state;
                parser* parent;
                std::string str;
            };

            struct array_parser : parse_functor_base {
                std::optional<parse_error_t> on_left_square_bracket()noexcept {
                    auto p = parent;
                    auto err = p->builder->on_array_begin();
                    p->_stack.back() = C_parser{ {}, state_t::parsing_C, p };
                    return err;
                }

                state_t state;
                parser* parent;
            };

            struct C_parser : parse_functor_base {
                std::optional<parse_error_t> on_null()noexcept {
                    if (state == state_t::parsing_C) {
                        auto p = parent;
                        p->_stack.emplace_back(elements_parser{ {}, state_t::parsing_elements, p });
                        return p->on_null();
                    }
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_true()noexcept {
                    if (state == state_t::parsing_C) {
                        auto p = parent;
                        p->_stack.emplace_back(elements_parser{ {}, state_t::parsing_elements, p });
                        return p->on_true();
                    }
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_false()noexcept {
                    if (state == state_t::parsing_C) {
                        auto p = parent;
                        p->_stack.emplace_back(elements_parser{ {}, state_t::parsing_elements, p });
                        return p->on_false();
                    }
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_string_begin()noexcept {
                    if (state == state_t::parsing_C) {
                        auto p = parent;
                        p->_stack.emplace_back(elements_parser{ {}, state_t::parsing_elements, p });
                        return p->on_string_begin();
                    }
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_number_begin(const char c)noexcept {
                    if (state == state_t::parsing_C) {
                        auto p = parent;
                        p->_stack.emplace_back(elements_parser{ {}, state_t::parsing_elements, p });
                        return p->on_number_begin(c);
                    }
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_left_square_bracket()noexcept {
                    if (state == state_t::parsing_C) {
                        auto p = parent;
                        p->_stack.emplace_back(elements_parser{ {}, state_t::parsing_elements, p });
                        return p->on_left_square_bracket();
                    }
                    return parse_error_t{};
                }

                std::optional<parse_error_t> on_right_square_bracket()noexcept {
                    auto p = parent;
                    auto err = p->builder->on_array_end();
                    p->_stack.pop_back();
                    if (p->_stack.empty()) {
                        p->builder->on_document_end();
                        return parse_error_t::happy_ending;
                    }
                    return err;
                }

                std::optional<parse_error_t> on_left_brace()noexcept {
                    if (state == state_t::parsing_C) {
                        auto p = parent;
                        p->_stack.emplace_back(elements_parser{ {}, state_t::parsing_elements, p });
                        return p->on_left_brace();
                    }
                    return parse_error_t{};
                }

                state_t state;
                parser* parent;
            };

            struct elements_parser : parse_functor_base {
                std::optional<parse_error_t> on_null()noexcept {
                    auto p = parent;
                    p->_stack.back() = D_parser{ {}, state_t::parsing_D, p };
                    p->_stack.emplace_back(value_parser{ {}, state_t::parsing_value, p });
                    return p->on_null();
                }

                std::optional<parse_error_t> on_true()noexcept {
                    auto p = parent;
                    p->_stack.back() = D_parser{ {}, state_t::parsing_D, p };
                    p->_stack.emplace_back(value_parser{ {}, state_t::parsing_value, p });
                    return p->on_true();
                }

                std::optional<parse_error_t> on_false()noexcept {
                    auto p = parent;
                    p->_stack.back() = D_parser{ {}, state_t::parsing_D, p };
                    p->_stack.emplace_back(value_parser{ {}, state_t::parsing_value, p });
                    return p->on_false();
                }

                std::optional<parse_error_t> on_string_begin()noexcept {
                    auto p = parent;
                    p->_stack.back() = D_parser{ {}, state_t::parsing_D, p };
                    p->_stack.emplace_back(value_parser{ {}, state_t::parsing_value, p });
                    return p->on_string_begin();
                }

                std::optional<parse_error_t> on_number_begin(const char c)noexcept {
                    auto p = parent;
                    p->_stack.back() = D_parser{ {}, state_t::parsing_D, p };
                    p->_stack.emplace_back(value_parser{ {}, state_t::parsing_value, p });
                    return p->on_number_begin(c);
                }

                std::optional<parse_error_t> on_left_square_bracket()noexcept {
                    auto p = parent;
                    p->_stack.back() = D_parser{ {}, state_t::parsing_D, p };
                    p->_stack.emplace_back(value_parser{ {}, state_t::parsing_value, p });
                    return p->on_left_square_bracket();
                }

                std::optional<parse_error_t> on_left_brace()noexcept {
                    auto p = parent;
                    p->_stack.back() = D_parser{ {}, state_t::parsing_D, p };
                    p->_stack.emplace_back(value_parser{ {}, state_t::parsing_value, p });
                    return p->on_left_brace();
                }

                state_t state;
                parser* parent;
            };

            struct D_parser : parse_functor_base {
                std::optional<parse_error_t> on_right_square_bracket()noexcept {
                    auto p = parent;
                    p->_stack.pop_back();
                    assert(!p->_stack.empty());
                    return p->on_right_square_bracket();
                }

                std::optional<parse_error_t> on_comma()noexcept {
                    auto p = parent;
                    p->_stack.back() = elements_parser{ {}, state_t::parsing_elements, p };
                    return {};
                }

                state_t state;
                parser* parent;
            };

            using sub_parser = std::variant<
                json_parser,
                object_parser,
                A_parser,
                B_parser,
                pair_parser,
                value_parser,
                array_parser,
                C_parser,
                elements_parser,
                D_parser
            >;

            std::vector<sub_parser> _stack;
        };

        struct document_printer {
            document_printer(int max_depth = 0)noexcept {}

            std::optional<parse_error_t> on_document_begin()noexcept {
                print_space();
                std::cout << "document begin\n";
                ++_depth;
                return {};
            }

            std::optional<parse_error_t> on_document_end()noexcept {
                --_depth;
                print_space();
                std::cout << "document end\n";
                return {};
            }

            std::optional<parse_error_t> on_object_begin()noexcept {
                print_space();
                std::cout << "object begin\n";
                ++_depth;
                return {};
            }

            std::optional<parse_error_t> on_object_end()noexcept {
                --_depth;
                print_space();
                std::cout << "object end\n";
                return {};
            }

            std::optional<parse_error_t> on_array_begin()noexcept {
                print_space();
                std::cout << "array begin\n";
                ++_depth;
                return {};
            }

            std::optional<parse_error_t> on_array_end()noexcept {
                --_depth;
                print_space();
                std::cout << "array end\n";
                return {};
            }

            std::optional<parse_error_t> on_key(std::string key)noexcept {
                print_space();
                std::cout << std::format("key:\"{}\"\n", key);
                return {};
            }

            std::optional<parse_error_t> on_string(std::string str)noexcept {
                print_space();
                std::cout << std::format("string:\"{}\"\n", str);
                return {};
            }

            std::optional<parse_error_t> on_number(std::string num)noexcept {
                print_space();
                std::cout << std::format("number:{}\n", num);
                return {};
            }

            std::optional<parse_error_t> on_bool(const bool b)noexcept {
                print_space();
                std::cout << std::format("boolean:{}\n", b ? "true" : "false");
                return {};
            }

            std::optional<parse_error_t> on_null()noexcept {
                print_space();
                std::cout << "null\n";
                return {};
            }

            int get()noexcept { return 0; }

            private:
            void print_space()const noexcept {
                std::string str(_depth * 4, ' ');
                std::cout << str;
            }

            int _depth = 0;
        };

    }


    struct value {

        value()noexcept = default;

        template<class T>
            requires
        std::same_as<T, double> ||
            std::same_as<T, bool> ||
            std::constructible_from<std::string, T> ||
            std::constructible_from<std::vector<value>, T> ||
            std::constructible_from<std::map<std::string, value>, T> ||
            std::constructible_from<std::monostate, T>
            value(T&& x)noexcept :
            data{ std::forward<T>(x) } {}


        value(std::initializer_list<value> arr)noexcept {
            data.emplace<std::vector<value>>(std::move(arr));
        }

        value(std::initializer_list<std::pair<const std::string, value>> obj)noexcept {
            data.emplace<std::map<std::string, value>>(std::move(obj));
        }

        value(const value&)noexcept = default;
        value(value&&)noexcept = default;

        value& operator=(const value& other)noexcept = default;
        value& operator=(value&& other)noexcept = default;

        value& operator=(std::initializer_list<value> arr)noexcept {
            data.emplace<std::vector<value>>(std::move(arr));
            return *this;
        }

        value& operator=(std::initializer_list<std::pair<const std::string, value>> obj)noexcept {
            data.emplace<std::map<std::string, value>>(std::move(obj));
            return *this;
        }

        bool is_number()const noexcept {
            return std::holds_alternative<double>(data);
        }

        bool is_string()const noexcept {
            return std::holds_alternative<std::string>(data);
        }

        bool is_null()const noexcept {
            return std::holds_alternative<std::monostate>(data);
        }

        bool is_bool()const noexcept {
            return std::holds_alternative<bool>(data);
        }

        bool is_array()const noexcept {
            return std::holds_alternative<std::vector<value>>(data);
        }

        bool is_object()const noexcept {
            return std::holds_alternative<std::map<std::string, value>>(data);
        }

        auto& get_object() {
            return std::get<std::map<std::string, value>>(data);
        }

        const auto& get_object()const {
            return std::get<std::map<std::string, value>>(data);
        }

        auto& get_array() {
            return std::get<std::vector<value>>(data);
        }

        const auto& get_array()const {
            return std::get<std::vector<value>>(data);
        }

        auto& get_number() {
            return std::get<double>(data);
        }

        auto get_number()const {
            return std::get<double>(data);
        }

        auto& get_string() {
            return std::get<std::string>(data);
        }

        const auto& get_string()const {
            return std::get<std::string>(data);
        }

        bool& get_bool() {
            return std::get<bool>(data);
        }

        bool get_bool()const {
            return std::get<bool>(data);
        }

        std::monostate get_null()const {
            return std::get<std::monostate>(data);
        }

        void emplace_null(const std::monostate = {})noexcept {
            data.emplace<std::monostate>();
        }

        double& emplace_number()noexcept {
            data.emplace<double>();
            return std::get<double>(data);
        }

        void emplace_number(const double x)noexcept {
            data.emplace<double>(x);
        }

        bool& emplace_bool()noexcept {
            data.emplace<bool>();
            return std::get<bool>(data);
        }

        void emplace_bool(const bool b)noexcept {
            data.emplace<bool>(b);
        }

        std::string& emplace_string()noexcept {
            data.emplace<std::string>();
            return std::get<std::string>(data);
        }

        void emplace_string(std::string str)noexcept {
            data.emplace<std::string>(std::move(str));
        }

        auto& emplace_array()noexcept {
            data.emplace<std::vector<value>>();
            return std::get<std::vector<value>>(data);
        }

        void emplace_array(std::vector<value> arr)noexcept {
            data.emplace<std::vector<value>>(std::move(arr));
        }

        auto& emplace_object()noexcept {
            data.emplace<std::map<std::string, value>>();
            return std::get<std::map<std::string, value>>(data);
        }

        void emplace_object(std::map<std::string, value> obj)noexcept {
            data.emplace<std::map<std::string, value>>(std::move(obj));
        }

        auto& operator[](const std::string& key) {
            return get_object()[key];
        }

        const auto& operator[](const std::string& key)const {
            return get_object().at(key);
        }

        auto& operator[](const size_t index) {
            return get_array()[index];
        }

        const auto& operator[](const size_t index)const {
            return get_array()[index];
        }

        auto operator==(const value& other)const noexcept {
            return data == other.data;
        }

        void swap(value& other)noexcept {
            std::swap(data, other.data);
        }

        std::string type()const noexcept {
            if (std::holds_alternative<std::monostate>(data))
                return "null";
            else if (std::holds_alternative<std::vector<value>>(data))
                return "array";
            else if (std::holds_alternative<std::map<std::string, value>>(data))
                return "object";
            else if (std::holds_alternative<double>(data))
                return "number";
            else if (std::holds_alternative<std::string>(data))
                return "string";
            else if (std::holds_alternative<bool>(data))
                return "boolean";
            return "";
        }

        std::variant<
            std::monostate,                 // null
            bool,                           // true false
            double,
            std::string,
            std::vector<value>,             // array
            std::map<std::string, value>    // object
        > data;
    };

    static constexpr std::monostate null{};
    using array = std::vector<value>;
    using object = std::map<std::string, value>;
    using document = std::variant<std::monostate, array, object>;

    namespace detail {
        struct document_builder {

            document_builder(int depth)noexcept :_max_depth{ depth } {}

            std::optional<parse_error_t> on_document_begin()noexcept {
                _stack.push_back(&_res);
                return {};
            }

            std::optional<parse_error_t> on_document_end()noexcept {
                _finish = true;
                return {};
            }

            std::optional<parse_error_t> on_object_begin()noexcept {
                assert(!_stack.empty());
                ++_depth;
                if (_depth > _max_depth)
                    return parse_error_t::too_deep;
                std::visit([this](auto s)noexcept {
                    if constexpr (std::is_same_v<json::document*, decltype(s)>) {
                        s->template emplace<json::object>();
                        _stack.emplace_back(&std::get<json::object>(*s));
                    }
                    else if constexpr (std::is_same_v<json::array*, decltype(s)>) {
                        s->emplace_back();
                        s->back().data.template emplace<json::object>();
                        _stack.emplace_back(&std::get<json::object>(s->back().data));
                    }
                    else if constexpr (std::is_same_v<json::object::iterator, decltype(s)>) {
                        s->second.data.template emplace<json::object>();
                        _stack.back() = &std::get<json::object>(s->second.data);
                    }
                    }, _stack.back());
                return {};
            }

            std::optional<parse_error_t> on_object_end()noexcept {
                assert(std::get_if<json::object*>(&_stack.back()) != nullptr);
                _stack.pop_back();
                --_depth;
                return {};
            }

            std::optional<parse_error_t> on_array_begin()noexcept {
                assert(!_stack.empty());
                ++_depth;
                if (_depth > _max_depth)
                    return parse_error_t::too_deep;
                std::visit([this](auto s)noexcept {
                    if constexpr (std::is_same_v<json::document*, decltype(s)>) {
                        s->template emplace<json::array>();
                        _stack.emplace_back(&std::get<json::array>(*s));
                    }
                    else if constexpr (std::is_same_v<json::array*, decltype(s)>) {
                        s->emplace_back();
                        s->back().data.template emplace<json::array>();
                        _stack.emplace_back(&std::get<json::array>(s->back().data));
                    }
                    else if constexpr (std::is_same_v<json::object::iterator, decltype(s)>) {
                        s->second.data.template emplace<json::array>();
                        _stack.back() = &std::get<json::array>(s->second.data);
                    }
                    }, _stack.back());
                return {};
            }

            std::optional<parse_error_t> on_array_end()noexcept {
                assert(std::get_if<json::array*>(&_stack.back()) != nullptr);
                _stack.pop_back();
                --_depth;
                return {};
            }

            std::optional<parse_error_t> on_key(std::string key)noexcept {
                assert(std::get_if<json::object*>(&_stack.back()) != nullptr);
                return std::visit([&, this](auto s)noexcept->std::optional<parse_error_t> {
                    if constexpr (std::is_same_v<json::object*, decltype(s)>) {
                        if (s->contains(key))
                            return parse_error_t::duplicate_key;
                        auto [iter, _] = s->emplace(std::move(key), json::value{});
                        _stack.emplace_back(iter);
                    }
                    return {};
                    }, _stack.back());
            }

            std::optional<parse_error_t> on_string(std::string str)noexcept {
                assert(!_stack.empty());
                std::visit([&, this](auto s)noexcept {
                    if constexpr (std::is_same_v<json::array*, decltype(s)>) {
                        s->emplace_back();
                        s->back().data.template emplace<std::string>(std::move(str));
                    }
                    else if constexpr (std::is_same_v<json::object::iterator, decltype(s)>) {
                        s->second.data.template emplace<std::string>(std::move(str));
                        _stack.pop_back();
                    }
                    }, _stack.back());
                return {};
            }

            std::optional<parse_error_t> on_number(std::string num)noexcept {
                assert(!_stack.empty());
                double x;
                if (auto [_, e] = std::from_chars(num.data(), num.data() + num.size(), x); e != std::errc())
                    x = 0;
                std::visit([=, this](auto s)noexcept {
                    if constexpr (std::is_same_v<json::array*, decltype(s)>) {
                        s->emplace_back();
                        s->back().data.template emplace<double>(x);
                    }
                    else if constexpr (std::is_same_v<json::object::iterator, decltype(s)>) {
                        s->second.data.template emplace<double>(x);
                        _stack.pop_back();
                    }
                    }, _stack.back());
                return {};
            }

            std::optional<parse_error_t> on_bool(const bool b)noexcept {
                assert(!_stack.empty());
                std::visit([=, this](auto s)noexcept {
                    if constexpr (std::is_same_v<json::array*, decltype(s)>) {
                        s->emplace_back();
                        s->back().data.template emplace<bool>(b);
                    }
                    else if constexpr (std::is_same_v<json::object::iterator, decltype(s)>) {
                        s->second.data.template emplace<bool>(b);
                        _stack.pop_back();
                    }
                    }, _stack.back());
                return {};
            }

            std::optional<parse_error_t> on_null()noexcept {
                assert(!_stack.empty());
                std::visit([=, this](auto s)noexcept {
                    if constexpr (std::is_same_v<json::array*, decltype(s)>) {
                        s->emplace_back();
                    }
                    else if constexpr (std::is_same_v<json::object::iterator, decltype(s)>) {
                        _stack.pop_back();
                    }
                    }, _stack.back());
                return {};
            }

            bool finish()const noexcept {
                return _finish;
            }

            json::document get()noexcept {
                return std::move(_res);
            }

            private:
            using state_type = std::variant<
                json::document*,
                json::object*,
                json::object::iterator,
                json::array*
            >;

            bool _finish = false;
            document _res;
            std::vector<state_type> _stack;
            int _depth = 0;
            int _max_depth;
        };

        struct serializer {
            std::optional<serialize_error_t> operator()(const json::document& dom)noexcept {
                return std::visit(*this, dom);
            }

            std::optional<serialize_error_t> operator()(const json::value& v)noexcept {
                return std::visit(*this, v.data);
            }

            std::optional<serialize_error_t> operator()(const json::array& arr)noexcept {
                js += '[';
                for (const auto& v : arr) {
                    auto err = std::visit(*this, v.data);
                    if (err)
                        return err;
                    js += ',';
                }
                if (js.back() == ',')
                    js.back() = ']';
                else js += ']';
                return {};
            }

            std::optional<serialize_error_t> operator()(const json::object& obj)noexcept {
                js += '{';
                for (const auto& [k, v] : obj) {
                    auto err = (*this)(k);
                    if (err)
                        return err;
                    js += ':';
                    err = std::visit(*this, v.data);
                    if (err)
                        return err;
                    js += ',';
                }
                if (js.back() == ',')
                    js.back() = '}';
                else js += '}';
                return {};
            }

            std::optional<serialize_error_t> operator()(const std::string& str)noexcept {
                js += '\"';
                for (auto i = 0; i < str.size(); ++i) {
                    const auto c = str[i];
                    if (auto n = std::countl_one((uint8_t)c); n == 0) {
                        switch (c) {
                            case '\"':
                                js += "\\\"";
                                continue;
                            case '\\':
                                js += '\\';
                                js += '\\';
                                continue;
                            case '\b':
                                js += '\\';
                                js += 'b';
                                continue;
                            case '\f':
                                js += '\\';
                                js += 'f';
                                continue;
                            case '\n':
                                js += '\\';
                                js += 'n';
                                continue;
                            case '\r':
                                js += '\\';
                                js += 'r';
                                continue;
                            case '\t':
                                js += '\\';
                                js += 't';
                                continue;
                            default:
                                if (std::iscntrl(c)) {
                                    return serialize_error_t::unknown_utf8_bytes;
                                }
                                js += c;
                                continue;
                        }
                    }
                    else if (n > 1 and n < 5) {
                        if (i + n > str.size())
                            return serialize_error_t::unknown_utf8_bytes;
                        while (n--) {
                            js += str[i++];
                        }
                        --i;
                        continue;
                    }
                    else {
                        return serialize_error_t::unknown_utf8_bytes;
                    }
                }
                js += '\"';
                return {};
            }

            std::optional<serialize_error_t> operator()(const bool b)noexcept {
                js += b ? "true" : "false";
                return {};
            }

            std::optional<serialize_error_t> operator()(const std::monostate&)noexcept {
                js += "null";
                return {};
            }

            std::optional<serialize_error_t> operator()(const double x)noexcept {
                std::format_to(std::back_inserter(js), "{}", x);
                return {};
            }

            const std::string& get()const noexcept {
                return js;
            }

            std::string get()noexcept {
                return std::move(js);
            }

            private:
            std::string js;
        };
    }

    std::expected<std::string, serialize_error_t> to_json(const auto& dom)noexcept {
        detail::serializer serializer;
        auto err = serializer(dom);
        if (err)
            return std::unexpected(*err);
        return serializer.get();
    }

    template<Builder B = detail::document_builder>
    std::expected<decltype(std::declval<B>().get()), parse_error_t> parse(const char* data, size_t size, int depth = 19)noexcept {
        B builder{ depth };
        detail::parser parser{ &builder };
        detail::lexer lexer{ &parser };

        auto err = lexer(data, size);
        if (err) {
            if (*err != parse_error_t::happy_ending)
                return std::unexpected(*err);
            auto end = lexer.skip_space(data + lexer.bytes, data + size);
            if (end != data + size)
                return std::unexpected(parse_error_t{});
            return builder.get();
        }
        return std::unexpected(parse_error_t{});
    }

    template<Builder B = detail::document_builder>
    std::expected<decltype(std::declval<B>().get()), parse_error_t> from_file(const std::string& path, size_t buf_size = 4096, int depth = 19)noexcept {
        assert(buf_size > 0);
        assert(!path.empty());
        B builder{ depth };
        detail::parser parser{ &builder };
        detail::lexer lexer{ &parser };

        std::vector<char> buffer(buf_size);
        std::FILE* file = nullptr;

#ifdef _MSC_VER
        if (fopen_s(&file, path.c_str(), "r"))
            return std::unexpected(parse_error_t::read_file_error);
#else 
        file = fopen(path.c_str(), "r");
#endif // _MSC_VER

        if (!file)
            return std::unexpected(parse_error_t::read_file_error);

        std::unique_ptr<FILE, std::function<void(FILE*)>> guard{ file, [ ](FILE* f)noexcept { ::fclose(f); } };

        while (true) {
            auto n = ::fread(buffer.data(), 1, buffer.size(), file);
            if (n < buffer.size()) {
                if (::ferror(file))
                    return std::unexpected(parse_error_t::read_file_error);
                // EOF
                if (n == 0) {
                    return std::unexpected(parse_error_t::early_EOF);
                }
                auto err = lexer(buffer.data(), n);
                if (err) {
                    if (*err != parse_error_t::happy_ending)
                        return std::unexpected(*err);
                    // finish
                    auto end = lexer.skip_space(buffer.data() + lexer.bytes, buffer.data() + n);
                    if (end != buffer.data() + n) // extra content
                    {
                        return std::unexpected(parse_error_t::extra_content);
                    }
                    return builder.get();
                }
                // early EOF
                return std::unexpected(parse_error_t::early_EOF);
            }
            // n == buffer.size()
            auto err = lexer(buffer.data(), buffer.size());
            if (err) {
                if (*err == parse_error_t::happy_ending) {
                    auto end = lexer.skip_space(buffer.data() + lexer.bytes, buffer.data() + buffer.size());
                    if (end != buffer.data() + buffer.size()) // extra content
                        return std::unexpected(parse_error_t::extra_content);
                    break;
                }
                return std::unexpected(*err);
            }
        }

        // May be have extra content
        while (true) {
            auto n = ::fread(buffer.data(), 1, buffer.size(), file);
            if (n == 0)
                return builder.get();
            auto end = lexer.skip_space(buffer.data(), buffer.data() + n);
            if (end != buffer.data() + n) // extra content
                return std::unexpected(parse_error_t::extra_content);
            if (n < buffer.size()) {
                if (::ferror(file))
                    return std::unexpected(parse_error_t::read_file_error);
                // EOF
                return builder.get();
            }
        }

        std::unreachable();
    }

}