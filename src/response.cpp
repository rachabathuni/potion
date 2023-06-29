#include <filesystem>
#include <fstream>
#include <iostream>

#include "../includes/response.hpp"
#include "../includes/request.hpp"
#include "../includes/utils.hpp"

namespace fs = std::filesystem;


Response::Response(Request* r) : request(r) {

}

void Response::set_static_folder(std::string sf) {
  STATIC_FOLDER = sf;
}

void r_test(std::string route) {
  std::cout << last_index_of(route, '.') << std::endl;
  
}

route_struct_t Response::send_string(std::string str) {
  
  std::string headers = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n";

  route_struct_t routeStruct;
  char* buffer = new char[str.length() + headers.length()];
  for (size_t i = 0; i < headers.length(); i++) {
    buffer[i] = headers[i];
  }
  for (size_t i = 0; i < str.length(); i++) {
    buffer[i+headers.length()] = str[i];
  }
  
  routeStruct.buffer = buffer;
  routeStruct.buffer_size = str.length() + headers.length();
  return routeStruct;

}


route_struct_t Response::serve_static_file(std::string file_path) {
  
  //std::string static_path = app->config["STATIC_FOLDER"] + file_path;
  route_struct_t routeStruct; 
  //std::cout << file_path << std::endl;
  if (file_path[0] == '/') {
    file_path = file_path.substr(1, file_path.length()-1);
  }
  std::string fp = STATIC_FOLDER + "/" + file_path;
  fs::path path = fp;
  //std::cout << fp << std::endl;
  if (!file_exists(fp)) {
    return send_status_code(404);
  }
  //std::cout << "here\n";  
  size_t f_size = fs::file_size(path);
  
  set_header("Content-Type", extension_to_content_type(get_file_extension(file_path)));
  set_header("Content-Length", std::to_string(f_size));
  std::string headers = build_headers(200, true);
  
  size_t header_len = headers.length();
  size_t buffer_size = f_size + header_len;
  char* buffer = new char[buffer_size];

  for (size_t i = 0; i < header_len; i++) {
    buffer[i] = headers[i];
  }

  std::ifstream file(path);
  file.read(buffer + header_len, f_size);
  
  routeStruct.buffer = buffer;
  routeStruct.buffer_size = buffer_size;

  return routeStruct;

}

route_struct_t Response::render(std::string file_path) {
  fs::path path = file_path;
  fs::path p = fs::current_path() / path;
  size_t f_size = fs::file_size(p);
  
  set_header("Content-Type", "text/html");
  set_header("Content-Length", std::to_string(f_size));
  std::string headers = build_headers(200, true);

  size_t header_len = headers.length();
  size_t buffer_size = f_size + header_len;

  char* buffer = new char[buffer_size];
  
  for (size_t i = 0; i < header_len; i++) {
    buffer[i] = headers[i];
  }

  std::ifstream file(p);
  file.read(buffer + header_len, f_size);

  route_struct_t routeStruct;
  routeStruct.buffer = buffer;
  routeStruct.buffer_size = buffer_size;
  return routeStruct;
}

route_struct_t Response::send_status_code(int status_code) {

  std::string str = "HTTP/1.1 " + std::to_string(status_code) + " " + code_to_phrase(status_code) + "\r\n";
  route_struct_t routeStruct;
  char* buffer = new char[str.length()];
  string_to_char(str, buffer);
  routeStruct.buffer = buffer;
  routeStruct.buffer_size = str.length();
  return routeStruct;
}

