//
// Created by Sascha on 10/2/22.
//

#include "string_utils.h"

/**
 * some code taken from
 * https://www.techiedelight.com/trim-string-cpp-remove-leading-trailing-spaces/
 */

const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string &s) {
  return rtrim(ltrim(s));
}

/*
 * https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
 *
 * modified to eat all whitespaces between strings
 */

std::vector<std::string> string_split(std::string s, char delimiter) {
  size_t pos_start = 0, pos_end, delim_len;
  std::string delimiter_str;
  std::string token;
  std::vector<std::string> res;

  delimiter_str.push_back(delimiter);
  delim_len = delimiter_str.size();

  while ((pos_end = s.find (delimiter_str, pos_start)) != std::string::npos) {
    token = s.substr (pos_start, pos_end - pos_start);
    // set pos_start to possible new starts of next token
    pos_start = pos_end + delim_len;
    // try to remove leading white spaces from next token
    for(size_t next_pos=pos_end+1; next_pos<s.size(); next_pos++) {
      if(s[next_pos] != delimiter) {
        pos_start = next_pos;
        break;
      }
    }
    res.push_back (token);
  }

  res.push_back (s.substr (pos_start));
  return res;
}