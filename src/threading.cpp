
#include "threading.hpp"
#include "tcpserver_unix.hpp"

void ThreadPool::startThreads(int num_threads, request_handler_function_t* requestHandler, TCPServer server) {
  
  for (int i = 0; i < num_threads; i++) {
    std::thread thd([this, requestHandler, server] {worker(requestHandler, server); });
    threads.emplace_back(std::move(thd));
  } 
}


void ThreadPool::worker(request_handler_function_t* requestHandler, TCPServer server) {
  while (1) {
     
    int socket = b_queue.pop();

    requestHandler(socket, server); //here the entire request needs to be handled and send back. This function takes the request as a string as well as the newsockfd in order to write to the socket
                          //when this function ends, the program has sent back something based on the handler function etc
                          //this function is called after a connection has been accepted, it hands off the connection to a thread from the pool. The thread is "joined back" into the pool
                          //as it searches for more jobs to take on
  }
}

void ThreadPool::addJob(int socket) {
  
  b_queue.push(socket);
  
}

template <class T>
void BlockingQueue<T>::push(T value) {
  
  std::unique_lock<std::mutex> lck(mtx);
  queue.push(value);
  lck.unlock();
  cv.notify_one();

}
template <class T>
T BlockingQueue<T>::pop() {
  std::unique_lock<std::mutex> lock(mtx);
  while(queue.empty()) {
    cv.wait(lock);
  }

  T value = queue.front();
  queue.pop();
  return value;
  

}


