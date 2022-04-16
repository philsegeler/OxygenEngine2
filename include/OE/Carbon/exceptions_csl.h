#ifndef CSL_EXCEPTIONS_H_
#define CSL_EXCEPTIONS_H_

#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>


namespace csl {


    /*
     *
     * Lexer Errors
     *
     */


    class lexer_error_t {
    public:
        virtual std::string what() const throw() = 0;

        virtual ~lexer_error_t() = default;

        std::string name_;
    };

    class unexpected_symbol_error_t : public lexer_error_t {
    public:
        unexpected_symbol_error_t(char unexpected, std::size_t line_num, std::size_t col_num)
                : unexpected_(unexpected), line_num_(line_num), col_num_(col_num) {
            name_ = "csl::unexpected_token_error_t";
        };


        std::string what() const throw() {
            std::stringstream result_ss;

            result_ss << "Unexpected Symbol \"" << unexpected_ << "\" (Decimal: " << static_cast<int>(unexpected_) << ") ";
            result_ss << "at line " << line_num_ << ':' << col_num_ << ": ";

            return result_ss.str();
        }

    private:
        char unexpected_;

        const std::size_t line_num_;
        const std::size_t col_num_;
    };


    /*
     *
     * Parser Errors
     *
     */


    // Sadly, ParseError cannot inherit from std::exception, since that would not allow the
    // return type to be a string, which is necessary in this case in order to get a proper
    // error message (Returning "result_ss.str().c_str()" doesn't work, since it returns a
    // pointer to an object that gets destroyed a soon as the scope of "what()" is left,
    // thereby basically returning garbage)
    class parser_error_t {
    public:
        virtual std::string what() const throw() = 0;

        virtual ~parser_error_t() = default;

        std::string name_;
    };


    class unexpected_token_error_t : public parser_error_t {
    public:
        unexpected_token_error_t(std::string unexpected, std::string expected, std::size_t line_num, std::size_t col_num)
            : unexpected_(unexpected), expected_(expected), line_num_(line_num), col_num_(col_num) {
            name_ = "csl::unexpected_token_error_t";
        };


        std::string what() const throw() {
            std::stringstream result_ss;

            result_ss << "Unexpetcted Token \"" << unexpected_ << "\" ";
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


    /*
     *
     * Interpreter Errors
     *
     */


    class interpreter_error_t {
    public:
        virtual std::string what() const throw() = 0;

        virtual ~interpreter_error_t() = default;

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
    class semantic_error_t : public interpreter_error_t {
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

} // namespace csl


#endif /* EXCEPTIONS_H_ */
