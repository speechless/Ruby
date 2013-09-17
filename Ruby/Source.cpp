/*	Project Ruby
 *	> HTTP packet parser
 */

#include <iostream> // Debug >> Remove this

#include <stdlib.h> 
#include <string>

std::string _HTTP_BUFFER, _HTTP_MESSAGE;

/*	ParseConnection(std::string packet, std::string &message, std::string &buffer)
 *
 *	Description: Parses several broken HTTP packets into single complete messages.
 *
 *	Usage:
 *		int Result = ParseConnection(networkPacket, httpPacket, buffer);
 *		if (Result == 0) {
 *			// do work based on httpPacket
 *		}
 *		else if (Result == 2) {
 *			// Error, close and reset client connection
 *		}
 *		else {
 *			// do nothing and continue with network loop
 *		}
 *
 *	@packet[in]: incoming data from network socket
 *	@message[out]: complete parsed message
 *	@buffer[n/a]: do not touch, used as state memory
 *
 *	@return 0: success, full message receieved and stored in "message"
 *	@return 1: failure, message header not complete
 *	@return 2: failure <fatal>, content length not found
 *	@return 3: failure, message body not complete
 */

int ParseConnection(std::string packet, std::string &message, std::string &buffer)
{
	buffer.append(packet);

	// Look for end of header
	int header = buffer.find("\r\n\r\n");
	if (header == std::string::npos) {
		return 1; // Message not complete
	}

	// Determine HTTP request
	// GET request
	if (buffer.find("GET") == 0) {
		message = buffer.substr(0, header + std::string("\r\n\r\n").length());
		buffer.erase(buffer.begin(), buffer.begin() + header + std::string("\r\n\r\n").length());
		return 0;
	}
	
	// POST request
	else if (buffer.find("POST") == 0) {
		// Look for body content length
		int length = buffer.find("Content-Length: ");
		if (length == std::string::npos) {
			return 2; // Content length not found
		}

		// Translate between "Content Length: " and "\r\n" to numbers
		length += std::string("Content-Length: ").length();
		int digits = buffer.find("\r\n", length+1);
		std::string numberText = buffer.substr(length,digits-length);
		int number = atoi(&numberText.at(0));

		// Check if complete message has been recieved
		if (buffer.length() < header + std::string("\r\n\r\n").length() + number) {
			return 3; // Entire message not recieved.
		}

		// Copy message from buffer to message
		message = buffer.substr(0, header + std::string("\r\n\r\n").length() + number);

		// Clear remove message from buffer
		buffer.erase(buffer.begin(), buffer.begin() + header + std::string("\r\n\r\n").length() + number);

		return 0;
	}

	return 0;
}

int main (int argc, char *argv[])
{
	ParseConnection("POST 200 OK\r\n", _HTTP_MESSAGE, _HTTP_BUFFER);
	ParseConnection("Content-Type: text/html\r\n", _HTTP_MESSAGE, _HTTP_BUFFER);
	ParseConnection("Content-Length: 32\r\n", _HTTP_MESSAGE, _HTTP_BUFFER);
	ParseConnection("\r\nhome=Cosby&favorite+flavor=flies", _HTTP_MESSAGE, _HTTP_BUFFER);
	ParseConnection("GET 200 OK\r\n", _HTTP_MESSAGE, _HTTP_BUFFER);

	std::cout << ">> Message <<" << std::endl;
	std::cout << _HTTP_MESSAGE << std::endl;
	std::cout << ">> Buffer <<" << std::endl;
	std::cout << _HTTP_BUFFER << std::endl;

	return 0;
}