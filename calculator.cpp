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
        // If the current character is not a space,
        // put the i'th element into the pos'th index.
        // In the case that the current character IS a space,
        // pos and i will become offset from each other because
        // i increments and pos does not. This allows us to
        // shift based on the number of spaces we are removing.
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

        // If the symbol is not in our list of valid symbols,
        // throw an error.
        if (!is_valid_symbol(input.substr(i, 1)))
        {
            throw InvalidParseException("Invalid symbol");
        }
    }

    return input;
}

// This checks the passed in string against the list of valid symbols,
// to quickly check if it is a good token or not.
// std::string is used because we use iterators to iterate across the list of tokens,
// and the tokens are stored as strings due to multi-digit numbers, and it is just easier
// to use std::string than it is to put (*iterator)[0], which is also less readable.
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

    // Go over every character in the string,
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
            // then this token is invalid because two decimals is meaningless.
            if (currentNumDecimal)
            {
                throw InvalidParseException("Invalid decimal");
            }

            nums += current;
            // Set the decimal flag for the above exception case.
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

    // Count number of parentheses, open and close.
    int openPar = 0, closePar = 0;
    while (iter != tokens.end())
    {
        if (*iter == "(")
            openPar++;

        if (*iter == ")")
            closePar++;

        iter = std::next(iter);
    }

    // While we are here, check the last symbol to see if it's valid or not,
    // this could prevent doing work we don't need to do.
    // Invalid is any symbol that isn't ), because something like (2 + 2) is valid.
    iter--;
    if (is_valid_symbol(*iter) && (*iter) != ")")
    {
        throw InvalidParseException("Invalid symbol at end");
    }

    // If there is a mismatched number of open vs close parentheses,
    // something is wrong.
    if (openPar != closePar)
    {
        throw InvalidParseException("Parentheses mismatch");
    }
}

// Inrements the current token by popping the front of the list,
// and then getting the new front element.
void Calculator::increment_token()
{
    // This is here to catch any weird cases,
    // if we get here with an empty list.
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


// This function handles recursively starting back at add_subtract,
// in order to handle sub-expressions. 2 + (2 + 2) is handled by recursively
// passing in the (2 + 2) expression, and then treating the result as one number.
// This function also treats single digits as an expression. That is to say, in
// 2 + (2 + 2), the first 2 is returned as the result of a single digit expression,
// in the eyes of this algorithm.
double Calculator::expression()
{
    double result;

    // Parentheses case
    if (currentToken == "(")
    {
        increment_token();
        result = add_subtract();
        increment_token();
        return result;
    }
    
    // Negative case
    if (currentToken == "-")
    {
        // If we somehow get inside this condition,
        // the last token is a - and is thus invalid.
        if (tokens.size() <= 1)
        {
            throw InvalidParseException("Invalid negative");
        }

        std::list<std::string>::iterator nextToken = tokens.begin();
        nextToken = std::next(nextToken);

        // If next is not a symbol, meaning it is numeric,
        // then this is a negative number.
        if (!(is_valid_symbol(*nextToken)))
        {
            increment_token();
            result = -atof(currentToken.c_str());
            increment_token();
            return result;
        }

        // If next is a parentheses, meaning an expression,
        // then this is a negative expression and we recurse.
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

    // Normal number case

    // String to double conversion.
    // We use doubles to support decimals, as well as precision.
    // atof returns 0.0 if it is passed a non-numeric input,
    // but we cleaned out tokens and handled unexpected ones,
    // so that should never happen.
    result = atof(currentToken.c_str());
    increment_token();

    return result;
}

// This function handles the MD part of PEMDAS, checking for 
// multiplication and then division. It serves as the middle step in our
// add_subtract -> multiply_divide -> expression hierarchy.
// It calls expression as the first instruction in order to immediately
// fall through to the PE part of PEMDAS, and get the symbol it is meant to check.
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

// This function handles the AS part of PEMDAS, checking for
// addition and then subtraction. It is the first step in our
// add_subtract -> multiply_divide -> expression hierarchy. It immediately
// calls multiply_divide, which then calls expression, so we fall through to
// the PE part of PEMDAS and work backwards. This will be the last function
// of the three to resolve completely, as it is the last step of PEMDAS.
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