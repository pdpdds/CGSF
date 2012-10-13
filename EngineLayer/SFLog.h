#pragma once

#define SFLOG_TRACE(logger, expression) LOG4CXX_TRACE(logger, expression)
#define SFLOG_DEBUG(logger, expression) LOG4CXX_DEBUG(logger, expression)
#define SFLOG_INFO(logger, expression) LOG4CXX_INFO(logger, expression)
#define SFLOG_WARN(logger, expression) LOG4CXX_WARN(logger, expression)
#define SFLOG_ERROR(logger, expression) LOG4CXX_ERROR(logger, expression)
#define SFLOG_FATAL(logger, expression) LOG4CXX_FATAL(logger, expression)