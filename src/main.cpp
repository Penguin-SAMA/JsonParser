#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

struct JSONObject {
    std::variant<
        std::monostate,                             // null
        bool,                                       // true
        int,                                        // 42
        double,                                     // 3.14
        std::string,                                // "hello"
        std::vector<JSONObject>,                    // [42, "hello"]
        std::unordered_map<std::string, JSONObject> // {"hello": 985, "wrold":211}
        >
        inner;
};
