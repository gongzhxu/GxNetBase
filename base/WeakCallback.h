#ifndef _WEAK_CALLBACK_H_
#define _WEAK_CALLBACK_H_

#include <memory>
#include <functional>

template<typename T>
class WeakCallback
{
public:

    WeakCallback(const std::function<void (T*)>& function,
                 const std::weak_ptr<T>& object):
        _function(function),
        _object(object)
    {
    }

    void operator()() const
    {
        std::shared_ptr<T> pObj(_object.lock());
        if(pObj)
        {
            m_function(pObj);
        }
    }

private:
    std::function<void (T*)> _function;
    std::weak_ptr<T> _object;
};

template<typename T>
WeakCallback<T> makeWeakCallback(void (T::*function)(),
                                     const std::shared_ptr<T>& object)
{
    return WeakCallback<T>(function, object);
}

template<typename T>
WeakCallback<T> makeWeakCallback(void (T::*function)() const,
                                     const std::shared_ptr<T>& object)
{
    return WeakCallback<T>(function, object);
}

#endif
