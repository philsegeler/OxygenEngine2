#include <ctre.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>


namespace csl {


    /*
     *
     * Configuration structs and using declarations
     *
     */


    using lexer_input_it_t = std::string_view::iterator;


    // The predefined token types are implemented with a class instead of an enum,
    // in order to be able to inherit from the predefined types when defining the application-specific types
    class token_type_predef_t {
    public:
        static const unsigned eoi   = 0;
        static const unsigned undef = 1;
        static const unsigned skip  = 2;
    };

    template <typename T, typename U>
    struct token_def_t {
        unsigned type = token_type_predef_t::undef;

        const T condition;
        const U action;
    };

    // Needed for compilation with clang
    template <typename T, typename U>
    token_def_t(int, T, U) -> token_def_t<T, U>;


    struct token_t {
        unsigned     type = token_type_predef_t::undef;
        std::string_view content;
    };


    /*
     *
     * Template metaprogramming utilities
     *
     */


    template <typename... T>
    inline void for_each(unsigned& type, lexer_input_it_t& it, const lexer_input_it_t& end_it, T... cws);

    template <typename T, typename... U>
    inline void for_each(unsigned& type, lexer_input_it_t& it, const lexer_input_it_t& end_it, T cw, U... cws) {
        if (cw.condition(it, end_it)) {
            type = cw.type;
            it++;
            cw.action(it, end_it);
        }
        else {
            for_each(type, it, end_it, cws...);
        }
    }

    template <>
    inline void for_each(unsigned& type, lexer_input_it_t& it, const lexer_input_it_t& end_it) {
    }


    /*
     *
     * class generic_lexer_t
     *
     */


    template <token_def_t... TokenDefs_>
    class generic_lexer_t {
    public:
        class iterator_t {
        public:
            // TODO: Make this forward at some point
            using iterator_category = std::input_iterator_tag;
            using value_type        = token_t;
            using difference_type   = std::size_t;
            using reference         = token_t&;
            using pointer           = token_t*;

            iterator_t(generic_lexer_t<TokenDefs_...>* lexer) : lexer_(lexer) {
                t_ = lexer_->next_token();
            }
            iterator_t(token_t t) : t_(t){};

            const token_t* operator->() const {
                return &t_;
            }
            const token_t& operator*() const {
                return t_;
            }

            iterator_t& operator++() {
                t_ = lexer_->next_token();
                return *this;
            }
            iterator_t operator++(int) {
                iterator_t tmp(*this);
                ++(*this);
                return tmp;
            }

            bool operator==(const iterator_t& rhs) {
                return (this == &rhs) || ((t_.type == token_type_predef_t::eoi) && ((*rhs).type == token_type_predef_t::eoi));
            }
            bool operator!=(const iterator_t& rhs) {
                return !((*this) == rhs);
            }

        private:
            generic_lexer_t<TokenDefs_...>* lexer_;
            token_t                         t_;
        };


        generic_lexer_t(std::string& input) : input_(input), input_it_(input_.begin()), end_it_(input_.end()){};

        token_t next_token() {
            if (input_it_ == end_it_) return {token_type_predef_t::eoi, std::string_view(*&input_it_, 0)};

            next_token_type_ = token_type_predef_t::undef;

            auto temp = input_it_;
            for_each(next_token_type_, input_it_, end_it_, TokenDefs_...);

            if (next_token_type_ == token_type_predef_t::undef) {
                input_it_++;
            }
            else if (next_token_type_ == token_type_predef_t::skip) {
                return next_token();
            }


            return {next_token_type_, std::string_view(*&temp, input_it_ - temp)};
        }

        iterator_t begin() {
            return iterator_t(this);
        }

        iterator_t end() {
            return iterator_t({token_type_predef_t::eoi, std::string_view(*&end_it_, 0)});
        }

        std::size_t get_line_num() const {
            std::size_t result = 1;

            auto it = input_.begin();
            while (it != input_it_) {
                if (*it == '\n') result++;
                it++;
            }

            return result;
        }

        std::size_t get_col_num() const {
            std::size_t result = 1;

            auto it = input_it_;

            while ((*it != '\n') && (it != input_.begin())) {
                result++;
                it--;
            }

            return result;
        }

    private:
        const std::string_view input_;
        lexer_input_it_t       input_it_;
        const lexer_input_it_t end_it_;

        unsigned next_token_type_ = token_type_predef_t::undef;
    };


} // namespace csl
