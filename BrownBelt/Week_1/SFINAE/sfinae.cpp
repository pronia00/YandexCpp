#include <iostream>

struct A {};

std::string to_string(const A&)
{
    return "I am a A!";
}

// Type B with a serialize method.
struct B
{
    std::string serialize() const
    {
        return "I am a B!";
    }
};



// Type C with a "wrong" serialize member (not a method) and a to_string overload.
struct C
{
    std::string serialize;
};

std::string to_string(const C&)
{
    return "I am a C!";
}

struct D : A
{
    std::string serialize() const
    {
        return "I am a D!";
    }
};

struct E
{
    struct Functor
    {
        std::string operator()(){
            return "I am a E!";
        }
    };

    Functor serialize;
};

// Primary template, inherit from std::false_type.
// ::value will return false.
// Note: the second unused template parameter is set to default as std::string!!!


auto l1 = [](B& b) { return b.serialize(); };
auto l2 = [](B& b) -> std::string { return b.serialize(); };
auto l3 = [](B& b) -> decltype(b.serialize()) { return b.serialize(); };

auto l4 = [](int a, int b) { return a + b; };
struct l4UnnamedType
{
    int operator() (int a, int b) {
        return a + b;
    }
};


// lambda l5 is equivalent to struct l5UnnamedType
auto l5 = [](auto &t) -> decltype(t.serialize()) { return t.serialize(); };
struct l5UnnamedType
{
    template <typename T> 
    auto operator() (T& t) -> decltype(t.serialize())  
    {
        return t.serialize(); 
    }
};


template <typename UnnamedType>  struct container
{
private: 

    template <typename Param> constexpr auto TestValidity(int ) 
    -> decltype(std::declval<UnnamedType>()(std::declval<Param>()), std::true_type())
    {
        return std::true_type();
    }

    template <typaname Param> constexpr auto TestValidity(...) 
    {
        return std::false_type();
    }

public: 

    template <typename Param> constexpr auto operator()(const Param& p) {
        return TestValidity<Param>(int());
    }

};

template <typename UnnamedType> constexpr auto is_valid(const UnnamedType& t) 
{
    return container<UnnamedType>();    
}

auto hasSerialize = is_valid([](auto&& x) -> decltype(x.serialize()) {});
auto test = is_valid([] const auto& t) -> decltype(t.serialize()) {});

template <class T> auto serialize(T& obj) 
-> typename std::enable_if<decltype(hasSerialize(obj))::value, std::string>::type
{
    return obj.serialize();
}

template <class T> auto serialize(T& obj) 
-> typename std::enable_if<!decltype(hasSerialize(obj))::value, std::string>::type
{
    return to_string(obj);
}

int main() {
    // A a;
    // B b;
    // C c;
    // D d;
    // E e;

    // std::cout << serialize(a) << std::endl;
    // std::cout << serialize(b) << std::endl;
    // std::cout << serialize(c) << std::endl;
    // std::cout << serialize(d) << std::endl;
    // std::cout << serialize(e) << std::endl;
    // std::cout << serialize(f) << std::endl;

    return 0;
}