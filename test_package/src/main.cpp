// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/version.hpp>

#include <iostream>

int main()
{
    using namespace saam;
    std::cout << "saam version:" << version::major() << '.' << version::minor() << '.'<< version::patch() << std::endl;
    return 0;
}
