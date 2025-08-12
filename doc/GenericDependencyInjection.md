<!--
SPDX-FileCopyrightText: Leica Geosystems AG

SPDX-License-Identifier: MIT
-->

# Dependency injection

Distributing component references to other components is usually done with dependency injection.
There are multiple ways to implement this propagation.

## Raw Reference/Pointer injection

The component that is injected into the other, receives a C++ pointer or reference to its dependency.
This is a very efficient way, but there is no guarantee that the injected reference will not survive the lifetime
of the dependency. The reference/pointer can go dangling any time.

``` c++
class component_b
{
  public:
    explicit component_b(component_a* comp_a = nullptr)
        : comp_a_(comp_a)
    {
    }

    int calculate(int a)
    {
        if (!comp_a_)
            throw error("Missing component_a dependency.");

        return comp_a_->negate(a) + 2;
    }

private:
    component_a* comp_a_;
};
```

The injection can be optional or mandatory if we use pointer.
For references it is mandatory, or it must be wrapped to some `nullable` decorator,
like `std::optional<std::reference_wrapper<T>>`.

## Smart Pointer injection

This approach gives lifetime guarantees, but comes with a likely unwanted restriction.
Unfortunately, the multiple owners of the pointed dependency prevents to nominate a dedicated main owner.
This owner would tipically be a composition root.
When the composition root releases the dependency instance `component_a`, the `component_b`
may still hold it - it is also an owner. 
This extra ownership sometimes goes unnoticed and makes the system shutdown very unreliable.

Shared ownership is quite often not needed. We just need a central place
(composition root) where components are owned, and other components just use non-owning
references to communicate with each-other. Components usually do not want to influence
the lifecycle of other components.

``` c++
class component_b
{
  public:
    explicit component_b(std::shared_ptr<component_a> comp_a = {})
        : comp_a_(std::move(comp_a))
    {
    }

    int calculate(int a)
    {
        if (!comp_a_)
            throw error("Missing component_a dependency.");

        return comp_a_->negate(a) + 2;
    }

private:
    std::shared_ptr<component_a> comp_a_;
};

```

## Smart Reference injection

Smart References address the symmetrical ownership model problem of the Smart Pointer injection by
providing an assymetric ownership model.
Here, there is a dedicated owner `saam::var` and there are `saam::ref` references that do not own.
Another advantage is, the component must not necesarily be located on the heap, they can be also on the stack.

``` c++
class component_b
{
  public:
    explicit component_b(std::optional<saam::ref<component_a>> comp_a = {})
        : comp_a_(std::move(comp_a))
    {
    }

    int calculate(int a)
    {
        if (!comp_a_)
            throw error("Missing component_a dependency.");

        return comp_a_.value()->negate(a) + 2;
    }

private:
    saam::ref<component_a> comp_a_;
};
```

`saam::ref` are always valid. A `nullable` decorator is needed to provide optional content.

## Piggy backed Smart Reference injection into Smart Pointer injection API

If the API of a library supports only Smart Pointer dependency injection, then we can still use 
Smart References (to some extent). This approach has some limitations - so use it on your own risk.

Given `component_b`, the API of which supports Smart Pointer dependency injection.
``` c++
class component_b
{
  public:
    explicit component_b(std::shared_ptr<component_a> comp_a = {})
        : comp_a_(std::move(comp_a))
    {
    }

    ....
};
```

Then we have a `component_a` instance that needs to be injected into `component_b`. The problem is, `component_a`
uses Smart Reference, instead of Smart Pointer.

The solution is to capture the Smart Reference into the custom deleter of the `std::shared_ptr`.

``` c++
int main()
{
    saam::var<component_a> comp_a;

    auto a_comp_ref = comp_a.borrow();
    std::shared_ptr<component_a> a_comp_shared_ptr(&(*a_comp_ref), [a_comp_ref = std::move(a_comp_ref)](component_a*){});
    component_b comp_b(a_comp_shared_ptr);
}
```

The first parameter of the `std::shared_ptr` constructor is the `component_a` instance pointer.
It is obtained from the Smart Reference.

The second parameter is the custom deleter. The deleter lambda captures the reference.
As long as the `std::shared_ptr` or any of its copy is alive, the reference is kept alive.

This ensures, that the dangling of the original `component_a` instance is reported.
The caveat is; when the `std::shared_ptr`s are copied, the Smart Reference is not copied with them.
The creation point stack trace in `saam::ref` is not accurate any more. It can still hint
for the code area where the problem is likely occured, but the exact place of dangling reference
must be analysied from the code.

## Generic Reference injection

It seems that with Smart Reference injection the ideal solution was found.
Dependency injection is a cross-cutting concern of the system. If there is decision for an approach,
then it must be followed in the whole system. In this case all components of the system shall use
Smart References. As it is not a system library, this can be a liability.

Generic Reference injection offers an STL-only solution that can cover any of the dependency injection
scenarios above.

