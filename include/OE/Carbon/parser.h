#ifndef CSL_PARSER_H_
#define CSL_PARSER_H_


#if (__cplusplus > 201703L)
#define CPP20
#endif


#include <map>
#include <ranges>
#include <sstream>
#include <variant>
#include <vector>
#include <iostream>

#include <OE/Carbon/exceptions_csl.h>
#include <OE/Carbon/lexer.h>


// clang-format off

/* TODO: Fix this grammar. It is not the exact grammar the parser recognizes
 *
 * -------------------------- Grammar ------------------------
 *
 * S                    = element
 *
 * element              = openTag (elementContent)* closeTag	// special requirement: first
 * 																// identifiers of openTag and
 * 																// closeTag must be identical
 *
 * elementContent       = element | genericAssignment
 *
 * openTag              = "<" IDENTIFIER generic* ">"
 * closeTag             = "</" IDENTIFIER ">"
 *
 * genericAssignment    = IDENTIFIER "=" signleAssignmentTail
 *                      | IDENTIFIER "=" listAssignmentTail
 *
 * singleAssignmentTail = VALUE
 *
 * listAssignmentTail   = "{" NUMBER "}"
 *                      | "{" NUMBER (";" NUMBER)* "}"
 *
 *
 * value                = NUMBER | STRING
 *
 */

// clang-format on


namespace csl {

    /*
     *
     * Lexer configuration
     *
     */


    enum class token_type {
        lt,
        gt,
        open_brace,
        close_brace,
        eq,
        semicolon,
        comma,
        slash,
        whitespace,
        lt_slash,
        string,
        identifier,
        integer,
        floating_point
    };


    using lt_def             = token_def_t<token_type::lt, "<">;
    using gt_def             = token_def_t<token_type::gt, ">">;
    using open_brace_def     = token_def_t<token_type::open_brace, "\\{">;
    using close_brace_def    = token_def_t<token_type::close_brace, "\\}">;
    using eq_def             = token_def_t<token_type::eq, "=">;
    using semicolon_def      = token_def_t<token_type::semicolon, ";">;
    using comma_def          = token_def_t<token_type::comma, ",">;
    using slash_def          = token_def_t<token_type::slash, "/">;
    using whitespace_def     = token_def_t<token_type::whitespace, "[ \\t\\n]">;
    using lt_slash_def       = token_def_t<token_type::lt_slash, "</">;
    using string_def         = token_def_t<token_type::string, "\"(.*?)\"">;
    using identifier_def     = token_def_t<token_type::identifier, "[\\w][\\w\\d]*">;
    using integer_def        = token_def_t<token_type::integer, "[\\-]?[\\d]+">;
    using floating_point_def = token_def_t<token_type::floating_point, "[\\-]?[\\d]+\\.[\\d]+">;


    using lexer_t =
        generic_lexer_t<whitespace_def, floating_point_def, integer_def, semicolon_def, eq_def, open_brace_def, close_brace_def,
                        identifier_def, lt_slash_def, lt_def, gt_def, slash_def, string_def, comma_def>;


    /*
     *
     * Error handling
     *
     */


    inline std::string get_token_type_string_rep(token_type t) {
        switch (t) {
        case token_type::identifier:
            return "Identifier, ";
            break;
        case token_type::string:
            return "String, ";
            break;
        case token_type::integer:
            return "Integer, ";
            break;
        case token_type::floating_point:
            return "Float, ";
            break;
        case token_type::lt:
            return "'<', ";
            break;
        case token_type::gt:
            return "'>', ";
            break;
        case token_type::lt_slash:
            return "\"</\", ";
            break;
        case token_type::open_brace:
            return "'{', ";
            break;
        case token_type::close_brace:
            return "'}', ";
            break;
        case token_type::eq:
            return "'=', ";
            break;
        case token_type::comma:
            return "',', ";
            break;
        case token_type::semicolon:
            return "';', ";
            break;
        case token_type::slash:
            return "'/', ";
            break;
        default:
            return "[Unknown Type]";
        }
    }

    // Sadly, ParseError cannon inherit from std::exception, since that would not allow the
    // return type to be a string, which is necessary in this case in order to get a proper
    // error message (Returning "result_ss.str().c_str()" doesn't work, since it returns a
    // pointer to an object that gets destroyed a soon as the scope of "what()" is left,
    // thereby basically returning garbage)
    class parser_error_t {
    public:
        virtual std::string what() const throw() = 0;
        virtual ~parser_error_t()                = default;

