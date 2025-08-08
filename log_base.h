#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace log {
    enum LogLevel {
        ERROR = 0,
        WARN,
        INFO,
        DEBUG,
        FUCK
    };

    inline std::string level_to_str(const LogLevel level) {
        switch (level) {
            case ERROR: {
                return "ERROR";
            };
            case WARN: {
                return "WARN";
            };
            case INFO: {
                return "INFO";
            };
            case DEBUG: {
                return "DEBUG";
            };
            default:
            case FUCK: {
                return "FUCK";
            };
        }
    }

    struct LogRecord {
        std::string message;
        LogLevel level;
        time_t time;
        uint32_t id;
    };

    class Formatter {
    public:
        virtual ~Formatter() = default;

        virtual std::string format(LogRecord record) = 0;
    };

    class Handler {
    public:
        virtual ~Handler() = default;

        virtual void publish(LogRecord record) = 0;
    };

    class Filter {
    public:
        virtual ~Filter() = default;

        virtual bool isLogable(LogRecord record) = 0;
    };

    class Logger {
    public:
        virtual ~Logger() = default;

        virtual void log(std::string message, LogLevel level) = 0;
        virtual void debug(std::string message) = 0;
        virtual void info(std::string message) = 0;
        virtual void warn(std::string message) = 0;
        virtual void error(std::string message) = 0;
    };

    class LoggerConfig {
        public:
            LogLevel level;
            Formatter *formatter = nullptr;
            std::vector<Filter*> filters;
            std::vector<Handler*> handlers;

            LoggerConfig(LogLevel level, Formatter *formatter, std::vector<Filter*> filters, std::vector<Handler*> handlers):
                level(level),
                formatter(formatter),
                filters(std::move(filters)),
                handlers(std::move(handlers)) {};

            LoggerConfig(const LoggerConfig&) = delete;
            LoggerConfig &operator=(const LoggerConfig&) = delete;

            LoggerConfig(LoggerConfig&& other) noexcept:
                level(other.level),
                formatter(std::exchange(other.formatter, nullptr)),
                filters(std::move(other.filters)),
                handlers(std::move(other.handlers)) {};

            ~LoggerConfig() {
                clearResources();
            }

            LoggerConfig& operator=(LoggerConfig&& other) noexcept {
                if (this != &other) {
                    clearResources();

                    level = other.level;
                    formatter = std::exchange(other.formatter, nullptr);
                    filters = std::move(other.filters);
                    handlers = std::move(other.handlers);
                }
                return *this;
            }

            static class Builder {
                private:
                    LogLevel _level = INFO;
                    Formatter *_formatter = nullptr;
                    std::vector<Filter*> _filters;
                    std::vector<Handler*> _handlers;
                public:
                    Builder() = default;
                    Builder(const Builder&) = delete;
                    Builder &operator=(const Builder&) = delete;

                    Builder &level(const LogLevel level) {
                        _level = level;
                        return *this;
                    }

                    ~Builder() {
                        this->clearResources();
                    }

                    Builder &formatter(Formatter *formatter) {
                        if (formatter != nullptr && _formatter != formatter) {
                            if (_formatter != nullptr) {
                                delete _formatter;
                                _formatter = nullptr;
                            }
                            _formatter = formatter;
                        }
                        return *this;
                    }

                    Builder &addFilter(Filter *filter) {
                        if (filter != nullptr) {
                            _filters.push_back(filter);
                        }
                        return *this;
                    }

                    Builder &addHandler(Handler *handler) {
                        if (handler != nullptr) {
                            _handlers.push_back(handler);
                        }
                        return *this;
                    }

                    LoggerConfig build() {
                        if (_formatter == nullptr) {
                            class DefaultFormatter: public Formatter {
                                std::string format(LogRecord record) override {
                                    return "["+ level_to_str(record.level) +"]: " + record.message;
                                }
                            };

                            _formatter =  new DefaultFormatter();
                        }
                        return LoggerConfig(
                            _level,
                            std::exchange(_formatter, nullptr),
                            std::move(_filters),
                            std::move(_handlers)
                        );
                    }
                private:
                    void clearResources() {
                        if (_formatter != nullptr) {
                            delete _formatter;
                            _formatter = nullptr;
                        }

                        for (Filter *filter : _filters) {
                            delete filter;
                        }
                        _filters.clear();

                        for (Handler *handler : _handlers) {
                            delete handler;
                        }
                        _handlers.clear();
                    }
            };
        private:
            void clearResources() {
                if (formatter != nullptr) {
                    delete formatter;
                    formatter = nullptr;
                }

                for (Filter *filter : filters) {
                        delete filter;
                }
                filters.clear();

                for (Handler *handler : handlers) {
                        delete handler;
                }
                handlers.clear();
            }
    };
}
