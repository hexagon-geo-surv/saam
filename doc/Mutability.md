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
These are the `saam::guard` objects. Guards are a mutex lock bundled with a smart reference to
the protected object.

Create a mutable proxy from the synchronized object. The template parameter being not `const`
indicates that this is a unique lock—there can be only one at a time.
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

    void calculate()
    {
        auto m_guard{sync_m.commence_mut()};
        m_guard->data += 1;
        // Pass on the locked guard to the implementation functions by reference
        calculate_step1(m_guard);
    }

  private:
    void calculate_step1(const saam::guard<members> &m_guard)
    {
        // Work on the mutable guard - the guard is const, but not the "member" object.
        m_guard->data += 10;
    }

};

```
