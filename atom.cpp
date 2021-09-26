#ifndef ATOM_CPP
#define ATOM_CPP

#include "atom.hpp"
#include "logger/logger.hpp"
#include <fcntl.h>
#include <future>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>

using namespace stx_logger;
using namespace stx_logger::stx_converter;
using namespace atom;

atomid_t atom_connection::GlobalID = 0;
atom_connection::~atom_connection() {
  out() << " {-}  ::  ATOM  ::  id_" << m_id
        << "  :: Base destructor (default). ";
}

int atom_connection::set_noblock() {
  if (fcntl(m_fd, F_SETFD, fcntl(m_fd, F_GETFD, 0) | O_NONBLOCK) < 0)
    return -1;
  return 0;
}

int atom_server::epoll_ctl_add(atomevent_t events) {
  return epoll_ctl_add(events, *this);
}

int atom_server::epoll_ctl_add(atomevent_t events, const atom_connection &c) {
  struct epoll_event ev;
  ev.events = events;
  ev.data.fd = c.m_fd;
  // std::cout << c.m_fd << "  " << ev.data.fd << " " << m_epfd << std::endl;
  int a = epoll_ctl(m_epfd, EPOLL_CTL_ADD, c.m_fd, &ev);
  return a;
}

atom_server::atom_server(const sockaddr_in &addr,                      //
                         atomtype_t type, atomsize_t size)             //
    : atom_connection(addr), m_type(type), m_size(size), m_clients() { //
  out() << " {+}  :: SERVER ::  id_" << m_id << "  :: Server constructor. ";
  if ((m_fd = socket(m_addr.sin_family, m_type, 0)) < 0)
    throw std::runtime_error(" {!}  ERROR  on creating socket device. ");
  if (bind(m_fd, (sockaddr *)&m_addr, get_addr_len()) < 0)
    throw std::runtime_error(" {!}  ERROR  on binding socket. ");
  if (set_noblock() < 0)
    throw std::runtime_error(" {!}  ERROR  on setting \"no blocking\" flags ");
  if (listen(m_fd, m_size) < 0)
    throw std::runtime_error(" {!}  ERROR  on listening. ");
  if ((m_epfd = epoll_create1(0)) < 0)
    throw std::runtime_error(" {!}  ERROR  on creating epoll device. ");
  if (epoll_ctl_add(EPOLLIN | EPOLLOUT | EPOLLET) < 0)
    throw std::runtime_error(
        " {!}  ERROR  on connecting server to epoll device. ");
  out() << " {i}  :: SERVER ::  " << *this << " " << m_fd << " " << m_epfd;

  run();
}

atom_server::~atom_server() {
  out() << " {-}  :: SERVER ::  id_" << m_id << "  :: Server destructor. ";
  for (auto &c : m_clients) {
    delete c.second;
  }
  close(m_epfd);
  close(m_fd);
}

atom_client &atom_server::accept() {
  atom_client *tmp = new atom_client();
  tmp->m_fd = ::accept(m_fd, (sockaddr *)&(tmp->m_addr), &(tmp->m_addr_len));
  out() << " {i}  :: CLIENT ::  " << *tmp;
  if (tmp->m_fd < 0)
    throw std::runtime_error(" {!}  ERROR  on accepting client ");
  if (tmp->set_noblock() < 0)
    throw std::runtime_error(" {!}  ERROR  on setting \"no blocking\" flags ");
  m_clients[tmp->m_fd] = tmp;
  if (epoll_ctl_add(EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP, *tmp) < 0)
    throw std::runtime_error(
        " {!}  ERROR  on connecting client to epoll device. ");
  out() << " [+]  :: CLIENT ::";
  return *tmp;
}

atom_server &atom_server::disconnect(const atom_client &cli) {
  m_clients.erase(cli.m_fd);
  delete &cli;
  return *this;
}

atom_server &atom_server::run() {
  int ndfs = 0;
  epoll_event *events;
  while (ndfs >= 0) {
    ndfs = epoll_wait(m_epfd, events, m_size, -1);
    m_events = events;
    for (int i = 0; i < ndfs; i++) {
      if (m_events[i].data.fd == m_fd) {
        accept().send_msg("Hello bayba!\n");
      } else if (m_events[i].events & EPOLLIN) {
        m_clients[m_events[i].data.fd]->process_msg();
      } else {
        m_clients[m_events[i].data.fd]->process_unexpected();
      }
      if (m_events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
        disconnect(*m_clients[m_events[i].data.fd]);
      }
    }
  }
  return *this;
}

atom_client::atom_client() //
    : atom_connection() {  //
  out() << " {+}  :: CLIENT ::  id_" << m_id << "  :: Client constructor. ";
}

atom_client::~atom_client() {
  out() << " {-}  :: CLIENT ::  id_" << m_id << "  :: Client destructor. ";
  close(m_fd);
}

atom_client &atom_client::send_msg(const std::string &msg) {
  write(m_fd, msg.c_str(), msg.length());
  return *this;
}

void atom_client::process_msg() {
  char buf[256];
  bzero(buf, sizeof(buf));
  read(m_fd, buf, sizeof(buf));
  std::string msg(buf);
  std::string normalized;
  std::string delimetr("\n");
  std::string token;
  size_t pos = 0;
  if (msg.length() > 0) {
    while ((pos = msg.find(delimetr)) != msg.npos) {
      normalized += msg.substr(0, pos);
      normalized += " ";
      msg.erase(0, pos + delimetr.length());
    }
    token = normalized;
    msg = buf;
    while (!normalized.empty()) {
      pos = 35;
      out() << " [+]  ::  DATA  ::  id_" << m_id
            << "  :: " << normalized.substr(0, pos);
      normalized.erase(0, pos);
    }
  }
}

void atom_client::process_unexpected() {
  out() << " [+]  ::  DATA  ::  id_" << m_id << "  :: UNEXPECTED. ";
}

#endif /* ATOM_CPP */
