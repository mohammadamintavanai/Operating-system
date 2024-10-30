#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <map>
const char* OPEN_WITH = "open";
const char* OP = "open";
const char* NUM = "22";
using namespace std;
int stringToNumber(const char* str) {
    std::stringstream ss(str);
    int number;
    ss >> number; // Convert string to integer
    return ss.fail() ? 0 : number; // Return 0 if conversion failed
}
void splitArray(const char* input, char splitChar, char*& firstPart, char*& secondPart) {
    size_t inputLength = strlen(input);
    size_t firstPartLength = 0;
    size_t secondPartLength = 0;
    bool splitOccurred = false;

    // First pass: Determine lengths of both parts
    for (size_t i = 0; i < inputLength; ++i) {
        if (input[i] == splitChar) {
            splitOccurred = true;
            continue; // Skip the split character
        }
        
        if (!splitOccurred) {
            firstPartLength++;
        } else {
            secondPartLength++;
        }
    }

    // Allocate memory for the new arrays
    firstPart = new char[firstPartLength + 1]; // +1 for null terminator
    secondPart = new char[secondPartLength + 1]; // +1 for null terminator

    // Second pass: Fill the new arrays
    size_t firstIndex = 0;
    size_t secondIndex = 0;
    
    splitOccurred = false; // Reset the flag for the second pass

    for (size_t i = 0; i < inputLength; ++i) {
        if (input[i] == splitChar) {
            splitOccurred = true; // Mark that we have passed the split character
            continue; // Skip the split character
        }
        
        if (!splitOccurred) {
            firstPart[firstIndex++] = input[i];
        } else {
            secondPart[secondIndex++] = input[i];
        }
    }

    // Null terminate the new arrays
    firstPart[firstPartLength] = '\0';
    secondPart[secondPartLength] = '\0';
}
bool areStringsEqual(const char* str1, const char* str2) {
    // Compare characters until the end of one string is reached
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return false; // Characters are not equal
        }
        str1++;
        str2++;
    }
    // Check if both strings reached the end (i.e., both are of the same length)
    return *str1 == *str2;
}
char* concatenateStringAndNumber(const char* str, int number) {
    // Convert number to string using stringstream
    std::stringstream ss;
    ss << number;
    std::string numberStr = ss.str();

    // Calculate the lengths of the input string and the number string
    size_t strLength = strlen(str);
    size_t numberStrLength = numberStr.length();

    // Allocate memory for the new concatenated string
    char* result = new char[strLength + numberStrLength + 1]; // +1 for null terminator

    // Copy the original string and the number string into the result
    strcpy(result, str);
    strcat(result, numberStr.c_str());

    return result;
}
char* concat_two_string(char* s1,char* s2){
    size_t s1_lengh = strlen(s1);
    size_t s2_lengh = strlen(s2);
    char* result = new char[s1_lengh+s2_lengh+1];
    strcpy(result,s1);
    strcpy(result,s2);
    return result;
}

bool startsWith(const char* str, const char* prefix) {
    // Get lengths of both strings
    size_t prefixLength = strlen(prefix);
    size_t strLength = strlen(str);

    // If the prefix is longer than the string, it can't start with it
    if (prefixLength > strLength) {
        return false;
    }

    // Compare the beginning of str with the prefix
    return strncmp(str, prefix, prefixLength) == 0;
}
int main(){

    bool a = startsWith(OPEN_WITH,OP);
    printf("%d",a);
    return 0;
}