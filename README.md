
Welcome to elf, the programming language.

Try online:

https://elflang.netlify.app


This project is unreleased, meaning that
it is not meant for public consumption,
yet... aka use at your own risk.


There are many incomplete features, and
spurious code sprinkled here and there.


This project is ideal for those interested
in a minimalist, embeddable scripting
language, written in vanilla C.


elf has pretty standard and minimalist syntax,
ideal for getting stuff done quickly.

```
table = {1,2,3}
// call pf, printf
pf(table) \\ {1,2,3}

add3 = fun(x,y,z) ? (x + y + z)
add3(1,2,3)
```

We have fairly primitive support for
the typical stuff you'd expect from
a dynamically typed language.


### TODO:

- Better Online Playground

- Incremental GC

- Tracing JIT!

- Multithreading!
