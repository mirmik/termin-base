#include <fstream>
#include "json.h"

using namespace std::literals::string_literals;

char nos::json::parser::readnext()
{
    symbno++;
    char c = readnext_impl();
    if (c == '\n')
    {
        lineno++;
        symbno = 0;
    }
    return c;
}

std::string nos::json::parser::errloc()
{
    return " lineno:" + std::to_string(lineno) +
           " symbno:" + std::to_string(symbno);
}

char nos::json::parser::readnext_skipping()
{
    char c;

__try__:
    c = readnext();

    if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        goto __try__;

    if (c == '/')
    {
        c = readnext();

        switch (c)
        {
        case '*':
            while (true)
            {
                c = readnext();

                if (c == '*')
                    if ((c = readnext()) == '/')
                    {
                        goto __try__;
                    }
            }

        case '/':

            while (c != '\n')
                c = readnext();

            goto __try__;

        default:
            break;
        }
    }

    return c;
}

void nos::json::parser::append_unicode_escape(trent::string_type &out)
{
    uint32_t codepoint = 0;
    for (int i = 0; i < 4; ++i)
    {
        char c = readnext();
        if (!std::isxdigit(static_cast<unsigned char>(c)))
            throw std::runtime_error("json: invalid unicode escape"s +
                                     errloc());
        codepoint = (codepoint << 4) | hex_to_value(c);
    }

    if (codepoint >= 0xD800 && codepoint <= 0xDBFF)
    {
        char slash = readnext();
        char u = readnext();
        if (slash != '\\' || u != 'u')
            throw std::runtime_error("json: invalid surrogate pair"s +
                                     errloc());
        uint32_t low = 0;
        for (int i = 0; i < 4; ++i)
        {
            char c = readnext();
            if (!std::isxdigit(static_cast<unsigned char>(c)))
                throw std::runtime_error("json: invalid unicode escape"s +
                                         errloc());
            low = (low << 4) | hex_to_value(c);
        }
        if (low < 0xDC00 || low > 0xDFFF)
            throw std::runtime_error("json: invalid surrogate pair"s +
                                     errloc());

        codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
    }

    append_utf8_codepoint(out, codepoint);
}