        std::string name_;
    };

    class unexpected_symbol_error_t : public parser_error_t {
    public:
        unexpected_symbol_error_t(std::string unexpected, std::string expected, std::size_t line_num, std::size_t col_num)
            : unexpected_(unexpected), expected_(expected), line_num_(line_num), col_num_(col_num) {
            name_ = "csl::unexpected_symbol_error_t";
        };


        std::string what() const throw() {
            std::stringstream result_ss;

            result_ss << "Unexpetcted Symbol \"" << unexpected_ << "\" ";
            result_ss << "at line " << line_num_ << ':' << col_num_ << ": ";
            result_ss << "Expected " << expected_;

            return result_ss.str();
        }

    private:
        std::string unexpected_;
        std::string expected_;

        const std::size_t line_num_;
        const std::size_t col_num_;
    };

    // TODO: Put this in a more logical place
    class interpreter_error_t {
    public:
        virtual std::string what() const throw() = 0;
        virtual ~interpreter_error_t()           = default;
        std::string name_;
    };

    class unset_object_error_t : public interpreter_error_t {
    public:
        unset_object_error_t(std::string_view object) : object_(object) {
            name_ = "csl::unset_object_error_t";
        }


        std::string what() const throw() {
            std::stringstream result_ss;

            result_ss << "Object \"" << object_ << "\" is not contained in the";
            result_ss << " syntax tree obtained by the parser";

            return result_ss.str();
        }

    private:
        std::string_view object_;
    };

    // TODO: Somehow show line and col number
    // Choose ONE of the two to inherit from... this breaks error handling otherwise
    // class semantic_error_t : parser_error_t, interpreter_error_t {
    class semantic_error_t : public parser_error_t {
    public:
        semantic_error_t(const char* msg) : msg_(msg) {
            name_ = "csl::semantic_error_t";
        }

        std::string what() const throw() {
            return msg_;
        }

    private:
        const char* msg_;
    };


    /*
     *
     * parser_t data structures
     *
     */


    template <typename T, typename U>
    class csl_map_t {
    public:
        U& operator[](const T& t) {
            return map_[t];
        }

        const U& at(const T& t) const {
            try {
                return map_.at(t);
            } catch (std::out_of_range& e) {
                throw unset_object_error_t(t);
            }
        }

        bool contains(const T& t) const {
            return (map_.count(t) > 0);
        }

    private:
        std::map<T, U> map_;
    };


    template <typename T>
    using parser_map_t = csl_map_t<std::string_view, T>;

    using single_assignment_t = std::string_view;
    using list_assignment_t   = std::vector<std::string_view>;

    using generic_assignment_t = std::variant<single_assignment_t, list_assignment_t>;

    struct element {
        parser_map_t<std::vector<element>> elements;

        parser_map_t<single_assignment_t> attributes;

        parser_map_t<single_assignment_t> single_assignments;
        parser_map_t<list_assignment_t>   list_assignments;
    };


    /*
     *
     * class parser_t
     *
     */


    // TODO: See if this can be implemented with a FSM
    // TODO: How does performance change if the values aren't returned but set with a reference
    // that was passed in as an argument?
    template <typename TokenIteratorType_>
    class parser_t {
    public:
        parser_t(TokenIteratorType_ begin, TokenIteratorType_ end) : token_it_{begin}, end_it_{end} {
        }

        element parse() {
            expect_token(token_type::lt);
            ++token_it_;

            return parse_element();
        }

    private:
        TokenIteratorType_ token_it_;
        TokenIteratorType_ end_it_;

