/*	Project Ruby
 *	> HTTP packet parser
 */

#include <iostream> // Debug >> Remove this

#include <stdlib.h> 
#include <string>
#include <sstream> // Testing HTTPlib::GetValue() template abilities

std::string _HTTP_BUFFER, _HTTP_MESSAGE;

/*	ParseConnection (std::string packet, std::string &message, std::string &buffer)
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
int ParseConnection (std::string packet, std::string &message, std::string &buffer)
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

/*	HTTPlib namespace
 *	
 *	Description: library of core HTTP serializers and parsers
 */
namespace HTTPlib {
	/*	GetPath(std::string message, std::string& path)
	 *
	 *	Description: Extracts path from HTTP header
	 *
	 *	@message[in]: HTTP packet to search
	 *	@path[out]: path extracted from HTTP path
	 *
	 *	@return 0: success
	 *	@return 1: failure
	 */
	int GetPath (std::string message, std::string& path)
	{
		int position = message.find("/");
		if (position == std::string::npos) {
			return 1;
		}
		
		int length = message.find(" ", position+1);
		if (position == std::string::npos) {
			return 1;
		}

		path = message.substr(position, length-position);
		return 0;
	}

	/*	GetValue (std::string message, std::string field, Type &value)
	 *
	 *	Description: Extracts targeted value from HTTP post requests
	 *
	 *	Example: name=Cosby&age=21&city=New%20York
	 *	Usage:
	 *		
	 *		int valueAge, Result;
	 *		std::string valueName, valueCity;
	 *
	 *		Result = GetValue <int> (networkPacket, "name", valueName);
	 *		if (Result != 0) {
	 *			// error
	 *		}
	 *
	 *		Result = GetValue (networkPacket, "city", valueCity);
	 *		if (Result != 0) {
	 *			// error
	 *		}
	 *
	 *		Result = GetValue <std::string> (networkPacket, "name", valueName);
	 *		if (Result != 0) {
	 *			// error
	 *		}
	 *
	 *	@message[in]: HTTP packet to search
	 *	@field[in]: Field to search for
	 *	@DataType[in]: Data type of value to be obtained (optional)
	 *	@field[out]: Value assigned to field
	 *
	 *	@return 0: success
	 *	@return 1: failure
	 */
	template <class DataType>
	int GetValue (std::string message, std::string field, DataType &value)
	{
		int location = message.find(field + "=");
		
		if (location == std::string::npos) {
			return 1;
		}
		
		location += std::string(field + "=").length();
		
		int length = message.find("&", location+1);
			
		std::string buffer = message.substr(location, length-location); 
		std::stringstream ss;
		ss << buffer;
		ss >> value;
		ss.clear();

		return 0;
	}

	/*	CreateHTTPPacket (std::string responseCode,	std::string humanReadable,
	 *		std::string contentType, std::string httpBody, std::string &httpPacket)
	 *
	 *	Description: Creates HTTP packet
	 *
	 *	@responseCode[in]: a response status code that gives the result of the request
	 *	@humanReadable[in]: an English reason phrase describing the status code
	 *	@contentType[in]: MIME-type of the data in the body
	 *	@httpBody[in]: http body to be sent as payload
	 *	@httpPacket[out]: http packet to be sent to client
	 *
	 *	@return 0: success
	 */
	int CreateHTTPPacket (std::string responseCode,	std::string humanReadable,
		std::string contentType, std::string httpBody, std::string &httpPacket)
	{
		std::string buffer;
		std::stringstream ss;
		ss << httpBody.length();
		ss >> buffer;

		httpPacket =
			"HTTP/1.1 " + responseCode + " " + humanReadable + "\r\n"
			"Content-Type: " + contentType + "\r\n"
			"Content-Length: " + buffer + "\r\n\r\n" + httpBody;

		return 0;
	}
}

int main (int argc, char *argv[])
{
	// ParseConnection() test
	ParseConnection("POST /path/file.html HTTP/1.1\r\n", _HTTP_MESSAGE, _HTTP_BUFFER);
	ParseConnection("Content-Type: text/html\r\n", _HTTP_MESSAGE, _HTTP_BUFFER);
	ParseConnection("Content-Length: 39\r\n", _HTTP_MESSAGE, _HTTP_BUFFER);
	ParseConnection("\r\nname=Cosby&age=21&favorite+flavor=flies", _HTTP_MESSAGE, _HTTP_BUFFER);
	ParseConnection("GET 200 OK\r\n", _HTTP_MESSAGE, _HTTP_BUFFER);

	std::cout << ">> Message <<" << std::endl;
	std::cout << _HTTP_MESSAGE << std::endl << std::endl;
	std::cout << ">> Buffer <<" << std::endl;
	std::cout << _HTTP_BUFFER << std::endl << std::endl;

	// GetPath() test
	std::cout << ">> Path <<" << std::endl;
	std::string path;
	HTTPlib::GetPath(_HTTP_MESSAGE, path);
	std::cout << path << std::endl << std::endl;

	// GetValue() test
	std::cout << ">> Value <<" << std::endl;
	std::string valueName;
	HTTPlib::GetValue <std::string> (_HTTP_MESSAGE,"name",valueName);
	int valueAge;
	HTTPlib::GetValue <int> (_HTTP_MESSAGE,"age",valueAge);
	std::cout << "name:" << valueName << std::endl << "age:" << valueAge << std::endl << std::endl;

	// CreateHTTPPacket() test
	std::string httpPacket;
	std::string webpage =
		"<!doctype HTML>"
		"<html>"
		"<head>"
			"<title>Login Page</title>"
		"</head>"
		"<body>"	
			"<form action=\"login_form\" method=\"post\">"
			"Username: <input type=\"text\" name=\"username\"><br>"
			"Password: <input type=\"password\" name=\"password\"><br>"
			" <input type=\"submit\" value=\"Submit\">"
			"</form>"
		"</body>"
		"</html>";

	HTTPlib::CreateHTTPPacket("200","OK","text/html",webpage, httpPacket);

	std::cout << ">> HTTP Packet <<" << std::endl;
	std::cout << httpPacket << std::endl;

	return 0;
}
