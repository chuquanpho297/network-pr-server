#include "util.h"
#include <stdexcept>

string encodeSpace(string str)
{
    string result;
    for (char c : str)
    {
        if (c == ' ')
        {
            result += "%20";
        }
        else
        {
            result += c;
        }
    }
    return result;
}

string convertDateTimeFormat(const std::string &input)
{
    // Check if the input format is correct
    if (input.length() != 19 || input[10] != ' ')
    {
        throw invalid_argument("Input format must be 'YYYY-MM-DD HH:MM:SS'");
    }

    // Create the output format
    std::string output = input;
    output[10] = 'T'; // Replace the space with 'T'

    return output;
}