#include <iostream>
#include "event_bus.h"

using namespace std;

EventBus e;

struct T_Event: EventBus::Event {
    string value;

    T_Event() = default;

    explicit T_Event(string str): value(str) {};
};

int main() {

    auto h0 = e.subscribe<T_Event>("test", [](const T_Event &event) {
        cout << "value: " + event.value << endl;
    });

    e.publish<T_Event>("test", T_Event("abab"));

    return 0;
}