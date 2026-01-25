<!--
SPDX-FileCopyrightText: Leica Geosystems AG

SPDX-License-Identifier: MIT
-->

# Concurrency model
The STL defines concurrency primitives (mutexes, locks, condition variables), but there is no association between these primitives and the resources they protect. `saam` enhances these primitives, just as it does with references.

## Smart mutex

`saam::synchronized` associates a mutex with the protected object instance.
Similar to `saam::var`, a `saam::synchronized` owns the object it is expected to protect.
There is no direct access to the underlying object; all access goes through the smart mutex infrastructure.

Let's create an integer and bundle it with a mutex.
```cpp
saam::synchronized<int> number(5);
```
Note that the underlying type is never a `const` type. `const` types cannot be mutated and therefore
do not require thread synchronization. Reading is safe from any number of threads without synchronization.

## Guards

Similar to `saam::ref`, smart mutex uses proxy objects to provide access to the underlying type.
These are the `saam::guard` objects.

Create a mutable proxy from the synchronized object. The template parameter being not `const`
indicates that this is a unique lock—there can be only one at a time.
Unique locks allow reading and writing to the underlying object.
```cpp
saam::guard<int> number_mut_guard{number};
```

Alternatively, `saam::synchronized` factory methods can create guards.
This syntax is useful to create temporary guards in a long expression.
```cpp
auto doubled = *number.commence() + *number.commence();
```

Shared guards provide immutable (read-only) access, but multiple such guards may exist at the same time.
There is a factory function for this as well.
```cpp
saam::guard<const int> number_immut_guard{number};
auto number_immut_guard{number.commence()};
```

The `synchronized` instance and its related `guard` instances are bound via smart references. If the
`synchronized` object is destroyed before all its `guard`s are released, `saam` will panic.

### Guard blindfolding

Sometimes it is necessary to suspend the watch of a guard for some time. This is done by blindfolding the guard.
The `blindfold` is a RAII object that temporarily unlocks the mutex.

```cpp
saam::guard<const int> number_immut_guard{number};

// As long as the blindfold is alive, the guard the lock is free. As soon as the blindfold
// is released the guard is on duty again.
{
    saam::guard<const int>::blindfold number_immut_guard_blindfold{number_immut_guard};
    *number_immut_guard = 5; // Using a blindfolded guard results in a moved-from saam::ref access - the process crashes.
}

*number_immut_guard = 5; // Blindfold is gone, the lock is alive, the guard is usable again.
```

## Condition variables

`saam` associates condition variables with the `synchronized` object. This cohesion helps ensure that
the correct mutex is locked when a condition is waited on.

```cpp
saam::synchronized<int> number(5);
saam::synchronized<int>::condition greater_than_5_condition(number, [](const int &val) { return val > 5; });
```

Waiting for a condition variable uses familiar STL syntax.
```cpp
// guard assures synchrony
saam::guard<int> number_immut_guard{number};

// mutex is released during waiting
// the exit criterion always takes a const type as parameter
greater_than_5_condition.wait(number_immut_guard);

// waiting has finished, the guard is locked again, and it is safe to access the variable
print(*number_immut_guard);
```

Waiting works with both immutable (see above) and mutable guards.
If the guard and the condition variable are not associated with the same synchronized object, a panic will occur.
```cpp
saam::guard<int> number_mut_guard{number};

greater_than_5_condition.wait(number_mut_guard);

// mutable guard allows modifications
*number_mut_guard = 10;
```

The waiting time can be limited on the wait function.
```cpp
// Relative waiting time
auto wait_result = greater_than_5_condition.wait(number_mut_guard, std::chrono::milliseconds(4));
if (wait_result == saam::condition::wait_result::timeout)
    // timeout

// Or it can be absolute
wait_result = greater_than_5_condition.wait(number_mut_guard, std::system_clock::time_point(........));
```

Conditions can be triggered, similar to STL:
```cpp
// Notify all waiting threads
greater_than_5_condition.notify_all();
// Notify one waiting thread
greater_than_5_condition.notify_one();
```

## Recommended integration into classes

The following case study shows how to synchronize member variables of a class. The example is also extended with another concept,
[Initialization Order Fiasco](https://www.youtube.com/watch?v=KWB-gDVuy_I&t=893s), which is not only useful for the smart mutex,
but also overcomes the constructor initialization list constraints.

```cpp
class my_class
{
    // Data members of the class are grouped into one struct, protected by a mutex
    struct members
    {
        int data = 0;
        std::vector<std::string> data_collection;

        static members create(int data)
        {
            // Prepare the members; the object does not exist yet
            // If something is wrong with the creation, exit at this stage
            std::vector<std::string> data_collection(10, "Hello");

            // Push the created components into the struct
            return {.data = data, .data_collection = std::move(data_collection)};
        }
    };

    // Smart mutex to synchronize member variables
    saam::synchronized<members> sync_m;

  public:

    my_class(int data)
        // Move the members under the control of the mutex
        // Return value optimization (RVO) may eliminate the cost of this function call
        : sync_m(members::create(data))
    {
    }

    // The smart mutex is not movable (because the STL mutex is also not movable)
    // Move only the "members" from the other instance under the control of "this" smart mutex.
    my_class(my_class &&other)
        : sync_m(*std::move(other.sync_m.lock())) // Locked "other" prevents modifications in "other" during the move operation
    {
    }

};

```
