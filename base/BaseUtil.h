#ifndef _BASE_UTIL_H_
#define _BASE_UTIL_H_

#include "AsyncLogging.h"
#include "BaseConn.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketOps.h"
#include "StringOps.h"
#include "FileOps.h"
#include "CurrentThread.h"
#include "ConfigReader.h"
#include "TimeStamp.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "ThreadPool.h"
#include "Buffer.h"
#include "ConnsMap.h"

#define NOTUSED_ARG(v) ((void)v)		// used this to remove warning C4100, unreferenced parameter
#define ASSERT_ABORT(c) if(!(c)) { LOG_FATAL("program abort"); abort();}
#define MAX_VALUE(a,b) (a > b? a: b)
#define MIN_VALUE(a,b) (a < b? a: b)

#if 1

#define LOG_RAW(fmt, args...)   getLogger().append(MakeLoggerPtr(fmt, ##args))
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

/*
  write the instance info to the server.pid file

  @param id the server id
  @return hostname the server hostname
*/
void writeinfo(uint32_t id, const char * hostname);

/*
  get the log instance
*/
AsyncLogging & getLogger();


#endif // _BASE_UTIL_H_
