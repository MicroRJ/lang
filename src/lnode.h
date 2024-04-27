/*
** See Copyright Notice In elf.h
** (Y) lnode.h
** IR?...
*/


#define NO_NODE (-1)


typedef int lnodeid;


typedef enum lnodety {
	NT_ANY, NT_SYS,
	NT_NIL, NT_BOL, NT_INT, NT_NUM,
	NT_OBJ, NT_TAB, NT_FUN, NT_STR
} lnodety;


typedef enum lnodeop {
	NODE_NONE = 0,
	NODE_NOP  = 1,
	/* -- 4.9.24 ------------------------------------
	the following are set up in counterpart pairs,
	this enables us to easily derive their opposite
	operation by simply applying the (^1) operation.
	must be even odd pairs and must start on an even
	value - learned this trick from Mike Pall @ LuaJIT */
	NODE_AND, NODE_OR,
	NODE_EQ, NODE_NEQ,
	NODE_BITSHL, NODE_BITSHR,
	NODE_ADD, NODE_SUB,
	NODE_MUL, NODE_DIV,
	NODE_LT, NODE_GT,
	NODE_LTEQ, NODE_GTEQ,
	/* end */
	NODE_BITXOR, NODE_MOD,

	NODE_TYPEGUARD,

	NODE_LOAD,
	NODE_FILE,
	/* constant values, closure points to prototype
	in proto table in module, all other constants
	are embedded in the node and the generator decides
	how to emit the instruction. */
	NODE_CLOSURE, NODE_STRING, NODE_TABLE,
	NODE_INTEGER, NODE_NUMBER, NODE_NIL,
	/* l-values, represent some address,
	source is x */
	NODE_GLOBAL, NODE_LOCAL, NODE_CACHE,
	NODE_THIS,// this
	NODE_INDEX,// {x}[{x}]
	NODE_FIELD,// {x}.{x}
	NODE_METAFIELD,// {x}:{x}
	NODE_CALL,// {x}({x})
	/* some builtin instruction node, x is the
	builtin id, (the token type) and z the
	inputs */
	NODE_BUILTIN,// ID({x})
	/* psuedo ops */
	NODE_RANGE_INDEX,// [{x}..{x}]
	NODE_RANGE,// {x}..{x}
	NODE_GROUP,// ({x})
} lnodeop;


/* -- todo: make this more compact */
typedef struct lNode {
	lnodeop k;
	lnodety t;
	llineid line;
	/* todo: eventually remove this */
	int level;
	/* ---------------------------
	x,y are node inputs/operands,
	if more than 2 are required,
	use z*.
	r represents a tangible memory
	location, used for register
	allocation. */
	struct { lnodeid x,y,*z; };
	llocalid r;
	/* todo?: don't quite union these two for debugging? */
	union {
		char   *s;
		elf_int i;
		elf_num n;
	} lit;
} lNode;


lnodeid elf_nodexyz(elf_FileState *fs, llineid, lnodeop k, lnodety t, lnodeid x, lnodeid y, lnodeid *z);
lnodeid elf_nodebinary(elf_FileState *fs, llineid, lnodeop k, lnodety t, lnodeid x, lnodeid y);
lnodeid elf_nodeunary(elf_FileState *fs, llineid, lnodeop k, lnodety t, lnodeid x);
lnodeid elf_nodenullary(elf_FileState *fs, llineid, lnodeop k, lnodety t);

lnodeid elf_nodegroup(elf_FileState *fs, llineid, lnodeid x);

lnodeid elf_nodenil(elf_FileState *fs, llineid);
lnodeid elf_nodeint(elf_FileState *fs, llineid, elf_int i);
lnodeid elf_nodenum(elf_FileState *fs, llineid, elf_num n);
lnodeid elf_nodestr(elf_FileState *fs, llineid, char *);
lnodeid elf_nodetab(elf_FileState *fs, llineid, lnodeid *z);
lnodeid elf_nodecls(elf_FileState *fs, llineid, lnodeid x, lnodeid *z);

