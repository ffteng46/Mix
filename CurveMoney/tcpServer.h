#ifndef TCPSERVER_H
#define TCPSERVER_H
#pragma once
#include <boost/asio.hpp>
void simpleAsamble(char *ch);
int startTCPServer(void);
int tradeEngineWriter(boost::shared_ptr<boost::asio::ip::tcp::socket> _socket);
int tradeEngineReader(boost::shared_ptr<boost::asio::ip::tcp::socket> _socket);

#endif // TCPSERVER_H
