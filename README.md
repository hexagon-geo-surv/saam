<!--
SPDX-FileCopyrightText: Leica Geosystems AG

SPDX-License-Identifier: MIT
-->

# saam - Smart Aliasing and Mutability

Smart pointers give extra safety around raw C++ pointers. This library aims to improve
the raw C++ references (&) and STL mutexes (std::mutex) with safety guarantees.

## Smart reference
As smart pointers smartens up raw C++ pointers, this library smartens up the raw C++ references.
Smart references express aliasing (non-ownership, borrowing) relation opposed to ownership expressed by smart pointers.
With the extra provided safety, dangling reference situations can be detected during runtime.

## Smart mutexing
STL `std::mutex` type and its variants are not coupled with the data they protect,
so it is possible to access the data without a locked mutex.
Smart mutex encapsulates the protected data members.
No access to the data is possible by bypassing the protection of the mutex.

This smart mutex implementation also extends the encapsulation to the condition variables.

[User manual](doc/UserManual.md)

# Contributing

We are happy to receive improvement ideas, bug reports, fixes for this repository.

[Contribution rules](doc/Contributing.md)

[Reporting vulnerabilities](SECURITY.md)

For design improvement ideas, questions, please use the "Discussions" tab.

# Legal

[MIT License](LICENSES/MIT.txt)
