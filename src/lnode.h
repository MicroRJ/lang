/*
** See Copyright Notice In lang.h
** (Y) lnode.h
** IR?...
*/


#define NO_NODE (-1)


typedef int lnodeid;


typedef enum lnodety {
	NT_ANY = 0,
	NT_NIL, NT_BOL, NT_INT, NT_NUM,
	NT_TAB, NT_FUN, NT_STR
} lnodety;

typedef enum lnodeop {
	Y_NONE = 0,
	Y_NOP  = 1,
	/* -- 4.9.24 ------------------------------------
	the following are set up in counterpart pairs,
	this enables us to easily derive their opposite
	operation by simply applying the (^1) operation.
	must be even odd pairs and must start on an even
	value - learned this trick from Mike Pall @ LuaJIT */
	Y_LOG_AND, Y_LOG_OR,
	Y_EQ, Y_NEQ,
	Y_BSHL, Y_BSHR,
	Y_ADD, Y_SUB,
	Y_MUL, Y_DIV,
	Y_LT, Y_GT,
	Y_LTEQ, Y_GTEQ,

	Y_BXOR, Y_MOD,

	NODE_LOAD,

	Y_LOADFILE,
	/* constant values, closure points to prototype
	in proto table in module, all other constants
	are embedded in the node and the generator decides
	how to emit the instruction. */
	N_FUN, Y_STRING, N_TABLE,
	Y_INTEGER, Y_NUMBER, N_NIL,
	/* l-values, represent some address,
	source is x */
	Y_GLOBAL, N_LOCAL, N_CACHE,

	N_INDEX,// {x}[{x}]
	N_FIELD,// {x}.{x}
	Y_CALL,// {x}({x})
	Y_MCALL,// {x}:{x}({x})
	/* some builtin instruction node, x is the
	builtin id, (the token type) and z the
	inputs */
	Y_BUILTIN,// ID({x})

	Y_MCALL_CLONE,
	Y_MCALL_SPLIT,
	Y_MCALL_MATCH,
	Y_MCALL_REPLACE,
	Y_MCALL_INDEXOF,
	Y_MCALL_LENGTH,
	Y_MCALL_INSERT,
	Y_MCALL_LOOKUP,

	/* psuedo nodes */
	Y_RANGE_INDEX,// [{x}..{x}]
	Y_RANGE,// {x}..{x}
	Y_GROUP,// ({x})
} lnodeop;


typedef struct lNode {
	lnodeop k;
	lnodety t;
	llineid  line;
	/* todo: eventually remove this */
	int level;
	/* --- x,y are node inputs/operands, if more
	than 2 are required, use z* once a node is
	processed in its entirety, it is allocated
	somewhere, in which case we transition to
	using r, which represents a tangible memory
	location.
	* this could change in the future, we could
	use complementary nodes to append more inputs
	to a node. --- */
	struct {
		lnodeid x,y,*z;
	};
	llocalid r;
	/* todo?: don't quite union these two for debugging? */
	union {
		char const *s;
		llongint    i;
		lnumber     n;
	} lit;
} lNode;


lnodeid langN_xyz(FileState *fs, llineid, lnodeop k, lnodety t, lnodeid x, lnodeid y, lnodeid *z);
lnodeid langN_xy(FileState *fs, llineid, lnodeop k, lnodety t, lnodeid x, lnodeid y);
lnodeid langN_x(FileState *fs, llineid, lnodeop k, lnodety t, lnodeid x);
lnodeid lanN_node(FileState *fs, llineid, lnodeop k, lnodety t);

lnodeid langN_group(FileState *fs, llineid, lnodeid x);

lnodeid langN_longint(FileState *fs, llineid, llongint i);
lnodeid langN_number(FileState *fs, llineid, lnumber n);
lnodeid langN_string(FileState *fs, llineid, char *);
lnodeid langN_table(FileState *fs, llineid, lnodeid *z);
lnodeid langN_closure(FileState *fs, llineid, lnodeid x, lnodeid *z);
lnodeid langN_nil(FileState *fs, llineid);

