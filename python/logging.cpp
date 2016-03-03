#include "logging.h"

namespace surfrec {

logger::LogChannel pylog("pylog", "[maxflow] ");

logger::LogLevel getLogLevel() {
	return logger::LogManager::getGlobalLogLevel();
}

void setLogLevel(logger::LogLevel logLevel) {
	logger::LogManager::setGlobalLogLevel(logLevel);
}

} // namespace surfrec
