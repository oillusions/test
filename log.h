#pragma once
#include <iostream>
#include "log_base.h"

class ExampleFormatter: public olog::Formatter {
    public:
        std::string format(const olog::LogRecord &record) override {
            return  "(" + std::to_string(record.id) + ") [" + olog::level_to_str(record.level) + "]: " + record.message;
        }
};

class ExampleHandler final : public olog::Handler {
    public:
    const olog::LoggerConfig *config{};
        ExampleHandler() = default;
        void publish(const olog::LogRecord &record) override {
            std::cout << config->formatter->format(record) << std::endl;
        }
};

class LevelFilter: public olog::Filter {
    private:
        const olog::LogLevel _level;
    public:
        explicit LevelFilter(const olog::LogLevel level): _level(level) {};
        bool isLogable(const olog::LogRecord &record) override {
            return record.level != _level;
        }
};

class ExampleLogger: public olog::Logger {
    private:
        const olog::LoggerConfig _config;
    public:
        explicit ExampleLogger(olog::LoggerConfig config): _config(std::move(config)) {
            for (auto &handler : _config.handlers) {
                dynamic_cast<ExampleHandler*>(handler.get())->config = &_config;
            }
        };
        void log(const std::string &message, olog::LogLevel level) override {
            static uint32_t counter = 0;
            if (static_cast<uint8_t>(level) > static_cast<uint8_t>(_config.level)) {
                return;
            }
            olog::LogRecord record = {
                .message = message,
                .level = level,
                .time = counter++,
                .id = counter,
            };

            for (auto &filter : _config.filters) {
                if (!filter->isLogable(record)) {
                    counter--;
                    return;
                }
            }
            for (auto &handler : _config.handlers) {
                handler->publish(record);
            }
        }
};

