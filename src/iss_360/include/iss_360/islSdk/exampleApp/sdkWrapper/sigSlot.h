#ifndef SIGSLOT_H_
#define SIGSLOT_H_

//------------------------------------------ Includes ----------------------------------------------

#include <functional>
#include <vector>

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
    template<typename... Args>
    class Signal;

    template<typename... Args>
    class Slot
    {
    private:
        std::function<void(Args...)> callback;
        std::vector<Signal<Args...>*> sigArray;

        void remove(Signal<Args...>* sig)
        {
            for (size_t i = 0; i < sigArray.size(); i++)
            {
                if (sigArray[i] == sig)
                {
                    sigArray.erase(sigArray.begin() + i);
                    break;
                }
            }
        }

    public:
        template<typename T>
        Slot(T* inst, void (T::* func)(Args...)) : callback([=](Args... args) {(inst->*func)(args...); }) {}
        Slot(std::function<void(Args...)> func) : callback(func) {}

        ~Slot()
        {
            while (sigArray.size())
            {
                sigArray[0]->disconnect(*this);
            }
        }

        void operator()(Args... p)
        {
            callback(p...);
        }
        friend class Signal<Args...>;
    };

#ifndef PYTHON_WRAPPER

    template<typename... Args>
    class Signal
    {
    private:
        std::vector<Slot<Args...>*> slotArray;
        std::function<void(uint32_t)> callback;

        int32_t find(const Slot<Args...>& slot)
        {
            for (uint32_t i = 0; i < slotArray.size(); i++)
            {
                if (slotArray[i] == &slot)
                {
                    return i;
                }
            }
            return -1;
        }

        bool_t addSlot(Slot<Args...>& slot)
        {
            if (find(slot) < 0)
            {
                slot.sigArray.push_back(this);
                slotArray.push_back(&slot);

                if (callback != nullptr)
                {
                    callback(slotArray.size());
                }
            }
            return false;
        }

    public:

        // default constructor
        constexpr Signal() : callback(nullptr) {}
        template<typename T>  Signal(T* inst, void (T::* func)(uint32_t)) : callback([=](uint32_t count) {(inst->*func)(count); }) {}
        Signal(std::function<void(uint32_t)> func) : callback(func) {}

        // copy/move constructor
        Signal(const Signal&) = delete;
        Signal(Signal&&) noexcept = delete;

        // copy/move assignment
        Signal& operator=(const Signal&) = delete;
        Signal& operator=(Signal&&) noexcept = delete;

        ~Signal()
        {
            for (size_t i = 0; i < slotArray.size(); i++)
            {
                slotArray[i]->remove(this);
            }
        }

        template<typename T> void setSubscribersChangedCallback(T* inst, void (T::* func)(uint32_t))
        {
            callback = ([=](uint32_t count) {(inst->*func)(count); });
        }

        void setSubscribersChangedCallback(void (*func)(uint32_t))
        {
            callback = func;
        }

        void connect(Slot<Args...>& slot)
        {
            addSlot(slot);
        }

        void disconnect(Slot<Args...>& slot)
        {
            int32_t i = find(slot);

            if (i >= 0)
            {
                slotArray[i]->remove(this);
                slotArray.erase(slotArray.begin() + i);

                if (callback != nullptr)
                {
                    callback(slotArray.size());
                }
            }
        }

        bool_t hasSubscribers(void)
        {
            return slotArray.size() != 0;
        }

        void operator()(Args... p)
        {
            for (size_t i = 0; i < slotArray.size(); i++)
            {
                (*slotArray[i])(p...);
            }
        }
    };

#else

    class BaseSignal
    {
    public:
        BaseSignal(std::function<void(uint32_t)> func) : callback(func), callbackEnable(false) {}
        ~BaseSignal() {}
        std::function<void(uint32_t)> callback;
        bool_t callbackEnable;

        void pyEnableCallback(bool_t enable)
        {
            callbackEnable = enable;

            if (callback != nullptr)
            {
                callback(callbackEnable);
            }
        }
    };

    template<typename... Args>
    class Signal : public BaseSignal
    {
    private:
        std::vector<Slot<Args...>*> slotArray;
        void* pyObj;
        void(*pyCallback)(void* pyObj, Args...);

        int32_t find(Slot<Args...>& slot)
        {
            for (size_t i = 0; i < slotArray.size(); i++)
            {
                if (slotArray[i] == &slot)
                {
                    return i;
                }
            }
            return -1;
        }

        bool_t addSlot(Slot<Args...>& slot)
        {
            if (find(slot) < 0)
            {
                slot.sigArray.push_back(this);
                slotArray.push_back(&slot);

                if (callback != nullptr)
                {
                    callback(slotArray.size() || callbackEnable);
                }
            }
            return false;
        }

    public:

        // default constructor
        constexpr Signal() :  pyObj(nullptr), pyCallback(nullptr), BaseSignal(nullptr) {}
        template<typename T>  Signal(T* inst, void (T::* func)(uint32_t)) : pyObj(nullptr), pyCallback(nullptr), BaseSignal([=](uint32_t count) {(inst->*func)(count); }) {}
        Signal(std::function<void(uint32_t)> func) : pyObj(nullptr), pyCallback(nullptr), BaseSignal(func) {}

        // copy/move constructor
        Signal(const Signal&) = delete;
        Signal(Signal&&) noexcept = delete;

        // copy/move assignment
        Signal& operator=(const Signal&) = delete;
        Signal& operator=(Signal&&) noexcept = delete;

        ~Signal()
        {
            for (size_t i = 0; i < slotArray.size(); i++)
            {
                slotArray[i]->remove(this);
            }
        }

        template<typename T> void setSubscribersChangedCallback(T* inst, void (T::* func)(uint32_t))
        {
            callback = ([=](uint32_t count) {(inst->*func)(count); });
        }

        void setSubscribersChangedCallback(void (*func)(uint32_t))
        {
            callback = func;
        }

        void connect(Slot<Args...>& slot)
        {
            addSlot(slot);
        }

        void disconnect(Slot<Args...>& slot)
        {
            int32_t i = find(slot);

            if (i >= 0)
            {
                slotArray[i]->remove(this);
                slotArray.erase(slotArray.begin() + i);

                if (callback != nullptr)
                {
                    callback(slotArray.size() || callbackEnable);
                }
            }
        }

        bool_t hasSubscribers(void)
        {
            return slotArray.size() != 0 || callbackEnable;
        }

        void pySetCallback(void* obj, void(*func)(void* pyObj, Args...))
        {
            pyObj = obj;
            pyCallback = func;
        }

        void operator()(Args... p)
        {
            for (size_t i = 0; i < slotArray.size(); i++)
            {
                (*slotArray[i])(p...);
            }

            if (callbackEnable && pyCallback)
            {
                pyCallback(pyObj, p...);
            }
        }
    };
#endif
}
//--------------------------------------------------------------------------------------------------
#endif
