#include "stringUtils.h"
#include <ctype.h>

std::string StringUtils::toLowerCase(std::string input) {
  for (long unsigned int i=0; i<input.length(); i++) {
      input[i] = tolower(input.at(i));
  } 
  return input;
}
