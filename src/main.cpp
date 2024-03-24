#include "print.h"
#include <charconv>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
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
        print(inner);
    }
};

template <class T>
std::optional<T> try_parser_num(std::string_view str) {
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

JSONObject parse(std::string_view json) {
    if (json.empty()) {
        return JSONObject{std::nullptr_t{}};
    }

    if ('0' <= json[0] && json[0] <= '9' || json[0] == '+' || json[0] == '-') {
        std::regex num_re{"[+-]?[0-9]+(\\.[0-9]*)?([eE][+-]?[0-9]+)?"};
        std::cmatch match;
        if (std::regex_search(json.data(), json.data() + json.size(), match, num_re)) {
            std::string str = match.str();

            if (auto num = try_parser_num<int>(str)) {
                return JSONObject{*num};
            }

            if (auto num = try_parser_num<double>(str)) {
                return JSONObject{*num};
            }
        }
    }

    if (json[0] == '"') {
        std::string str;
        enum {
            Raw,
            Escaped,
        } phase = Raw;
        for (size_t i = 1; i < json.size(); ++i) {
            char ch = json[i];
            if (phase == Raw) {
                if (ch == '\\') {
                    phase = Escaped;
                } else if (ch == '"') {
                    break;
                } else {
                    str += ch;
                }
            } else if (phase == Escaped) {
                str += unescaped_char(ch);
                phase = Raw;
            }
        }
        return JSONObject{std::move(str)};
    }

    return JSONObject{std::nullptr_t{}};
}

int main() {
    std::string_view str3 = R"JSON("hello\nsdas")JSON";
    print(std::get<std::string>(parse(str3).inner).c_str());
}