lnodeid elf_nodeload(elf_FileState *fs, llineid line, lnodeid x, lnodeid y);

lnodeid elf_nodelocal(elf_FileState *fs, llineid line, lnodeid i);
lnodeid elf_nodecache(elf_FileState *fs, llineid line, lnodeid i);
lnodeid elf_nodeglobal(elf_FileState *fs, llineid line, lnodeid i);

lnodeid elf_nodetypeguard(elf_FileState *fs, llineid line, lnodeid x, lnodety y);
lnodeid elf_nodemetafield(elf_FileState *fs, llineid line, lnodeid x, lnodeid y);
lnodeid elf_nodefield(elf_FileState *fs, llineid line, lnodeid x, lnodeid y);
lnodeid elf_nodeindex(elf_FileState *fs, llineid line, lnodeid x, lnodeid y);

lnodeid elf_nodeloadfile(elf_FileState *fs, llineid line, lnodeid x);

lnodeid elf_noderangedindex(elf_FileState *fs, llineid line, lnodeid x, lnodeid y);

lnodeid elf_nodebuiltincall(elf_FileState *fs, llineid line, ltokentype k, lnodeid *z);
lnodeid elf_nodecall(elf_FileState *fs, llineid line, lnodeid x, lnodeid *z);


lvaluetag elf_nodettotag(lnodety ty) {
	switch (ty) {
		case NT_SYS: return TAG_SYS;
		case NT_NUM: return TAG_NUM;
		case NT_INT: return TAG_INT;
		default: LNOBRANCH;
	}
	return TAG_NIL;
}




/* 4.2.26
-- DEAR DIARY,  ------------------------------------------
----------------------------------------------------------
Please don't read this...

IR System (Nodes):

-- What is an IR?
By definition IR stands for intermediate representation,
and as the name implies it is the transitionary medium
between source and machine/bytecode code.
-- Why do you need an IR?
Generally you start needing an IR when you want to apply
more advanced optimization passes.
Simply put it is easier to apply optimizations on some
sort of well behaved graph than machine code.
This tends to be a very general pattern in software, for
the most part it is useful to have useful constructs...
and that is IR, something useful for optimizations, in
the same way that tokens are useful to the parser.
So if you were to ask, what is a token?
You could say, whatever is useful to the parser.
Now as it stands this is probably not very useful to somebody
that's just getting started but is one of those things that'll
really click once you get the hang of it.
-- How do you build an IR?
As you wish. There are various general concepts and ideas that
apply to pretty much all IR's.
However, as anything now days, this can become a very political
question and so it really depends on the school of thought. (Rant)
	You see, some people have actually devoted their whole lives
to "coming" up with these concepts and "furthering" their
research. And so the general pattern with academic publications
is that they have to reflect the amount of time the author has
wasted I mean spent researching the subject. So instead of a
couple paragraphs of decent English or even better, code, they
write these long obfuscated essays where every word has to be
more than 7 letters and use oxford spelling, I mean who cares?
Who really does care?
	The point is this, by doing stuff and looking at what other
very select individuals have made and what seems to be logically
and intuitively and objectively good and most practical, I can
confidently say you that you don't need that much gunk, as a
matter of fact you need very little IR if any to get code
generation going on.
And this, is "liberating".

-- Why is this here?:

	A problem is not merely a situation where things don't go as
planned; rather, it's a discrepancy between desired outcomes
and actual results.
	To put it in other words, a genuine problem can be thought of
as a boulder in the middle of the road, a road that goes from A
to B, where you can barely peek through and see the destination.
But being misinformed or ignorant is like having a road that leads
nowhere, or even worse, having multiple roads that lead to
erroneous destinations, each with their own dubious boulders.
	In essence, while a genuine problem obstructs the path to a
clear destination, misinformation or ignorance can lead down
divergent paths, none of which lead to the desired outcome.
Thus, it becomes crucial to not only address genuine problems
but also to rectify misinformation or ignorance to ensure progress
toward the intended goal.
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

*/