#ifndef CALCULATOR_H
#define CAULCULATOR_H

#include <string>
#include <list>
#include <vector>

class Calculator
{
    private:
        const std::vector<std::string> valid_symbols{"*", "/", "+", "-", "(", ")", "."};

        std::list<std::string> tokens;
        std::string currentToken;

        std::string clean_input(std::string);
        void tokenize_input(const std::string);
        void validate_tokens();
        void increment_token();

        bool is_valid_symbol(std::string);

        double expression();
        double multiply_divide();
        double add_subtract();

    public:
        double parse(std::string);
};

#endif