The trick is to  wrap the dependency into the type erased callable: `std::function`.
The `std::function` is initialized by a lambda expression that captures a reference to the
dependency object. The type erasure of the `std::function` hides the reference differences.
Using this method allows using Smart References, Smart Pointers, or non-owning references/pointers.

``` c++
template <typename T>
using dependency = std::function<T &()>;

class component_b
{
  public:
    explicit component_b(dependency<component_a> comp_a = {})
        : comp_a_(std::move(comp_a))
    {
    }

    int calculate(int a)
    {
        if (!comp_a_)
            throw error("Missing component_a dependency.");

        return comp_a_().negate(a) + 2;
    }

private:
    dependency<component_a> comp_a_;
};
```
`std::function` is a nullable type, so it naturally supports mandatory or optional
dependencies.

### Factories

It is very easy to create generic factory functions for all the above mentioned
DI approaches.

#### Raw Reference/Pointer factory

In the first case, `comp_a` is taken by raw reference and injected into `comp_b`.
The `make_dependency` factory function simplifies the syntax.

``` c++
template <typename T>
dependency<T> make_dependency(T &instance)
{
    return [&instance]() -> T & { return instance; };
}

int main 
{
    component_a comp_a;
    component_b comp_b(make_dependency(comp_a));
}
```

A factory function with raw pointer argument is very similar.
``` c++
template <typename T>
dependency<T> make_dependency(T *ptr)
{
    return [ptr]() -> T & { return *ptr; };
}
```

#### Smart Pointer factory

In case of smart pointers, the syntax is the very similart. Note that the
API of `component_b` is the same as before - it was not adjusted to take smart pointer.
`component_b` just uses the dependency and all lifetime related concerns
are outsourced to the dependency injector.

``` c++
template <typename T>
dependency<T> make_dependency(std::shared_ptr<T> ptr)
{
    return [ptr = std::move(ptr)]() -> T & { return *ptr; };
}

int main 
{
    std::shared_ptr<component_a> comp_a = std::make_shared<component_a>();
    component_b comp_b(make_dependency(comp_a));
}
```

`std::unique_ptr` cannot be used directly, because it would make the `std::function`
move-only. The constructor of `std::function` expect copyability. For `std::unique_ptr`
the most obvious solution is to promote it to `std::shared_ptr` and use it like that as
dependency.

``` c++
template <typename T>
dependency<T> make_dependency(std::unique_ptr<T> ptr) = delete;
```

#### Smart Reference factory

And finally, creating a component from a smart reference follows the very
same syntax.

``` c++
template <typename T>
dependency<T> make_dependency(saam::var<T> &var)
{
    return [ref = var.borrow()]() -> T & { return *ref; };
}

int main 
{
    saam::var<component_a> comp_a;
    component_b comp_b(make_dependency(comp_a));
}
```

If we have a smart reference only, then this can be also wrapped to a `dependency`.
``` c++
template <typename T>
dependency<T> make_dependency(saam::ref<T> ref)
{
    return [ref = std::move(ref)]() -> T & { return *ref; };
}
```

This DI technique is not part of the `saam` library. Putting it there would enforce a library dependency
to `saam` and it would make this technique pointless. Instead, the best way is to copy-paste this code into
the library - it is just a very thin wrapper anyways.

### saam::any_ptr

The principle can be packaged into a smart pointer-like API. This type erased smart pointer is called `saam::any_ptr`.

This new smart pointer:
- can express ownership (via raw pointer, smart pointer) or aliasing (via raw pointer, raw reference, smart reference)
- has a pointer API: arrow operator, dereferencing operator, bool operator
- can be default constructed
- nullable: the underlying lambda is empty
- can be null pointer: the underlying lambda returns a nullptr
- copy and movable
- covariant

The same example above, but this time with `saam::any_ptr`.

``` c++
class component_b
{
  public:
    explicit component_b(saam::any_ptr<component_a> comp_a = {})
        : comp_a_(std::move(comp_a))
    {
    }

    int calculate(int a)
    {
        if (!comp_a_)
            throw error("Missing component_a dependency.");

        return comp_a_->negate(a) + 2;
    }

private:
    saam::any_ptr<component_a> comp_a_;
};
```

### Cost

The cost of this technique is likey close to zero, compared to the simplest case (Raw Reference/Pointer injection).

#### Memory

When an `std::function` wraps a lambda, the capture of the lambda must be stored.
`std::function` contains a Small Object Optimization buffer, that is usually 2-3 pointers large.
Considering the sizes of our reference types (Raw Reference/Pointer, Smart Pointer, Smart Reference)
we have a fair chance that all of them can be captured without extra heap allocation.

#### Runtime

Considering, that the factory methods, the lambdas in the factory method and the `std::function` are all
fully visible for the compiler (there is no translation unit boundary), the compiler is likely able to optimize
them all away.

