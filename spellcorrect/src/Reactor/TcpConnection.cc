///
/// @file    TcpConnection.cc
///

#include "TcpConnection.h"
#include "EpollPoller.h"
#include <string.h>
#include <stdio.h>


namespace lwh
{

	TcpConnection::TcpConnection(int sockfd,EpollPoller *loop)
	: sockfd_(sockfd)
	, sockIO_(sockfd)
	, localAddr_(lwh::Socket::getLocalAddr(sockfd))
	, peerAddr_(lwh::Socket::getPeerAddr(sockfd))
	, isShutdownWrite_(false)
	,loop_(loop)  
	{
		sockfd_.nonblock();
	}


	TcpConnection::~TcpConnection()
	{
		if(!isShutdownWrite_)
		{
			isShutdownWrite_ = true;
			shutdown();
		}
		printf("~TcpConnection()\n");
	}

	std::string TcpConnection::receive()
	{
		char buf[65536];
		memset(buf, 0, sizeof(buf));
		size_t ret = sockIO_.readline(buf, sizeof(buf));
		if(ret == 0)
		{
			return std::string();
		}
		else
			return std::string(buf);
	}

	void TcpConnection::send(const std::string & msg)
	{
		sockIO_.writen(msg.c_str(), msg.size());
	}


	void TcpConnection::shutdown()
	{
		if(!isShutdownWrite_)
			sockfd_.shutdownWrite();
		isShutdownWrite_ = true;
	}

	std::string TcpConnection::toString()
	{
		char str[100];
		snprintf(str, sizeof(str), "%s:%d -> %s:%d",
				localAddr_.ip().c_str(),
				localAddr_.port(),
				peerAddr_.ip().c_str(),
				peerAddr_.port());
		return std::string(str);
	}
	
	//注册和执行回调函数
	void TcpConnection::setConnectionCallback(TcpConnectionCallback cb)
	{
		onConnectionCb_ = cb;
	}

	void TcpConnection::setMessageCallback(TcpConnectionCallback cb)
	{
		onMessageCb_ = cb;
	}

	void TcpConnection::setCloseCallback(TcpConnectionCallback cb)
	{
		onCloseCb_ = cb;
	}

	void TcpConnection::handleConnectionCallback()
	{
		if(onConnectionCb_)
			onConnectionCb_(shared_from_this());
	}

	void TcpConnection::handleMessageCallback()
	{
		if(onMessageCb_)
			onMessageCb_(shared_from_this());
	}

	void TcpConnection::handleCloseCallback()
	{
		if(onCloseCb_)
			onCloseCb_(shared_from_this());
	}

	void TcpConnection::sendInLoop(const std::string & msg)
	{
		loop_ -> runInLoop(std::bind(&TcpConnection::send,this,msg));
	}
}// end of namespace lwh
