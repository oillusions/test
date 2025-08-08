#include <iostream>
#include <memory>
#include <thread>

#include "event_bus.h"
#include "log.h"

using namespace std;
using namespace olog;

EventBus e;
unique_ptr<Logger> tlog;

struct T_Event: EventBus::Event {
    string value;

    T_Event() = default;

    explicit T_Event(string str): value(str) {};
};



int main() {
    tlog = make_unique<ExampleLogger>(LoggerConfig::Builder()
        .level(FUCK)
        .formatter(make_unique<ExampleFormatter>())
        .addFilter(make_unique<LevelFilter>(FUCK))
        .addHandler(make_unique<ExampleHandler>())
        .build());
    auto h0 = e.subscribe<T_Event>("test", [](const T_Event &event) {
        tlog->log("value: " + event.value, FUCK);
        tlog->log("value: " + event.value, INFO);
        // cout << "value: " + event.value << endl;
    });

    vector<thread> threads = vector<thread>();
    for (uint32_t i = 0; i < 10; i++) {
        threads.emplace_back([i] {
            e.publish<T_Event>("test", T_Event("abab " + to_string(i * i)));
        });
    }

    for (thread &t : threads) {
        t.join();
    }

    return 0;
}