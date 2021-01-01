#include <connectionHandler.h>
 
using boost::asio::ip::tcp;

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
 
ConnectionHandler::ConnectionHandler(string host, short port,std::map<std::string,short> map): host_(host), port_(port), io_service_(), socket_(io_service_), sendingOpCode(0), gettingOpCode(0),opMap(map){}
    
ConnectionHandler::~ConnectionHandler() {
    close();
}
 
bool ConnectionHandler::connect() {
    std::cout << "Starting connect to " 
        << host_ << ":" << port_ << std::endl;
    try {
		tcp::endpoint endpoint(boost::asio::ip::address::from_string(host_), port_); // the server endpoint
		boost::system::error_code error;
		socket_.connect(endpoint, error);
		if (error)
			throw boost::system::system_error(error);
    }
    catch (std::exception& e) {
        std::cerr << "Connection failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}
 
bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead) {
    size_t tmp = 0;
	boost::system::error_code error;
    try {
        while (!error && bytesToRead > tmp ) {
			tmp += socket_.read_some(boost::asio::buffer(bytes+tmp, bytesToRead-tmp), error);			
        }
		if(error)
			throw boost::system::system_error(error);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite) {
    int tmp = 0;
	boost::system::error_code error;
    try {
        while (!error && bytesToWrite > tmp ) {
            tmp += socket_.write_some(boost::asio::buffer(bytes, bytesToWrite - tmp), error);
        }
		        if(error)
		                throw boost::system::system_error(error);

    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}
 
bool ConnectionHandler::getLine(std::string& line) {
    return getFrameAscii(line);
}

bool ConnectionHandler::sendLine(std::string& line) {
    return sendFrameAscii(line);
}
 

bool ConnectionHandler::getFrameAscii(std::string& frame) {
    std::cout<<"getting string"<<endl;
    char ch = '1';
    // Stop when we encounter the null character.
    // Notice that the null character is not appended to the frame string.
    unsigned int counter = 0;
    char opBytes [2] ;
    char messageBytes [2] ;
    int i = 0;
    try {
        while(i < 2 | ch != '\0'){
            if(!getBytes(&ch, 1))
            {
                return false;
            }
            if(counter > 3 & ch != '\0'){
                frame.append(1, ch);
            }
            if (counter < 2){
                std::cout<<ch<<endl;
                opBytes[counter] = ch;
            }
            if (counter > 1 & counter < 4){
                messageBytes[counter - 2] = ch;
            }
            counter = counter + 1;
            if (counter == 2){
                gettingOpCode = bytesToShort(opBytes);
                if (gettingOpCode == 12) frame = frame + "ACK";
                else frame = frame + "ERROR";

            }
            if (counter == 4){
                OpMessage = bytesToShort(messageBytes);
                frame = frame + " ";
                frame = frame + std::to_string(OpMessage);
                if (gettingOpCode == 13) break;
            }
            i = i + 1;
	    }
    } catch (std::exception& e) {
	std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
	return false;
    }
    return true;
}
 
 
bool ConnectionHandler::sendFrameAscii(const std::string& frame) {
    if (sendingOpCode == 1 | sendingOpCode == 2 | sendingOpCode == 3) {
        char arr[frame.length() + 3];
        shortToBytes(sendingOpCode, &arr[0]);
        unsigned int freeSlot = 2;
        for (const char &c : frame) {
            if (c == ' ') {
                arr[freeSlot] = '\0';
                freeSlot = freeSlot + 1;
            } else {
                arr[freeSlot] = c;
                freeSlot = freeSlot + 1;
            }
        }
        arr[freeSlot] = '\0';
        sendBytes(arr, frame.length() + 3);
    }
    else if (sendingOpCode == 4 | sendingOpCode == 11) {
        char arr[2];
        shortToBytes(sendingOpCode, &arr[0]);
        sendBytes(arr, 2);
    }
    else if (sendingOpCode == 8) {
        char arr[frame.length() + 3];
        shortToBytes(sendingOpCode, &arr[0]);
        unsigned int freeSlot = 2;
        for(const char& c : frame) {
            arr[freeSlot] = c;
            freeSlot = freeSlot + 1;
        }
        arr[freeSlot] = '\0';
        sendBytes(arr, frame.length() + 3);
    }
    else {
        char arr[4];
        shortToBytes(sendingOpCode, &arr[0]);
        unsigned int freeSlot = 2;
        for(const char& c : frame) {
            arr[freeSlot] = c;
            freeSlot = freeSlot + 1;
        }
        arr[freeSlot] = '\0';
        sendBytes(arr, 4);
    }
}
 
// Close down the connection properly.
void ConnectionHandler::close() {
    try{
        socket_.close();
    } catch (...) {
        std::cout << "closing failed: connection already closed" << std::endl;
    }
}

void ConnectionHandler::prepareLine(std::string &line){
    std::string code;
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    pos = line.find(delimiter);
    if (pos != line.npos){
        code = line.substr(0, pos);
        line.erase(0, pos + delimiter.length());
        sendingOpCode = opMap[code];
    }
    else{
        sendingOpCode = opMap[line];
    }
}

void ConnectionHandler::shortToBytes(short num, char* bytesArr){
    bytesArr[0] = ((num >> 8) & 0xFF);
    bytesArr[1] = (num & 0xFF);
}

short ConnectionHandler::bytesToShort(char* bytesArr){
    short result = (short)((bytesArr[0] & 0xff) << 8);
    result += (short)(bytesArr[1] & 0xff);
    return result;
}

short ConnectionHandler::getCode(){
    return sendingOpCode;
}