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

#define NOTUSED_ARG(v) ((void)v)		// used this to remove warning C4100, unreferenced parameter
#define ASSERT_ABORT(c) if(!(c)) exit(-1);

extern AsyncLogging g_baselog;

#if 1

#define LOG_TRACE(fmt, args...)  if(g_baselog.getLogLevel() <= Logger::TRACE) g_baselog.append(MakeLoggerPtr(Logger::TRACE, __FILE__, __LINE__, __FUNCTION__, fmt, ##args))
#define LOG_DEBUG(fmt, args...)  if(g_baselog.getLogLevel() <= Logger::DEBUG) g_baselog.append(MakeLoggerPtr(Logger::DEBUG, __FILE__, __LINE__, __FUNCTION__, fmt, ##args))
#define LOG_INFO(fmt, args...)   if(g_baselog.getLogLevel() <= Logger::INFO) g_baselog.append(MakeLoggerPtr(Logger::INFO, __FILE__, __LINE__, __FUNCTION__, fmt, ##args))
#define LOG_WARN(fmt, args...)   g_baselog.append(MakeLoggerPtr(Logger::WARN, __FILE__, __LINE__, __FUNCTION__, fmt, ##args))
#define LOG_ERROR(fmt, args...)  g_baselog.append(MakeLoggerPtr(Logger::ERROR, __FILE__, __LINE__, __FUNCTION__, fmt, ##args))
#define LOG_FATAL(fmt, args...)  g_baselog.append(MakeLoggerPtr(Logger::FATAL, __FILE__, __LINE__, __FUNCTION__, fmt, ##args))

#else

#define LOG_TRACE(fmt, args...)
#define LOG_DEBUG(fmt, args...)
#define LOG_INFO(fmt, args...)
#define LOG_WARN(fmt, args...)
#define LOG_ERROR(fmt, args...)
#define LOG_FATAL(fmt, args...)

#endif // 0

void writePid();

#endif // _BASE_UTIL_H_
