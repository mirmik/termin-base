#ifndef nos_TRENT_JSON_H
#define nos_TRENT_JSON_H

/**
    @file
*/

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iosfwd>
#include "trent.h"
#include <stdexcept>
#include <string>

namespace nos
{
    namespace json
    {
        class parser
        {
        protected:
            int symbno = 0;
            int lineno = 1;
            char onebuf = 0;

        public:
            virtual ~parser() = default;
            virtual char readnext_impl() = 0;

            char readnext();
            std::string errloc();
            char readnext_skipping();

            trent parse();

        protected:
            trent parse_mnemonic();
            trent parse_numer();
            trent parse_string();
            trent parse_list();
            trent parse_dict();

            void append_unicode_escape(trent::string_type &out);
            static void append_utf8_codepoint(trent::string_type &out,
                                              uint32_t codepoint);
            uint8_t hex_to_value(char c);
        };

        class parser_cstr : public parser
        {
            const char *ptr = nullptr;

        public:
            explicit parser_cstr(const char *str) : ptr(str) {}
            parser_cstr(const parser_cstr &) = default;
            parser_cstr &operator=(const parser_cstr &) = default;

            char readnext_impl() override;
        };

        class parser_str : public parser
        {
            std::string storage = {};
            size_t index = 0;

        public:
            explicit parser_str(const std::string &str) : storage(str) {}
            explicit parser_str(std::string &&str) : storage(std::move(str)) {}
            parser_str(const parser_str &) = default;
            parser_str &operator=(const parser_str &) = default;

            char readnext_impl() override;
        };

        class parser_input_stream : public parser
        {
            std::istream &is;

        public:
            explicit parser_input_stream(std::istream &is) : is(is) {}

            char readnext_impl() override;
        };

        nos::trent parse(const char *str);
        nos::trent parse(const std::string &str);
        nos::trent parse_file(const std::string &path);

        // Serialize trent to JSON string
        std::string dump(const nos::trent &t, int indent = -1);
    }
}

#endif
