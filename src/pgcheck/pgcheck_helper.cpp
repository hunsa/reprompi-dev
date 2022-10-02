//
// Created by Sascha on 9/22/22.
//

#include <cstdio>
#include <iostream>
#include <array>


/**
 * not my code
 * https://raymii.org/s/articles/Execute_a_command_and_get_both_output_and_exit_code.html
 * @param cmd
 * @return
 */

std::string exec_command(const std::string &command) {
  std::array<char, 1048576> buffer {};
  std::string result;

  FILE *pipe = popen(command.c_str(), "r");
  if (pipe == nullptr) {
    throw std::runtime_error("popen() failed!");
  }
  try {
    std::size_t bytesread;
    while ((bytesread = std::fread(buffer.data(), sizeof(buffer.at(0)), sizeof(buffer), pipe)) != 0) {
      result += std::string(buffer.data(), bytesread);
    }
  } catch (...) {
    pclose(pipe);
    throw;
  }
  pclose(pipe);
  return result;
}

