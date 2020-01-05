#include <iostream>
#include <string>


int main() {

    enum class Color { red = 0, white, black };
    Color Red = Color::red;

    std::cout << static_cast<int>(Color::red) << std::endl;
    return 0;

}