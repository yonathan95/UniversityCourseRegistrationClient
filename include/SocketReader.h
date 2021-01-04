#include <mutex>
#include <vector>
#include "connectionHandler.h"

#ifndef CLIENT_SOCKETREADER_H
#define CLIENT_SOCKETREADER_H
class SocketReader{
private:
    ConnectionHandler &connectionHandler;
    std::mutex & mutex;
    std::vector<std::string> &outputs;
    bool &stopper;
public:
    SocketReader (ConnectionHandler &connectionHandler, std::mutex & mutex1, std::vector<std::string> &outputs1, bool &stopper);

    void readFromSocket();

};
#endif //CLIENT_SOCKETREADER_H