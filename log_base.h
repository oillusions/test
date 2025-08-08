#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace olog {
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

        virtual std::string format(const LogRecord &record) = 0;
    };

    class Handler {
    public:
        virtual ~Handler() = default;

        virtual void publish(const LogRecord &record) = 0;
    };

    class Filter {
    public:
        virtual ~Filter() = default;

        virtual bool isLogable(const LogRecord &record) = 0;
    };

    class Logger {
    public:
        virtual ~Logger() = default;

        virtual void log(const std::string &message, LogLevel level) = 0;
        virtual void debug(const std::string &message) {
            log(message, DEBUG);
        }
        virtual void info(const std::string &message) {
            log(message, INFO);
        }
        virtual void warn(const std::string &message) {
            log(message, WARN);
        }
        virtual void error(const std::string &message) {
            log(message, ERROR);
        }
    };

    class LoggerConfig {
        public:
            LogLevel level;
            std::unique_ptr<Formatter> formatter = nullptr;
            std::vector<std::unique_ptr<Filter>> filters;
            std::vector<std::unique_ptr<Handler>> handlers;

            LoggerConfig(LogLevel level,
                std::unique_ptr<Formatter> formatter,
                std::vector<std::unique_ptr<Filter>> filters,
                std::vector<std::unique_ptr<Handler>> handlers):
                    level(level),
                    formatter(std::move(formatter)),
                    filters(std::move(filters)),
                    handlers(std::move(handlers)){};

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

            class Builder
            {
              private:
                LogLevel _level = INFO;
                std::unique_ptr<Formatter> _formatter = nullptr;
                std::vector<std::unique_ptr<Filter>> _filters;
                std::vector<std::unique_ptr<Handler>> _handlers;

              public:
                Builder() = default;
                Builder(const Builder &) = delete;
                Builder &operator=(const Builder &) = delete;

                Builder &level(const LogLevel level)
                {
                    _level = level;
                    return *this;
                }

                ~Builder()
                {
                    this->clearResources();
                }

                Builder &formatter(std::unique_ptr<Formatter> formatter)
                {
                    _formatter = std::move(formatter);
                    return *this;
                }

                Builder &addFilter(std::unique_ptr<Filter> filter)
                {
                    _filters.push_back(std::move(filter));
                    return *this;
                }

                Builder &addHandler(std::unique_ptr<Handler> handler)
                {
                    _handlers.push_back(std::move(handler));
                    return *this;
                }

                LoggerConfig build()
                {
                    if (_formatter == nullptr)
                    {
                        class DefaultFormatter : public Formatter
                        {
                            std::string format(const LogRecord &record) override
                            {
                                return  "[" + olog::level_to_str(record.level) + "]: " + record.message;
                            }
                        };

                        _formatter = std::make_unique<DefaultFormatter>();
                    }
                    return {_level, std::exchange(_formatter, nullptr), std::move(_filters),
                                        std::move(_handlers)};
                }

              private:
                void clearResources()
                {
                    _formatter.reset();
                    _filters.clear();
                    _handlers.clear();
                }
            };

        private:
            void clearResources() {
                formatter.reset();
                filters.clear();
                handlers.clear();
            }
    };
}
