#pragma once
#include <cstdint>
#include <functional>


class EventBus {
    public:
        class Event {
            public:
                virtual ~Event() = default;
        };

        class Subscription {
            private:
                friend class EventBus;
                EventBus* _bus;
                const std::string _eventID;
                const uint32_t _handlerID;
            public:
                Subscription(EventBus* bus, std::string eventID, uint32_t handlerID): _bus(bus), _eventID(eventID), _handlerID(handlerID) {};
                Subscription(const Subscription&) = delete;
                Subscription& operator=(const Subscription&) = delete;

                ~Subscription() {
                    if (_bus != nullptr) {
                        unsubscribe();
                    }
                }

                void unsubscribe() {
                    if (_bus != nullptr) {
                        auto it = _bus->_handlers.begin();
                        while (it != _bus->_handlers.end()) {
                            if (it->handlerID == _handlerID) {
                                delete it->handler;
                                _bus->_handlers.erase(it);
                                _bus = nullptr;
                                break;
                            }
                            ++it;
                        }
                        _bus = nullptr;
                    }
                }
        };


        template<typename T>
        Subscription subscribe(std::string eventID, std::function<void(const T&)> callback) {
            uint32_t handlerID = _handlerIDCounter++;
            _handlers.push_back(HandlerRecord(eventID, handlerID, new Handler<T>(callback)));
            return {this, eventID, handlerID};
        }

        template<typename T>
        void publish(std::string eventID, T&& event) {
            std::vector<HandlerBase*> handlersToCall;
            for (const HandlerRecord& record : _handlers) {
                if (record.eventID == eventID) {
                    handlersToCall.push_back(record.handler);
                }
            }
            for (HandlerBase* handler : handlersToCall) {
                handler->invoke(event);
            }
        }

        ~EventBus() {
            for (HandlerRecord record : _handlers) {
                delete record.handler;
            }
            _handlers.clear();
        }

    private:
        struct HandlerBase {
            virtual ~HandlerBase() = default;

            virtual void invoke(const Event &) = 0;
        };
        template<typename T>
        struct Handler: HandlerBase {
            std::function<void(const T&)> func;
            explicit Handler(std::function<void(const T&)> f): func(std::move(f)) {}
            void invoke(const Event &e) override {
                func(static_cast<const T&>(e));
            }
        };

        struct HandlerRecord {
            std::string eventID;
            uint32_t handlerID;
            HandlerBase *handler;

            HandlerRecord(std::string eventID, uint32_t handlerID, HandlerBase* handler): eventID(eventID), handlerID(handlerID), handler(handler) {};
        };

        uint32_t _handlerIDCounter = 0;
        std::vector<HandlerRecord> _handlers = std::vector<HandlerRecord>();
};
