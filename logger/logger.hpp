#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "logger_gen.hpp"
#include <fstream>
#include <string>

namespace stx_logger {
template <class writer_F>
std::string logger<writer_F>::line = "\n";

namespace stx_converter {

template <> struct converter<int> {
  static std::string get_string(const int &msg) { return std::to_string(msg); }
};

template <> struct converter<long int> : public converter<int> {};
template <> struct converter<long long int> : public converter<int> {};
template <> struct converter<unsigned int> : public converter<int> {};
template <> struct converter<long unsigned int> : public converter<int> {};
template <> struct converter<long long unsigned int> : public converter<int> {};

template <> struct converter<std::string> {
  static std::string get_string(const std::string &msg) { return msg; }
};

template <int len> struct converter<char[len]> : public converter<std::string> {};
template <> struct converter<char*> : public converter<std::string> {};
template <> struct converter<const char*> : public converter<std::string> {};


} // namespace stx_converter

/* Logger to file stream. */
struct file_writer {
  std::string path = {};
  inline void operator()(const std::string &str) {
    std::ofstream(path, std::ios_base::app) << str;
  }
};

inline logger<file_writer> file(const std::string &path) {
  return logger<file_writer>(file_writer{path});
}

/* Logger to out stream. */
struct out_writer {
  inline void operator()(const std::string &str) { std::cout << str; }
};

inline logger<out_writer> out() { return logger<out_writer>(); }

/* Logger to error stream */
struct err_writer {
  inline void operator()(const std::string &str) { std::cout << str; }
};

inline logger<err_writer> err() { return logger<err_writer>(); }

} // namespace stx_logger

#endif /* LOGGER_HPP */