void nos::json::parser::append_utf8_codepoint(trent::string_type &out,
                                              uint32_t codepoint)
{
    if (codepoint <= 0x7F)
    {
        out.push_back(static_cast<char>(codepoint));
    }
    else if (codepoint <= 0x7FF)
    {
        out.push_back(static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else if (codepoint <= 0xFFFF)
    {
        out.push_back(static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else if (codepoint <= 0x10FFFF)
    {
        out.push_back(static_cast<char>(0xF0 | ((codepoint >> 18) & 0x07)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else
    {
        throw std::runtime_error("json: invalid codepoint");
    }
}

uint8_t nos::json::parser::hex_to_value(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F')
        return 10 + (c - 'A');
    throw std::runtime_error("json: invalid hex digit"s + errloc());
}

nos::trent nos::json::parser::parse()
{
    if (onebuf == 0)
    {
        onebuf = readnext_skipping();
    }

    if (isdigit(onebuf) || onebuf == '-')
        return parse_numer();

    if (onebuf == '"')
        return parse_string();

    if (onebuf == '\'')
        return parse_string();

    if (onebuf == '[')
        return parse_list();

    if (onebuf == '{')
        return parse_dict();

    if (isalpha(onebuf))
        return parse_mnemonic();

    throw std::runtime_error("unexpected_symbol "s + onebuf + errloc());
}

nos::trent nos::json::parser::parse_mnemonic()
{
    char buf[32];
    char *ptr = &buf[1];

    buf[0] = onebuf;

    while (isalpha(onebuf = readnext_skipping()))
    {
        *ptr++ = onebuf;
    }

    *ptr = 0;

    if (strcmp("true", buf) == 0)
    {
        return true;
    }
    if (strcmp("false", buf) == 0)
    {
        return false;
    }
    if (strcmp("nil", buf) == 0 || strcmp("null", buf) == 0)
    {
        return trent();
    }

    throw std::runtime_error("unexpected_mnemonic "s + errloc());
}

nos::trent nos::json::parser::parse_numer()
{
    std::string buf;
    buf.push_back(onebuf);
    bool allow_sign = false;

    while (true)
    {
        char c = readnext();

        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.')
        {
            buf.push_back(c);
            allow_sign = false;
            continue;
        }

        if (c == 'e' || c == 'E')
        {
            buf.push_back(c);
            allow_sign = true;
            continue;
        }

        if ((c == '+' || c == '-') && allow_sign)
        {
            buf.push_back(c);
            allow_sign = false;
            continue;
        }

        onebuf = c;
        break;
    }

    if (std::isspace(static_cast<unsigned char>(onebuf)))
        onebuf = 0;

    return strtod(buf.c_str(), nullptr);
}

nos::trent nos::json::parser::parse_string()
{
    trent::string_type str;

    char delim = onebuf;

    while (true)
    {
        char c = readnext();

        if (c == 0)
            throw std::runtime_error("json: unterminated string"s + errloc());

        if (c == delim)
            break;

        if (c == '\\')
        {
            char esc = readnext();
            switch (esc)
            {
            case '"':
                str.push_back('"');
                break;
            case '\'':
                str.push_back('\'');
                break;
            case '\\':
                str.push_back('\\');
                break;
            case '/':
                str.push_back('/');
                break;
            case 'b':
                str.push_back('\b');
                break;
            case 'f':
                str.push_back('\f');
                break;
            case 'n':
                str.push_back('\n');
                break;
            case 'r':
                str.push_back('\r');
                break;
            case 't':
                str.push_back('\t');
                break;
            case 'u':
                append_unicode_escape(str);
                break;
            default:
                throw std::runtime_error("json: invalid escape sequence"s +
                                         errloc());
            }

            continue;
        }

        if (c == '\n' || c == '\r')
            throw std::runtime_error("json: unexpected newline"s + errloc());

        str.push_back(c);
    }

    onebuf = 0;
    return str;
}

nos::trent nos::json::parser::parse_list()
{
    trent js(trent::type::list);

    onebuf = readnext_skipping();

    if (onebuf == ']')
    {
        onebuf = 0;
        return js;
    }

    while (true)
    {
        trent r = parse();
        js.as_list().push_back(r);

        if (onebuf == 0)
            onebuf = readnext_skipping();

        if (onebuf == ']')
        {
            onebuf = 0;
            return js;
        }

        if (onebuf != ',')
        {
            throw std::runtime_error("json::parse_list: wait_, expect_"s +
                                     onebuf + errloc());
        }

        onebuf = readnext_skipping();
    }
}

nos::trent nos::json::parser::parse_dict()
{
    trent js(trent::type::dict);

    onebuf = readnext_skipping();

    if (onebuf == '}')
    {
        onebuf = 0;
        return js;
    }

    while (true)
    {
        trent key = parse();

        if (!key.is_string())
            throw std::runtime_error("json: object key must be string"s +
                                     errloc());

        onebuf = readnext_skipping();

        if (onebuf != ':')
            throw std::runtime_error("json:parse_dict_0 wait_: expect_"s +
                                     onebuf +
                                     " lineno:" + std::to_string(lineno) +
                                     " symbno:" + std::to_string(symbno));

        onebuf = 0;

        trent value = parse();

        auto keystr = std::move(key.as_string());

        js.as_dict().emplace(std::move(keystr), std::move(value));

        if (onebuf == 0)
            onebuf = readnext_skipping();

        if (onebuf == '}')
        {
            onebuf = 0;
            return js;
        }

        if (onebuf != ',')
        {
            throw std::runtime_error("json:parse_dict_1 wait_: expect_"s +
                                     onebuf +
                                     " lineno:" + std::to_string(lineno) +
                                     " symbno:" + std::to_string(symbno));
        }

        onebuf = readnext_skipping();

        if (onebuf == '}')
        {
            onebuf = 0;
            return js;
        }
    }
}
char nos::json::parser_cstr::readnext_impl()
{
    return *ptr++;
}

char nos::json::parser_str::readnext_impl()
{
    if (index >= storage.size())
        return 0;
    return storage[index++];
}

char nos::json::parser_input_stream::readnext_impl()
{
    return is.get();
}

nos::trent nos::json::parse(const char *str)
{
    parser_cstr parser(str);
    return parser.parse();
}

nos::trent nos::json::parse(const std::string &str)
{
    parser_str parser(str);
    return parser.parse();
}

nos::trent nos::json::parse_file(const std::string &str)
{
    std::ifstream t(str);
    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    std::string buffer(size, ' ');
    t.seekg(0);
    t.read(&buffer[0], size);

    return parse(buffer);
}

// Helper for escaping strings in JSON
static std::string escape_json_string(const std::string& s) {
    std::string result;
    result.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 32) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    result += buf;
                } else {
                    result += c;
                }
                break;
        }
    }
    return result;
}

static void dump_impl(const nos::trent& t, std::string& out, int indent, int current_indent) {
    auto add_indent = [&]() {
        if (indent >= 0) {
            out += '\n';
            out.append(current_indent, ' ');
        }
    };

    switch (t.get_type()) {
        case nos::trent_type::nil:
            out += "null";
            break;

        case nos::trent_type::boolean:
            out += t.as_bool() ? "true" : "false";
            break;

        case nos::trent_type::numer: {
            double val = static_cast<double>(t.as_numer());
            if (val == static_cast<double>(static_cast<int64_t>(val))) {
                out += std::to_string(static_cast<int64_t>(val));
            } else {
                char buf[32];
                snprintf(buf, sizeof(buf), "%.17g", val);
                out += buf;
            }
            break;
        }

        case nos::trent_type::string:
            out += '"';
            out += escape_json_string(t.as_string());
            out += '"';
            break;

        case nos::trent_type::list: {
            out += '[';
            const auto& list = t.as_list();
            bool first = true;
            for (const auto& item : list) {
                if (!first) out += ',';
                first = false;
                if (indent >= 0) {
                    out += '\n';
                    out.append(current_indent + indent, ' ');
                }
                dump_impl(item, out, indent, current_indent + indent);
            }
            if (!list.empty()) {
                add_indent();
            }
            out += ']';
            break;
        }

        case nos::trent_type::dict: {
            out += '{';
            const auto& dict = t.as_dict();
            bool first = true;
            for (const auto& [key, val] : dict) {
                if (!first) out += ',';
                first = false;
                if (indent >= 0) {
                    out += '\n';
                    out.append(current_indent + indent, ' ');
                }
                out += '"';
                out += escape_json_string(key);
                out += '"';
                out += ':';
                if (indent >= 0) out += ' ';
                dump_impl(val, out, indent, current_indent + indent);
            }
            if (!dict.empty()) {
                add_indent();
            }
            out += '}';
            break;
        }
    }
}

std::string nos::json::dump(const nos::trent& t, int indent) {
    std::string result;
    dump_impl(t, result, indent, 0);
    return result;
}
