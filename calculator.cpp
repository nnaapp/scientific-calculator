#include "calculator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>

double Calculator::parse(std::string input)
{
    try
    {
        input = clean_input(input);
        tokenize_input(input);
        validate_tokens();

        if (!tokens.empty())
        {
            currentToken = tokens.front();
            return add_subtract();
        }

        // If tokens are empty, then the answer is always 0.
        return 0;
    }
    catch (InvalidParseException e)
    {
        printf("%s\n", e.message().c_str());
        return 0;
    }
}

std::string Calculator::clean_input(std::string input)
{
    int inputLen = input.length();

    // Removes all spaces from the input.
    // This normalizes it to just digits and symbols.
    int pos = 0;
    for (int i = 0; i < inputLen; i++)
    {
        if (input[i] != ' ')
        {
            input[pos] = input[i];
            pos++;
        }
    }
    input.resize(pos); // Remove the now unused space, left by removing the spaces
    inputLen = input.length();

    // Iterates over the input string, removing all spaces,
    // and checks to make sure that any non-digit characters
    // are valid math operators. Any non-supported symbols,
    // and any letters will cause an error to be thrown.
    for (int i = 0; i < inputLen; i++)
    {
        // Skip this iteration if the char is a digit.
        if (isdigit(input[i]))
        {
            continue;
        }

        // Iterate over the string and set a flag to true
        // if the symbol is found when searching the list
        // of valid symbols.
        std::string cur_char_string;
        cur_char_string.push_back(input[i]);
        bool isValidOperator = is_valid_symbol(cur_char_string);

        // If symbol wasn't found, throw an error.
        if (!isValidOperator)
        {
            throw InvalidParseException("Invalid symbol");
        }
    }

    return input;
}

bool Calculator::is_valid_symbol(std::string str)
{
    for (int i = 0; i < valid_symbols.size(); i++)
    {
        if (str == valid_symbols[i])
            return true;
    }

    return false;
}

// Turns the cleaned input string into a list of "token" strings,
// where each token string represents a number or a math operator.
void Calculator::tokenize_input(const std::string str)
{
    std::string nums, token;
    int len = str.length();

    // Go over every character (char) in the string,
    // check if it is a digit or a math operator,
    // and either keep adding until you encounter a non-digit
    // or push the digits and then push the operator.
    //
    // This forms a list (we use list for pop_front(), mostly) or strings,
    // where each string is either an operator or a full number.
    bool currentNumDecimal = false;
    for (int i = 0; i < len; i++)
    {
        char current = str[i];

        // If character is numeric, concatonate it to the current number.
        if (isdigit(current))
        {
            nums += current;
            continue;
        }
        
        // If character is a decimal, see if there has already been a decimal
        // in this number, otherwise concatonate it to the current number.
        if (current == '.')
        {
            // If there is already a decimal in the current number,
            // then the tokens are invalid because that number cannot exist.
            if (currentNumDecimal)
            {
                /*printf("ERROR: invalid decimal\n");
                tokens.clear();
                // probably throw an error here
                return;*/
                throw InvalidParseException("Invalid decimal");
            }

            nums += current;
            currentNumDecimal = true;
            continue;
        }

        // If non-numeric, it must be an operator because we cleaned the string in the parse() function.
        // Push number to list and then push the operator after it.
        if (!nums.empty())
        {
            tokens.push_back(nums);
            nums.clear();
        }

        token += current;
        tokens.push_back(token);
        token.clear();
        currentNumDecimal = false;
    }

    // Push the final number if any.
    if (!nums.empty())
    {
        tokens.push_back(nums);
        nums.clear();
    }
}

void Calculator::validate_tokens()
{
    std::list<std::string>::iterator iter = tokens.begin();

    int parentheses_count = 0;
    while (iter != tokens.end())
    {
        if (*iter == "(" || *iter == ")")
            parentheses_count++;

        iter = std::next(iter);
    }

    iter--;
    if (is_valid_symbol(*iter) && (*iter) != ")")
    {
        throw InvalidParseException("Invalid symbol at end");
    }

    // If the number of parentheses is odd, meaning that
    // the remainder of count / 2 is non-zero, then there is a
    // mismatch somewhere.
    if (parentheses_count % 2 != 0)
    {
        throw InvalidParseException("Parentheses mismatch");
    }
}

void Calculator::increment_token()
{
    if (tokens.empty())
    {
        currentToken.clear();
        return;
    }

    tokens.pop_front();

    if (tokens.empty())
    {
        currentToken.clear();
        return;
    }

    currentToken = tokens.front();
}


// rename this, name is obscure and not helpful
double Calculator::expression()
{
    double result;

    // parentheses case
    if (currentToken == "(")
    {
        increment_token();
        result = add_subtract();
        increment_token();
        return result;
    }
    
    // negative case
    if (currentToken == "-")
    {

        if (tokens.size() < 2)
        {
            // If there is 1 (or somehow less) tokens left,
            // and this one is a negative, then it is invalid.
            throw InvalidParseException("Invalid negative");
        }

        std::list<std::string>::iterator nextToken = tokens.begin();
        nextToken = std::next(nextToken);

        // If next is not a symbol, meaning it is numeric
        if (!(is_valid_symbol(*nextToken)))
        {
            increment_token();
            result = -atof(currentToken.c_str());
            increment_token();
            return result;
        }

        // If next is a parentheses, meaning an expression
        if ((*nextToken) == "(")
        {
            increment_token();
            increment_token();
            result = -add_subtract();
            increment_token();
            return result;
        }

        // If we get here, the thing following the negative is unexpected,
        // and makes the expression invalid.
        throw InvalidParseException("Invalid negative");
    }

    // If we get inside of this condition, there is
    // a symbol where the calculator is expecting
    // a number. This means the input is invalid and 
    // we cannot finish.
    if (is_valid_symbol(currentToken))
    {
        throw InvalidParseException("Unexpected symbol");
    }

    // normal number case

    // String to double conversion.
    // We use doubles to support decimals, as well as precision.
    result = atof(currentToken.c_str());
    increment_token();

    return result;
}

double Calculator::multiply_divide()
{
    double result = expression();

    while (currentToken == "*" || currentToken == "/")
    {
        if (currentToken == "*")
        {
            increment_token();
            result *= expression();
        }

        if (currentToken == "/")
        {
            increment_token();
            double divisor = expression();

            if (divisor == 0)
            {
                throw InvalidParseException("Division by zero");
            }
 
            result /= divisor;
        }
    }

    return result;
}

double Calculator::add_subtract()
{
    double result = multiply_divide();

    while (currentToken == "+" || currentToken == "-")
    {
        if (currentToken == "+")
        {
            increment_token();
            result += multiply_divide();
        }
        
        if (currentToken == "-")
        {
            increment_token();
            result -= multiply_divide();
        }
    }

    return result;
}