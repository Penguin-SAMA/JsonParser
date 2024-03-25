#include "print.h"
#include <charconv>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

struct JSONObject {
    std::variant<
        std::nullptr_t,                             // null
        bool,                                       // true
        int,                                        // 42
        double,                                     // 3.14
        std::string,                                // "hello"
        std::vector<JSONObject>,                    // [42, "hello"]
        std::unordered_map<std::string, JSONObject> // {"hello": 985, "wrold":211}
        >
        inner;

    void do_print() const {
        printnl(inner);
    }
};

template <class T>
std::optional<T> try_parse_num(std::string_view str) {
    T value;
    auto res = std::from_chars(str.data(), str.data() + str.size(), value);
    if (res.ec == std::errc() && res.ptr == str.data() + str.size()) {
        return value;
    }
    return std::nullopt;
}

char unescaped_char(char c) {
    switch (c) {
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    case '0':
        return '\0';
    case 't':
        return '\t';
    case 'v':
        return '\v';
    case 'f':
        return '\f';
    case 'b':
        return '\b';
    case 'a':
        return '\a';
    default:
        return c;
    }
}

std::pair<JSONObject, size_t> parse(std::string_view json) {
    if (json.empty()) {
        return {JSONObject{std::nullptr_t{}}, 0};
    } else if (size_t off = json.find_first_not_of(" \n\t\r\v\f\0"); off != 0 && off != json.npos) {
        auto [obj, eaten] = parse(json.substr(off));
        return {std::move(obj), eaten + off};
    } else if ('0' <= json[0] && json[0] <= '9' || json[0] == '+' || json[0] == '-') {
        std::regex num_re{"[+-]?[0-9]+(\\.[0-9]*)?([eE][+-]?[0-9]+)?"};
        std::cmatch match;
        if (std::regex_search(json.data(), json.data() + json.size(), match, num_re)) {
            std::string str = match.str();
            if (auto num = try_parse_num<int>(str)) {
                return {JSONObject{*num}, str.size()};
            }
            if (auto num = try_parse_num<double>(str)) {
                return {JSONObject{*num}, str.size()};
            }
        }
    } else if (json[0] == '"') {
        std::string str;
        size_t i;
        enum {
            Raw,
            Escaped,
        } phase = Raw;
        for (i = 1; i < json.size(); ++i) {
            char ch = json[i];
            if (phase == Raw) {
                if (ch == '\\') {
                    phase = Escaped;
                } else if (ch == '"') {
                    i += 1;
                    break;
                } else {
                    str += ch;
                }
            } else if (phase == Escaped) {
                str += unescaped_char(ch);
                phase = Raw;
            }
        }
        return {JSONObject{std::move(str)}, i};
    } else if (json[0] == '[') {
        std::vector<JSONObject> res;
        size_t i;
        for (i = 1; i < json.size();) {
            if (json[i] == ']') {
                i += 1;
                break;
            }
            auto [obj, eaten] = parse(json.substr(i));
            if (eaten == 0) {
                i = 0;
                break;
            }

            res.push_back(std::move(obj));
            i += eaten;
            if (json[i] == ',') {
                i += 1;
            }
        }
        return {JSONObject{std::move(res)}, i};
    } else if (json[0] == '{') {
        std::unordered_map<std::string, JSONObject> res;
        size_t i;
        for (i = 1; i < json.size();) {
            if (json[i] == '}') {
                i += 1;
                break;
            }

            auto [keyobj, keyeaten] = parse(json.substr(i));
            if (keyeaten == 0) {
                i = 0;
                break;
            }

            i += keyeaten;
            if (!std::holds_alternative<std::string>(keyobj.inner)) {
                i = 0;
                break;
            }

            if (json[i] == ':') {
                i += 1;
            }

            std::string key = std::move(std::get<std::string>(keyobj.inner));
            auto [valobj, valeaten] = parse(json.substr(i));
            if (valeaten == 0) {
                i = 0;
                break;
            }
            i += valeaten;
            res.insert_or_assign(std::move(key), std::move(valobj));
            if (json[i] == ',') {
                i += 1;
            }
        }
        return {JSONObject{std::move(res)}, i};
    }

    return {JSONObject{std::nullptr_t{}}, 0};
}

int main() {
    std::string_view str = R"JSON({
    "work": 996, 
    "school": {"hello": 985, "world": 211},
 })JSON";

    auto [obj, eaten] = parse(str);
    print(obj);
    return 0;
}
