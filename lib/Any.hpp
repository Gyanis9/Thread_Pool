//
// Created by guo on 24-12-2.
//

#ifndef THREAD_POOL_ANY_HPP
#define THREAD_POOL_ANY_HPP

#include <memory>
#include <iostream>

class Any {
public:
    Any() = default;

    ~Any() = default;

    Any &operator=(const Any &) = delete;

    Any(const Any &) = delete;

    Any &operator=(Any &&) = default;

    Any(Any &&) = default;

    template<typename T>
    Any(T data):m_base(std::make_unique<Derive < T>>

    (data)){}

    template<typename T>
    T cast_() {
        Derive <T> *pd = dynamic_cast<Derive <T> *>(m_base.get());
        if (pd == nullptr) {
            throw "type is not match!";
        }
        return pd->m_data;
    }


private:
    class Base {
    public:
        virtual ~Base() = default;
    };

    template<typename T>
    class Derive : public Base {
    public:
        explicit Derive(T data) : m_data(data) {}

        ~Derive() override = default;

        T m_data;
    };

private:
    std::unique_ptr<Base> m_base;

};


#endif //THREAD_POOL_ANY_HPP
