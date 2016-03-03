#include "logging.h"

namespace _maxflow {

logger::LogChannel pylog("pylog", "[maxflow] ");

logger::LogLevel getLogLevel() {
	return logger::LogManager::getGlobalLogLevel();
}

void setLogLevel(logger::LogLevel logLevel) {
	logger::LogManager::setGlobalLogLevel(logLevel);
}

} // namespace _maxflow
