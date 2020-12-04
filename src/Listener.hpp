/**
 * @file Listener.hpp
 * @author paul
 * @date 31.12.18
 * @brief Implements a simple generic listener with multiple receivers
 */

#ifndef WEBSOCKETCPP_LISTENER_HPP
#define WEBSOCKETCPP_LISTENER_HPP

#include <list>
#include <functional>

namespace websocketcpp::util {
    /**
     * Implements a generic event listener with arbitrary messages.
     * @tparam Args the type(s) of the message
     */
    template<typename ...Args>
    class Listener {
    public:
        /**
         * Subscribe to the listener
         * @param listener a functor which should get called
         */
        template<typename T>
        void subscribe(T &&listener) const;

        /**
         * Call all listeners
         * @param args the arguments with which to call.
         */
        void operator()(Args... args) const;

        /**
         * Get the number of subscribed listeners
         */
        auto getSubscribed() const -> std::size_t;

        /**
         * Type of the required functor.
         */
        using type = std::function<void(Args...)>;
    private:
        mutable std::list<std::function<void(Args...)>> listeners;
    };

    template<typename... Args>
    void Listener<Args...>::operator()(Args... args) const {
        for (const auto &listener : this->listeners) {
            listener.operator()(args...);
        }
    }

    template<typename... Args>
    auto Listener<Args...>::getSubscribed() const -> std::size_t {
        return this->listeners.size();
    }

    template<typename... Args>
    template<typename T>
    void Listener<Args...>::subscribe(T &&listener) const {
        this->listeners.emplace_back(std::forward<T>(listener));
    }
}

#endif
