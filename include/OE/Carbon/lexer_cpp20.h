#include <ctre.hpp>
#include <string_view>


namespace csl {


    /*
     *
     * Configuration structs and using declarations
     *
     */


    template <auto Type_, ctll::fixed_string Regex_>
    struct token_def_t {
        constexpr static auto token_type = Type_;
        constexpr static auto matcher    = ctre::starts_with<Regex_>;
    };

    template <typename Type_>
    struct token_t {
        Type_            type;
        std::string_view content;
    };


    /*
     *
     * Template metaprogramming utilities
     *
     */


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


    template <typename First, typename... Rest>
    constexpr inline auto get_matched_token(std::string_view::iterator& it) {
        using token_type = typename get_token_defs_token_type<First>::type;

        if (const auto m = First::matcher(it)) {
            return token_t<token_type>{First::token_type, m.to_view()};
        }
        else {
            if constexpr (sizeof...(Rest) > 0)
                return get_matched_token<Rest...>(it);
            else
                throw 1;
        }
    }

    /*
     *
     * class generic_lexer_t
     *
     */


    template <typename... TokenDefs_>
    class generic_lexer_t {

        using token_type = typename get_token_defs_token_type<TokenDefs_...>::type;

    public:
        class iterator_t {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type        = token_t<token_type>;
            using difference_type   = std::ptrdiff_t;
            using reference         = token_t<token_type>&;
            using pointer           = token_t<token_type>*;

            constexpr iterator_t() = default;
            constexpr iterator_t(generic_lexer_t* lexer) : lexer_(lexer) {
            }

            constexpr const token_t<token_type>* operator->() const {
                return &lexer_->get_token();
            }
            constexpr const token_t<token_type>& operator*() const {
                return lexer_->get_token();
            }

            constexpr iterator_t& operator++() {
                lexer_->next();
                return *this;
            }
            constexpr iterator_t operator++(int) {
                iterator_t tmp(*this);
                ++(*this);
                return tmp;
            }

            constexpr bool operator==(const iterator_t& rhs) const {
                return (at_end() && rhs.at_end());
            }
            constexpr bool operator!=(const iterator_t& rhs) const {
                return !(*this == rhs);
            }

        private:
            generic_lexer_t* lexer_ = nullptr;

            constexpr bool at_end() const {
                return ((lexer_ == nullptr) || lexer_->at_end());
            }
        };


        constexpr generic_lexer_t(const std::string_view input) : input_it_(input.begin()), end_it_{input.end()} {};

        constexpr void next() {
            if (input_it_ == end_it_) {
                t_ = token_t<token_type>{{}, end_it_};
                return;
            }

            t_ = get_matched_token<TokenDefs_...>(input_it_);
            input_it_ += t_.content.size();
        }

        constexpr token_t<token_type>& get_token() {
            return t_;
        }

        constexpr iterator_t begin() {
            return ++iterator_t{this};
        }
        constexpr iterator_t end() {
            return iterator_t{nullptr};
        }
        constexpr iterator_t begin() const {
            return ++iterator_t{this};
        }
        constexpr iterator_t end() const {
            return iterator_t{nullptr};
        }

        constexpr bool at_end() const {
            return (input_it_ == end_it_);
        }

    private:
        token_t<token_type>        t_;
        std::string_view::iterator input_it_;
        std::string_view::iterator end_it_;
    };


} // namespace csl
