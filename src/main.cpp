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

JSONObject parse(std::string_view json) {
    if (json.empty()) {
        return JSONObject{std::nullptr_t{}};
    }

    if ('0' <= json[0] && json[0] <= '9' || json[0] == '+' || json[0] == '-') {
        std::regex num_re{"[+-]?[0-9]+(\\.[0-9]*)?([eE][+-]?[0-9]+)?"};
    }

    return JSONObject{std::nullptr_t{}};
}

int main() {
    std::string_view str3 = "12";
    print(parse(str3));
}
