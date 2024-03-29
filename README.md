
Look, I couldn't even come up with a
name for this project, nonetheless,
hopefully it turns into something.

Look at the 'code' tree to figure how
how the language works.

This project is unreleased, meaning that
it is not meant for public consumption,
yet... aka use at your own risk.

There are many incomplete features, and
spurious code sprinkled here and there,
but the core structure is fine and the
code is nicely organized.

This project is ideal for those interested
in a minimalist, embeddable scripting language,
written in vanilla C.

lang has pretty standard and minimalist syntax,
ideal for getting stuff done quickly.

```
table = {1,2,3}
// call pf, printf
pf(table) \\ {1,2,3}

add3 = fun(x,y,z) ? { leave x + y + z }

add3(1,2,3)
```

We have fairly primitive support for
the typical stuff you'd expect from
a dynamically typed language.

But then again, the only dynamically
typed language I've ever used is this one.


### TODO:

- Can we figure out a name for this
language? Ideally 3-5 letters.

- Help me port this, see lsys.h for list
of functions that need porting.

- Can we extend corelib please?

- Explicit stack state in code generation,
both for safety and future optimizations...

- Can we borrow some features of SSA to
figure out the types of expressions at
compile time to generate slightly more
optimized bytecode?

- Figure out the garbage collector
and memory stuff, spurious code alert.

- Can we get some machine code generation?

- Add native multi-threading support,
break the language entirely and start
from scratch again.