lnodeid langN_load(FileState *fs, llineid line, lnodeid x, lnodeid y);

lnodeid langN_local(FileState *fs, llineid line, lnodeid i);
lnodeid langN_cache(FileState *fs, llineid line, lnodeid i);
lnodeid langN_global(FileState *fs, llineid line, lnodeid i);

lnodeid langN_field(FileState *fs, llineid line, lnodeid x, lnodeid y);
lnodeid langN_index(FileState *fs, llineid line, lnodeid x, lnodeid y);

lnodeid langN_loadfile(FileState *fs, llineid line, lnodeid x);

lnodeid langN_rangedindex(FileState *fs, llineid line, lnodeid x, lnodeid y);

lnodeid langN_builtincall(FileState *fs, llineid line, ltokentype k, lnodeid *z);
lnodeid langN_metacall(FileState *fs, llineid line, lnodeid x, lnodeid y, lnodeid *z);
lnodeid langN_call(FileState *fs, llineid line, lnodeid x, lnodeid *z);







/*


IR System (Nodes):


Notes for the learner:

-- What is an IR?
By definition IR stands for intermediate
representation, and as the name implies
it is the transitionary medium between
source and machine/bytecode code.
-- Why do you need an IR?
Generally you start needing an IR when you want
to apply more advanced optimization passes.
Simply put (as it should) it is easier to apply
optimizations on some sort of well behaved graph
than machine code.
This tends to be a very general pattern in
software, for the most part it is useful to have
useful constructs... and that is IR, something
useful for optimizations, in the same way that
tokens are useful to the parser.
So if you were to ask, what is a token?
You could say, whatever is useful to the parser.
Now as it stands this is probably not very useful
to somebody that's just getting started but is one
of those things that'll really click once you get
the hang of it.
-- How do you build an IR?
As you wish. There are various general concepts
and ideas that apply to pretty much all IR's.
However, as anything now days, this can become
a very political question and so it really
depends on the school of thought. (Rant)
You see, some people have actually devoted their
whole lives to "coming" up with these concepts and
"furthering" their research and so the general
pattern with academic publications is that they
have to reflect the amount of time the author has
wasted I mean spent researching the subject, so
instead of a couple paragraphs of decent English
or even better, code, they write these long
obfuscated essays where every word has to be more
than 7 letters and use oxford spelling, I mean
who cares? Who really does care?
The point is this, by doing stuff and looking at
what other very select individuals have made
and what seems to be logically and intuitively
and objectively good and most practical, I can
confidently say you that you don't need that
much gunk, as a matter of fact you need very
little IR if any to get code generation going on.
And this, is "liberating".

-- Key Notes For The Reader:

A problem is not merely a situation where things
don't go as planned; rather, it's a discrepancy
between desired outcomes and actual results.
For instance, being unable to hit a baseball
with enough power constitutes a genuine problem
because it hinders achieving the goal of hitting
the ball effectively.
However, statements like 'I don't understand why
I can't just run to first base' may not represent
a problem per se, but rather a lack of knowledge
or understanding of the rules of the game.
Therefore, it's essential to distinguish between
genuine problems and instances where one is simply
misinformed or uninformed.
In essence, a problem only truly exists when it's
acknowledged as such, requiring active effort to
resolve.
To put it in other words, a genuine problem can be
thought of as a boulder in the middle of the road,
a road that goes from A to B, where you can barely
peek through and see the destination. But being
misinformed or ignorant is like having a road that
leads nowhere, or even worse, having multiple roads
that lead to erroneous destinations, each with
their own dubious boulders.
In essence, while a genuine problem obstructs
the path to a clear destination, misinformation
or ignorance can lead down divergent paths,
none of which lead to the desired outcome.
Thus, it becomes crucial to not only address
genuine problems but also to rectify misinformation
or ignorance to ensure progress toward the
intended goal.
And so my main method of learning is by doing,
trying things out myself.
I find that I come up with very specific and
concise questions naturally in a manner that useful
for guiding me through the fog.
I think is like asking for directions to somewhere,
you may not know the address, but you have some
sort of description of what the place looks like
and its surroundings, and sooner or later you'll
find that one person that'll know what you're
talking about.
As you try and fail  the fog dissipates and you
might even see the location yourself.

As it turns out, in my personal experience as
a professional human, capable of learning,
sometimes the hardest thing to do is to know
what to ask.
And when you don't even know what to ask is
like someone asking you, "where you heading"
and you're like "I don't know", nobody got
that time for that.
I think this stems from the nature of wanting,
to want is not to get. Wanting is a very dense
abstract thing, and to go from wanting to a
detailed and clear plan in order to get what
you want, well that's rather difficult.

I believe that there's a subtle distinction
between learning and problem solving.
For instance, when you're confused about something,
you're not really solving any problems, or at-least
the problem you have is not inherent in the concept,
but rather the medium, yourself.
So doing stuff will help you figure out which one
of those it is.
Just because you verbalize what you deem to be the
solution or some sort of guidance to get to the
solution will not mean your brain will actually
learn anything.
Is like believing that by thinking you're a professional
athlete you will eventually become one.
It certainly does help to be positive, but positive
will only take you so far.

Then I compare what I did and what others have done,
then I have a good basis for determining what I
could improve on or I have something unique and
useful that I can share with other people.
So whenever you have the very general "why"
questions, I encourage to first try and do things
yourself, and I can almost guarantee that those
questions will turn into very specific, easier to
answer ones. Now, inevitably you could go down
some rabbit whole where no amount doing will save
you, this is called failing and this is good, now
figure out why you failed, this is called learning.

To recapitulate, If you're asking this question
it's probably because you've never made a
programming language before or at least you don't
have much experience doing so. If so AND you're
actually interested in making one, I would highly
suggest you disregard this step and instead make
a programming language that generates bytecode
directly from source, naturally, you'll want to
generate better code, and you will start realizing
why you might need some sort of intermediate step.



-- How do you build an IR?
In short, however you like, which is why it
can be daunting at first, because freedom is
complex, it is entirely up to you.
Generally it helps to have experience making
really simple programming languages, for instance
programming languages that generate bytecode
directly from source. This way you get a feel
for what you might need. Your brain naturally
starts to come up with ideas for generating
optimized code, and naturally, you end up realizing
that use sort of intermediary step could be
useful.



A \node in the abstract sense has the following
main properties:

Takes multiple inputs, singular or no output and
is typed.

There are two main ways to access or traverse
nodes.

Using the node stack, or directly through an
index where the node is allocated.

Nodes are deallocated once they are done with,
since nodes can be moved around, we free them in
chunks, this way we can still use a simple linear
allocator model.

Nodes that are no longer in use are replaced with
NOP nodes, this is the only instance where a node
is to be modified directly, in practice, nodes
are immutable.

Not all nodes evaluate to some tangible value,
their main purpose is expression some sort of
dependency, which in turn implies flow or order.
On the other hand, Some nodes serve as pseudo
instructions, meaning that they may not necessarily
adhere to to the typical rule-set but nevertheless
are useful.

this
is why we have the stack, the stack contains all the nodes that
yield some value. By design, a node can take multiple inputs and
yield a single value, this property is beneficial for us.


redesign of the ir, instead of using references to the ir
directly for lower level functions, we can use localids, which
will map to an ir stack, the ir stack will contian the ir
instructions, this will allow for a few nifty things, for instance,
easy static analizys of types, say you have the following code:

let x = 0
if x == 0 ? {
	x = {}
	x = x + 1
}

here we can mark the block where x = {} as an if block and associate
it with a local id, this way, we can traverse the stack to find the
latests store to x to determine the current type and value of x.

*/