#ifndef _BASE_UTIL_H_
#define _BASE_UTIL_H_

#include "AsyncLogging.h"
#include "BaseConn.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketOps.h"
#include "CurrentThread.h"
#include "ConfigFileReader.h"
#include "TimeStamp.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "ThreadPool.h"
#include "Buffer.h"

#define NOTUSED_ARG(v) ((void)v)		// used this to remove warning C4100, unreferenced parameter
#define ASSERT_ABORT(c) if(!(c)) { LOG_FATAL("program abort"); exit(EXIT_FAILURE);}

#if 1

#define LOG_TRACE(fmt, args...)  if(getLogger().getLogLevel() <= Logger::TRACE) getLogger().append(MakeLoggerPtr(Logger::TRACE, __FILE__, __LINE__, __FUNCTION__, fmt, ##args))
#define LOG_DEBUG(fmt, args...)  if(getLogger().getLogLevel() <= Logger::DEBUG) getLogger().append(MakeLoggerPtr(Logger::DEBUG, __FILE__, __LINE__, __FUNCTION__, fmt, ##args))
#define LOG_INFO(fmt, args...)   if(getLogger().getLogLevel() <= Logger::INFO) getLogger().append(MakeLoggerPtr(Logger::INFO, __FILE__, __LINE__, __FUNCTION__, fmt, ##args))
#define LOG_WARN(fmt, args...)   getLogger().append(MakeLoggerPtr(Logger::WARN, __FILE__, __LINE__, __FUNCTION__, fmt, ##args))
#define LOG_ERROR(fmt, args...)  getLogger().append(MakeLoggerPtr(Logger::ERROR, __FILE__, __LINE__, __FUNCTION__, fmt, ##args))
#define LOG_FATAL(fmt, args...)  getLogger().append(MakeLoggerPtr(Logger::FATAL, __FILE__, __LINE__, __FUNCTION__, fmt, ##args))

#else

#define LOG_TRACE(fmt, args...)
#define LOG_DEBUG(fmt, args...)
#define LOG_INFO(fmt, args...)
#define LOG_WARN(fmt, args...)
#define LOG_ERROR(fmt, args...)
#define LOG_FATAL(fmt, args...)

#endif // 0

void writePid();
AsyncLogging & getLogger();
void sprintfex(std::string & str, const char * format, ...);

#endif // _BASE_UTIL_H_
