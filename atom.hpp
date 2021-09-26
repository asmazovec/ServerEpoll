#ifndef ATOM_HPP
#define ATOM_HPP

#include "logger/logger_gen.hpp"
#include <arpa/inet.h>
#include <map>
#include <netinet/ip.h>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>

#define __DEF_PORT 6060
#define __DEF_SIZE 1024
#define __DEF_SAFAMILY AF_INET
#define __DEF_INADDR INADDR_ANY
#define __DEF_TYPE SOCK_STREAM

namespace atom {
class atom_connection;
class atom_server;
class atom_client;

using atomcontainer_t = std::map<int, atom_client *>;
using atomevent_t = uint32_t;
using atomtype_t = uint16_t;
using atomsize_t = int;
using atomid_t = uint64_t;
using atomfd_t = int;

/*
 * ::  ATOM  ::  Base container class for server/client
 * */
class atom_connection {
  friend atom_server;

private:
  static atomid_t GlobalID;

protected:
  atomid_t m_id;
  atomfd_t m_fd;
  sockaddr_in m_addr;
  socklen_t m_addr_len;

  atom_connection(const sockaddr_in &addr)                            //
      : m_addr(addr), m_id(GlobalID++), m_addr_len(sizeof(m_addr)) {} //

  atom_connection()                                               //
      : m_id(GlobalID++), m_addr(), m_addr_len(sizeof(m_addr)) {} //

  int set_noblock();

public:
  atomid_t get_id() const { return m_id; }
  sockaddr_in get_addr() const { return m_addr; }
  socklen_t get_addr_len() const { return sizeof(m_addr); }
  virtual ~atom_connection() = 0;
};

/*
 * :: SERVER ::  Server
 * */
class atom_server : public atom_connection {
private:
  atomcontainer_t m_clients;
  epoll_event *m_events;
  atomtype_t m_type;
  atomsize_t m_size;
  atomfd_t m_epfd;

  int epoll_ctl_add(atomevent_t events);
  int epoll_ctl_add(atomevent_t events, const atom_connection &c);

public:
  bool is_run();
  atom_server(const sockaddr_in &addr =      //
              {__DEF_SAFAMILY,               // AF_INET
               htons(__DEF_PORT),            // 6060
               {__DEF_INADDR}},              // INADDR_ANY
              atomtype_t type = __DEF_TYPE,  // SOCK_STREAM
              atomsize_t size = __DEF_SIZE); //
  virtual ~atom_server() override;

  atom_client &accept();
  atom_server &disconnect(const atom_client &cli);

  atom_server &run();
};

/*
 * :: CLIENT ::  Client
 *  */
class atom_client : public atom_connection {
  friend class atom_server;

public:
  atom_client();
  virtual ~atom_client() override;

  atom_client &send_msg(const std::string &msg);
  void process_msg();
  void process_unexpected();
};

} // namespace atom

using namespace atom;

namespace stx_logger {
namespace stx_converter {

template <>                         //
struct converter<atom_connection> { //
  static std::string get_string(const atom_connection &a) {
    std::stringstream out;
    char buf[256];
    sockaddr_in tmp_addr = a.get_addr();
    inet_ntop(tmp_addr.sin_family,          //
              (char *)&(tmp_addr.sin_addr), //
              buf, sizeof(buf));            //
    out << "id_" << a.get_id()              //
        << "  :: " << buf << ":"            //
        << ntohs(tmp_addr.sin_port);        //
    return out.str();
  }
};

template <>                                                           //
struct converter<atom_server> : public converter<atom_connection> {}; //

template <>                                                           //
struct converter<atom_client> : public converter<atom_connection> {}; //

} // namespace stx_converter
} // namespace stx_logger

#endif /* ATOM_HPP */
