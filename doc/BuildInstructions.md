<!--
SPDX-FileCopyrightText: Leica Geosystems AG

SPDX-License-Identifier: MIT
-->

# Build instructions of saam

Install with C++23 enabled.
```
conan install . 0.1.0@sgo/stable -pr:h sgo\x86_64-windows-vs2022-debug -pr:b sgo\x86_64-windows-vs2022-release -b missing -s compiler.cppstd=23 -o bc_mode=tracked
```