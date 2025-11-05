// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/version.hpp>
#include <saam/safe_ref.hpp>

#include <iostream>

int main()
{
    using namespace saam;
    std::cout << "saam version:" << version::major() << '.' << version::minor() << '.'<< version::patch() << std::endl;

    saam::var<int> smart_variable{42};
    *smart_variable.borrow() = 22;
    std::cout << *smart_variable.borrow() << std::endl;

    return 0;
}
