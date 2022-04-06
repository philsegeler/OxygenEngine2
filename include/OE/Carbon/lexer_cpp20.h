#include <ctre.hpp>
#include <iostream>
#include <optional>
#include <ranges>
#include <string>


namespace csl {


    /*
     *
     * Configuration structs and using declarations
     *
     */


    using lexer_input_it_t = std::string_view::iterator;


    template <auto Type_, ctll::fixed_string Regex_>
    struct token_def_t {
        constexpr static auto token_type = Type_;
        constexpr static auto matcher    = ctre::starts_with<Regex_>;
    };

    template <typename Type_>
    struct token_t {
        Type_            type;
        std::string_view content;

        constexpr bool operator==(const token_t& rhs) const {
            return ((type == rhs.type) && (content == rhs.content));
        }
    };


    /*
     *
     * Template metaprogramming utilities
     *
     */


    template <typename ReturnType, typename First, typename... Rest>
    constexpr inline std::optional<ReturnType> for_each(lexer_input_it_t& it) {
        if (const auto m = First::matcher(it)) {
            std::optional<ReturnType> result = ReturnType{First::token_type, m.to_view()};
            return result;
        }
        else {
            if constexpr (sizeof...(Rest) > 0)
                return for_each<ReturnType, Rest...>(it);
            else {
                return std::nullopt;
            }
        }
    }

    template <typename First_, typename... Rest_>
    struct get_first_pack_type {
        using type = First_;
    };


    template <typename... TokenDefs_>
    struct get_token_defs_token_type {
    private:
        using first_token_def  = typename get_first_pack_type<TokenDefs_...>::type;
        using first_token_type = decltype(first_token_def::token_type);

    public:
        using type = typename std::remove_cv<first_token_type>::type;
    };


    /*
     *
     * class generic_lexer_t
     *
     */


    template <typename... TokenDefs_>
    class generic_lexer_t {
    public:
        class iterator_t {

            using token_t_type = typename get_token_defs_token_type<TokenDefs_...>::type;

        public:
            using iterator_category = std::input_iterator_tag;
            using value_type        = token_t<token_t_type>;
            using difference_type   = std::ptrdiff_t;
            using reference         = token_t<token_t_type>&;
            using pointer           = token_t<token_t_type>*;

            iterator_t() = default;
            iterator_t(const std::string_view& input) : input_it_(input.begin()), end_it_(input.end()) {
            }
            iterator_t(const std::string_view::iterator& input_it, const std::string_view::iterator& end_it)
                : input_it_(input_it), end_it_(end_it) {
            }

            const token_t<token_t_type>* operator->() const {
                return &t_;
            }
            const token_t<token_t_type>& operator*() const {
                return t_;
            }

            iterator_t& operator++() {
                next();
                return *this;
            }
            iterator_t operator++(int) {
                iterator_t tmp(*this);
                ++(*this);
                return tmp;
            }

            bool operator==(const iterator_t& rhs) const {
                return ((t_ == rhs.t_) && (input_it_ == rhs.input_it_));
            }
            bool operator!=(const iterator_t& rhs) const {
                return !((*this) == rhs);
            }

        private:
            token_t<token_t_type>      t_;
            std::string_view::iterator input_it_;
            std::string_view::iterator end_it_;

            void next() {
                if (input_it_ == end_it_) {
                    t_ = token_t<token_t_type>{{}, end_it_};
                    return;
                }

                std::optional<token_t<token_t_type>> opt = for_each<token_t<token_t_type>, TokenDefs_...>(input_it_);
                if (opt) {
                    t_ = *opt;
                    input_it_ += t_.content.size();
                }
                else {
                    throw 1;
                }
            }
        };


        generic_lexer_t(const std::string_view input) : input_(input){};

        iterator_t& begin() {
            return ++iterator_t{input_.begin(), input_.end()};
        }

        iterator_t& end() {
            return ++iterator_t{input_.end(), input_.end()};
        }

    private:
        std::string_view input_;
    };


} // namespace csl
