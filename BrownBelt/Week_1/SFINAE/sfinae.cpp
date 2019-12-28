#include <iostream>
#include <iterator> 
#include <string>
#include <typeinfo>
#include <utility>

using namespace std;

template <class T> struct hasSerialize
{
    // For the compile time comparison.
    typedef char yes[1];
    typedef yes no[2];

    // This helper struct permits us to check that serialize is truly a method.
    // The second argument must be of the type of the first.
    // For instance reallyHas<int, 10> would be substituted by reallyHas<int, int 10> and works!
    // reallyHas<int, &C::serialize> would be substituted by reallyHas<int, int &C::serialize> and fail!
    // Note: It only works with integral constants and pointers (so function pointers work).
    // In our case we check that &C::serialize has the same signature as the first argument!
    // reallyHas<std::string (C::*)(), &C::serialize> should be substituted by 
    // reallyHas<std::string (C::*)(), std::string (C::*)() &C::serialize> and work!
    template <typename U, U u> struct reallyHas;

    // Two overloads for yes: one for the signature of a normal method, one is for the signature of a const method.
    // We accept a pointer to our helper struct, in order to avoid to instantiate a real instance of this type.
    // std::string (C::*)() is function pointer declaration.
    template <typename C> static yes& test(reallyHas<int (C::*)(), &C::serialize>* /*unused*/ ) { }
    template <typename C> static yes& test(reallyHas<int (C::*)() const, &C::serialize>* /*unused*/) {}

    // The famous C++ sink-hole.
    // Note that sink-hole must be templated too as we are testing test<T>(0).
    // If the method serialize isn't available, we will end up in this method.
    template <typename> static no& test(...) { /* dark matter */ }
    
    // The constant used as a return value for the test.
    // The test is actually done here, thanks to the sizeof compile-time evaluation.
    static const bool value = sizeof(test<T>(0)) == sizeof(yes);
};

struct S {
    int serialize() {
        return {};
    }
};
    
template <bool B, class T = void> 
struct _enable_if {};

template<class T>
struct _enable_if<true, T> {typedef T type; };


template <class T> typename _enable_if<hasSerialize<T>::value, std::string>::type serialize(const T& obj)
{
    return obj.serialize();
}

template <class T> typename _enable_if<!hasSerialize<T>::value, std::string>::type serialize(const T& obj) 
{
    return to_string(obj);
}

template <typename T> 
struct decTest {
    T type;
};
struct decvTest {
    decvTest(const decvTest&);
    int foo() const { return 1;}
};

int main() {
    std::string s;    
    decltype(s) test = "test";
    std::cout << test << std::endl; 

    double d;
    decltype(d) testd = 10;
    std::cout << testd << std::endl;

    
    decltype(decTest<std::string>().type) testStruct = "testStruct";
    std::cout << testStruct << std::endl;

    decltype(std::declval<decvTest>().foo()) testDecv = 10;
    std::cout << testDecv << std::endl;

        
    // _enable_if<true, int>::type t1;
    // _enable_if<hasSerialize<S>::value, int>::type t2;
    // _enable_if<false, int>::type t3;
    // _enable_if<hasSerialize<A>::value, int>::type t4;
    // std::cout << hasSerialize<int>::value << std::endl;
    // std::cout << hasSerialize<bool>::value << std::endl;
    // std::cout << hasSerialize<S>::value << std::endl;
    return 0;
}