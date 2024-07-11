#ifndef CALLBACK_MANAGER_H
#define CALLBACK_MANAGER_H

#include <functional>
#include <vector>

template <typename... Args>
class CallbackManager
{
public:
    using Callback = std::function<void(Args...)>;

    void registerCallback(const Callback &callback);
    void invoke(Args... args) const;

private:
    std::vector<Callback> callbacks;
};

template<typename... Args>
void CallbackManager<Args...>::registerCallback(const Callback& callback)
{
    callbacks.push_back(callback);
}

template<typename... Args>
void CallbackManager<Args...>::invoke(Args... args) const
{
    for (const auto& callback : callbacks)
    {
        callback(args...);
    }
}

#endif