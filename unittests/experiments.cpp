#include <array>
#include <string>
#include <unordered_set>
#include <vector>

#include "ioutils.hpp"
#include "sajson.h"

void parse(char *input_text) {
    const size_t AST_BUFFER_SIZE = 500;
    size_t ast_buffer[AST_BUFFER_SIZE];

    const sajson::document &doc = parse(
        // The bounded allocation mode attempts to fit the AST into the given
        // fixed-size buffer, returning ERROR_OUT_OF_MEMORY if it is not large
        // enough to parse this document.
        sajson::bounded_allocation(ast_buffer, AST_BUFFER_SIZE),
        // If sajson is asked to parse a mutable_string_view, it will not
        // allocate a copy and instead parse in-place.
        sajson::mutable_string_view(strlen(input_text), input_text));

    if (doc.is_valid()) {
        puts("success!");
    } else {
        printf("parse failed: %s\n", doc.get_error_message_as_cstring());
    }

    // Inspect the JSON data
    using namespace sajson;
    const auto node = doc.get_root();
    fmt::print("Type: {}\n", node.get_type());

    switch (node.get_type()) {
    case TYPE_NULL:
        break;

    case TYPE_FALSE:

        break;

    case TYPE_TRUE:

        break;

    case TYPE_ARRAY: {
        fmt::print("Found {}\n", "Array");

        break;
    }

    case TYPE_OBJECT: {
        fmt::print("Found {}\n", "Object");
        auto length = node.get_length();
        fmt::print("length: {}\n", length);
        const sajson::string level("LEVEL", 5);
        const sajson::string message("MESSAGE", 7);
        const sajson::string prefix("PREFIX", 6);
        const sajson::string raw_error("PREFIX", 6);

        fmt::print("{0}: {1}\n", level.data(), node.get_value_of_key(level).as_cstring());
        fmt::print("{0}: {1}\n", prefix.data(), node.get_value_of_key(prefix).as_cstring());
        fmt::print("{0}: {1}\n", message.data(), node.get_value_of_key(message).as_cstring());

        break;
    }

    case TYPE_STRING:
        fmt::print("Found {}\n", "String");
        break;
    case TYPE_DOUBLE:
    case TYPE_INTEGER:
        break;

    default:
        assert(false && "unknown node type");
    }
}

struct JsonExplorer {
  public:
    bool operator()(char *buffer) {
        const size_t AST_BUFFER_SIZE = 500;
        size_t ast_buffer[AST_BUFFER_SIZE];
        const sajson::document &doc = parse(sajson::bounded_allocation(ast_buffer, AST_BUFFER_SIZE),
                                            sajson::mutable_string_view(strlen(buffer), buffer));

        if (!doc.is_valid()) {
            printf("parse failed: %s\n", doc.get_error_message_as_cstring());
            return false;
        }

        // Traverse JSON data struture.
        traverse(doc.get_root());



        return true;
    }

    void traverse(const sajson::value &node) {
        using namespace sajson;
        switch (node.get_type()) {
        case TYPE_OBJECT: {
            auto length = node.get_length();
            for (auto i = 0u; i < length; ++i) {
                auto subnode = node.get_object_value(i);
                keys.emplace(std::string(node.get_object_key(i).data()));
                traverse(subnode);
            }
            break;
        }
        case TYPE_ARRAY: {
            auto length = node.get_length();
            for (size_t i = 0; i < length; ++i) { traverse(node.get_array_element(i)); }
            break;
        }
        case TYPE_NULL:
            break;

        case TYPE_FALSE:
            break;

        case TYPE_TRUE:
            break;

        case TYPE_STRING:
            break;
        case TYPE_DOUBLE:
            break;
        case TYPE_INTEGER:
            break;
        default:
            assert(false && "unknown node type");
        }
    }

	void print() const {
		fmt::print("Keys: ");
        // Display results;
        std::for_each(keys.cbegin(), keys.cend(), [](auto item) { fmt::print("{0} ", item); });
        fmt::print("\n");		
	}
	
  private:
    std::unordered_set<std::string> keys;
    static constexpr size_t AST_BUFFER_SIZE = 16384;
    std::array<size_t, AST_BUFFER_SIZE> ast_buffer;
};

int main(int argc, char *argv[]) {
    std::string buffer = ioutils::read<std::string>(argv[1]);
    fmt::print("buffer: {}\n", buffer);
    JsonExplorer explorer;
    explorer(&buffer[0]);
	explorer.print();
    return EXIT_SUCCESS;
}