/*
route_struct_t Response::send_status_code(PotionApp* app, uint16_t status_code) {
  
  //for now only 404 and 405 supported add later
  //add enum class later
  
  std::string status_404 = "HTTP/1.1 404 Not Found\r\n";
  std::string status_405 = "HTTP/1.1 405 Method Not Allowed\r\n";
  
  route_struct_t routeStruct;
  switch (status_code) {
      
    case 404: {
              
      char* buffer = new char[status_404.length()];
      string_to_char(status_404, buffer);
      routeStruct.buffer = buffer;
      routeStruct.buffer_size = status_404.length();
      break;
    }
    case 405: {
              
      char* buffer = new char[status_405.length()];
      string_to_char(status_405, buffer);
      routeStruct.buffer = buffer;
      routeStruct.buffer_size = status_405.length();
      break;
    }
    
    default: {
      error("Invalid status code return");
    }
  }

  return routeStruct;

}
*/

route_struct_t Response::send_file(std::string file_path, std::string content_type) {
  

  fs::path path = file_path;
  fs::path p = fs::current_path() / path;
  size_t f_size = fs::file_size(p);

  std::string http_response = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: " + content_type + "\r\n"
    "Content-Length: " + std::to_string(f_size) + "\r\n"
    "Accept-Ranges: bytes\r\n"
    "\r\n";


  size_t header_len = http_response.length();
  //size_t buffer_size = header_len + f_size;
  size_t buffer_size = f_size + header_len;
  char* buffer = new char[buffer_size];
  
  for (size_t i = 0; i < header_len; i++) {
    buffer[i] = http_response[i];
  }
  std::ifstream file(p);

  file.read(buffer + header_len, f_size);
  route_struct_t routeStruct;
  routeStruct.buffer = buffer;
  routeStruct.buffer_size = buffer_size;
  return routeStruct;
  
}


void Response::set_header(std::string key, std::string value) {
  header_vect_struct_t hv;
  hv.key = key;
  hv.value = value;
  headers_vect.push_back(hv);
}

std::string Response::build_headers(int status_code, bool content) {
  std::string phrase = code_to_phrase(status_code);
  if (phrase == "") {
    error("Invalid status code");
  }
  std::string headers_str = "HTTP/1.1 " + std::to_string(status_code) + " " + phrase + "\r\n";
  for (size_t i = 0; i < headers_vect.size(); i++) {
    
    headers_str = headers_str + headers_vect[i].key + ": " + headers_vect[i].value;
    headers_str += "\r\n";

  }
  if (content) {
    headers_str += "\r\n"; 
  }
  return headers_str;
}

std::string Response::code_to_phrase(int code) {
  
  switch (code) {
    
    //####### 1xx - Informational #######
	  case 100: return "Continue";
	  case 101: return "Switching Protocols";
	  case 102: return "Processing";
	  case 103: return "Early Hints";

	  //####### 2xx - Successful #######
	  case 200: return "OK";
	  case 201: return "Created";
	  case 202: return "Accepted";
	  case 203: return "Non-Authoritative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    case 207: return "Multi-Status";
    case 208: return "Already Reported";
    case 226: return "IM Used";

    //####### 3xx - Redirection #######
    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 307: return "Temporary Redirect";
    case 308: return "Permanent Redirect";

    //####### 4xx - Client Error #######
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authentication Required";
    case 408: return "Request Timeout";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Content Too Large";
    case 414: return "URI Too Long";
    case 415: return "Unsupported Media Type";
    case 416: return "Range Not Satisfiable";
    case 417: return "Expectation Failed";
    case 418: return "I'm a teapot";
    case 421: return "Misdirected Request";
    case 422: return "Unprocessable Content";
    case 423: return "Locked";
    case 424: return "Failed Dependency";
    case 425: return "Too Early";
    case 426: return "Upgrade Required";
    case 428: return "Precondition Required";
    case 429: return "Too Many Requests";
    case 431: return "Request Header Fields Too Large";
    case 451: return "Unavailable For Legal Reasons";

    //####### 5xx - Server Error #######
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Timeout";
    case 505: return "HTTP Version Not Supported";
    case 506: return "Variant Also Negotiates";
    case 507: return "Insufficient Storage";
    case 508: return "Loop Detected";
    case 510: return "Not Extended";
    case 511: return "Network Authentication Required";

	  default: return ""; 

	}



}
