#ifndef CALCULATOR_H
#define CAULCULATOR_H

#include <string>
#include <list>
#include <vector>

class Calculator
{
    private:
        const std::vector<char> valid_operators{'*', '/', '+', '-', '(', ')'};

        std::list<std::string> tokens;
        std::string currentToken;

        std::string clean_input(std::string);
        void tokenize_input(const std::string);
        void increment_token();
        double add_subtract();
        double multiply_divide();
        double factor();

    public:
        double parse(std::string);
};

#endif