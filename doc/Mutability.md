<!--
SPDX-FileCopyrightText: Leica Geosystems AG

SPDX-License-Identifier: MIT
-->

# Concurrency model
The STL defines concurrency primitives (mutexes, locks, condition variables), but there is no association between these primitives and the resources they protect. `saam` enhances these primitives, just as it does with references.

## Smart mutex

`saam::synchronized` is a bundle of a mutex with the protected object instance.
Similar to `saam::var`, a `saam::synchronized` owns the object it is expected to protect.
There is no direct access to the underlying object; all access goes through the smart mutex infrastructure.
Therefore the undelying object can be only accessed when a lock is acquired on the associated mutex.

Let's create an integer with a bundled mutex.
```cpp
saam::synchronized<int> number(5);
```
Note that the underlying type is never a `const` type. `const` types cannot be mutated and therefore
do not require thread synchronization. Reading is safe from any number of threads without synchronization.

## Guards

Similar to `saam::ref`, smart mutex uses proxy objects to provide access to the underlying type.
`saam::guard` objects are a bundle of a mutex lock with a smart reference to
the protected object.

Create a mutable proxy from the synchronized object. The template parameter being not `const`
indicates that this is a unique lock (there can be only one at a time).
Unique locks allow reading and writing to the underlying object.
```cpp
saam::guard<int> number_mut_guard{number};

// Or using a factory function of synchronized
auto number_mut_guard{number.commence_mut()};
```

Shared guards provide immutable (read-only) access. Multiple such guards may exist at the same time.
There is a factory function for this as well.
```cpp
saam::guard<const int> number_immut_guard{number};
auto number_immut_guard{number.commence()};
```

The `synchronized` instance and its related `guard` instances are bound via smart references. If the
`synchronized` object is destroyed before all its `guard`s are released, `saam` will panic.

When multiple `synchronized` instances shall be guarded at the same time, use the `commence_all` function.
The template parameter list specifies if the commenced guards shall be mutable or immutable.
Using this function prevents deadlocks when trying to acquire multiple guards.

```cpp
saam::synchronized<std::string> text("Hello world");
saam::synchronized<int> number(42);

auto [text_guard, number_guard] = commence_all<const std::string, int>(text, number);
```

The guard owns the lock, but it allows access to it via a reference. This access allowed to
be able to use condition variables with guards. Otherwise do not manipulate the state of the lock,
because the guard will be confused.

```cpp
saam::synchronized<int> synced_num(5);
std::condition_variable_any above_5_condition;

auto guard = synced_m.commence();
above_5_condition.wait(guard.get_lock(), [&guard]() { return *guard > 5; });
```

## Recommended integration into classes

The following case study shows how to synchronize member variables of a class. The example is also extended with another concept,
[Initialization Order Fiasco](https://www.youtube.com/watch?v=KWB-gDVuy_I&t=893s), which is not only useful for the smart mutex,
but also overcomes the constructor initialization list constraints.
[best_practice_test.cpp](../test/test_mutex/src/best_practice_test.cpp)
