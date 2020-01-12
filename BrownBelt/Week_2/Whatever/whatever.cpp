#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <memory>

int main() {

    auto deleter = [] (int* ptr) {
        std::cout << "std::unique_ptr deleted " << *ptr << std::endl;
        delete ptr;
    };
    std::unique_ptr<int, decltype(deleter)> smart_int(new int{5}, deleter);
    std::shared_ptr<int> shared_int = std::unique_ptr<int, decltype(deleter)> {new int{6}, deleter};
    std::cout << *smart_int << std::endl;
    std::cout << *shared_int << std::endl;
}