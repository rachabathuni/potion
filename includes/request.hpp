#ifndef HTTP_H_INCLUDED
#define HTTP_H_INCLUDED

#include <map>

#include "tcpserver_unix.hpp"


class Request {
  private:
    receive_struct_t receiveStruct;
    std::map<std::string, std::string> header_map;

  public:
    Request (receive_struct_t rs) : receiveStruct(rs) {};
    void parse_headers();
    std::string get_method();
    std::string get_single_header(std::string key);
    std::map<std::string, std::string> get_headers();
    
    std::string get_route();
    
};

#endif
