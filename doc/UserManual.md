<!--
SPDX-FileCopyrightText: Leica Geosystems AG

SPDX-License-Identifier: MIT
-->

# Pillars of Safe Memory Management

<img src="MemorySafety.svg" alt="Stack" style="width:150%;" />

## Ownership
> *Who is going to release the allocated memory?*

Problems to solve:
- No deallocation = memory leak
- Multiple deallocations

Since C++11, C++ has addressed these problems:
- Ownership of scope objects is automatically managed by the compiler via scopes and stack frames.
- Ownership of heap objects was automated by smart pointers.

## Aliasing
> *Who uses the allocated memory?*

The usage of a variable is independent of its ownership. In some cases, we just want to use an object, calling functions on it or accessing its members,
without worrying about its lifetime. We expect that its lifetime is guaranteed by an external mechanism.

Problems to solve:
- Dangling references: make sure that the lifetime assumption of a reference is guaranteed.

C++ does not provide a borrow checking mechanism like Rust. There is no way to tell from a raw reference if it is valid or not.
The [Aliasing](Aliasing.md) model of `saam` offers a reference checking mechanism that resembles the Rust borrow checker.

## Mutability
> *How to use the allocated memory simultaneously?*

Ensure safe access to the objects from multiple threads.

Problems to solve:
- Data races
- Deadlocks

Data races can be solved by enforcing the use of locked mutexes.
The current STL API does not enforce that a mutex is locked when its protected objects are accessed.

The [Mutability](Mutability.md) model of `saam` offers a Rust like association between the mutex and the protected variable(s).

## Bounding
> *How to stay within the bounds of the allocated memory?*

How can we harden our classes, so that only the owned memory is accessed and we do not reach beyond that.

Problems to solve:
- Memory corruption / invalid read: read/write beyond the managed memory

Usage of containers (std::vector, std::map, etc.) instead of raw buffers / arrays helps keeping the access of the memory under control.
For random access of an element in the container shall be done with bound checking.
Sweeping through the container shall be done by iterator chains (std::ranges).

# Smart resource-management tools

| Use case | Owner | Proxy | Destruction |
| -------- | -------- | ------- | ------- |
| Heap object | Heap block | std::shared_ptr, std::unique_ptr | released with last reference |
| Scoped object | saam::var | saam::ref | released with object |
| Synchronized object | saam::synchronized | saam::sentinel | released with object |

For all use cases, the `Owner` itself is just a container, it is not accessed directly. All access go through `Proxy` types.
