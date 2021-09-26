#ifndef LOGGER_GEN_HPP
#define LOGGER_GEN_HPP

#include <iostream>
#include <string>

namespace stx_logger {
namespace stx_converter {

template <class obj_T> struct converter;

template <class obj_T> std::string convert(const obj_T &msg) {
  return stx_logger::stx_converter::converter<obj_T>::get_string(msg);
}

} // namespace stx_converter

template <class writer_F> class logger {
  template <class obj_T> void write(const obj_T &msg) {
    m_container += stx_converter::convert(msg);
  }
  std::string m_container = {};
  writer_F m_writer;

public:
  static std::string line;
  logger() {}
  logger(const writer_F &writer) : m_writer(writer) {}
  inline ~logger() { m_writer(m_container + "\n"); }

  template <class obj_T>
  inline logger<writer_F> &operator<<(const obj_T &msg) {
    write(msg);
    return *this;
  }
};
} // namespace stx_logger

#endif /* LOGGER_GEN_HPP */
