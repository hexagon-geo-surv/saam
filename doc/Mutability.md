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

Similar to `saam::ref`, smart mutex uses proxy objects to provide access to the underlying type.
These are the `saam::sentinel` objects.

Create a mutable proxy from the synchronized object. The template parameter being not `const`
indicates that this is a unique lock—there can be only one at a time.
Unique locks allow reading and writing to the underlying object.
```cpp
saam::sentinel<int> locked_number_mut = number;
```

Alternatively, `saam::synchronized` factory methods can create sentinels.
This syntax is useful to create temporary sentinels in a long expression.
```cpp
auto doubled = *number.lock_mut() + *number.lock_mut();
```

Shared sentinels provide immutable (read-only) access, but multiple such sentinels may exist at the same time.
There is a factory function for this as well.
```cpp
saam::sentinel<const int> locked_number_immut = number;
auto locked_number_immut = number.lock();
```

The `synchronized` instance and its related `sentinel` instances are bound via smart references. If the
`synchronized` object is destroyed before all its `sentinel`s are released, `saam` will panic.


```cpp
int swap_number(int new_number)
{
    std::lock_guard lock(mutex_);
    return number_
}
```

```cpp
int swap_number(int new_number)
{
    return *locked_number_.lock();
}
```

## Condition variables

`saam` associates condition variables with the `synchronized` object. This cohesion helps ensure that
the correct mutex is locked when a condition is waited on.

```cpp
saam::condition value_changed_condition(number);
```

Waiting for a condition variable uses familiar STL syntax.
```cpp
// sentinel assures synchrony
saam::sentinel<const int> locked_number_immut = number;

// mutex is released during waiting
// the exit criterion always takes a const type as parameter
value_changed.wait(locked_number_immut, [](const int &val) { return val > 5; });

// waiting has finished, the sentinel is locked again, and it is safe to access the variable
print(*locked_number_immut);
```

Waiting works with both immutable (see above) and mutable sentinels.
If the sentinel and the condition variable are not associated with the same synchronized object, a panic will occur.
```cpp
saam::sentinel<int> locked_number_mut = number;

value_changed.wait(locked_number_mut, [](const int &val) { return val > 5; });

// mutable sentinel allows modifications
*locked_number_mut = 10;
```

The waiting time can be limited on the wait function.
```cpp
// Relative waiting time
auto wait_result = value_changed.wait(locked_number_mut, [](const int &val) { return val > 5; }, std::chrono::milliseconds(4));
if (wait_result == saam::condition::wait_result::timeout)
    // timeout

// Or it can be absolute
wait_result = value_changed.wait(locked_number_mut, [](const int &val) { return val > 5; }, std::system_clock::time_point(........));
```

The `synchronized` instance and its related `condition` instances are bound via smart references. If the
`synchronized` object is destroyed before all its `condition`s are released, `saam` will panic.

Conditions can be triggered, similar to STL:
```cpp
// Notify all waiting threads
value_changed_condition.notify(saam::condition::notification_scope::all_waiter);
// Notify one waiting thread
value_changed_condition.notify();
```

## Synchronized member variables

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
