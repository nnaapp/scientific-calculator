#include "calculator.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

// input of "-" seg faults, figure out why

double Calculator::parse(std::string input)
{
    input = clean_input(input);
    if (input.empty())
    {
        // REPLACE THIS WITH AN ERROR, THIS IS TEMP HANDLING 
        return 0.0;
    }
    tokenize_input(input);
    validate_tokens();

    if (!tokens.empty())
    {
        currentToken = tokens.front();
        return add_subtract();
    }
    
    return 0;
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
        //bool isValidOp = false;
        std::string cur_char_string;
        cur_char_string.push_back(input[i]);
        bool isValidOp = is_valid_symbol(cur_char_string);

        // If symbol wasn't found, throw an error.
        if (!isValidOp)
        {
            printf("Invalid input, use only numbers and math operator symbols\n");
            input.clear();
            return input;
            // REPLACE THIS WITH THROWING AN ERROR, THIS SUCKS
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
    for (int i = 0; i < len; i++)
    {
        char current = str[i];

        // If character is numeric, concatonate it to the current number.
        if (isdigit(current))
        {
            nums += current;
        }
        else
        {
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
        }
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

    int parentheses_count;
    while (iter != tokens.end())
    {
        if (*iter == "(" || *iter == ")")
            parentheses_count++;

        iter = std::next(iter);
    }

    if (parentheses_count % 2 != 0)
        tokens.clear();
}

void Calculator::increment_token()
{
    //printf("%d\n", tokens.size());
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
        // i think this incrementation causes problems, 
        // not sure when and where, but im trying removing it
        increment_token();
        return result;
    }
    
    // negative case
    if (currentToken == "-")
    {

        if (tokens.size() < 2)
        {
            printf("ERROR: invalid sign -\n");
            // Throw an error here, this means they have somehow gotten here with a hanging negative
            // and proceeding would break the world
            return 0;
        }

        std::list<std::string>::iterator nextToken = tokens.begin();
        nextToken = std::next(nextToken);

        if (isdigit((*nextToken).at(0)))
        {
            increment_token();
            result = -atof(currentToken.c_str());
            increment_token();
            return result;
        }

        if ((*nextToken) == "(")
        {
            increment_token();
            increment_token();
            result = -add_subtract();
            increment_token();
            return result;
        }

        printf("ERROR: invalid sign -\n");
        // Throw an error here, this means the - sign is not a negation and is in an invalid spot
        return 0;
    }

    if (is_valid_symbol(currentToken))
    {
        printf("invalid symbol\n");
        return 0.0;
        // throw an error here probably
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
            int divisor = expression();

            if (divisor == 0)
            {
                // throw an error here, this is impossible
                return 0;
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