        element parse_element() {
            element result;


            // Parse opening tag


            expect_token(token_type::identifier);

            std::string_view el_name = (*token_it_).content;
            ++token_it_;

            while (has_type(token_type::identifier)) {
                std::string_view att_name = (*token_it_).content;
                ++token_it_;

                expect_token(token_type::eq);
                ++token_it_;

                // TODO: Is emplace_back (or the equivalent for maps) applicable here?
                result.attributes[att_name] = parse_single_assignment();
            }

            expect_token(token_type::gt);
            ++token_it_;


            // Parse element content


            while (token_it_ != end_it_) {
                if (has_type(token_type::identifier)) {
                    std::string_view as_name = (*token_it_).content;

                    ++token_it_;

                    expect_token(token_type::eq);
                    ++token_it_;

                    generic_assignment_t as_generic = parse_assignment();

                    // TODO: Is access by index more efficient? This option weirdly seems to
                    // not be documented as well
                    // TODO: Is emplace_back (or the equivalent for maps) applicable here?
                    if (std::holds_alternative<single_assignment_t>(as_generic)) {
                        result.single_assignments[as_name] = std::get<single_assignment_t>(as_generic);
                    }
                    else if (std::holds_alternative<list_assignment_t>(as_generic)) {
                        result.list_assignments[as_name] = std::get<list_assignment_t>(as_generic);
                    }
                }
                else if (has_type(token_type::lt)) {
                    ++token_it_;
                    std::string_view sub_el_name = (*token_it_).content;
                    result.elements[sub_el_name].push_back(parse_element());
                }
                else {
                    break;
                }
            }


            // Parse closing tag


            expect_token(token_type::identifier, token_type::lt, token_type::lt_slash);
            ++token_it_;

            expect_token(token_type::identifier);

            if ((*token_it_).content != el_name)
                throw semantic_error_t("Closing tag identifier does not match opening tag"
                                       " identifier");

            ++token_it_;

            expect_token(token_type::gt);
            ++token_it_;

            return result;
        }

        // TODO: Get rid of RTTI use
        generic_assignment_t parse_assignment() {
            // TODO: Check where to put this in order to get better performance
            expect_token(token_type::integer, token_type::floating_point, token_type::identifier, token_type::open_brace,
                         token_type::string);

            if (has_type(token_type::open_brace)) {
                ++token_it_;
                return parse_list_assignment();
            }
            else {
                return parse_single_assignment();
            }
        }

        single_assignment_t parse_single_assignment() {
            single_assignment_t value = (*token_it_).content;
            ++token_it_;

            return value;
        }

        list_assignment_t parse_list_assignment() {
            list_assignment_t result;

            if (is_value()) {
                // TODO: Try emplace_back
                result.push_back((*token_it_).content);
                ++token_it_;

                while (has_type(token_type::semicolon)) {
                    ++token_it_;
                    expect_value();

                    // TODO: Try emplace_back
                    result.push_back((*token_it_).content);
                    ++token_it_;
                }
            }

            // TODO: Not only '}' is expected
            expect_token(token_type::close_brace);
            ++token_it_;

            return result;
        }

        template <typename... T>
        void expect_token(T... type) const {
            bool b = (has_type(type) || ...);

            if (!b) {
                // TODO: Test performance, if const char* is used here instead of std::string
                // TODO: Regardles, make all of this evaluate at compile-time
                std::string expected = (get_token_type_string_rep(type) + ...);
                expected.erase(expected.size() - 2, 2);
                std::string unexpected((*token_it_).content);

                // TODO
                throw unexpected_symbol_error_t(unexpected, expected, 0, 0);
                //                throw unexpected_symbol_error_t(unexpected, expected, lexer_.get_line_num(),
                //                                                lexer_.get_col_num() - (*token_it_).content.size());
            }
        }

        // TODO: Rename to match_type? Check literature
        bool has_type(token_type type) const {
            return ((*token_it_).type == type);
        }

        bool is_value() {
            return (has_type(token_type::integer) || has_type(token_type::floating_point) || has_type(token_type::identifier) ||
                    has_type(token_type::string));
        }

        void expect_value() {
            expect_token(token_type::integer, token_type::floating_point, token_type::identifier, token_type::string);
        }
    };

    // TODO: Forwarding reference?
    inline element parse(std::string& input) {
        lexer_t lexer(input);

        constexpr auto is_not_whitespace    = [](auto token) { return (token.type != token_type::whitespace); };
        constexpr auto remove_string_quotes = [](auto token) -> decltype(token) {
            if (token.type == token_type::string)
                return {token_type::string, token.content.substr(1, token.content.size() - 2)};
            else
                return token;
        };

        auto token_range = lexer | std::views::filter(is_not_whitespace);
//                                 | std::views::transform(remove_string_quotes);

        parser_t parser(token_range.begin(), token_range.end());
        return parser.parse();
    }
} // namespace csl


#endif
