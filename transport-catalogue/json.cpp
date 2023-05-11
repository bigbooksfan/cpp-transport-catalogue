#include "json.h"

using namespace std;

namespace json {
    namespace {
        Node LoadNode(istream& input);
        Node LoadNumber(std::istream& input) {
            std::string parsed_num;
            // reads the next character from input into parsed_num
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // reads one or more digits into parsed_num from input
            auto read_digits = [&input, read_char] {
                if (!isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // parse the integer part of a number
            if (input.peek() == '0') {
                read_char();
                // after 0 in JSON, no other numbers can go
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // parse the fractional part of a number
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // parse the exponential part of a number
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    return Node(stoi(parsed_num));
                }
                return Node(stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // did the stream end before the closing quote was encountered?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // encountered a closing quote
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // met the start of an escape sequence
                    ++it;
                    if (it == end) {
                        // the thread ended immediately after the backslash character
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // processing of one of the sequences: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // encountered an unknown escape sequence
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // string literal inside JSON cannot be interrupted by sequences \r or \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // read the next character and put it in the resulting string
                    s.push_back(ch);
                }
                ++it;
            }

            return move(Node(move(s)));
        }

        Node LoadNull(istream& input) {
            string s;
            char c;
            for (int i = 0; input >> c && i < 4; ++i) {
                s += c;
            }
            if (s != "null"s) {
                throw ParsingError("Unexpected value"s);
            }
            return move(Node());
        }

        Node LoadBool(istream& input) {
            string s;
            char c;

            for (; input >> c && s.size() < 5 && c != 'e'; s += c);
            s += c;

            if (s == "true"s) {
                return move(Node(true));
            }
            if (s == "false"s) {
                return Node(false);
            }
            throw ParsingError("Unexpected value"s);
        }

        Node LoadArray(istream& input) {
            Array result;
            char c;
            for (; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(move(LoadNode(input)));
            }

            if (c != ']') {
                throw ParsingError("] not expected"s);
            }

            return move(Node(move(result)));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c;
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            if (c != '}') {
                throw ParsingError("} not expected"s);
            }

            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            if (input >> c) {
                if (c == '[') {
                    return move(LoadArray(input));
                }
                else if (c == '{') {
                    return move(LoadDict(input));
                }
                else if (c == '"') {
                    return move(LoadString(input));
                }
                else if (c == 'n') {
                    input.putback(c);
                    return LoadNull(input);
                }
                else if (c == 't' || c == 'f') {
                    input.putback(c);
                    return LoadBool(input);
                }
                else {
                    input.putback(c);
                    return LoadNumber(input);
                }
            }
            else return {};
        }

    }  // namespace

    Node::Node(bool value)
        : data_(value) { }

    Node::Node(const int value)
        : data_(value) { }

    Node::Node(const double value)
        : data_(value) { }

    Node::Node(const string value)
        : data_(move(value)) { }

    Node::Node(const Array value)
        : data_(move(value)) { }

    Node::Node(const Dict map)
        : data_(move(map)) { }

    bool Node::AsBool() {
        if (!IsBool()) {
            throw logic_error("Is not Bool"s);
        }
        return get<bool>(data_);
    }

    int Node::AsInt() {
        if (!IsInt()) {
            throw logic_error("Is not Int"s);
        }
        return get<int>(data_);
    }

    double Node::AsDouble() {
        if (holds_alternative<double>(data_)) {
            return get<double>(data_);
        }
        if (IsInt()) {
            return static_cast<double>(get<int>(data_));
        }
        throw logic_error("Is not Double"s);
    }

    string& Node::AsString() {
        if (!IsString()) {
            throw logic_error("Is not String"s);
        }
        return get<string>(data_);
    }

    Array& Node::AsArray() {
        if (!IsArray()) {
            throw logic_error("Is not Array"s);
        }
        return get<Array>(data_);
    }

    Dict& Node::AsMap() {
        if (!IsMap()) {
            throw logic_error("Is not Map"s);
        }
        return get<Dict>(data_);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    Document& Document::operator= (Document& other) {
        root_ = other.root_;
        return *this;
    }

    Document& Document::operator= (Node& other) {
        root_ = other;
        return *this;
    }

    Document& Document::operator= (Document&& other) {
        root_ = move(other.root_);
        return *this;
    }

    Document& Document::operator= (Node&& other) {
        root_ = move(other);
        return *this;
    }

    Node& Document::GetRoot() {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintNode(const Node& node, const PrintContext& ctx, bool enter = true);

    void PrintValue(nullptr_t, const PrintContext& ctx, bool) {
        ctx.out << "null"sv;
    }

    void PrintValue(const string& value, const  PrintContext& ctx, bool) {
        ctx.out << '"';
        for (char c : value) {
            switch (c) {
            case '"':
                ctx.out << "\\\""sv;
                break;
            case '\n':
                ctx.out << "\\n"sv;
                break;
            case '\t':
                ctx.out << "\t"sv;
                break;
            case '\r':
                ctx.out << "\\r"sv;
                break;
            case '\\':
                ctx.out << "\\\\"sv;
                break;
            default:
                ctx.out << c;
                break;
            }
        }
        ctx.out << '"';
    }

    template <typename Number>
    void PrintValue(const Number value, const PrintContext& ctx, bool) {
        ctx.out << boolalpha;
        ctx.out << value;
    }

    void PrintValue(const Array& value, const PrintContext& ctx, bool enter = true) {
        auto print_elems = [&value](const PrintContext& ctx) {
            bool is_first = false;
            for (const auto& elem : value) {
                if (is_first) {
                    ctx.out << ",\n"sv;
                }
                is_first = true;
                ctx.PrintIndent();
                PrintNode(elem, ctx, false);
            }
        };

        if (enter) {
            ctx.PrintIndent();
        }
        ctx.out << "[\n"sv;

        print_elems(ctx.Indented());

        ctx.out << '\n';
        ctx.PrintIndent();
        ctx.out << "]"sv;
    }

    void PrintValue(const Dict& value, const PrintContext& ctx, bool enter = true) {
        auto print_elems = [&value](const PrintContext& ctx) {
            bool is_first = false;
            for (const auto& elem : value) {
                if (is_first) {
                    ctx.out << ",\n"sv;
                }
                is_first = true;
                ctx.PrintIndent();
                ctx.out << "\""sv << elem.first << "\": "sv;
                PrintNode(elem.second, ctx, false);
            }
        };
        if (enter) {
            ctx.PrintIndent();
        }

        ctx.out << "{\n"sv;

        print_elems(ctx.Indented());

        ctx.out << '\n';
        ctx.PrintIndent();
        ctx.out << "}"sv;
    }

    void PrintNode(const Node& node, const PrintContext& ctx, bool enter) {
        visit([&ctx, enter](const auto& value) {PrintValue(value, ctx, enter); }, node.GetValue());

    }

    void Print(Document& doc, std::ostream& out) {
        PrintContext ctx({ out, TAB, 0 });
        PrintNode(doc.GetRoot(), ctx);
        out << endl;
    }
}  // namespace json