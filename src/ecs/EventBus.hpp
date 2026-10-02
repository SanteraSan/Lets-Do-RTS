#pragma once

#include <functional>
#include <utility>
#include <vector>

template <typename Event>
struct EventBus {
    std::vector<std::function<void(const Event&)>> listeners;
};

template <typename Event>
void subscribe(EventBus<Event>& bus, std::function<void(const Event&)> listener) {
    bus.listeners.push_back(std::move(listener));
}

template <typename Event>
void publish(const EventBus<Event>& bus, const Event& event) {
    for (const std::function<void(const Event&)>& listener : bus.listeners) {
        listener(event);
    }
}
