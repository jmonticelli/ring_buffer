# `ring_buffer`

A simple, templated ring buffer implementation for people who just want to copy a single header file.

This was confirmed to compile with GCC 12.3.0 and Clang 15.0.7.

**NOTE**: As specified below, this is an extremely infantile library that has questionable maintenance as of now.
**Using this library in critical applications is not recommended. I consider this a toy project until further notice.**

## What this offers you

- A header only, dynamically allocated ring buffer that uses new-placement under the hood
- `push_back`, `push_front`, `emplace_back` and `emplace_front` insertion
- `pop_front`, `pop_back`
- Iterators

## What this does not have

- Statically allocated buffers
- Absolutely optimal performance
- Move construction
- Assignment operators
- Lots of miles

## Why?

Boost's circular buffer does a terrific job at doing what it does; however, I have found myself personally avoiding Boost on more occasions than not.
My only real complaint with Boost as I have dealt with it is that pulling in Boost seems heavyweight and I often only want to use one or two pieces of the Boost library.

Of course, pulling in dependencies is not difficult, but there may be constraints that exist that make adding a Boost library either unattractive or non-viable.

This library is supposed to be a self-contained, single-file header-only library.

## Notice of infancy

This is a toy project for me, I have no idea how much maintenance it will get, and for that reason, I do not reccomend this library to be used in its current state for anything that has production code.

I expect this library to work generally, but it is very minimally tested and was something I did over multiple weekends for fun.

## Comparison to Boost

Admittedly not having used Boost's `circular_buffer` much, I was likening the `push_back` and `push_front` operations to how I would use a `std::vector`, so I believe that `push_front` and `push_back` are semantically reversed from their meaning in Boost's `circular_buffer`.

That aside, this library is slower than Boost at the moment.

I wanted to provide diagrams, but I had a desire to improve performance to make this comparable to boost.

At worst, my benchmark showed that this library has slightly over 2x the runtime of Boost's implementation for insertion (comparing `push_back` to `push_front`).

Boost is simply faster.
