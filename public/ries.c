/* ries.c

    RIES -- Find Algebraic Equations, Given Their Solution
    Copyright (C) 2000-2024 Robert P. Munafo
    This is the 2024 May 20 version of "ries.c"


    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    If you got ries.c from the website mrob.com, the GNU General Public
    License may be retrieved from mrob.com/ries/COPYING.txt

    You may also find a copy of the license at www.gnu.org/licenses/


    The remainder of this large header comment is broken up into sections
    titled: HOW TO BUILD, UNFINISHED WORK, DETAILED NOTES ON ALGORITHM,
    REVISION HISTORY.

HOW TO BUILD:

  1. Boot your favorite UNIX-compatible computer (Step 0: install Linux,
     because Linux rules!  :-)
  2. Make a new directory, put this file there.
  3. Compile it with the following command:

     gcc ries.c -lm -o ries

     If the compilation fails and reports errors like "undefined reference",
     try moving the pieces around: "gcc -o ries ries.c -lm". Using the order
     "gcc -o ries -lm ries.c" is known to NOT work with recent versions
     of GCC.

     Try other flags for optimization if you wish (like: -m64, -O2 or -O3)
     Note that -ffast-math does *not* work; it prevents IEEE 754 compliance
     and breaks a few of the RIES algorithms.)

  4. Run ries and give it a number, for example:

     ries 2.5063

BUILD OPTIONS:

In the compile line you may add one or more of these options:

  -DRIES_WANT_LDBL
    Use this option to 'ask' RIES to use the 'long double' floating-point
    data type for all calculations. It will try to determine (via other
    predefined flags provided by the compiler) whether it is available
    and if so, will use it. This gives a few extra digits of precision on
    most Intel-based systems, and gives about 30 digits on PowerPC systems.
    On some systems (notably Cygwin) you'll get errors.

  -DRIES_USE_SA_M64
    Use this option to make RIES use standalone transcendental functions.
    These functions are in the separate source file "msal_math64.c", which
    should be available in the same place you found this source file.

  -DRIES_MAX_EXPRESSION_LENGTH=29
    Use a maximum expression length of 29 symbols, rather than the default.
    Longer expressions might be needed if you're using the --one-sided
    option a lot, but they also increase the amount of memory RIES uses
    while doing long searches.

BUILDING IN MICROSOFT VISUAL C++

If you want to build RIES in MS Visual Studio, start at the RIES
website (mrob.com/ries) and follow the "Source code" link. Download
and save the source file "ries-for-windows.c". Read its header comment
for further instructions.

RIES INSTRUCTIONS (MANUAL)

Instructions for actually using RIES are in the manual, which
should be in the same place you found this file, if not look on the
web at mrob.com/ries

The manual source is in nroff format (ries.1), and should also be
available in Postscript, PDF, and plain ASCII text. To use the nroff
version, copy it to the proper place (probably in /usr/share) e.g. :

  cp ries.1 /usr/share/man/man1

(substitute appropriate local manpage directory for your OS) then type
"man ries".

*/ /*

UNFINISHED WORK (TTD)

(Listed more or less in the order that I want to look at them, and
recent ones have date tags. Most of the undated notes are from prior
to December 2011)

20220318
  Add a msal_math128.c that uses the GCC __float128 type (test for
__FLT128_MANT_DIG__). Figure out how to reconcile it with library
routines (if any) e.g. sqrt can call library sqrt for the first
approximation. Perhaps also make a msal_math80.c to cover the case
when ldbl is 80-bit Intel native.

20160423
  Add --surprise-me option, which generates a target value according
to a suitably-distributed random number generator e.g.
e^(K*erf(rand(0..1))) where an approximation of the error function
would be acceptable (see en.wikipedia.org/wiki/Error_function, section
"Approximation with elementary functions")

    } else if (strcmp(pa_this_arg, "--surprise-me") == 0) {
      ries_val t;
      g_surprise_me = B_TRUE;

20150118
   There is an error in the manual: "... To exit on a match within
some ``epsilon'', use --max-match-distance with a nonzero epsilon; to
reject inexact matches use --max-match-distance ..." note that the
first behaviour does not happen (but would be a nice feature to have).
The first behaviour is accomplished by the option "-n1". Check
the rest of the manual for similar errors and make sure "-n1" and
--{min|max}-match-distance cross-reference each other.

2013.0314 Look into adding two more classes of numbers: -e for
"elementary" and something like -t for "transcendental". The
uncertainty has to do with which definitions I wish to use.
  -e for "elementary" fills the rather wide conceptual gap between -a
(algebraic) and -l (Liouvillian). I would like to have a class of
numbers where exponentiation is unrestricted, so sqrt(2)^sqrt(2) would
be allowed. That is kind of like what we'd get now with "-a
--any-exponents", but I don't want to allow x in exponents because I
consider the root of "x^x=2" to be non-elementary. So we need a
"--no-x-in-exponents" option to do -e the way I envision it. Since
trig functions are linked to exponents (Euler's formula; complex
exponential function) it seems to make sense to have a
"--no-x-in-trig" setting as well (which conveniently also makes
try.solve's job easier). I should allow e and the exponential function
but with no x in the exponent; logarithms should also be allowed, but
with no x inside a logarithm (otherwise we get things like "x*ln(x)=3"
which is just another form of "x*e^x=e^3"). This only needs one
setting, that is, --no-x-in-exponents also implies no x in arguments
to [E], [l] or in either argument of [L].
  -t would allow everything, and would also enable the W function if
it is available. The only difference versus -EW is that -t would not
generate an error when W is not available, but would just silently
proceed to find W-less solutions.

2013.0314: RIES has trouble finding an equation for
1.3566631192732151980, which is the root of 2^x+3^x=7. The best I
could do was "ries -p 1.3566631192732151980 -Sx+-23^5 -l5" which finds
the awkward "x-(x+2^x+3^x) = (2+2)^(2-3^3)-(2+5)". RIES suffers from
the LHS always having to start with [x). I could address this by
removing the LHS-RHS distinction when calling gen_forms, and find a
different way to regulate the balance between LHS's and RHS's in the
database. A change like this might be easier (or harder) is it is done
after (or before) I generalize the handling of restricted symbolsets.
It also clearly affects the "x on both sides of the equation" change.

2013.0202 --symbol.names is nice, but I need to add definable "begin"
and "end" strings for various things e.g. superscripts, and start
looking at user-redefinable fixity and precedence for things like
mapping [abv] -> "Surd[a,b]" for Mathematica.

2012.0103: One or more custom defined constants.
  To support this and a future defined-functions feature, each
user-defined thing needs to have a symbol auto-generated for it out of
the set of as-yet-unused byte values. Preferably this auto-generated
symbol will be a unique and yet-unused ASCII letter, but it might have
to use more than one letter. That means that (in addition to mapping
functions to turn names into symbols, and additional fields in the
sym_attr_block structure), I also need a new low-level output
formatter for -F0 and some of the debug_xxx output to replace the
simple printf("%s", expr) that I use now..
  This formatter should turn non-standard symbols into '(Nm)' where
"Nm" is an abbreviated version of the user's supplied name. This
symbol can also be used in the -O, -N, and -S options with parentheses
(which have no purpose in that context), so e.g. '-Op(Eg)' would allow
pi and the user-defined symbol 'Eg' to be used once each. To avoid
ambiguity the symbol might have to be auto-generated, and to avoid
driving users nuts I can give them hints on how to avoid that.
  Constants and (eventually) functions can share a common FORTH-like
syntax. Note the optional weight and abbreviation fields just before
the full name:
  # x e^-(x^2) is the inverse of Gosper's "Dilbert Lambda Function"
  --define : InverseDilbertLambda ( x -- x e^-(x^2) )
    dup dup* neg exp *
  ;
  --define : XeX ( x -- x*e^x ) dup exp * ;
  --define : Eg:EulerGamma # seft-a (constant)
    ( -- The Euler-Mascheroni constant, 0.57721... )
    ( --value-type TRAN )
    # 50 digits for when RIES goes to higher precision
    0.57721566490153286060651209008240243104215933593992
  ;
  --define : 14:H:Hypot   # seft-c (two-argument function), weight 14
    ( a b -- sqrt(a^2+b^2) )
    dup*   # ( a b^2 )
    swap   # ( b^2 a )
    dup* + sqrt
  ;
  Note that the FORTH comment delimiter is parentheses, which fits
nicely with the fact that parentheses are unneeded in postfix
expressions.
  In the first implementation of functions, you cannot include literal
constants in the function, but instead have to --define the constant
first with its own name.
  2014.1122: Everything in comments would be ignored except a token
starting with '--', like the ( --value-type TRAN ) example above. This
allows me to extend the FORTH syntax to deliver optional metadata. If
a constant has no explicitly given --value-type, it would be guessed
using guess_valtype() as is currently done with the target. Advanced
users who are making collections of constants in -p files would define
a --value-type for all.
  2012.0613: I might also want to allow immediate constants in
--eval-expression strings (and thus, in eval()) delimited by parentheses
or whitespace. If using parentheses, "ries --eval-expression '(27)q'"
would produce similar output to "ries 27 --eval-expression xq" or
"ries --eval-expression '33^q'". This improvement would be done along
with a loosening of the just-mentioned restriction on using immediates
within functions.

2013.0228 Add DUP, SWAP and OVER operators. These require a bit of
modification to the stack depth and undo handling, but they are
similar enough to seft-a and seft-b that it shouldn't be too hard.
With a default complexity about the same as a small integer, they
would allow duplication of common subexpressions, which seems to
happen fairly often in real formulas.

2013.0314 More creative use of attribute tags (TAG_INT, TAG_RAT, etc):
  --only-integer-exponents: exponents must be integer if the
argument does not contain x, and roots must be integer if the
argument *does* contain x, and all others are disallowed.
 2013.0309:
  --unit-fractions option: reciprocal only if argument is integer
(possibly useful for Egyptian fractions work)

2012.1222 Allow all odd integer roots of a negative argument (3
currently allowed, but better if any odd integer were detected, tags
should help with this; requires extra wizardry in try.solve); add an
option to display "3,/(...)" as "cbrt(...)" (Unicode handled separately
with a .ries profile)

2016.0131 The "unicode.ries" profile is disappointing for a few reasons:
  * The legend at the bottom of RIES' output still uses the standard
    ASCII symbols
  * sqrt(2) becomes "/(2)" (where / is the radical sign) but the parens
    shouldn't be there if the argument is only one character long
  * Nth root, e.g. 3,/7 for cube root, should use a "3" superscript or
    the Unicode cobe-root symbol (U+221B), and other special cases
For all of these we need some sort of user-definable cascading list of
transformation rules.

2013.0205 Output format option that prints any integer-valued
subexpression as an integer, so that "ries -s -l3 351.36306009596398"
can give "8 sqrt(1929)" instead of "8 sqrt((5-7^2)^2-7)". Attribute
tag should help with this, but infix conversion will need to call
eval() on subexpressions to figure out when a given subexpression can
be substituted with an integer (and recursive calls to infix
conversion pruned).

2012.1222: In --one-sided mode we could take multiple targets, enter
them all in the database as [x], and use the existing algorithm which
would thereby compare every match to each target. This requires a
significant change to exec() in that it must get its exec_x from a
(new) field in the pe, since the "x" value differs from one expression
to another.
  For --one-sided mode (with a single target or with multiple targets)
we really don't need a database at all: every generated expression can
just be compared to x on-the-fly. This would save memory and run all
in cache, possibly much faster, and allow for a simple multi-threaded
implementation. This can be combined with the multiple targets
improvement (in which case there would be a small database containing
the [x] expression for each target).

2013.0305: A similar idea to the "multiple targets" idea is a
"correlation search" operating mode. In this mode there are just two
targets T and U, and all expressions contain one or the other; any
match must have a T expression on the LHS and a U expression on the
RHS. This is like one-sided mode except that everything is an LHS, and
it's like the planned "x on both sides of the equation" mode in that
matches need to deal with both sides of the equation having a
derivative. One big difference is that the derivative with respect to
T might be independent of the derivative with respect to U (or they
might be partly correlated, or anti-correlated). Thus, each reported
match would report the "delta" as a vector (with direction and
magnitude) encoding how far and in what direction each of the two
targets T and U need to be altered to make that particular equation
match.

2013.0307: If exiting because of a parameter error, display a
traceback with the name(s) and character offsets of any include files
(Line numbers would be nice too, but might be difficult).

2012.1207: Review implementation of --try-solve-for-x:
 * Inverse trig functions are multi-branched. (If they give 7.012913
we might find "sin(x) = 2/3", when solving that for x we must recognize
it and solve to "x = arcsin(2/3)+2 pi" rather than just "x = arcsin(2/3)").
 * Add new options --LHS-addsym, --LHS-onlysyms, --LHS-onesym, and
--LHS-nosyms specify the symbol-set for LHS generation (and likewise
for RHS). This is a refactoring, to prepare for the next changes.
 * The -S and -N options need to have a different meaning when solving
for x. sym.attrs[i].sa_alwd (FKA sym_allowed) needs to be split into
LHS and RHS parts (to provide for the fact that, when solving, some
operators become their inverses: thus the option -SE should set
LHS.allowed['l'] and RHS.allowed['E'] to nonzero values).
 * To provide compatibility, the -S, -E, -O, and -N options work the
old way unless followed by an '=' character. For example, '-N=q' will
forbid sqrt in RHS and forbid x^2 in LHS.
 * Likewise, the symbol-related variables, like a_minw etc. need to be
duplicated so we have one set for LHS and another for RHS.
 * The sym_allowed handling needs to be even more subtle for LHS
expressions: In the expression [xl3^23/-], the [23/] will move
unchanged to the RHS, whereas the [l], [3^] and [-] will get changed
into [E], [3v] and [+] respectively on the RHS. The way to handle this
is for ge.2 to look at whether the argument(s) contain x (by testing
dx!=0) and can also take the current stack pointer into account when
deciding whether to add a symbol. For seft-b symbols like [l], if the
value on the stack contains x (or if the SP is precisely 1) then the
LHS sym_allowed array should be used, but if the SP is bigger than 1
the RHS sym_allowed should be used. Likewise the seft-c symbols should
check if the SP is precisely 2. This also implies that in
--solve-for-x mode the gf_1 and ge.1 complexity limits need to be
based on the union of LHS and RHS symbolsets.
 * Dealing with extra x's in the equation. Options include:
   - Use of -s implies or requires -Ox
   - Use of -s implies or requires the "X only on LHS" mode (opposite of
allowing x on both sides of an equation)
   - Continue with the current practice of simply pushing any extra
x's over to the RHS.
 * Dealing with trig functions. The options are:
   - Adding inverse trig functions, which would clutter the search
space even more.
   - Use of -s implicitly disables trig functions only on LHS (you can
explicitly enable them with an --LHS-syms option)
   - Use of -s implicitly disables trig functions, and/or implicitly
enables their inverses (which would be disabled by default)
   - Use of -s shifts the weights of trig functions to make them less
dense in the search space
   Perhaps these different options could be selected via an extra letter
after the -s
 * The -O option, or any similar option setting a specific non-zero
limit on the number of a particular symbol, cannot be implemented
efficiently. (Consider what happens if all expressions are inserted
into the same tree: then for any given expression, a large fraction of
the other expressions in the tree cannot be used to make a valid
solution: If we get "ln(x)=e^2" and solve for x, and they gave the
option -OE, that solution is not valid). To resolve this, the best
solution is probably to make "solve for x" mutually exclusive to the
-O option. As a sole exception, -Ox can still be implemented, so long
as all reported results involve just a single LHS and an RHS.
 * 2013.0215: One-sided simplification/reduction rules similar to
those already in cv.simplify. One simple example is [ss]->[4^]. A lot
of these are already implicit in the pruning rules, so I can look in
the comments next to the add_rule() calls to get the list of
simplifications. Virtually all will be simple string substitutions.
The unsimplified forms show up only when using -s.

*/ /*

2012.0503 (partly done on 20120505): Prior to 20120505, "ries -l-2 -i
143" gave the bizarre answer "(x-2)+3 = (3*4)^2". The "(x-2)+3"
(instead of x+1) results from the k_sig_loss value 0.01: Because 1 is
less than 143*k_sig_loss, RIES doesn't want to perform the addition
x+1. Similarly, "ries 7775" fails to find "x+1=6^5" because it doesn't
want to add 1 (or any integer) to x.
  The narrower, safer fix (implemented on 2012.0505) is to attempt
"(x+1)-1" and see if it is precisely equal to x. If so, then we can
assert that no significance loss has actually occurred, and permit an
exception to the k_sig_loss rule for addition/subtraction. Other
similar "conservative" tests might be possible, but add/subtract is
the big one.
  A broader fix to investigate is to always allow f(x)+K whenever the
variable subexpression f(x) is larger in magnitude than the constant
subexpression K. That will require testing with #qualify#.

2012.0511 (partly done on 20120720): The "four 4's problem" and many
others like it are small enough to implement with --one.sided and/or
--numeric.anagram. RIES's handling of these problems can be improved
with fairly little modification:
  * An option to not use rules like [xx/] that are meant to prune
"redundant" subexpressions like "4/4".
  * As a separate option (perhaps by giving "--numeric.anagram" vs.
"--strict-numeric-anagram"), accept an expression only if all of the
symbols have been "used up" in the expression. This will require
changing the "exhaustion timeout" detection: I can probably use a
"Still searching" message similar to the one I added to handle
--max-match.distance.

2012.0520: Presently the report.match routine has a bunch of arrays
called {r|l}_{e|f|g}scratch, and the escratch ones are being used for
two different purposes (char[] and symbol[]). The names should be
improved, and a dedicated symbol[] scratch array added. These changes
are logically combined with -Fmax, -FHTML, etc. output formats:

2013.0314: Augment --symbol.names in whatever ways are needed for a
user to implement output formats such as: raw symbols,
calculator-keys, HTML, RHTF, TeX, eqn, Maxima, etc.
  Each symbol should have a user-definable precedence, associativity
and layout (like Haskell "fixity"). If desired, an operator like *
(seft 'c', FORTH stack effect (a b -- c) ) could be redefined to be
displayed as any of the following:

                        arg_order  s_0         s_1        s_2
  Lisp:   (times a b)   a, b       '(times '   ' '        ')'
  RIES:   ba*           b, a       ''          ''         '*'
  infix:  a x b         a, b       ''          ' x '      ''
  HTML:   a&times;b     a, b       ''          '&times;'  ''

There also need to be flags indicating when things can be left out, as
is presently done with '*' in certain cases, and indicating when
parentheses can be left out, as is presently done with a product
inside a sum.
  It is important to note that user-defined functions leaving more
than one item on the FORTH stack cannot be expressed in infix. (If the
two outputs of a single function are immediately multipled together,
what goes on each side of the multiplication sign? It's even worse
when things are done to some of the outputs before they are combined.)

2011.1226: Consolidate the (current) multiple ways that roundoff, overflow,
loss of significance, and tautology errors are handled. This involves the
constants and variables: n_ovr, p_ovr, k_sin_clip, k_min_best.match,
k.vanished_dx, k_prune.deriv, and k_sig_loss. For example,
k_sig_loss*k_min_best.match should be equal to the magnitude of the
"machine epsilon" (2^-53 in the case of 64-bit double) determined by
init.formats().

2012.0106: More importantly, we need to allow matches between an LHS and
another LHS (producing solutions with X on both sides of the
equation). The match closeness becomes |(val_r - val_l)/(deriv_l -
deriv_r)| which is the same as the current formula except for the
addition of the deriv_r term.
 * (2011.1226) Start at the bottom (exec(), eval(), newton(), etc.) and work my
way up to the top (ge.1() etc.). Rename variables and parameters (like
the "!on_rhs" passed to exec() from ge.2) so the names agree with what
they actually do ("!on_rhs" should be something like "!no_x" or
"has_x").
 * Allow more user control over how much output contains x on both
sides, from old behaviour to "all eqns have x on both sides". The
default should be somewhere in the middle; this should be implemented
in the main outer loop where it tests "if (lhs_insert > rhs_insert)".
Also, the 'solve for x' and '-Ox' options should both force the old
behaviour (the latter because of the need to have different
sym.attrs[i].sa_alwd values for LHS vs. RHS).

2011.1226: Higher precision:
 * (partly implemented on 2013.0301) On Intel targets in GCC 4.2.1,
__LDBL_MANT_DIG__ is 64, implying that I can use "long double" to get
a bit more precision. To exploit this I will need to do runtime
testing to determine what precision I am actually getting, similar to
the code in hint.c
 * (complete by 2013.0626) Continue conversion to use of long double
and runtime adjustment of epsilons and cutoffs.
 * Add string-to-float and float-to-string conversion routines based
on f107_spfg and f107_sscan in f107_o.cpp. (I think these already
handle the exponent adequately, but I'll want smarter handling of
extra whitespace when there is no exponent). Convert all instances of
double printf and scanf to use these routines (including the debug_X
printfs).
 * Use three sets of flags:
    Desired precision: RIES_WANT_F53, RIES_WANT_F64 and RIES_WANT_F107;
      also RIES_WANT_LDBL etc. for users who don't know what their 'long
      double' precision but still want to use it.
    Available types and their precisions: RIES_HAVE_LDBL_F64,
      RIES_HAVE_LDBL_F107 and RIES_HAVE_F128
    Which type to use: RIES_VAL_DBL, RIES_VAL_LDBL and RIES_VAL_F128
 * Don't bother with C++; just use __float128 if it's available and
too bad if it's not.
 * No need to use macros like MUL(src1,src2,dst) because __float128 and
'long double' have fully implemented builtins.
 * Initialize the special constants (k.vanished_dx, k_prune.deriv,
etc.) differently depending on precision option. It should be possible
to add these gradually without breaking normal precision since the
rest of the code will just ignore the lower half of any quad
variables.

Consider options for utilizing multiple processors (threads / parallel
implementation). {As of 2012.0423} I have identified three possible
paths of development, below titled "Best", "Good" and "BAD":

2012.0423: Best: Perform additional searching *without* expanding the database.
This would be done when the memory limit is reached (perhaps in
response to a user option):
  - Walk the tree, applying all possible monadic transforms to each
node, to see if the result then produces a new match to another
existing node. For these purposes a "monadic transform" consists of
appending *either* a single seft-b symbol, *or* a seft-a followed by a
seft-c. For example, if this scan found [43L] in the tree, it would
append 's' to form the expression [43Ls], compute its value (which is
1.592289...) then search the tree for this value using a bt_find()
function. It might then find [xp/], which would be a near-match in the
case of x=5. This type of search can be efficiently parallelized by
having each of N threads traverse 1/N part of the tree. (Which in turn
suggests that we should maintain population-counts at each node and
add a bt_index() routine.)
  - Greater-complexity expressions can be synthesized by appending a
short, low-complexity complete subexpression and a seft-c symbol.
For example, [43L] : [3q] : [+] => [43L3q+], which would then match
[x2-].

2012.0109: Good, but hard to implement, and very memory-intensive:
  Within each petit-cycle, have N threads
running at any one time, each constraining itself to a part of the
expression search-space, distinguished by the first few symbols in the
expression. For example, after the complexity depth has gotten high
enough, we will have identified all possible combinations for the
first 3 symbols in the expression's postfix representation. The
searchspace can then be partitioned by having ge.2 perform a CRC
hashfunction on the first 3 symbols, and determine whether it should
prune or recurse depending on the low log_2(N) bits of the hash value.
  This requires having each task build up its own binary tree, which
in turn requires a parallel binary tree merge. For example, using 4
threads, each with its own binary tree divided into quartiles:
  * Add population-counts to each tree node and add a bt_index()
routine;
  * When it is time to merge, each of the 4 trees becomes read-only;
  * Each of 4 threads then builds up a new tree consisting of the I'th
quartiles of each of the old trees;
  * Combine these 4 new trees into a single big tree by creating 2 dummy
parent nodes and 1 dummy grandfather node;
  * Deallocate the old trees.
  The main problem with this approach is that it uses twice as much
memory during the merge process, and 4 times as much memory bandwidth;
and (more crucially) there is no clear way to determine in advance
whether there will be enough free memory to perform the merge.

2014.1122: Good, and a little less hard to implement:
  Run in normal single-threaded mode until the tree is big enough to
survey the location of quartiles as in the 2012.0109 proposal. Then,
permanently split the tree into N pieces (probably by a simple
traverse-and-copy). On subsequent patit-cycles, run N threads where
each thread imposes its own values of g_min_equ_val and g_max_equ_val.
Threads will find duplicate solutions, but are not fully redundant; as
a rough guess I imagine it will have a factor of sqrt(N) of
ineffeciency, so a 4-thread system will use 2 times as much memory
to find the same answers in half as much time.

2012.0423: BAD: The following does *not* work because of the elaborate
interleaved nature of the bt.insert algorithm (see note in oldries.c
mentioning "memory performance degradation"), however it could if we
return to the older 2-tree implementation.
  - Have 2 scans running at any one time, an RHS scan and a LHS scan.
Whenever a scan completes, initiate another. Every time a scan
completes, another thread runs through the outputs of the latest RHS
and LHS.

%%% options to add:

2009.0603: Higher complexity scores for trig functions unless argument
expression contains pi or x. {This was mostly addressed by the trig
argument scale change on 2011.1229}

2012.0102: Warren Smith suggests, "I dont mind sin(1), but sin(1) IN
AN EXPONENT like 3^sin(1) is just ridiculous.", suggesting an idea
like tables of rules that match the end of a forth subexpression (1S^
in this example) and if matched, either prune it outright or add to
the complexity score. (If adding to the complexity score, the
possibility of this has to be considered by the bounds-setting and
short-circuit recursion code). After further discussion he suggested
that no subexpression should be completely excluded, but an "entropy
function" should be used to weight entire expressions based on the
likelihood they would be found in actual maths. Things like 1S^ would
get a really high weight, and the expression database would be
"exponentially" more efficient at covering the types of expressions he
is interested in.
  This is distinct from --rational-trig-args, which only allows
all-or-nothing pruning. However, Warren's idea could be used by adding
an Identity-like operator that has a symbol weight and has the effect
of adding a "blessed" tag to the value. Once blessed, the value would
then be eligible for use in an exponent.

2012.0103: --log-base option to do for ln what --trig-argument-scale
does for sine and cosine.

*/ /*

Ideas from 2007.0703 or earlier (all of these are in MBP's first
backup, old PMG5 archives seem to have nothing; old iMac-G4 archives
might have more info):

variations on -l that allow specifying limit of search by
precision, by time, by memory usage, or by number of equations tested.
The present -l is usually correlated with all of these but is never
equal to any of them. ('-lmem=10M', '-ltime=20m', '-ldigits=8',
'-lexprs=3e6', -leqns=1e10', etc.) --max.memory is a different because
a small -l will still cause RIES to exit before the indicated amount
of  memory is used.

%%% command-line option: if no symbol '1', then 'r' shouldn't be
allowed. Likewise for '2' + '^' and 's'; '-' and 'n'; 'e' + '^' and
'E'; others? Which should be the default, the current simpler behavior or the
"more correct" behavior? The "simple" behavior is useful, as
illustrated by the 1.4142135 example in the manpage, because it
helps people find alternate ways to express the same solution.

macros (user-defined constants and functions):
  - use a separate symbolset namespace (implies macros cannot call
themselves or each other, which avoids lots of problems)
  - To use -O with a macro, include '_' in the -O symbolset
list; all symbols after '_' are macro names. -S assumes all macros
(why would you define a macro and not want to use it?); using -N
with a macro is a contradiction.
  - Macro can use any symbols, and symbols used within macro
expansions don't count against their sa.alwd (FKA sym_allowed) quotas
(this is probably easy; just need to make sure)
  - Need to implement stack-overflow detection in the metastack
routines, both for the stack and for the undo lists.
  - Test evaluator routine needs to make sure macros fit one of the
three allowed types ('a', 'b' or 'c') and that only types 'b' and 'c'
dip into the stack's current contents.
  - Type 'a' can be "precompiled" since they amount to custom
constants
  - Should I allow immediate operands (like 0.577...) to facilitate
defining constants that can't be computed any other way? How about
special operators, like 'exch'?
  - execution can probably be accomplished by having eval() call
itself recursively. It needs to handle its own error returns.

 - with many -i test cases the RHS expression totals are a lot bigger
than the LHS totals (while the insert totals are equal). There are
probably cases that come out the other way around, too. In cases like
this, the program could probably find more matches more quickly by
shifting the balance between LHS/RHS to favor the side that is
generating and inserting more expressions per unit time. (The present
implementation tests "lhs_insert > rhs_insert" which is usually the
best way to do it.) I could model this with formulas that take into
account the total amount of dead-ends, expressions, and inserts on
both sides, compared as a ratio to the equation total (and take the
1st derivative to figure out what side of the maximum you're on). The
idea is to maximize the number of full equations found per unit time
rather than keeping the number of LHS inserts equal to the number of
RHS inserts.
 - here's a way to allow printing more than one exact solution: When
inserting an LHS or RHS, if an identical item already exists in the
tree, overwrite it with the new one. This will cause at most one new,
distinct exact match per matchscan. Since this new match is of higher
aggregate complexity than the old one, it has at least a moderate
chance of being a mathematically distinct solution (-:
 - spend a lot of time looking at which expressions are generated
first, and try to improve the weights so it makes more sense. Why does
2.5063 find [x2q/]=[pq] before [xs]=[p2*]?
 - add constants: Feigenbaum, Euler's
 - add Gamma function (many notes on this below) with an option to
use "factorial" notation when displaying.
 - explore how to expand RIES to complex numbers and complex analytic
functions. Looks easy -- after all, complex data types are native in
GCC!

(Test cases are now in #qualify#)

*/ /*

TABLE OF FUNCTIONS

 sym  stack-effect  description
 0x1  --            Phantom symbol for argument-reversed version of -
 0x2  --            Phantom symbol for argument-reversed version of /
 0x4  --            Phantom symbol for argument-reversed version of ^

 ' '  --            Blank space: no operation (for --eval-expression)
  0   (-- 0)        The constant 0.0 (not currently used, but will have a
                    role soon as part of --numeric.anagram)
  1   (-- 1)        The constant 1.0
  2   (-- 2)        The constant 2.0
  3   (-- 3)        The constant 3.0
  4   (-- 4)        The constant 4.0
  5   (-- 5)        The constant 5.0
  6   (-- 6)        The constant 6.0
  7   (-- 7)        The constant 7.0
  8   (-- 8)        The constant 8.0
  9   (-- 9)        The constant 9.0
  !   (a -- f)      Factorial monad f(x) = x! = Gamma[x+1] (reserved, not
                    yet implemented)
  #   (n d -- x)    Digit paste operator x(n,d) = 10*n+d (reserved, not yet
                    implemented. This is to make --numeric.anagram more useful)
  %   (a b -- m)    (reserved for modulo or remainder?)
  ^   (a b -- f)    Power: f(a,b) = a^b
  *   (a b -- p)    Multiply+ p(a,b) = a*b
  (   --            Comment delimiter; used to bracket custom symbol names;
                    used as a placeholder during infix translation
  )   --            Comment delimiter; used to bracket custom symbol names;
                    used as a placeholder during infix translation
  -   (a b -- d)    Subtract: d(a,b) = a-b
  +   (a b -- s)    Add: s(a,b) = a+b
  :   --            Function definition start
  ;   --            Function definition end
  .   --            Placeholder for multiplication during infix formatting
  /   (a b -- r)    Divide: r(a,b) = a/b
  A   (a b -- A)    Arctangent A(a,b) = atan2(a,b) (reserved, not yet used)
  C   (x -- c)      Cosine c(x) = cos(pi x)
  E   (x -- f)      Exponential function f(x) = e^x
  G   (x -- g)      Gamma function g(x) = Gamma[x] = (x-1)! (reserved, not
                    yet implemented)
  I   (x -- x)      Identity function x(x) = x (not used in expressions, but
                    used as a placeholder during infix translation)
  L   (a b -- l)    Arbitrary logarithm l(a,b) = log_b(a)
  S   (x -- s)      Sine s(x) = sin(pi x)
  T   (x -- t)      Tangent t(x) = tan(pi x)
  W   (x -- w)      Lambert W function, e.g. w(10.0) = 1.74553...
  e   (-- e)        The constant 2.71828...
  f   (-- f)        The constant 1.61803...
  l   (a -- l)      Natural logarithm: l(a) = ln(a)
  n   (a -- n)      Negate: n(a) = -a
  p   (-- p)        The constant 3.14159...
  q   (a -- q)      Square root: q(a) = sqrt(a)
  r   (a -- r)      Reciprocal: r(a) = 1/a
  s   (a -- s)      Square: s(a) = a*a
  v   (a b -- v)    Root: v(a,b) = a^(1/b)
  x   (-- x)        The user's target number

*/ /*

ARCHITECTURE

 main -- Outer loop -- increment complexity and decide whether to add to
         RHS tree or LHS tree
   gen_forms -- setup first recursive level of gf_1
     gf_1 -- Add a symbol of type 'a', 'b' or 'c' and recurse if complexity
     :       allows
     :.gf_1 -- Recursive levels until form is complete
         ge_1 -- setup metastack and initial level of ge.2
           ge_2 -- Generate one step of FORTH code and update metastack,
           :       recurse if more symbols in the form
           :.ge_2 -- Recursive levels until form is complete
               canonval -- Try to put expression value in [1.0,2.0) (only if
                           --canon-reduction option is used)
               bt_insert -- Insert calculated result into LHS or RHS tree
                 check_exact_match (called for exact matches)
                   report_match -- Display match without doing Newton
                 check_sides -- Find closest neighbor of the opposite sidedness
                                and determine if the pair sets a new record.
                   check_match -- Check a single pair to see if it's a new
                                  solution
                     cv_simplify -- Simplify equation by removing e.g. "2*"
                                    from both sides
                       newton -- Use Newton's Method to locate ideal value of
                                 x for a given solution
                       report_match -- Display a match that converged by
                                       Newton.
                         try_solve -- Try to convert "expr1 = expr2" into
                                      "x = bigexpr"

DETAILED NOTES ON ALGORITHM

To reduce the search time, the graph theory "bidirectional search"
(also called "meet-in-the-middle") technique is used. Rather than
attempting to search for solutions of the form

  X = expression                                       (1)

it searches for solutions of the form

  f(X) = expression                                    (2)

where f(X) represents an expression containing X. Each side of
equation (2) is represented as a set (in memory, a ordered list,
implemented with a binary tree) of expressions and their values. Thus
there are two lists {For efficiency, these two lists are stored in a
single binary tree, so that I can check for a match after every new
insert. With a single tree, matching items end up being close to each
other in the tree}. Each list is kept in order by numerical value.
Then, the lists can easily be scanned to locate any matches. A match
consists of an element in the left-hand list whose value is close to
that of an element of the right-hand list. Keeping the lists in order
also allows duplicate expressions, like "2*pi", "pi+pi", and "pi*2",
to be ignored, because they will end up having the same value and
therefore end up being in the same spot in the list.

It is important to look for "simpler" matches before going to more
complex ones. Thus, expressions have a complexity score, computed by
assigning a certain number of points to each of the symbols in the
exression. Matches are checked in order of increasing total complexity
(which is the complexity of the left-hand side plus that of the
right-hand side)

In order to avoid the sorting problems necessary to search the
expressions in a perfectly increasing complexity order, complexity
scores are lumped into discrete finite quanta, which are actually
implemented by using small integers for each component of a complexity
score, rather than real numbers.

This does not actually cause everything to be searched in order, even
modulo the quantization errors. Some equations can be represented in
an unbalanced form with lower aggregate complexity than the
least-complex balanced form: an example is the cube root of 2: The
solutions are "x^3 = 2" and "x = 3,/2", both are unbalanced because
there is one symbol on one side of the = sign and three symbols on the
other. (",/" is the "Nth root" symbol). There are no good balanced
solutions. In such cases the unbalanced, low-complexity form will show
up later in the search than it "should".

*/ /*

To make generation and evaluation easy, expressions are represented in
a FORTH-like syntax with one character per symbol. Thus, "11+" is 1+1,
"2q" is sqrt(2), "ep^" is e^pi, etc. Symbols are categorized by their
stack-effect, which is abbreviated "seft" in the code.

In the actual FORTH language, the stack effect (seft) encapsulates
what a word does to the stack. It is described by a comment (some text in
parentheses) containing: the stack contents before the word is executed,
a "--" symbol, and the stack contents after the word executes.

There are three sefts in ries, 'a', 'b', and 'c'. These are the
similest and most common of a larget set of sefts found in FORTH-like
languages, of which the following are a representative sample:

 seft 0: ( -- )
   No operation (compare to b, b2 which don't change the stack level
   but do use something on the stack)

 seft a: ( -- K )
   Adds one thing (a constant) to the top of the stack

 seft a2: ( arg1 -- res1 res2 )
   Takes one value from the stack, performs an operation, and puts two
   results back on the stack. (Examples: DUP, divmod)

 seft a3: ( arg1 arg2 -- res1 res2 res3 )
   Takes two values from the stack, performs an operation, and puts three
   results back on the stack. (Example: OVER)

 seft b: ( arg -- result )
   Takes one value from the stack, performs an operation, and puts one
   result on the stack

 seft b2: ( arg1 arg2 -- res1 res2 )
   Takes two values from the stack, performs an operation, and puts two
   results back on the stack. (Examples: SWAP, polar to rectangular conversion)

 seft c: ( arg1 arg2 -- result )
   Takes two values from the stack, performs an operation, and puts one
   result on the stack (Examples: DROP, most binary operators)

The RIES symbols of each seft are:

  0: ' ' (blank space)

  a: x 1 2 p 3 e 4 5 f 6 7 8 9 (x, the digits, and the constants pi, e and phi)

  b: r s q l n S C T I (reciprocal, squared, square root, ln, negate, sine,
                        cosine, tangent, identity)

  c: + - * / ^ v L (add, subtract, multiply, divide, exponent, root, logarithm)

For ease in debugging, most symbols have letters that make sense, but
not always (e.g. "f" for phi, the golden ratio; "v" for root, which
is meant to represent the v-shaped part of the standard root sign).
The only constants are the digits 1 through 9 -- no zero, and no
multidigits or decimal fractions. To get 10 you have to do "25*" or
something similar; for 1.5 you have to do "32/". A fair amount of
effort has been put into setting the symbol scores such that the score
of "25*" is only a little higher than the score for "9".

example cases for deriving the symbol weights:

  (+) ~= (*) (because both occur equally often)
  (-) > (+), but not by much
  1, 2, 3, 4, 5, ... degrade gracefully and kind of like a slide rule
  (14*) = (22*), etc. (implies (n) proportional to log(n) for 1<=n<=9)
  (33*) = (9) (implies (*) = [..] where [..] is the weight of any two symbols)
  (25*) > (9), but only by a little (no problems so far)
  [2q] ~= [5] :: therefore (2q) + [.] ~= (5)
  [x2v] = [xq] :: (2v) + [.] = (q)
  [x2^] = [xs] :: (2^) + [.] = (s)
  [99+] can be >> [55*] (because smaller numbers are more likely)

%%% complexity score for a symbol can be context sensitive, e.g. 'l'
takes a higher score the second time it is used in an equation -- so
long as its range of possible scores is within the total range for its
seft. This might be a good way to eliminate some of the nonintuitive
aspects of the current system.

Once a set of symbol scores is worked out, it can be adjusted by
adding any constant to all the values (this adds a bias for long
expressions, or a bias against long expressions. In the above
"normalized" examples there is no bias, but the shorter ones will get
generated first anyway.)

The lists start out small and grow as the program searches. On the
first pass, the left-hand list consists simply of {X}, the single-
element expression for the search value, and the right-hand list
consists only of a few common constants, for example {1, 2, *e*,
*pi*}. After a few passes the left-hand list will include things like
"xv" (sqrt(x)) and "x1+" (x+1), and the right-hand side will have
similar forms such as "2v" (sqrt(2)) and "23/" (2/3). At that point,
if X is 4/9 a match will be found between "xv" and "23/", even though
the combination of "x" and "49/" has fewer symbols overall, because
the former matching is more evenly balanced. Another way of saying
this is that, since the complexity scores of "xv" and "23/" are both
lower than the score of "49/", the "xv = 23/" matching is found before
"49/" even gets a chance to be generated.

To minimize time spent generating nonsense expressions, expressions
are generated from "forms". A "form" is a symbolic expression of
expression syntax, like "aabc" for "12q+". Each letter represents a
different type of stack operation (called "seft" above). Type "a"
pushes one item, type "b" leaves the stack with the same number of
items, and "c" leaves the stack with one less item. A form is legal if
the stack depth (the "a" count minus the "c" count) is > 0 at all
times and = 1 at the end. Thus all forms start with an "a". The number
of forms for N={1,2,3,...,8} is {1,1,2,4,9,21,51,127} (the Motzkin
numbers; Sloane's A1006). By comparison, the total number of forms
without these restrictions would be 3^(N-1):
{1,3,9,27,81,242,729,2401}. The savings is considerable. In the
left-hand list the first symbol will be an "x" in all generated
expressions. Forms are generated on the fly and never stored in
memory. As the search proceeds, longer forms are generated as needed.

For each form there is a "minimum" expression and a "maximum"
expression, as rated by complexity score. For example, "x1+" is the
minimum expression for form "aac" and "99L" (log base 9 of 9) is the
maximum. These minimum and maximum expressions are easy to find,
because each symbol has its own score and the expression score is just
the sum of the symbol score. Thus it is easy to determine, at the
beginning of each pass, which forms can generate expressions that are
within the current complexity range.

Expressions are generated from each valid form. The generation of
expressions uses a recursive backtracking algorithm, starting with the
first symbol and moving to the right. At each step, it checks to see
of the symbols we have so far still allow an expression which falls
within the complexity range -- if not, the latest symbol is dropped or
changed. It also avoids generating certain patterns of symbols which
would be of no computational value, or which are always equivalent to
a different (sometimes shorter) set of symbols. Examples of this
optimization are:

  - "aa-" for any seft a symbol would be 0, so is not generated.

  - "aa+" is the same as "a2*", so is not generated.

  - "aa*" is the same as "as", so is not generated.

  - "aa/" and "aal" are equivalent to 1, and are not generated.

  - Almost any operator after "1" is not generated ("1x", "1/", "1r",
"1^", "1v", "1l", "1L", "1v" all do nothing useful)

  - "2^" and "2v" are equivalent to "s" and "q" respectively, and are
not generated.

  - "pS" is equivalent to 0

  - "sq", "qs", "nn", "rr" all amount to nothing and are not generated.

The expressions are also evaluated while they are being generated. Any
subexpression that causes an error, such as divide-by-zero, can be
skipped along with all expressions that start with that subexpression.
For example, any expression starting with a constant followed by "nq",
such as "2nq" which means "sqrt(-2)", will be skipped.

As the search proceeds, each equation (that is, each combination of an
LHS with an RHS) is checked to see what value X would have to be for
the equation to work out perfectly, and the difference between this X
and the user's supplied number is called the "delta". An equation
becomes a "record-setter" if its delta is smaller than any delta seen
so far.

If a strict "non-fuzzy" comparison were made, due to roundoff errors
it would be possible for the same solution to be found twice. For
example, if X is near 4/9 = 0.44444.., two solutions are "x = 49/" and
"xq = 23/". One solution might be printed after the other because of
roundoff in the square-root operation. To avoid this, every time a new
record-setter is found, the criterion for another record is set to
0.999 times the new delta.

Determining the value of X for which the equation works out perfectly
is a hard problem. In theory one would have to solve the equation for
X. That involves lots of shifting and rearranging of symbols, and if
there is more than one X in the equation it can get into some rather
complicated algorithms.

Instead, this program compares the LHS and RHS values directly. (The
difference between the LHS and RHS is the "lhs/rhs diff", and varies
from the "delta"). This leads to several problems. Sometimes two sides
of the equation form a really good match only because both sides
involve something raised to a very small power (or a very big root --
same thing). An example of this type of problem is 1.017262042 =
[49sv] (the 81st root of 4) and 1.017313996 = [38sv] (the 64th root of
3). They differ by only 1 part in 20000, but only because both numbers
are very close to 1. All 81st and 64th roots of small integers are
close to 1. This is referred to as the "error margin problem" (or
"loss of significance").

There is also the problem of zero subexpressions and constant
subexpressions involving X. An example is "x1el-^ = 42sL": The
subexpression "1el-" is "1-ln(e)" which evaluates to 0, and the entire
LHS is x^(1-ln(e))" which will always be 1. The right-hand side (log
base 2^2 of 4) is also 1, so this equation works for any X. In order
for the program to work, such "solutions" have to be detected and
eliminated. This is the "tautology problem".

Finally, there is the issue of reporting what X would perfectly solve
each equation. This is a desirable feature of the program because
users will often want to find out what the "exact" value would be for
a given equation without actually doing the algebra and calculations
themselves. In cases like [xle+q] = [1e-s], where there is only one X
in the equation, this could be done by the computer. However, there are
lots of important cases like [xx^] = [52*] where an analytic solution
does not exist and the value of X has to be found by some other method,
such as a numeric method. This is the "root-finding problem"

Conveniently, all three problems (the error-margin problem, the
tautology problem, and the root-finding problem) are handled in the
same way: by calculating the *derivative* of all terms and
subexpressions on the LHS. The derivative, when multiplied by the
lhs/rhs diff, gives a very good approximation to the delta, which as
mentioned above is the amount that X would need to be changed to make
the equation a perfect match.

This solves the error margin problem because it equalizes the field --
all equations can get rated in terms of how much X has to change to
make LHS and RHS match, rather than how much the LHS would have to change.

It solves the tautology problem because, by definition, an LHS with
a zero derivative is constant with respect to X, and therefore
constitutes a tautology solution.

It also solves the root-finding problem quite conveniently. If you have
an equation like X^X = 10, and you have an approximation for X, you can
use derivatives and Newton's method to find a better approximation for
X. When RIES takes the derivative of the LHS and multiplies it by the
lhs/rhs diff, it is essentially performing one step of Newton's method.
The resulting value can be added to X to form a better approximation to
the root of the equation.

Derivatives are evaluated for LHS expressions as the expressions are
generated. In the following table, the calculation of the derivative
for each symbol is shown, based on the values of the operands (A and
B) and their derivatives (da and db). Some of these will be familiar
to anyone who has taken calculus:

  seft '0' symbols:
  ' '  no-op

  (other sefts will go here: roll, drop, dup, over, etc.)

  seft 'a' symbols:
  3  any constant   0
  x  target         1
  seft 'b' symbols:
  e  exponential    e^A da
  l  natural log    da / A
  n  negate         - da
  s  squared        2 A da
  q  square root    da / 2 sqrt(A)
  r  reciprocal     - da / A^2
  A  arctan(a/b)    da / (1+A^2)
  C  cosine         -sin(A) da
  G  Gamma          %%% reserved (mutex with !)
  !  factorial      %%% reserved (mutex with G)
  S  sine           cos(A) da
  T  tangent        (1 + tan^2(A)) da
  W  LambertW       W(a)/(a(1+W(a))) da
  seft 'c' symbols:
  +  plus           da + db
  -  minus          da - db
  *  times          A db + B da            (note "product rule" below)
  /  divide         B da - A db / B^2      (note "quotient rule" below)
  ^  power          A^B (ln(A) db + B da / A)
  v  Bth root       BvA (da / A B - db ln(A) / B^2)
  L  log base B     (da / A ln B) - (ln A / ln B) (db / B ln B)
  #  paste digits   %%% reserved

  product rule   d/dx f(x) g(x) = df g(x) + dg f(x)
  quotient rule  d/dx f(x)/g(x) = (df g(x) - dg f(x)) / g(x)^2
  chain rule     d/dx f(g(x)) = df/dg dg/dx

  An example of applying the above to derive the formula for the derivative
  of A^B (where A and B are both functions of x):

    A^B = exp(ln(A) B)
    d/dx exp(Y) = exp(Y) dY
    d/dx ln(A) = 1/A da
    d/dx ln(A) B = db ln(A) + B/A da      (by "product rule")
    d/dx A^B = d/dx exp(ln(A) B)
             = exp(ln(A) B) d/dx (ln(A) B)
             = exp(ln(A) B) (db ln(A) + B/A da)
             = A^B (db ln(A) + da B / A)
             = A^B ln(A) db + B A^(B-1) da    (standard form)

    d/dx ln(A) / ln(B) = (ln(B) da / A - ln(A) db / B) / ln(B)^2

    log(a+b) = log(a(1+b/a))
             = log(a) + log(1+b/a)


  Another example for atan2(a,b):

    d/dx(arctan(a/b)) = 1/((A/B)^2+1) d/dx(A/B)
                      = 1/((A/B)^2+1) (B da - A db / B^2)
                      = (B da - A db) / (B^2 * ((A/B)^2+1))
                      = (B da - A db) / (A^2 + B^2)

  which agrees with Wikipedia (at en.wikipedia.org/wiki/Atan2#Derivative):

    Informally representing the function atan2 as the angle function
    theta(x,y) = atan2(y,x) [...] yields the following for the differential:

      dtheta = d/dx(atan2(y,x) dx) + d/dy(atan2(y,x) dy)
             = (-y/(x^2+y^2)) dx + (x/(x^2+y^2)) dy

   */ /*

It is fairly easy to show that in any equation that constitutes a
solution, if any expressions or subexpressons evaluate to 0, the
entire equation can be replaced with an equivalent form that does not
involve 0. The equivalent form is never more complex and is usually
simpler. It can also be shown that any solution involving a
subexpression that contains x and has a 0 derivative can be reduced
to a simpler solution that does not.

                    Why RIES Calculates Derivatives

Consider the value X = 2.5063. Each of the following solutions (or an
algebraic equivalent) will appear when you run RIES on it:

       2 x = 5                     for X = 2.5              *
       x^2 = 2 pi                  for X = 2.506628274631   *
       x^x = 1+9                   for X = 2.5061841455888
     x^2+e = 9                     for X = 2.5063356063267

The exact solutions to these equations are all different values,
of course, and they all form successively closer approximations to
the supplied value 2.5063.

*/ /*

%%% the following description doesn't match the numbers; don't know
how to fix it...

Look at the last two. Notice that the supplied value, 2.5063, is
between the exact solutions of these two. Also, the equations can both
be expressed in a different way:

     2 X = 5      instead of   X = 5/2
     X^2 = 2 pi   instead of   X = sqrt(2 pi)

Consider what would have happened if the last digit had been one
higher or one smaller. How much does it change the "closeness" of the
match? With the original two equations, where there is just an "X" on
the left-hand side, this is easy to figure out: if X is 0.0001 lower,
it's 0.0001 closer fit for the first equation, and 0.0001 further from
the second. But look at the alternate forms: If you subtract 0.0001
from X, that subtracts 0.0002 from 2 X, but it subtracts 0.0005 from
X^2. That's a big difference -- if we're using these forms of the
equations (as RIES does) to determine how well the two sides match,
then this altered value of X makes the match "move" further with
respect to one equation than it does with respect to the other!

The reason this happens is, of course, because we're looking at an
expresson on the left-hand side, rather than just a single "X".
Putting an expression on the left-hand side makes it harder to see how
close a match you've got.

If you still don't believe this, consider 1.047246, and exclude sine
and cosine from the function set:

<pre>
  ries 1.047246 -NSC

  Your target value: T =         1.047246

            3 x = pi                for X = T - 4.84488e-05
            x^5 = 3 root-of 2       for X = T + 4.81228e-05
  1/ln(sqrt(x)) = 4^e               for X = T + 1.77179e-05
  ...
</pre>

We see that 1.047246 is an equally good solution for the following:

     x = pi / 3          (too high by 0.00005)
     x = 15th root of 2   (too low by 0.00005)

but if you express the solutions as the #ries# output does:

     3 x = pi            (3.14172, too high by 0.00013)
     x^5 = cuberoot(2)   (1.25992, too low by 0.00029)

suddenly it looks like the 3 x = pi solution is more than twice as
good. #ries# notices this and compensates for it regardless of the
form in which the equation is actually found. If you run RIES on the
value 1.047246 it will present both solutions, in the following form:

     X * 3 = pi
     X ^ 5 = 3 v 2

The philosophy adopted by RIES is that the "true" form for evaluating
the closeness of a match is the form where there is just one X, and
nothing else, on the left-hand-side of the equation, and just numbers
and symbols, but no X's, on the right-hand-side. (Let's call this the
"normalized form".) Put all equations into normalized form, calculate
the value on the right and look at the difference between X and this
value to determine how good the match is.

But now go back to our 2.5063 example above -- one of its solutions
was:

     X^X = 10

This equation *cannot* be reduced to something with just one X on the
left-hand side -- there is no "inverse-of-X-to-the-X" function fn[1]. What
does RIES do?

fnd[1] Actually, there is, but it uses an obscure function called the
"Lambert W function" which is the inverse of *y*=*xe^{x}*. More
details are [here|+numbers:xxy_root2] if you are interested.

RIES calculates derivatives. By using the value of 2.5063 for X and
calculating the derivative of X^X for this value of X, you get (about)
19. A derivative of 19 means that any small change in X will cause a
19-times-bigger change in X^X. That's important, because it allows us
to compare the closeness of the match on "equal footing" with other,
normalized equations like X = sqrt(2 pi).

Derivatives work so well, in fact, that RIES does not even have to
bother solving its equations for x. ven if it were easy to do this
(which it is not), leaving the equations unsolved is still an
important speed improvement. Part of the reason RIES is so fast is
that it generates left-hand-sides and right-hand-sides separately
(like a wl[Bidirectional_search] in graph theory) and tries all the
combinations to find possible solutions. It can do this a lot quicker
because a half-equation is smaller than a full equation, and therefore
there are less possibilities to check out.

Furthermore, derivatives allow RIES to quickly and easily check a
possible equation to discover the value that X would have to be in
order to both sides to match exactly (it is essentially performing one
step of Newton's method).



Examples of LHS and RHS expressions for the test case 2.5063. These
are shown in groups that correspond to the pairs that would actually
generate matches in a search:

  expression  value    deriv.
   5/2        2.5       0
   hyprt(10)  2.506184  0
   x          2.5063    1.0

   ,/25       5         0
   2 x        5.0126    2.0

   x^2        6.281540  5.01
   2 pi       6.283185  0

   x^2+e      8.999822  5.01
   9          9         0

   9+1        10        0
   x^x        10.00222  19.19

The derivative is based on the concept of an imagined error-bar in x
that is assumed to be small enough so that all reported matches are
relevant, but which is not zero. Thus, it is in units of the
infinitesimal quantity "epsilon".

Let us now imagine that there is a constant "g" equal to (pi-1) *
2.5063 ~= 5.367473. Then we would have the following grouping:

  expression  value    deriv.
   pi^2-2     7.869604  0
   x+g        7.873774  1.0
   pi x       7.873774  3.14

The matching "pi x = pi^2-2" constitutes a closer match than "x+g =
pi^2/2" because in the former case, x would only need to be decreased
by about 1/pi as much to make it an exact match. So, the closeness of
a match is measured as |LHS-RHS|/derivative, where smaller is better.

To enable actual error bars to be provided with data, a value of
"epsilon" must be adopted for use with notionally precise supplied
values. This can be gleaned from the number of supplied digits in the
input, or the precision of the floating point format can be used. In
the latter case we would take the data value divided by 2 to the power
of the number of digits in the mantissa.

   */ /*

Future enhancements:

  See "UNFINISHED WORK" section above

 */ /*

REVISION HISTORY

 20000207 Begin (it does not do much except parse the parameters)
 20000208 Add parsing of level adjust and more design comments
 20000209 Write code to generate forms. "Discover" the Motzkin numbers.
 20000210 Optimize gf.1() by making it compute the stack depth as it
goes along rather than repeating the whole stack history on each test.
This improves time to generate all forms of length <=19 from 188
seconds down to to 37.9 seconds. Add tracking of min and max weights,
and start writing expression generator.
 20000215 It now generates forms within min and max possible
complexity limits, to avoid generating expressions from forms that
cannot possibly fall within the current complexity limit. Add a little
optimization; speeds up a deep search from 53 seconds to 37 seconds.
 20000217 It generates expressions, but doesn't prune for complexity
or evaluate. For the initial groups of 5, the numbers of expressions
evaluted are: {0, 12, 12, 72, 1368, 1368, 17928, 345672, 5875272,...}
 20000217 Prune for complexity limits; each expression is now
generated exactly once. The numbers are down to: {0, 1, 3, 18, 69,
182, 1046, 5358, 27123,...} Still need to prune foolishness like "11+"
and "nn".
 20000217 Prune almost all obvious trivial patterns. The numbers are
now down to {0,1,3,13,43,122,486,2186,9775,...}. Then prune [JK+] and
[JK-] for small integers, [jK*] and [jK+] for any j<K, and a couple
lesser things, and they're down to {...,43,106,391,1861,8608,...}.

20000217 Write metastack routines and exec(), and prune on eval errors
like divide-by-zero. This brings the numbers down to
{0,1,3,13,41,96,336,1605,7129,...}

20000217 Add using.x (formerly "on_rhs") variable and AM_RHS so it
generates LHS and RHS expressions, and make main outer loop call
gen_forms twice. In the LHS case there is lots of pruning. Figure out
that there is a substantial problem with error factors (derivative
of X) that will make lots of bogus "solutions" show up in the output.

20000218 Finish figuring out how to deal with the bogus solutions --
compute derivatives on all LHS expressions. This also solves the
"trivial solutions" problem (e.g. "x - x = ln(1)"

20000220 Add derivative calculation to exec() and the ms_xxx
routines. Initial output looks good, but I'm a bit worried about the
very high derivative values on things like [x8s^] -- they might
cause lots of bogus matches on things that aren't even close, like
"x8s^ = 6".

20000220 Add pruning of zero subexpressions, and of LHS expressions
with zero derivative. Examine output to check this; discover a bug
that ultimately turns out to be because I had ">> 2" instead of ">> 1"
in the s[] and ds[] declarations in struct metastack.

20000220 Refine the termination condition (now it counts generated
expressions, rather than cutting off at a certain complexity score --
this is to gain independence from the specifics of the weights) and
discover another dimensioning bug in struct metastack. Benchmark
changes in PASS_GRAN: When it is set to 4, 2, and 1 the execution time
for generating all expressions up to complexity 64 (2236462
expressions) is 3.57, 4.16, and 5.24 respectively. This actually a lot
better than I thought it would be -- I thought there would be a lot
more overhead from repeating the same subexpression evaluations over
and over again.
*/ /*
20000220 Make it increase LHS and RHS complexity limits at independent
rates, such that the population remains equal between the two sides.
Add a bunch of comments documenting what's been done so far.

20000221 Write bt.insert(); it now reports exact matches! It finds,
for example, [x3+] = [4s] for X=13. However, for X=143 it reports
[xrx*] = [1], with a derivative of 1.9e-19. Add PRUNE_DERIV to try to
fix this, and it starts reporting [xxqL] = [2] with a derivative of
0.00258248 -- this turns out to be a bug in the deriv formula for 'q'.
Eventually increase PRUNE_DERIV to around 1e-14, then decide to make
it a variable and add p_ovr and n_ovr. Write bt_prev() and bt_next(),
but not using them yet.

20000221 Figure out that I can check just the nearest LHS on either
side of an RHS, and vice versa. Write check.sides() and check.match.
It now finds answers and prints them out!

20000221 Fix bug in exact-match reporting. When given 1.5065916, it
reports:

   match: [xe+s1+] = [p6*]  (solution is X+5.14855e-08)

Make it report delta you'd have to add to X to get each match to be
exact. Test cases that currently produce bogus results: 'ries -l1 27',
'ries -0.28676844', 'ries 403'

20000222 Adjust best.match by 0.999 each time to avoid long strings of
roundoff-error results. This fixes the './ries 403' case. Rename to
"ries" (it used to be called "misc"). Start analyzing memory usage.
Figure out how to save some memory in the node structures, and more
importantly, how to group nodes together in physical memory such that
the program degrades more gracefully when physical memory limits are
exceeded.

20000222 Implement -S and -O command-line options. This has the
side-effect of making certain bogus match bugs easier and quicker to
reproduce.

20000222 Change -S and -O to -N and -S respectively.

20000222 Convert manpage to #nroff# format. Improve implementation of
-S/-O/-N precedence. Implement -O option. Figure out why restricted
symbolsets create very small numbers of equations (the level controls
the number of expressions generated, which is always larger than the
number of expressions inserted in the tree)

20000222 Fix bug that caused negative X's to give no solutions -- it
was initializing best.match to a fraction of X, and not taking the
absolute value! Implement -i option so I can find an expression for
70458.

20000223 Ignore -i if target is non-integer. Adjust weights for seft
'b' operators; 'l' now appears much less often in expressions. Add -y
option.

20000223 Write add.rule(); convert all AM_xx rules to add.rule()
calls. Now the AM_xx rules degrade gracefully with the symbolset
options. Change -y option to -x.

20000223 Clean up formatting in report.match(); add complexity score to
output. Write perl script to benchmark and take statistics on -i option
for a wide rance of integers; leave it running overnight. Change all 'int'
to 's16'.
*/ /*
20000224 Today's date as a mathematical expression:
                   (((4^(4^(1/e))-pi)^2)-pi)^2  the repetition is cool.

20000224 Write initial version of infix.1, then add a few rules
(ordering of bare symbols in '+' and '*'; always reverse order for 'v'
and 's'). Add -F option.

20000224 Add memory usage statistic.

20000225 Write infix.preproc; implement parentheses precedence.

20000225 Write eval() and newton(); printed X values are now (almost
always) exact roots of their equations. Add AM_1K rules.

20000226 Add copyright and GPL notices; add URL to first printf

20000226 Add symbol definition strings

20000227 Add sin and cos operators. Eliminate several bogus exact
match errors related to roundoff and loss of accuracy, e.g.
cosine(0.0001). Fix major bug in k_prune.deriv test: it only pruned
positive small derivs, not negative small derivs. Add symbol FORTH
names and write postfix.formatter (but it isn't used yet). Don't report
matches if newton() returned an error.

20000227 Add -ie variant to -i option (only_exact)

20000228 Kill another missing-fabs bug; remove prune on "almost exact
matches" which is now adequately covered by the derivative tests.

20000228 Add 'E' operator, AM_l and AM_E attributes, and PS_REVPOW.
check.match now uses newton() to evaluate score more accurately.
Add loss-of-significance test to '-' operator.

20000228 check.match calling newton was royally slowing things down.
Now it uses the old, much quicker test and then uses newton() as a
confirmation test. This cuts time for "ries 2.5063" from 4.09 down to
1.39 on the Cyrix 180 (which is about what it has been since 0223).
Add statistics of pruned subexpressions ("dead-ends").

20000229 Fix bug in newton() that prevented success if target was
negative. Add debug printf's 'mnr'.

20000229 Add time display; add debug printf's 'opqsABCDEFG'.

20000229 Add debug printf's 'Hy' and a few notes about complex analytic
definitions

20000301 Change debug printf 's'; add another loss-of-significance test
to '+' and '-'; add debug printf's 'IJKLtuvx'. Add 'I' operator and
special-case tests for no defined symbols of each seft (this is a massive
optimization for "ries -ie 7 '-S1+*-/^v'")

20000302 Add debug printf 'w' and improve 'r'. Add arctan function (but
not using it yet) and some notes about derivatives.

200003xx Benchmarks on a 333-MHz Celeron:

  -command-------------------------- -mem-  time  equations
  ries     2.5063141592653589         960K   0.3  9.5619e7
  ries -l1 2.5063141592653589        3072K   1.5  1.0306e9
  ries -l2 2.5063141592653589        10.7M   6.5  1.2832e10
  ries -l3 2.5063141592653589        32.7M  24.4  1.2042e11

200203xx Benchmarks on an 800-MHz PowerPC G4 (iMac, model M6498LL/A):

  -command-------------------------- -mem-  time  equations
  ries     2.5063141592653589         960K   0.1  9.5062e7
  ries -l1 2.5063141592653589        3072K   0.8  1.0248e9
  ries -l2 2.5063141592653589        10.6M   3.6  1.2765e10
  ries -l3 2.5063141592653589        32.6M  14.3  1.1976e11
  ries -l4 2.5063141592653589         114M  70.3  1.4609e12

20020610 Figures from an unknown test (I can't find a record of the
details). I think the numbers are: memory usage; total
expressions/total distinct; total equations.

 -l0  1136K   32198/14576  53165000
 -l1  2876K  145183/51492  666660000
 -l2  7632K  530668/152904 5712000000
 -l3 27100K 2311204/559419 78120000000

200310xx Benchmarks on an 800-MHz PowerPC G4 (iBook G4, model M9164LL/A):

  -command-------------------------- -mem-  time  equations
  ries     2.5063141592653589         960K   0.1  9.5062e7
  ries -l1 2.5063141592653589        3072K   0.7  1.0248e9
  ries -l2 2.5063141592653589        10.6M   3.2  1.2765e10
  ries -l3 2.5063141592653589        32.6M  12.8  1.1976e11
  ries -l4 2.5063141592653589         114M  62.3  1.4609e12
  ries -l5 2.5063141592653589         398M   388  1.7825e13

20050715 Benchmarks on a 2-GHz PowerPC G5 (part of a dual system,
model M9455LL/A):

  -command-------------------------- -mem-  time  equations
  ries     2.5063141592653589         960K   0.0  9.5062e7
  ries -l1 2.5063141592653589        3072K   0.2  1.0248e9
  ries -l2 2.5063141592653589        10.6M   1.5  1.2765e10
  ries -l3 2.5063141592653589        32.6M   6.6  1.1976e11
  ries -l4 2.5063141592653589         114M  36.2  1.4609e12
  ries -l5 2.5063141592653589         398M   260  1.7825e13
  ries -l6 2.5063141592653589        1.41G  2183  2.2297e14

20070511 Alan Eliasen emails me to tell me that 'ries -l4 193707721'
gives the spurious result "x.(1/S(p))/x = 1/S(p) (exact match) {107}".
This is a problem I have known about for a while. I reproduce the
problem and reduce the symbol set to make it appear faster; it can be
reproduced with:

   ries -F '-SxpSr/ *' -l0 1002353667

Add rule ("",'S', AM_pi, 0), which eliminates "sin(pi)". It now reports
the following more complex version of the same bug:

   x.(1/S(p.p/p))/x = 1/S(p.p/p)  (exact match)  {181}

{This isn't actually a problem, it's doing what it should: When the
symbol '1' is not allowed, then "K/K" for any constant K *is* allowed.
However "ries -F '-SxpSr/ *1' -l1 1002353667" does produce the bug.
See 20090513 for fix. -20090513}

20090216 Clean up the comments, and add the "ARCHITECTURE" section,
plus a few notes on how to approach a multithreaded implementation.
 Here are the benchmarks from the 733 MHz Pentium 3 (from the manpage,
which I have now decided to update):

     memory        equations  digits     runtime
      usage           tested  matched  (733MHz P3)
 -l0  960K         95,000,000    6+      0.1 sec
 -l1  3.1M      1,030,000,000    7+      0.7 sec
 -l2  11 M     12,800,000,000    8+      3.3 sec
 -l3  33 M    120,000,000,000    9+      12 sec
 -l4  115M  1,470,000,000,000   11+      63 sec

and here are the benchmarks from the 2.33-GHz Core 2 Duo (MacBook
Pro):

  -command-------------------------- -mem-  time  equations
  ries     2.5063141592653589         960K   0.0  9.4018e7
  ries -l1 2.5063141592653589        3008K   0.1  1.0153e9
  ries -l2 2.5063141592653589        10.6M   0.6  1.2643e10
  ries -l3 2.5063141592653589        32.4M   2.7  1.1865e11
  ries -l4 2.5063141592653589         113M  13.6  1.4461e12
  ries -l5 2.5063141592653589         396M  81.7  1.7642e13
  ries -l6 2.5063141592653589        1.40G   651  2.2062e14

20090510 Add standard output format, and make it the default.
"-F" now takes a numeric argument (which formerly it did not)
and "-F" alone defaults to "-F0" which is what it did before.
{Formerly "F" was for "FORTH", and now it stands for "Format".}
  Future idea: A -W option to view weights (same as -S) or define
weights individually. --include could then be used to select weight
presets (e.g. a set of weights for electrical engineering). {This was
implemented on 20121209}

20090511 In infix modes, display operator 'L' (log base A of B)
in advance of both of its arguments. No longer put parens around
single-symbol argument of negate (e.g. emit "-x" instead of "-(x)").

20090513 Add rules to put bare 'x' after non-x-containing expressions
and bare 'pi', 'phi' and 'e'. Finish postfix.formatter (which had
never been brought into spec with the other formatters) and add it
as a 4th output option.
  As noted at 20070511, there has been a problem with expressions like
"pi*pi/pi". The command "ries '-SxpSr/ *1' -l1 1002353667 -F0" currently
shows the problem. I fix this by adding the three-symbol pattern AM_KxK
and the rule ("", '/', AM_KxK, 0).

20090515 "ries -l6 2.5063141592653589" running uncontested on the
2.26-GHz Nehalem uses 1.40G of memory and tests 2.2154e14 equations in
442.4 seconds. Another test: 1.3525746932102463: 1.43G, 2.3011e14,
463.2. These times are 47% and 40% faster than the Core 2 Duo.
  Slight improvements to -Ds output. Improve rules for emitting "*",
" ", or "" for multiplication in different situations. Add rule
("12345678n", '-', AM_jK), and def_amkey/sym_amkey[] optimization.

20090808 Remove "val" parameter of my_alloc()

*/ /*

20101218 Increase default level to -l2 (while preserving the effect
of all '-l' values when such an option is provided: "ries 1.234" is now
the same as "ries -l2 1.234", but "ries -l1 1.234" does the same thing
it used to do).
  Print 'x' instead of 'X'; use strcmp and strncmp in a few places.

20111216 Increase MAX_ELEN from 16 to 19 since we have room (struct
size is 64 in either case). Comment out unused obt_xxx declarations.

20111218 check.sides now checks more than one neighboring LHS if the
newly-inserted node is an RHS. Oddly, this does not cause more matches
to be reported, and I am not sure why.

20111219 print.end now displays max LHS and RHS complexity values; add
--find.expression option.
  Add k.vanished_dx and use it in several places to enforce a stricter
constraint on derivatives. This eliminates many of the more obscure
tautologies like "x^(4/ln(sqrt(x)))".
  Add --eval-expression option, and error numbers and error strings to
support it.

20111220 Restore ERR_EXEC_TRIG_LOW_DX error for RHS expressions;
restore legend in normal infix display format; eval() now reports
ERR_EVAL_TOO_LONG.

20111221 Slight improvements to postfix.formatter; report (most)
unrecognized or badly-formatted command options.
  Catch a few more tautologies by comparing magnitude of dx to
magnitude of x.

20111222 Add --version option.

20111223 -S without any symbols displays the symbol table and exits.

20111224 Enhance -Ds output by explicitly mentioning the Newton-Raphson
step.

20111226 Add loss-of-significance tests to each of the operators that
seem to need it: ln(x), e^x, sin, cos, +, -, a^b, a,/b, and log_a(b).
  Significance loss errors have always been easy to find in ries
output, by giving a target value that has a simple solution (like
"ries -1.4142135", which is used throughout this note). After ries
gives the simple solution (in this case "x^2 = 2"), subsequent
solutions can incorporate the simple solution in a
loss-of-significance tautology.
  The first of these solutions was {2012.0505: Now considered okay, now
that addition actually checks for loss of precision explicitly}
"1/(x+sqrt(2)) = e^(4^2) {86}". This incorporates the direct solution
into a tautology by turning "x^2=2" into "x+sqrt(2)=0", which isn't
exactly 0 because x is not exactly sqrt(2). Significance can be said
to be lost because many of the significant bits of the two terms "x"
and "sqrt(2)" cancel each other out in the addition. {2012.0505: In
this case, handled in exec() case '+', the loss of significance in
addition is easy to test rigorously: if (a+b)-b is equal to a, and
(a+b)-a is equal to b, then no information has in fact been lost.}
  This "x + -x" case is probably the most common source of significance
loss that I had not addressed until today. I fixed it by adding the
"fabs(rv) < (fabs(a) * k_sig_loss)" tests in exec() label
"add_common:"
  "ries -1.4142135" then gave "-ln(x^2-1) = 1/7^8 {109}" until I added
the k_sig_loss test in exec() case 'l'
  It then gave "-sin(pi x^2) = (1/5)^9 {111}" until I added the
k_sig_loss test in exec() case 'S'
  It then gave "log_2(x^2-1) = 1/-(e^(e^e)) {117}" until I added the
k_sig_loss test in exec() case 'L'
  Then it gave "1/(1/(x-1)-x) = (e^(4^2)),/2 {122}" until I added the
k_sig_loss test in exec() case 'v'
  It then gave "(x-1/(x+1))^2 = e^(1/(e^7)^2) {125}" until I added the
k_sig_loss test in exec() case 'E'
  After doing all of the above there was only one operator ('^') that
seemed to need a k_sig_loss test.

20111227 Change many details of formatting and printing to accomodate
the recent changes in handling of significant digits. Add constants
k_nominal_digits and k_usable_digits and associated format strings
fmt_g_xxx and use these in most places a floating-point value is
displayed. Redo the equation justify (space-padding) code to make the
best use of 80 columns while still showing centered equations and 15
significant digits when the -x option is given.
  Add init.numerics(); k_phi and k_pi are now computed from scratch.
  New 'z' option to -D and debug_z flag to show messages printed by
init.numerics().

*/ /*

20111228
  Add more k_sig_loss tests in sin and cos.
  Display CPU time as "%.3f" rather than the old "%d.%d" that showed
seconds and tenths.
  RIES now exits when (best.match < k_0) regardless of the got.exact
flag; this fixes a bug that would cause RIES to loop forever if it had
not yet gotten an "exact match" at the point when best.match goes
negative.
  init.numerics() now computes k_e and detects the size of the ULP
(Unit in Last Place or "least significant bit").
  Add --min-match.distance option and g_min.matchsize to prevent
reporting of really close matches (useful in e.g. finding classical
approximation formulas for pi)
  Add --significance-loss-margin and k_sig_loss to allow going back
to RIES behaviour before all the new k_sig_loss tests were added.
  Share code by combining two exit tests into exact_exit() routine.
  ln(a) and log_a(b) functions now no longer reject a case like
ln(1.23e10) (in which only about 1 significant digit is lost)
  k_prune.deriv had been used for three different purposes, and now is
being replaced with three separate variables. The three purposes of
k_prune.deriv were:
 1. Detecting convergence in newton(). Now using new k_newton_settled
 2. Pruning subexpression tautologies in ge.2(). Now using k.vanished_dx
 3. Setting n_ovr and p_ovr in init2(). For now this is unchanged.

20111229 The big trig change: S and C now take units of pi radians, so
"3rS" now produces the value sin(pi/3)=0.866025... To support this I
also add the --trig-argument-scale option, which if given with the
parameter "1" gives the old trig units (radians). As a bonus, users
can now get degrees or grads or any other angle units.
  To avoid confusion, the trig functions are now called "sinpi" and
"cospi" in the output, unless --trig-argument-scale is set to
something other than pi; and the function definitions make this
obvious by adding a message like "For the trig functions, 360 units
are a full circle." at the end.
  While I'm at it, I add the tangent function 'T'. This had been in
the manual and in the symbolset since the start, but had not yet been
written in exec(). This of course makes a lot of things solve more
quickly, such as Gosper's "0.9674026381747" (the root of "tan(x) =
3x/2", which previously needed a -l5 search, but with the unit change
and the tangent function, a -l0 search is sufficient.)
  One result of the trig units change is that there are a lot more
natural identities connecting common fractions and small radicals. For
example sqrt(2) is 2 sinpi(1/4), sqrt(3) is 2 sinpi(1/3), etc. This
makes ries generate and insert fewer expressions at a given complexity
level. It's so good in fact that you get higher complexity *and* less
memory usage even after adding the tangent function. Here are some
benchmarks, showing how much memory it used up and what level of
complexity it was able to achieve:
                     --------old--------     --------new--------
                      complexity              complexity
  target-value         LHS:RHS memory time     LHS:RHS memory time
  1.5063 -l2            66:61   13632 KiB .319  66:62  12928  .342
  .328106566874978253   80:76  486848 KiB 39.5  81:76  461312 38.0
  1.9511889024071       80:76  494016 KiB 39.7  81:76  458432 37.8
  .922524879060934752   80:76  499264 KiB 40.4  80:77  464320 40.8

(All except the first is at level -l5; the old figures were measured
using the options "--trig-argument-scale 1 -NT")

20111230 Add a pruning rule for [K+K-]->[] and pattern AM_KpK (which I noticed
after updating the sin(4)/cos(4)=1.1578212823 example from the manpage
and discovering that "ries 1.1578212823" prints "x+1-1 = tanpi(4/pi)".)
  Time measurement is now done by wall-clock time rather than CPU
cycle time. We do this with the gettimeofday() function. Formerly we
used getrusage(), which under-reports elapsed time on Nehalem and
later Intel microarchitectures when the core that we are running in
also has another thread running. This change also involved adding the
inittime() routine.
  Make several changes to facilitate porting this file to other systems:
 * #include <limits.h> and use LONG_MAX instead of __LONG_MAX__ (which
was a GCC-only predefined constant)
 * The symbol stack effect attribute (formerly called "class") is now
called "seft" (both in the comments and more importantly in variable
names) to facilitate porting to C++ (where "class" cannot be used as a
variable name because it is a language reserved keyword)
 * As mentioned above we now measure time with gettimeofday(); in
Windows we provide a gettimeofday() based on _ftime().
 * #include "stdafx.h" has been added before the other includes, to
facilitate Visual C++
 * #define RIES_VERSION ... has been moved to fall after the includes.

20111231 Add bt_first(), not yet used.

20120102 Increase k_min_best.match when fabs(target)>1.0, to avoid a
"search forever without exiting" condition that would always happen when
fabs(target)>16.0
  Add --include/-p (load profile) option, implemented in new routines:
file_read, delimit_args, and parse.args (which includes all the
argument-parsing formerly in main()).
  An included file contains options, delimited by blank space with
optional comments starting with '#'. Each non-blank non-comment
'token' is treated as if it were a string in argv[], with the strings
inserted into argv[] at the point where the settings-file option is
given, with recursion to support nested includes. Max 10MiB per file,
max recursion depth is 27 levels. This is to provide for all the fancy
new features I'd like to add someday (like user-defined symbol-weights
and functions).
  Add bt_depth and bt_stats (which is run by debug_y) to see if
rebalancing the binary tree will help. I conclude that it will not
help much: in a typical run the average tree depth with rebalancing
would be 17.5, and the actual (unbalanced) average depth is 24.5.

20120103 Add check.exact_match(), removing more RIES-specific code
from bt.insert.
  Add next_isparam(), clean up argument parsing and fix a bug with
parsing --significance-loss-margin argument.

20120104 Improve several of the debug_n printfs; add a bunch of
debug_q printfs to answer the question of why the check.sides change
of 20111218 did not produce more matches.
  Rename the old "on_rhs" variable to "using.x".
  Remove a lot of obsolete comments relating to an old idea for
maintaining two separate (LHS and RHS) lists. Update the comments on
derivative formulas and the Gamma function.
  check.sides now always checks only one expression to either side of
the newly-inserted expression. This causes a very few changes in
output, mainly in the first one or two reported matches. The reasons
are explained below in a block comment "THE CHECK_SIDES PARADOX".
  To support the check.sides investigation I added the -D0 option and
debug_0, which does a complete dump of the expressions database after
every gen_forms pass.

20120105 Add stack overflow and underflow checks in eval(). Increase
MAX_ELEN again to 21 (the expr struct is still 64 bytes).
  Add MAX_SEFT_POP checks in add.symbol().
  Add better documentation of the workings of the metastack routines.
  Implement canon.val() but leave it disabled by default right now.
This tries to transform expressions into forms that have a value in
[1.0,2.0), which also transforms expressions. For example, compare:

  ries 1.50631415926535897932 --canon-reduction 0
                    x-1 = 1/2                    for x = T - 0.00631416  {50}
                1/ln(x) = sqrt(6)                for x = T - 0.00213357  {62}
                    x^4 = 2+pi                   for x = T - 0.000489459 {68}
                (x+1)^2 = 2 pi                   for x = T + 0.000314115 {69}
                   x^pi = tanpi(sqrt(2))         for x = T - 0.000224565 {70}
                    x^2 = tanpi(1/e)             for x = T + 9.42083e-05 {60}
                  x^2-2 = 1/(1+e)                for x = T - 1.35846e-05 {79}
           1/(log_3(x)) = 3-1/pi                 for x = T + 8.87321e-06 {87}
            1/x+sqrt(2) = 1/ln(phi)              for x = T - 2.77223e-06 {86}
           1/(log_7(x)) = 5-1/4                  for x = T - 6.29902e-07 {94}
               . . .         . . .                         . . .
     log_(2/sqrt(3))(x) = 3-1/2^e                for x = T - 1.21748e-09 {128}
     max complexity:          66           62          128
          dead-ends:     2290352      5560466      7850818  CPU time: 0.369
        expressions:      183663       420208       603871
           distinct:       92770       113599       206369  Memory: 12928KiB

  ries 1.50631415926535897932 --canon-reduction nr2
                      x = 3/2                    for x = T - 0.00631416  {48}
            (1/ln(x))/2 = sqrt(6)/2              for x = T - 0.00213357  {98}
                  x^4/2 = (2+pi)/2               for x = T - 0.000489459 {104}
              (x+1)^2/2 = 2 pi/2                 for x = T + 0.000314115 {105}
               -(x-4)/2 = pi,/2                  for x = T - 5.21371e-05 {95}
          (1/(x^2-2))/2 = -(pi-5)                for x = T + 2.16534e-05 {110}
          (1/(x^2-2))/2 = (1+e)/2                for x = T - 1.35846e-05 {115}
                e^(x-1) = (1/pi+3)/2             for x = T - 5.75694e-06 {101}
            1/(x/phi)^2 = 8,/pi                  for x = T + 7.44845e-07 {94}
              sqrt(3-x) = -tanpi(e-1)            for x = T - 6.44308e-07 {101}
    (1/-(log_7(1/x)))/2 = (5-1/4)/2              for x = T - 6.29902e-07 {144}
               . . .         . . .                         . . .
       sinpi(1/5),/x/2 = 1/-cospi(1/ln(sqrt(7))) for x = T + 1.15156e-10 {151}
     max complexity:          66           62          128
          dead-ends:     2290352      5560466      7850818  CPU time: 0.369
        expressions:      183663       420208       603871
           distinct:       63206        67234       130440  Memory: 8192KiB

The intent is to get further with a given amount of memory by
collapsing equivalent equations like "x=-2" and "-x=2" into the same
solution. It succeeds in doing this, but also causes different matches
to be discovered and reported (despite the fact that the main loop is
still making the same choices about whether to call gen_forms on LHS
or on RHS). I need to investigate this further to discover why (one
possibility is that the new solutions come from things that would have
matched earlier, but didn't, because they lie in different places on
the number line and wouldn't meet up without needing more complexity).

20120107 -pfoo now tries opening "foo.ries" if "foo" cannot be opened.
  Add thrash_check() and related real-time memory allocation
benchmarking (not yet finished). This is based on ideas from a
discussion with Charles Greathouse yesterday.

20120108 More work on thrash_check(); add --min-memory option (which
presently does nothing, but eventually will prevent RIES from quitting
early in the event of unanticipated system slowness). Add --max.memory
option (which makes RIES definitely quit before exceeding a given
amount of memory).

20120113 Move nested functions out of parse.args() for compatibility
with non-GCC compilers.
  Presently, "ries 0.9674026381747 --eval-expression xp/T" shows a
different answer on different computers: I get "tanpi(x/pi) =
1.4511039572620501" on MP16 and "tanpi(x/pi) = 1.4511039572620503" on
MBP. This causes commands, such as "ries 0.9674026381747 -l3", to
produce different results ("x/tanpi(x/pi) = 2/3" on MP16 and the
bizarre "x/tanpi(x/pi) = log_(sqrt(8))(2)" on MBP).
  I suspect a variation in the maths library routines under different
versions of MacOS. To fix this I probably need to bring in my own
tangent function. Sources for one version are in
"sun-trig-functions.txt". {This was addressed on 20121215, when I
added msal_math64.c and the RIES_USE_SA_M64 switch}

20120115 Add --memory-abort-threshold option and fix some segfaults
that were happening if the user does not give an expected option
argument.
  Add typedef sym_attr_block and use it for all the symbol table arrays.

20120121 Move all symbol tables into a single array of sym_attr_block

20120122 Add CANONVAL_MUL2 operation and stub cv.simplify() routine.

20120423 Notes on what I need to accomplish to finish canon.val,
decanon, and cv.simplify routines:

  canon.val adds 'n', 'r', and/or '2/', '4/' to try to put an
  expression's value into the range [1, 2). Call it just before
  calling bt.insert. It should work on the existing expression
  structure and metastack state, and it needs to stop if MAX_ELEN is
  reached. We also need a decanon routine to undo its work before
  proceeding with the rest of ge.2.
    cv.simplify operates on two expressions (an equation), removing
  any unnecessary symbols that may have been added by canon.val. It
  takes two expressions A and B and removes any trailing '2/', '4/',
  'r' and 'n', or possibly shifts symbols from one expression to the
  other, all so as to make the equation A=B lower-complexity. Call
  this in check.match right after the initial test passes and before
  running newton() on the equation. This will most probably involve
  having two dedicated expression data structures so it (and newton(),
  etc.) will see our simplified equation rather than the two
  expressions that are actually in the database.
    The purpose of this is to conflate values like -2, -1, 1/2, 1, 2
  into a single tree entry, which should make more effective use of
  memory, at the expense of the output being a little less
  well-ordered by complexity-score.
    {These changes were begun on 20120514, but not finished until bug
  fixes on 20121210.}

20120425 RIES was used by Randall Munroe for XKCD #1047 (xkcd.com/1047)
{This led to changes allowing RIES to function as the back-end to an
online RIES server.}

20120428 Changes for UNIX and Windows compatibility (suggested by
Markus Milleder, via the xkcd forums).
  Also change Nth root symbol ',/' to '"/' because the '"' looks more
like the superscript 'n' that I use in the HTML for the ORIES server.
The server is mostly functional, at mrob.com/ries

20120503 Add --wide-output option (mainly for use by the ORIES
server). This is a 132-column version of the standard 80-column
output, adding two new columns. It displays the normal output plus the
-x version of the column showing equation roots, and a new column
showing the ratio (target/delta) in the form "1 part in 50" as in
xkcd.com/1047

20120505 There has been a bug for a while, causing "ries -l-2 -i 143"
to find the bizarre equation "(x-2)+3 = (3*4)^2" rather than the much
more obvious (and lower-complexity) "x+1 = (3*4)^2". This resulted
from sig-loss pruning and the k_sig_loss value 0.01: Because 1 is less
than 143*k_sig_loss, RIES disallowed the addition x+1. Similarly,
"ries 7775" failed to find "x+1=6^5" or even an alternate like
"(x+9)-8=6^5" because it couldn't add 1 (or any integer) to an x that
large.
  As a "narrow, conservative" fix, I have altered exec() case '+' to
attempt "(x+1)-1" and see if it is precisely equal to x. If so, then
we can assert that no significance loss has actually occurred
  Also add parse_target() to support future --mad option.

20120508 Numerous small changes to avoid warnings about sign mismatch
and using 'char' as an array index. Also add functions sym.strsym and
sym.strcmp.

20120509 Add --max.match-distance option, bringing it a bit closer to
supporting --mad.

20120510 Add the symbol ' ' which does nothing, so that --eval-expression
expressions can be formatted with whitespace if desired.
  Display a "still looking..." message if no results have been reported
after the first 2 seconds.

20120511 Add --numeric.anagram option and get it basically working. It
still needs a lot of testing, and the loop exit condition based on
search level needs a lot of work.

20120514 A bunch of little changes to fix compiler warnings.

20120515 Handle the gen_total / searchmax limits differently for
unidirectional searches like that done by --numeric.anagram. Prior to
this change, "ries -i 143 --numeric.anagram 143" would search forever
and finds nothing.

20120516 Add pf_float_wid; prune_count etc. are now doubles. Main loop
termination tests are now more flexible and general-purpose (important
for --numeric.anagram and other restricted searches).

20120518 Eliminate use of "s64" (long long 64-bit int) and associated
header files; I can use double instead for my large integers. Clean up
some size_t and char * typecasting. This is all for compatibility with
non-GCC compilers.

20120520 More cleanup, thanks to suggestions by Markus Milleder.

20120522 Add boolean datatype; eliminate all uses of explicit 32-bit
integers; add validate_types() and smarter detection/definition of s16.

20120613 Add --derivative.margin option. Also test different values of
the initial k.vanished_dx setting, and discover that it's safe to use
a much lower value like 1e-10. It's possible that most of the problems
fixed on 20111219 were also fixed by the sig_loss tests added later
in 201112; I'll have to do a lot of testing to be sure.

20120720 Add --one-sided option; --numeric.anagram now implies
--one-sided.

20120725 Change first argument of sym.strsym() from 'expr' to 'exp1' to
avoid namespace conflict with typedef 'expr'

*/ /*

20121202 Better implementation of "--max.match-distance 0"
  Implement the --match.all-digits option, which makes RIES work more
like ISC. The capabilities has already been implemented, primarily in
parse.target(), the --max.match-distance option, and the "Still
searching" message in the main loop.

20121205 If the target value is large in magnitude, automatically set
k_vanished.dx as if the --derivative.margin option was given.

20121206 Add several formatting routines (symstrncpy0, sym.strncat,
endstack, expr_break, expr_print_infix, eqn_print_infix) and
try.solve() to implement the --try-solve-for-x option.
  This is in a rather rough state, as exemplified by the command "ries
0.8183431428522 --try-solve-for-x", which will happily report "x =
sqrt(phi+pi)+3". I can fix that within try.solve() by calling eval()
after taking the square root of both sides to see if the LHS
is negative, and if so, append [n] to the RHS.

20121207 Fix the square root sign problem with 0.8183431428522, and
add some notes to try.solve.
  Increase NEWTON_MAX_ITER, and add notes about chaotic oscillation:

  2012.1207: "ries 0.2322795271987 -l0" does not find "sinpi(x) = 2/3"
  because Newton does not converge: sin(x) in that region causes a
  divergent oscillation. "ries 0.23227952719876987" works because it
  happens to hit on an exact match, which thus does attempt to call
  newton(). The oscillation can be seen by passing -Dn: RIES is clearly
  trying all sorts of equations involving trig functions like [xS],
  [x2-T], etc. I could "fix" this by making the Newton iteration only go
  halfway to the next point each time:
    curr = curr + 0.5 * ((rhs_val - lhs_val) / (lhs_dx - rhs_dx));
  and increasing NEWTON_MAX_ITER to about 100, but that also introduces
  bogus solutions like "tanpi(-x) = 7^2" that I might not want. It's
  clear that trig functions in x are being rejected fairly often and if
  I change this behaviour, it will change a lot of RIES output.

20121208 Fix derivative formula for sine (case 'S' in exec()). It
has been wrong ever since I added --trig-argument-scale, and was
causing newton() to not converge in many (if not most) cases.

20121209 Add period-2 loop detection in newton(); this might allow
eliminating k_newton_settled later.
 Add sym.strlen, sym.strtrail, bothtrail and sym.strclip and begin
implementing cv.simplify.
 debug_p replaces debug_q for the "first score not good enough" case.
 new flag debug_e replaces debug_E; debug_E replaces debug_F.
 debug_F replaces canon.val cases of debug_G
 add new flag debug_Q for cv.simplify
 make debug_S and debug_s distinct variables (but 'S' and 's' debug
functions still print the same messages)
 Fix derivative formula for cosine.
 report.match now takes symstr arguments; copy expressions before
calling cv.simplify (which fixes heisenbugs that were caused by it
modifying the expression strings in the actual database nodes).
 Add a couple more rules to infix.preproc to improve --canon-reduction
output.
 new flag debug_N replaces debug_o.
 debug_o replaces debug_p (including the debug_q case changed earlier today).
 debug_p is now used by infix.preproc
 debug_F and debug_f are now distinct (canon.val for LHS and RHS respectively)
 Fix derivative formula for tangent and increase its symbol weight to 6
(sine and cosine are both 3)
 Add --symbol.weights option. A few simple tests like "ries 2.5063
--symbol-weights 1:T" quickly reveal that these "Gosper pi-scaled"
trig functions allow for pathological matches like
"tanpi(tanpi(tanpi(tanpi(1/tanpi(tanpi(ln(x))))))) = 1/3" mainly
because of the way the derivative scales up with each nested call of a
trig function. Al address this, I limit the ranges of sin, cos and tan
to +-pi (after applying the scale factor). Of course you don't want to
reduce symbol weights so drastically, and more realistic weight
adjustments (like "ries 2.5063 --symbol-weights 8:T") are more useful.
 Due to the argument syntax you cannot set a negative weight, and
large weight values automatically disable the symbol.
 Currently, The automatic setting of k.vanished_dx for large values
causes tautology errors when RIES is given large values. For example,
currently "ries -l3 1234567890" gives "x^2-sqrt(x)^4 = 2^8"

20121210 Pass parameters using_x, a_minw, a_maxw, etc. through the
recursive expression generation routines, rather than having them be
global variables.
 Add tautology warning messages when the target is very large or the
--derivative.margin is too small. Here are examples, in all cases using
a command like "ries -l4 4243743 --derivative.margin 2.0e-11", RIES
gives an answer like "x^2-sqrt(x)^4 = 1/-(2^8)"
    Target      min. --d-m
  42437432.1    10.0e-9
  4243743.1     188e-11
  424374.1      11.8e-11
  112474.1      3.0e-11

20121211 Add a bunch of comments in ge.2(), etc.
  Add cv.simplify rule [Arr]=[B] -> [A]=[B]

20121212 Fix a bug in infix.1(): "7 times negative x" is now displayed
as "7*-x", before this fix it was "7 -x" which looks too much like
"7-x"..

20121214 Improve k_vanished.dx checks and error messages.
  It has been a while since I did benchmarks using the standard test
(2.506+pi/10^4 with no options), so I am going to run the benchmark
on all my machines again.
  Although I have been doing the same test since the beginning (see
above under the dates 200003xx, 200310xx, 20050715, 20090216 and
20090515) the benchmarks are not directly comparable because of many
changes in the code. Improvements in error-checking, pruning rules,
the addition of the tangent function, and recent fixes to the
derivatives of trig functions have all affected the number of
generated expressions and therefore the running time and memory usage
for a given search level. (For details of all the changes, see above
under the dates 20101218, 20111219, 20111220, 20111221, 20111226,
20111228, 20111229, 20111230, 20120505, 20120613, and 20121208). Here
is a representative sampling of statistics and running times from
"ries 2.5063141592653589 -l4" over the past few years:

 Date and time     expressions   complexity                    time (Xeon
 YYYYMMDD.hh:mm    LHS      RHS    LHS RHS   equations  memory   E5520)
 20090217.02:36  1344958  1080143   76  70   1.453e12   155.3M   10.6s
 20090515.18:16  1344952  1080143   76  70   1.453e12   155.3M   10.5s
 20090516.01:13  1332922  1360355   76  71   1.813e12   172.4M   12.6s
 20111219.21:00  1332922  1360355   76  71   1.813e12   172.4M   12.2s
 20111220.23:28  1327806  1360355   76  71   1.806e12   172.1M   12.4s
 20111221.23:46  1327806  1360355   76  71   1.806e12   172.1M   12.3s
 20111226.18:27  1327757  1360355   76  71   1.806e12   172.1M   12.4s
 20111228.22:24  1251691  1280315   76  71   1.603e12   162.1M   11.60s
 20111230.06:26  1020295  1252891   76  72   1.278e12   145.5M   11.61s
 20120505.22:54  1032922  1281451   76  72   1.324e12   148.2M   11.55s
 20120613.23:35  1032922  1281451   76  72   1.324e12   148.2M   11.82s
 20121209.09:58  1032947  1281451   76  72   1.324e12   148.2M   11.80s
 20121210.07:24   930662  1127640   76  72   1.049e12   131.8M   10.74s

Here are the current times for each level from -l0 to -l6, on all
machines back to the 800 MHz G4 (which is 10 years old).

                       PowerPC  PowerPC   Core 2    Xeon    Core i7
                         G4*      G5*      Duo     E5520&   2720QM#
 Level  -mem-   eqns.  800 MHz   2.0GHz   2.23GHz  2.27Ghz  2.20GHz
  -l0  1.11Mb  6.89e7   0.333s   0.085s   0.035s   0.026s   0.022s
  -l1  3.47Mb  7.18e8   0.954s   0.304s   0.125s   0.102s   0.077s
  -l2  11.9Mb  8.46e9   3.495s   1.245s   0.518s   0.436s   0.364s
  -l3  39.3Mb  9.23e10  14.70s   6.407s   2.724s   2.125s   1.507s
  -l4  131 Mb  1.04e12  69.29s   34.13s   12.75s   10.70s   7.760s
  -l5  441 Mb  1.19e13    %      165.0s   64.15s   51.59s   38.29s
  -l6  1.51Gb  1.39e14           794.0s   322.2s   252.3s   190.1s

 * For the G4 and G5 I was using a 32-bit binary, so the memory usage
   was a bit smaller.
 % The G4 did not have enough memory to run the -l5 or -l6 tests.
 & The Xeon E5520 has a base speed of 2.27Ghz, but during these tests
   all cores were otherwise idle, so it was running near its single-core
   turbo speed of 2.53 GHz.
 # The i7-2720QM has a base speed of 2.2 GHz, and all cores were otherwise
   idle during these tests. However, its turbo frequency depends on the
   amount of recent activity. Each test was run after the CPU had been
   completely idle for 1 minute. For tests under 25 seconds, this means
   the clock speed was near the maximum 3.3 GHz.

*/ /*

20121215 Standalone math source (msal_math64.c) now includes SIN and
COS functions.

20121216 Fix a simple bug in symstrcmp that prevented
--find.expression from working.

20121217 Add --explicit-multiply option.

20121218 Allocate twice as much space for the RHS part of the equation in
try.solve and in report.match

20121223 Slight optimization inside the next symbol test loop of ge.2.
  Fix a few compilation warnings (missing prototypes; mismatched
integer types in some debug printfs)

20130121 Fix bug in -S option when used with --numeric.anagram (that
made it act as though the -S option had not been given).

20130127 While working on a simple Perl script to generate OEIS sequence
A005245 I discovered that the command:

  ries --one-sided -ie -S1+* 23 -l5

does not find any answer. I suspect this is a bug in complexity limit
optimizations. {After investigation I figure out it's simply that
MAX_ELEN is too low.}

20130129 Do a thorough inspection of all places where I check
expression lengths against MAX_ELEN and find a few places it can be
improved to make maximum use of the avaiable space. Allocate one more
symbol for certain expression scratch buffers. It should be able to
use all MAX_ELEN symbols now. Add RIES_MAX_EXPRESSION_LENGTH option, which
you can give when compiling RIES to make a RIES that handles longer
expressions.
  Make "exhaustion timeout" message suggestions more relevant;
don't show it at all if a result has been given; don't show "(for more
results, use the option '-l3')" suggestion if exhaustion timeout error
was given. Also add NO_IDENTITY_OPTIMIZATION code (currently disabled).

20130130 Add a 32-bit signed integer data type and corresponding
ifdefs and runtime sizeof testing. Symbol attributes mark is now 32
bits. Add rules for associative operators: for example, it now prunes
[ABC++] = A+(B+C) in favor of [AB+C+] = (A+B)+C.

20130201 Now using debug_B for the derivative prune messages formerly
displayed by debug_e and debug_E; re-use debug_E for "rejected
(duplicate value)"; debug_G now (properly) prints its message only
when an expression has actually been added, and uses infix notation
because -DG/-Dg may be useful to users who wish to use RIES to
generate a database of expressions for processing by another program.

20130203 Add Lambert W function define and test loop; then move the
test code to msal_math64.c. To see LambertW tests, compile with
RIES_USE_SA_M64 defined then invoke RIES with option -Dz.

20130218 "--max-match.distance 0" now causes RIES to exit if and when
it gets an 'exact' match. Add a roundoff-error disclaimer about
'exact' matches (unless -i was given).

20130228 Add null argument '-' (useful if you want to end a sequence
of arguments to something like --symbol.weights).
  -S, -O and -N now pre-empt each other in the order they are given
(which is necessary for users who combine different profiles).
  Add the -E option (replacing the old "or-mode" behavior of -S; this
will be necessary for future optional functions like A and W, and also
facilitates combining multiple profiles.

20130301 Change almost all occurrences of 'double' declarations to
one of
  ries_val: The value of an expression or subexpression
  ries_dif: The value of a derivative, error, uncertainty, the distance of
            a match, etc.
  stats_count: For counting generated expressions, equations, etc.
  time_flt: For measuring time and memory usage
 Add init.formats()

20130302 Move Gamma function code (now tested) to msal_math64.c. As
with LambertW (see 20130203) it is not yet available in expressions or
equations but you can see the internal tests of Gamma by compiing with
-DRIES_USE_SA_M64, then invoke RIES with the option -Dz.
  Add default --include/-p profile feature. It now looks in
getenv("HOME") or getenv("USERPROFILE") in Windows for either
"ries_profile.txt" or ".ries_profile" (either name works in all OS's).
This profile is loaded first before any of the other options; however
if you give no options RIES still prints the brief help and exits.
  Add a '-p' option that:
   - If given first, disables the default profile
   - If given anywhere else, loads the profile at that point (so you can
     give an option which is applied before loading the profile by
     e.g. "ries -p foo bar -p baz qux")
  Make the '--version' option display the path to the defaults file,
to aid field diagnostics.

20130303 Add attribute tags (TAG_INT, TAG_RAT, TAG_NONE) and implement
their calculation. This provides a more robust way to implement the -i
and -ie options, and more importantly it paves the way for new
features.
  Add --absolute-roots as synonym for -x; add --relative-roots option
(opposite of '-x').
  Use newly-added attribute tags to implement -i option (replacing
many calls to float() with hopefully less aggregate work; look in
exec() and search for 'g_restrict.subexpr'.)

20130305 Add --no-solve-for-x option (which merely undoes
--try-solve-for-x). Add setup_abc_mmw() as part of refactoring the
handling of restricted symbolsets.

20130306 Add --symbol.names option; this involves several changes to
how we define the symbols, all of which will help with future
improvements like user-defined constants.

20130307 '=' symbol can now be renamed. Add -r and -re options to
ensure that all solutions are rational when solved for x. (These are
just shorthand for using -N to exclude lots of symbols). Increase
allocation of temp buffers in infix.1, fixing a crash that happened
when using an all-seft-b symbolset and solving for x, e.g. "ries
3.1415926535897932 -SeElq -s".

20130308 Add --no-refinement option, which causes it to never
decrease best.match, and therefore print *all* matches that do better
then the specified distance. This produces results similar to the ISC
(except still ordered roughly by increasing complexity) and may be
useful to folks (like the 137 cultists) who are happy with any formula
within a known error bound. It required adding sym.strneq,
symstrsymstr, unique.eqn and the g.matches memory block (none of which
are used unless you choose the option).
  Add the --max-matches option, 100 by default.

20130309 Add -n as a synonym for --max-matches, --integer-subexpressions
as a synonym for -i, and --rational-subexpressions as a synonym for -r.

20130310 Add the g_nr_deltas array to provide another type of pruning
for equivalent answers for --no-refinement.
  Add the "-c" (alias --constructible-subexpressions) option which is
similar to -r but also allows phi, square and square root. Do a little
refactoring of how the -D, -E, -i, -N, -O and -r options are handled.
 Begin refactoring the tags manipulation to use TYPE_xxx values: since
each attribute is a subset of another (e.g. the integers are a subset
of the rationals) I don't really need to use bit-fields.

20130311 Continued refactoring of tags, add TYPE_CONS and TYPE_ALG
tagging. Implement -a option; add tagname().

20130312 Add --rational-exponents option, which tests the tag of the
argument and generates the new error ERR_EXEC_ILLEGAL_EXPONENT; the
error is also generated if the argument contains x on the assumption
that the user probably does not believe x to be rational if he is
looking for an answer in the form of an algebraic number.
  Add --rational-trig-args option, which works similarly and also sets
k_sincos_arg_scale to pi; it generates the new error
ERR_EXEC_TRIG_ARGTYPE.
  Add -a (algebraic subexpressions) option, using these new restrict
options. To support this, the target number is automatically tagged
with the same type as the selected restriction (-r, -c, -a) unless it
is obviously an integer or half- or quarter-integer; if no restriction
option was given we tag x as transcendental. There are now four
classes of restrictions with easy command-line options: -i, -r, -c,
and -a.
  Add ":.:." syntax for defining a blank-space character in symbol
names.

20130314 Add --any-exponents and --any-trig-args options.

20130317 Compute rv_maxint and disallow -i when target is too large;
remove utf8-related code (obviated by --symbol.names)

20130318 Expand brief.help.
  Add "-l" option (without a numeric argument, i.e. a bare "-l" rather
than something like "-l3") to restrict answers to those that have
Liouvillian numbers as roots. With -l it finds that 1.632526919438153
is sqrt(2)^sqrt(2) and finds that 1.132997565885066 is a root of
x^5+x=3, but does not find that 2.31645495878561 is the root of x^x=7.
Along with the full unrestricted defaults, and the option "-Ox", this
makes for a rather full set of options to select popular classes of
numbers as the roots of the reported results.

20130219 Test g_restrict_exponents in functions 'l', 'E' and 'L'.

20130613 Add --no-slow-messages option; auto-set k_min_best.match and
k_vanished_dx for small targets.

20130626 Use "%Lg" in various debug printfs to support RIES_VAL_LDBL.
Add macros EXP, FABS, etc. and (ries_dif) typecasts in several places;
get long double precision pretty much working.

20130801 Initialize k_ulp sooner so it can be used by -ce
  --max-match-distance and --match-all-digits options now cancel each
other.

20130803 In --symbol-names, allow redefining a symbol to itself (e.g.
':^:^' when -F format is selected) by accepting space_sym definition
only once.

20130805 Benchmarks of standalone maths library vs. standard libm.
Using the command "time ./ries-libm 2.5063141592653589 -l5
--max-match-distance 1e-10", and SIMULTANEOUSLY running the same
command with the "ries-sa-math" library, the time is 0m45.373s with
the sa-math library and 0m45.568s using libm (the sa-math library is
actually faster).

20130809 Add ieee.paranoia()

20130810 Alphabetize the order of sections in parse_args()

20130811 Add ERR_EXEC_ILLEGAL_DERIV; exec() checks for overflow and
NaN in derivative calculations in several operators
  Print error and exit if target value is zero.

20130812 Pass root directly to report.match to get full precision in
the case where ries_val has much more precision than ries_dif.
Increase precision of calculations of pi and e in init.numerics. "long
double" now gives 31 usable digits on the PowerPC G5 (where GCC 4.0.1
and later provide double-double arithmetic), and show.version now
shows the architecture (PPC/Intel) and precision:

  ries --version
  ries version of 2013 Aug 13, Copyright (C) 2000-2013 Robert P. Munafo
    architecture: PowerPC
    precision: long double (33 nominal, 31 usable)
    mathlib: standard
    profile: -p/Users/munafo/.ries_profile
  RIES is free software; see the source for copying conditions.  There is NO
  warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  Dual-G5 /Users/munafo
   : ries 2.50618 -x

     Your target value: T = 2.50618                               mrob.com/ries

      x = 5/2                    for x = 2.5                               {50}
      x = e^3/8                  for x = 2.510692115398458467616066206823  {67}
      x = sqrt(2*pi)             for x = 2.506628274631000502415765284811  {55}
      x = x"/(1+9)               for x = 2.506184145588769256292940922378  {70}
      x = pi^(ln(sqrt(6))^2)     for x = 2.506182746702861729338863939787  {84}
      x = e^(5"/sqrt(3/7))       for x = 2.506182083318281693444031686493  {99}
      x = ln(pi*e+1+e)           for x = 2.506180094504077068176274103417  {96}
      x = e^(log_(pi^2)(7))+1/6  for x = 2.506180001393763345666526465285 {107}
      x = sqrt(1/((1/e-ln(6))+sqrt(x)))
                                 for x = 2.506180000397503542091976573633 {118}
                    (for more results, use the option '-l3')

    log_A(B) = logarithm to base A of B = ln(B) / ln(A)  sqrt(x) = square root
    e = base of natural logarithms, 2.71828...  A"/B = Ath root of B
    ln(x) = natural logarithm or log base e  pi = 3.14159...

                           --LHS--      --RHS--      -Total-
       max complexity:          67           62          129
            dead-ends:     2842183      5668638      8510821  Time: 2.106
          expressions:      198545       370110       568655
             distinct:      103576       107460       211036  Memory: 13248KiB

          Total equations tested:             11130276960 (1.113e+10)

By comparison, typical run time for the same command on PMG5 in double
precision is about 1.49, and memory usage is 10624KiB.

20130816 Add ries_strncpy, ries_intpow, ries_to_digits,
ries_snprinf_int, ries_strlen, ries_spfg, spfg and spff. Use these
in a few debug statments.

20130818 Remove trailing 0 digits from ries_spfg output (for example:
2.50618e+12 instead of 2.50618000e+12); fix a few compilation
warnings.

20130820 Add ries_spfg_test and msal_test_spfg

20140226 Eliminate conversion to double in ries_to_digits, fixing an
'exponent adjust failed' error when trying to print values like
1.23e-789 (with long double which supports exponents down to e-4951)

20140227 Minor refactoring; add did_newton parameter to report_match

20140228 Add some logic to check.exact_match, duplicating some of the tests
done by check.match.

20140303 Generate ERR_EXEC_ZERO_DERIV error in '^', 'L' and 'v'
operations when the derivative calculation underflows to zero. This
eliminates several recently-reported tautology errors.

20140831 Expand brief_help() a little,

20141014 Add --min-equate-value and --max-equate-value options.
 20141105 Add Lambert W function (duplicating and expanding on work by
Mark Shoulson).
 20141106 Increase weight of 'tan' (that is, the tangent function when
--trig-argument-scale is not the default) to 6, bringing it in line with
'tanpi' (where the weight was always 6).
  Bug-fixes in Lambert derivative and error handling, in debug_z mode
call new test routine msal_test_lambertl
 20141117 Make -i (integer subexpressions) more efficient by restricting
symbolset as is done for -r. All class restrictions now also exclude W.
 20141122 RIES license is now GPL version 3 (formerly GPL v2). Reorder
the info in --version a little bit.

 20141207 Add test of each subexpression against g_restrict.subexpr so
that if you do e.g. "-a -EL" the L will be enabled but effectively
ignored; however I also canonize the unofficially supported "-a -Ep"
by mutating the types of the predefined constants in a similar way as
was already done for the target value. This is how user-defined
constants without a user-defined class will need to be handled.
  Add --any-subexpressions to enable the old behaviour, e.g. the user
can use "-a --any-subexpressions -EL" to get the equivalent of the old
"-a -EL".
 20141212 struct form now includes stk[] and arg1[], supporting rules
that check the first argument of binary operators; add one rule using
AM_a1_e.
 20141213 Add AM_a1_1 rules; #qualify# tests show that several more
results are found in -l5 tests, indicating that these rules increase
pruning for deep searches.
 20141216 Replace all "sprintf" with snprintf.
 20141217 Add #defines for most of the snprintf tempbuf sizes.

20160104 Enable cube root of a negative argument ("-9.8222414378011"
vector in qualify tests it)
 20160131 Add mem_used_bytes for more precise reporting of how much
memory would be needed for small tasks; to support this I did some
signed-vs.-unsigned cleanup.
 20160423 Finish re-indenting the big block of options tests in 
parse.args()

20170211 Add '--show-work' as a synonym for '-Ds'

20180713 Add '--max-trig-argument' option.
 20180802 Rename '--max-trig-argument' to '--max-trig-cycles'

20210417 Clean up arctan code and get it working.
 20210418 Better formatting of atan2 in infix.1(), don't print
2nd argument if it is 1.

20240422 Legend now explains the two forms of atan2 (one-argument
and two-argument) separately, giving only one explanation when only
one of the forms appears in output.

*/ /*

BUGS and TO-DO

  See "UNFINISHED WORK" section above

THE CHECK_SIDES PARADOX

The RIES algorithm maintains a single list of RHS and LHS expressions
sorted in numerical order. These are distributed pretty much randomly,
and when a new expression is added, RIES checks the preceding and following
list items to see if it can form an equation with the new item.
  The closeness of a match is the difference in values divided by the
derivative of the LHS:

  closeness = |LHS(x)-RHS|/(d/dx LHS(x))

If the new node is an LHS, then the values of LHS(x) and d/dx LHS(x)
are the same for every match-comparison that is made. One RHS in each
direction (upward and downward) is all that needs to be tested,
because any further RHS's will generate a larger value of
|LHS(x)-RHS|.
  However, when the new node is an RHS, the value of d/dx LHS(x) will
differ for each LHS that is found while scanning upward and downward
for candidate matches. Therefore, even after finding a new optimal
solution, the possibility exists that there might be another
even-closer optimal solution if you keep scanning further, coming from
an LHS that has a much lower value of d/dx LHS(x).
  Here is a concrete example. La and Lb are two LHS's with derivatives
of 1 and 10. Ra and Rb are two RHS's. They are shown here as if laid
out on a number line to make the example clearer. The nodes are
inserted in the order: La, Lb, Ra, Rb.

  value:  1   2   3   4   5   6   7   8   9
          Ra                      Rb  La  Lb
   d/dx:  -                       -   1   10

When Ra is inserted, the match La=Ra is found, with closeness
(8-1)/1=7. Then Rb is inserted, and a new match La=Rb is found, with
closeness (8-7)/1=1. These are the only two matches it will report.
  However, the match Lb=Ra is has a closeness of (9-1)/10=0.8, which is
closer, and Lb=Rb has a closeness of (9-7)/10=0.2 which is closer
still. Lb=Ra should have been reported instead of La=Rb. The program
should report La=Ra, Lb=Ra, and Lb=Rb (in that order).

For a real-life example, use the command:

  ries .328106566874978253 -l-4 --trig-argument-scale 1 -NT -Dy0

which only gives one result, "5 x = sqrt(e)". With -D0 it dumps the
entire table of values on each complexity pass. Look through this
output for the first occurrance of [xrS] and [9r] together:

      28        xrS { 35} = 0.093664890868384448   , dx = 9.2481888991224963
      29        xp/ { 34} = 0.10443956395812863    , dx = 0.31830988618379069
      30         xs { 24} = 0.10765391922648457    , dx = 0.65621313374995649
      31        x3/ { 35} = 0.10936885562499275    , dx = 0.33333333333333331
      32         9r { 26} = 0.1111111111111111
      33         8r { 26} = 0.125

The output shows that [9r] is inserted a few passes later than 'xrS',
so check.sides is looking at [9r]'s neighbors [x3/] and [8r].
  For a while in 201112 I made RIES look past the nearest neighbor,
and this command reported the result "sin(1/x) = 1/9". But as you can
see, sin(1/x) matches 1/9 more closely than x/pi, x^2, and x/3,
because the derivative of sin(1/x) is so much higher.

This is an example of the above-described problem, and is the reason
why for a while RIES was checking multiple neighbors on each insert.

The Paradox:

Despite the foregoing, the present algorithm (in which check.sides
only looks at the one closest neighbor on both sides of a newly-added
expression) turns out to work very well.

To understand why, look at the output of ries 1.506591651 -Dy0 and
find the first appearance of [1p6*-] :

     448     x1+p^n { 56} = -17.937341258252982    , dx = -22.481451854903934
     449      1p6*- { 51} = -17.849555921538759
     450      xe+sn { 51} = -17.84955591743638     , dx = -8.4497469589180909
     451       xTe/ { 49} = -17.762306262640966    , dx = 857.98450178681276
     452       x7^n { 46} = -17.61849853924631     , dx = -81.859931782354067

When [1p6*-] is inserted, the LHS expressions [xe+sn] and [xTe/] are
already present. [xTe/] has a much bigger derivative (over 100 times
as large), so it looks like a good candidate for a match that would be
missed if check.sides only looked at the first neighbor of [1p6*-].

However, the distance in x values from [1p6*-] to [xTe/] is over 20
million times larger in magnitude than the distance from [1p6*-] to
[xe+sn], so the higher derivative of [eTe/] doesn't stand a chance.

When checking for other similar cases, the same thing always happens:
when an RHS and LHS forms a new record close match, any other LHS's
in the area don't come anywhere close to being another new match.

The reason for this is in the statistics. Going back to the
.328106566874978253 example, consider the range of values between 0
and 1. By the time there are 1000 expressions in this range, the
average distance between expressions will be about 0.001. However, the
distance between the two *closest* expressions will be much smaller,
somewhere on the order of e/10^-6.
  Now consider what happens when you insert another 1000 expressions
at random places in the range (0..1). There is a reasonably good
chance that one of these new 1000 points will come closer to an
existing point than any of the other old points was. This will be a
new match. However, the odds of having *another* new match at the same
time are very very low -- about 1 in 1000. In other words, in order
to get a situation where there are two good LHS matches near an RHS,
all three have to be within 10^-6 of each other.


 */

/* stdafx.h (the precompiled header for Microsoft Visual C++) is included
   by "ries-for-windows.c", which then proceeds to include ries.c. The
   following intentionally generates an error in the event that someone
   tries to compile ries.c directly in Microsoft Developer Studio. */
#ifndef __GNUC__
# ifdef _WIN32
#   ifndef RIES_USED_RFWC
      please_compile_ries_for_windows.c please_compile_ries_for_windows.c;
    /* INSTRUCTIONS FOR COMPILING RIES IN MICROSOFT DEVELOPER
       STUDIO (USING VISUAL C++) ARE PROVIDED IN THE SOURCE FILE
      "ries-for-windows.c" */
#   endif
# endif
#endif

/* %%% Incomplete: We need a three-stage algorithm: detect what is the
   precision of long double, check to see what they have requested, then
   decide which type of float to use, then actually declare things. */
#ifdef RIES_WANT_LDBL
# define RIES_VAL_LDBL
#endif

/* If neither mathlib is selected, default to using the standard library
   provided by the compiler and runtime environment (as it is typically
   a few percent faster) */
#ifndef RIES_USE_SA_M64
# ifndef RIES_USE_STD_M64
#   define RIES_USE_STD_M64
# endif
#endif

/*
  We include <math.h> etc. first, because msal_math64 still uses part of
  the std math library, and its version info uses printf
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef RIES_USE_SA_M64
# include "msal_math64.c"
# ifdef RIES_VAL_LDBL
#   define SIN(x) (msal_sinl((x)))
#   define COS(x) (msal_cosl((x)))
#   define TAN(x) (msal_tanl((x)))
#   define LAMBERTW(x) (msal_lambertwl((x)))
#   define GAMMA(x) (msal_lanczos_gamma((x)))
# else
#   define SIN(x) (msal_sin((x)))
#   define COS(x) (msal_cos((x)))
#   define TAN(x) (msal_tan((x)))
#   define LAMBERTW(x) (msal_lambertw((x)))
#   define GAMMA(x) (msal_lanczos_gammal((x)))
# endif
#else
# ifdef RIES_VAL_LDBL
#   define SIN(x) (sinl((x)))
#   define COS(x) (cosl((x)))
#   define TAN(x) (tanl((x)))
# else
#   define SIN(x) (sin((x)))
#   define COS(x) (cos((x)))
#   define TAN(x) (tan((x)))
# endif
  /* You need to use the stand-alone library to get Lambert and Gamma. */
# define DUMMY_LAMBERT 1
# define LAMBERTW(x) (0.0)
# define GAMMA(x) (0.0)
#endif

/* The following functions are acceptable as-is but need long double
variants. */
#ifdef RIES_VAL_LDBL
# define ARCTAN2(x,y) (arctan2l((x),(y)))
# define EXP(x) (expl((x)))
# define FABS(x) (fabsl((x)))
# define FLOOR(x) (floorl((x)))
# define LOG(x) (logl((x)))
# define LOG10(x) (log10l((x)))
# define POW(x,y) (powl((x),(y)))
# define SQRT(x) (sqrtl((x)))
#else
# define ARCTAN2(x,y) (arctan2((x),(y)))
# define EXP(x) (exp((x)))
# define FABS(x) (fabs((x)))
# define FLOOR(x) (floor((x)))
# define LOG(x) (log((x)))
# define LOG10(x) (log10((x)))
# define POW(x,y) (pow((x),(y)))
# define SQRT(x) (sqrt((x)))
#endif

/*                      ---------gettimeofday---------
   RIES uses gettimeofday() to measure how much time was used in the
   search. To port RIES to another OS, add another #ifdef case to
   provide another gettimeofday() function. */
#ifdef _WIN32
/* Windows version. Note that _WIN32 is defined even if building for a
   64-bit target (which in addition defines _WIN64) */

/* <sys/timeb.h> and <sys/types.h> are needed for _ftime()
   http://msdn.microsoft.com/en-us/library/z54t9z5f(v=vs.71).aspx  */
# include <sys/timeb.h>
# include <sys/types.h>

/* <winsock.h> defines the UNIX-compatible "timeval" structure that we take
   as a parameter, allowing us to emulate the UNIX routine that RIES
   was designed to use. It's in winsock because network protocols use a lot
   of data structures that were originally defined by UNIX systems. */
# include <winsock.h>

  /* from www.linuxjournal.com/article/5574 */
  void gettimeofday(struct timeval* t,void* timezone)
  {
    struct _timeb timebuffer;
    _ftime( &timebuffer );
    t->tv_sec=timebuffer.time;
    t->tv_usec=1000*timebuffer.millitm;
  }

#else
/* On UNIX, Linux, MacOS X, and CygWin systems the gettimeofday function
   is in the standard libraries and is defined by these #includes. */
# include  <sys/stat.h>
# include  <sys/time.h>
#endif

/* -------------- defines ------------------------------------------------- */

#define RIES_VERSION "2024 May 20"

/* Default search level. For backwards compatibility, the -l option adds
   a number to the DEFAULT_LEV_BASE value. Without a -l option, it acts as if
   -l was given with the parameter DEFAULT_LEV_ADJ. */
#define DEFAULT_LEV_BASE 2.0
#define DEFAULT_LEV_ADJ 2.0

/* Maximum number of matches to output. Changed with --max-matches or -n */
#define DEFAULT_MAX_MATCHES 100

/* Maximum length of a symbolic expression. NOTE: right now it's dimensioned
   to reflect a normal symbol set. However, in a run with a very limited
   symbol set the expressions grow in number a lot more slowly as the
   complexity score and length increase, and therefore a much higher MAX_ELEN
   would be necessary. The main problem is it affects the size of the
   list nodes, and therefore the memory footprint of the program. I guess
   I'll change it later, when I decide how to make the list nodes variable
   in size.

   %%% It might be possible to fix this by a method similar to that used in
   rubik2. In the RIES case, we only add an expression to the tree when there
   is a single item on its FORTH stack, and there is no way to predict if or
   when a subexpression will ever have a stack depth of 1. However, we could
   add nodes to an auxiliary list when their MAX_ELEN space is full, then any
   expressions that come from them would have a predecessor pointer pointing
   back to the aux. item.
     For example, suppose ge.2 is at depth 16 and the current partial
   expression is ep2+3+4+5+6+7+8+. At this point there is no room to add more,
   and nothing has been put in the tree from this expression yet (except the
   initial "e"). So, we allocate a node that just contains "ep2+3+4+5+6+7+8+"
   and start reusing the MAX_ELEN space. Any nodes inserted into the LHS/RHS
   tree will have their "predecessor" pointer pointing back to the
   "ep2+3+4+5+6+7+8+" node, so that if they generate a match, the full
   subexpression can be reconstructed.
   */
#ifdef RIES_MAX_EXPRESSION_LENGTH
# define MAX_ELEN RIES_MAX_EXPRESSION_LENGTH
#else
# define MAX_ELEN 21
#endif

/* EXPR_ALLOC gives enough space for the expression and a terminating null */
#define EXPR_ALLOC (MAX_ELEN+1)

/* The size of the memory blocks we use */
#define ALLOC_SIZE 65536L  /* %%% should be at least 8x the VM page size */

/* This can be increased to improve speed, but it also decreases the
   efficiency of memory usage by having the balance of LHS's and RHS's be
   further from a 1:1 ratio. */
#define PASS_GRAN 1

/* -------------- typedefs --------------------------------------------------
/
/  Our function validate_types() checks the sized integer types at runtime
/  and may instruct the user to recompile with -DSHORT_IS_S16, etc. so if
/  one of these is defined, we use it to explicitly define s16.             */
#ifdef SHORT_IS_S16
  typedef short s16;
# define HAVE_S16
#elif INT_IS_S16
  typedef int s16;
# define HAVE_S16
#endif

#ifdef INT_IS_S32
  typedef int s32;
# define HAVE_S32
#elif LONG_IS_S32
  typedef long s32;
# define HAVE_S32
#endif

/* Without user override, we just guess. short has been 16-bit on virtually
/  all C compilers since about 1995, so it's a pretty safe guess.
/  {2012.0522: If we really wanted to glean the definition from the
/  environment, the cases I know of so far are:
/
/    Per the "stdint.h" in http://code.google.com/p/msinttypes/
/    Visual Studio 6 and Embedded VC++ 4 have a broken __int16
/    Use "#if (_MSC_VER < 1300)" and declare typedef signed short int16_t;
/
/    Per Markus Milleder, stdint.h is not available prior to 2010.
/    Use "#if (_MSC_VER < 1600)" to see if stdint is not yet provided
/    and declare typedef signed __int16 int16_t;
/
/    In 2010 (mirabile dictu) MSFT finally added stdint.h
/
/    In many other environments, but not all, stdint.h is available.
/    Various GCC flags might allow figuring out which is which.           */
#ifndef HAVE_S16
  typedef signed short s16;
#endif

#ifndef HAVE_S32
  typedef signed int s32;
#endif

/* Similarly, these might be replaced by stdbool.h in the future. We would
/  need to use #ifdefs and #define overrides.
/  There are two possible issues:
/  * As with stdint.h, we need to provide for MSVC (see
/    stackoverflow.com/questions/8548521/ )
/  * Defining "true" to a specific numeric value (like "1"), or comparing a
/    booolean to a specific numeric value, causes failures:
/      b001 flag1, flag2;
/      flag1 = true;        // Default option
/      flag2 = (argc > 2);  // More than two arguments
/      ...
/      if (flag1 == flag2)   // FAIL if comparing 1 to __INT_MIN__
/    Given that our definitions, the stdbool.h version, and old compilers'
/    implementations may differ, we check for boolean sanity in
/    validate_types().                                                      */
typedef int b001;
#define B_FALSE (1==0)
#define B_TRUE (1==1)

/* stats_count variables would be 64-bit integers, if it were easy to get
/  that across all platforms. Since RIES is designed to work on really old,
/  small systems (like a 50-MHz 486) we instead use double and a custom
/  "sprintf" function. The 53 bits given by IEEE 64-bit doubles is enough
/  for the statistics RIES keeps.                                         */
typedef double stats_count;

/* time_flt is a floating-point value used to hold a time measurement. */
typedef double time_flt;

/* SYMBOL_RANGE is the dimension of an array capable of storing one of
   each possible value of typedef symbol. */
#define SYMBOL_RANGE 256
typedef unsigned char symbol;

/* NOTE: %%% Although it looks like I am trying to maintain
   independence from having the symbol type be 'char', I actually have
   not accomplished this in the code. However, it won't be a total
   mess to convert it to 16-bit symbols or something like that, if
   someone decides that's necessary. The worst part will designing a
   new command-line syntax for specifying the symbolset for a search.

   However, *BEWARE*! If you're increasing the symbol set
   significantly past its original level of about 40 symbols, the
   users will pay dearly in efficiency (runtime). Having a great
   variety of symbols will massively slow down the search. In
   particular, please resist the temptation to make one symbol for
   every integer (or even every prime number) from 1 to 1000, or some
   other arbitrary big number. There should not be more integer
   symbols than every other symbol combined! RIES is perfectly happy
   synthesizing the larger integers from the small ones on its own,
   just like it does for the fractions and irrationals.

   If there's a problem with a search for "163.0" yielding "(x+2)/5 = 2^5+1"
   as an answer, the solution isn't to make "163" a symbol. Instead, look
   at ways to make RIES generate better-looking output by automatically
   printing the subexpression "2^5+1" as "33" and, when possble, simplify
   the equation by moving the other 5 to the RHS. The ability to
   show "163" expressed in terms of "two 2's, two 5's and a 1" is one of
   the reasons RIES was created. */

/* phantom symbols */
#define PS_REVSUB 1   /* argument-reversed subtract */
#define PS_REVDIV 2   /* argument-reversed divide */
#define PS_cross 3    /* implied multiply (not used) */
#define PS_REVPOW 4   /* argument-reversed exponentiation */
#define IS_PHANTOM(x) (x < 10)

/* A "ries_val" is a numeric value attained by performing calculations
   according to an postfix expression. ries_val's are created mainly
   by successive calls to exec(). */
#ifdef RIES_VAL_LDBL
  typedef long double ries_val;
# define RV_SS_FMT "%Lf"
#else
  typedef double ries_val;
# define RV_SS_FMT "%lf"
#endif

/* A "ries_dif" is the value of a derivative with respect to x (as calculated
   within exec()), and can aldo be a measure of uncertainty or the difference
   between two candidate expressions when looking for a match. In all of
   these cases, full exponent range is needed but the mantissa/significand
   need not more precise than 3 or 4 decimal digits. */
typedef double ries_dif;

/* Types of numbers. As the value increases, the labaling is more
restrictive: TYPE_RAT > TYPE_ALG because the rationals are a subset of
the algebraic numbers. */
#define TYPE_NONE 0  /* unknown, i.e. beyond the functions in RIES */
#define TYPE_TRAN 1  /* "transcendental": includes Gamma[pi], root of x^x=7 */
#define TYPE_TCEL 2  /* Timothy Chow's "exponential-logarithmic": algebraic
                        adding the complex exponent and logarithm operations,
                        equivalently all trig and inverse trig functions, e.g.
                        pi = -i ln(-1); sin z = (e^(iz)-e^(-iz))/2i; and
                        arctan(x) = 1/2i ln((x-i)/(x+i) + k */
#define TYPE_ALG  3  /* Algebraic (roots of any polynomial with algebraic
                        coefficients), not always constructible */
#define TYPE_CONS 4  /* Constructible (rationals extended by square root,
                        nested square roots okay) */
#define TYPE_RAT  5  /* Rational (integers extended by division) */
#define TYPE_INT  6  /* Integer (closed under add, subtract, and multiply) */
#define TGMIN(a,b) (((a)<(b)) ? a : b)
typedef s16 ries_tgs; /* tgs-manip */
#define TAG_INT_P(tg)  ((tg) == TYPE_INT)
#define TAG_RAT_P(tg)  ((tg) >= TYPE_RAT)

/* Other tags that might be useful in the future:
  - "blessed rational": This number is irrational, but has been tagged as
    valid for use in integer-only contexts at the expense of a higher
    complexity score
  - "blessed irrational": Similar, for irrationals
*/


/* this struct is used for expressions that have been inserted into
/  the binary tree. (LHS and RHS expressions are both put in the same tree) */
typedef struct expr {
  ries_val val;           /* The expression's floating-point value */
  ries_dif der;           /* The derivative (for LHS expressions only: if
                             RHS, this will be 0.) */
  ries_tgs tags;          /* The numerical attribute tags, e.g. TYPE_RAT */
                          /* tgs-manip */
  struct expr *left;      /* left child tree or 0 if none */
  struct expr *up;        /* parent node, or 0 if we're the head */
  struct expr *right;     /* right child tree or 0 if none */
  s16    elen;            /* number of symbols in the expression, e.g. 8 */
  symbol sym[EXPR_ALLOC]; /* The expression in symbolic form, e.g. "p6*1-qe-"
                             this field is last to allow for a possible
                             future with variable length allocation */
} expr; /* 8+8+4+4+4+2+21+1=52 bytes, or 64 if using 64-bit pointers */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
       Obfuscated Binary Trees (An idea that seemed hackish at the time)

A way to save memory on the RHS (constant subexpression) nodes. These
notes date back to a time when I considered maintaining separate trees
for LHS and RHS nodes.

typedef struct rhs_node {
  ries_val val;           // The expression's floating-point value
  void * P_L;             // parent and left child pointers
  void * P_R;             // parent and right child pointers
  symbol sym[EXPR_ALLOC]; // The expression in symbolic form, e.g.
                          // "p6*1-qe-", null-terminated. This field
                          // is last to allow for variable length
                          // allocation in the future.
} rhs_node; // size: 8+4+4+12=28

               P

         P^L        P^R
      L                  R

The three links (parent, left-child and right-child) are XORed
together into the two fields P^R and P^L. As the tree is traversed,
the traversal routine always keeps track of what node it came from and
what that node's numeric value was. Then the three links are
reconstructed as follows:

If the current node was reached by going down, we know P. L and R are
reconstructed by XORing P with the two fields P^R and P^L.

If this node was reached by going up, we know either L or R. To find
out which one, we compare the val field of this node with that of the
node we just came from. If it's bigger, we came from the left child
and therefore, L is known. If it's smaller we came from the
right-child and R is known.

If R is known, P is reconstructed by XORing R with P^R. Then L is
reconstructed by XORing P with P^L. If L is known, the same method is
used to get P and R.

There is another, perhaps more direct way to save one pointer per node:
maintain a stack of parent pointers. As you descend, push parent pointers
on the stack, and pop them off as you ascend. However, it does not
appear that the methods can be combined to save two pointers per node:
if you descend, the node you arrive at has unknown L and R, both of
which would have to get reconstructed.

typedef struct obt_traversal {
  s16 side;          // LHS or RHS
  void * realnode;   // points to actual LHS or RHS node (0 if we're
                     // at the root)
  ries_val x;        // The value
  ries_dif dx;       // Derivative
  void * up;         // Reconstructed parent pointer
  void * left;       // Reconstructed left-child pointer
  void * right;      // Reconstructed right-child pointer
  // If they want the expression, they can get it from *realnode
} obt_traversal;

// %%% obt (which literally stands for "obfuscated binary tree")
// routines aren't written yet. {And probably never will be: I want to
// keep the option of going to a balanced binary tree, which will matter a
// lot if we need to partition the tree for efficient implementation of
// finite-use symbolsets as in --numeric.anagram. Rebalancing an OBT is
// just too much of a pain. -20120518}

void obt_new(obt_traversal * it);
void obt_clone(obt_traversal * from, obt_traversal * to);
void obt_up(obt_traversal * it);
void obt_left(obt_traversal * it);
void obt_right(obt_traversal * it);
-----------------------------------------------------------------------------*/

/*                        M E T A S T A C K !

   A metastack works like a stack of stacks. Imagine a normal stack
   with its push and pop operators. A metastack lets you treat the whole
   stack as an object being pushed and popped. Every time you do a push or
   pop, a copy of the *entire stack* gets pushed onto the metastack.
     Metastacks support a third operation, called "undo", which pops the
   previous version of the stack off of the metastack; this popped stack
   replaces the current stack. Thus, undo enables you to go "back in time",
   returning the stack to the state it was at some point in the past
   without having to remember what values were popped and pushed.
     This is useful in RIES's expression generator because it allows
   intermediate calculations to be reused from one expression to the next
   during the recursive scan, and upon backtracking the stacks from all
   earlier, shorter subexpressions are restored.

To illustrate the workings of the metastack data structures and
explain why they are allocated as they are, we need to understand the
consequences of the choice of MAX_ELEN (which determines the maximum
number of symbols in an expression generated by ge.1 and ge.2).
  First, realize that gen_forms is designed to generate only those
forms that leave a single item on the stack at the end, and ge.1/ge.2
will comply with this. So the expression "234+" is "incomplete"
because it leaves two items on the stack. Also, "23++" would never be
generated even as a partial expression because it causes a stack
underflow.
  Odd and even differ:
  max expression               sp
      length     form   expr   max
  MAX_ELEN = 4:
                 aabc   23s+    2
                 aacb   23+s    2
                 abac   2s3+    2
                 abbb   2sss    1
  MAX_ELEN = 5:
                 aaacc  234++   3
                 aabbc  23ss+   2
                 aacac  23+4+   2
                 aacbb  23+ss   2
                 ababc  2s3s+   2
                 abbac  2ss3+   2
                 abbbb  2ssss   1

   expr.   operation  emulated     metastack data
                      stack        ---uv---  ---duv-- uvp  -------ms-------  msp  --s-- -ds-- sp
   []                 [] (empty)   [ , , , ] [ , , , ] 0   [ , , , , , , , ]  0   [ , ] [ , ]  0
   [2]     ms_push    2,           [ , , , ] [ , , , ] 0   [v, , , , , , , ]  1   [2, ] [0, ]  1
   [23]    ms_push    2,3,         [ , , , ] [ , , , ] 0   [v,v, , , , , , ]  2   [2,3] [0,0]  2
   [23s]   ms_pop     2,           [3, , , ] [0, , , ] 1   [v,v,^, , , , , ]  3   [2, ] [0, ]  1
           ms_push    2,9,         [3, , , ] [0, , , ] 1   [v,v,^,v, , , , ]  4   [2,9] [0,0]  2
   [23]    ms_undo    2,           [3, , , ] [0, , , ] 1   [v,v,^, , , , , ]  3   [2, ] [0, ]  1
           ms_undo    2,3,         [ , , , ] [ , , , ] 0   [v,v, , , , , , ]  2   [2,3] [0,0]  2
   [23+]   ms_pop     2,           [3, , , ] [0, , , ] 1   [v,v,^, , , , , ]  3   [2, ] [0, ]  1
           ms_pop     []           [3,2, , ] [0,0, , ] 2   [v,v,^,^, , , , ]  4   [ , ] [ , ]  0
           ms_push    5,           [3,2, , ] [0,0, , ] 2   [v,v,^,^,v, , , ]  5   [5, ] [0, ]  1
   [23]    ms_undo    []           [3,2, , ] [0,0, , ] 2   [v,v,^,^, , , , ]  4   [ , ] [ , ]  0
           ms_undo    2,           [3, , , ] [0, , , ] 1   [v,v,^, , , , , ]  3   [2, ] [0, ]  1
           ms_undo    2,3,         [ , , , ] [ , , , ] 0   [v,v, , , , , , ]  2   [2,3] [0,0]  2
 */
#define MS_UV_MAX   ((MAX_ELEN)-1)
#define MS_UNDO_MAX ((MAX_ELEN) * 2)
#define MS_STK_MAX  (((MAX_ELEN)+1) >> 1)
typedef struct metastack {
  ries_val uv[MS_UV_MAX]; /* undo values */
  ries_dif udv[MS_UV_MAX];/* undo values (derivatives) */
  ries_tgs utg[MS_UV_MAX];/* undo values (tags) */
  s16    uvp;             /* undo values pointer */
  s16    ms[MS_UNDO_MAX]; /* metastack (undo opcodes) */
  s16    msp;             /* metastack pointer (undo opcodes index) */
  ries_val s[MS_STK_MAX]; /* current stack */
  ries_dif ds[MS_STK_MAX];/* stack of derivatives for LHS */
  ries_tgs tg[MS_STK_MAX];/* tags, e.g. integer or rational */
  s16    sp;              /* stack pointer */
} metastack;
/* the metastack undo opcodes. If you wanted to support more than simple
   push and pop you would add another opcode for each operation (example:
   a modify-in-place operator, something which normally requires a pop
   followed by a push) */
#define MSO_PUSH 1
#define MSO_POP  2

/* "pe" stands for "partial expression". This struct is used by the
   recursive expression generator. Typically about half of its symbols
   will be filled in, and the complexity of the symbols written so far is
   compered to rminw and rmaxw for pruning.
     Here's an example. Let's say the current weight limits are 30 to 31:
   we're trying to generate expressions whose total weight is either 30
   or 31. And suppose further that the partial expression currently contains
   two symbols with a total weight of 18. That means that the remaining
   symbols must add up to either 12 or 13 for the expression to be accepted.
     When it's generating expressions, RIES already knows what form (see
   below) the expression fits into, and therefore it knows how many symbols
   are left to generate and what types they are. Therefore, certain statements
   can be made about the weights of the symbols yet-to-be generated. Let's
   say the current form calls for two more symbols, both of type 'b', and
   suppose further that all 'b' symbols have weights of 8, 9 or 10. Well, that
   means that at this point in our example equation, even if the two remaining
   symbols are 8's, the total weight will be too high. Thus, the first two
   symbols form an impossible combination (from the point of view of trying
   to meet the current range of 30 to 31), and the expression generator
   can backtrack immediately, without having to proceed to explore all the
   combinations of two more type 'b' symbols that might be added here. That
   saves a LOT of time -- this optimization alone cuts the number of
   expressions by a factor of over 200 (for searches of 10,000,000 expressions)
   and even more if a larger number of expressions is searched.
*/
typedef struct pe {
  s16    cplx;              /* complexity of this partial-expression */
  s16    elen;              /* number of symbols, e.g. 2 */
  symbol sym[EXPR_ALLOC];   /* the symbols, e.g. "p6" */
  s16    pe_rminw[EXPR_ALLOC]; /* remaining minimum weight */
  s16    pe_rmaxw[EXPR_ALLOC]; /* remaining maximum weight */
} pe;

/* A "form" is a pseudo-expression consisting only of A's, B's and C's. The
   A's, B's and C's represent the three types of symbols that make up
   real expressions. Before real expressions are generated, RIES first
   determines what sequences of type-A, type-B and type-C symbols will
   constitute legal expressions. (It's easier to describe what an illegal
   expression is: It's an expression that causes the stack to underflow
   by executing an operation without enough operands, or an expression
   that leaves extra stuff on the stack when it's done.) */
typedef struct form {
  s16    min_weight;       /* min attainable complexity with this form */
  s16    max_weight;       /* max attainable complexity with this form */
  s16    stack;            /* stack depth at end of form */
  s16    flen;             /* number of symbols in form */
  symbol sym[EXPR_ALLOC];  /* the form, e.g. "aabc" */
  s16    stk[EXPR_ALLOC];  /* stack height after applying this symbol */
  s16    arg1[EXPR_ALLOC]; /* ptr to first arg of any 'c' symbols in sym */
} form;
#define ARG1_NA -1

typedef s32 attr_bits; /* Attribute bits for symbol rules */

typedef struct sym_attr_block {
  symbol seft;       /* the symbol's stack effect */
  s16    sa_wgt;     /* for scoring */
  s16    preempt_weight;
  attr_bits sa_mask; /* Attributes, masked with this, must be 0 */
  s16    sa_alwd;    /* Number of symbols allowed in each expression */
  s16    sa_ct;      /* used in ge.2() to keep track of how many symbols
                        are in expression; part of -O option. */
  const char * defn; /* symbol definition for legend */
  const char * desc; /* Used for -S (show symbolset) option */
  s16    def_given;
  s16    def_needed;
  const char * sa_name; /* The "normal" (infix ASCII) symbol name */
  const char * name_forth;/* The symbol's postfix (FORTH) name */
  attr_bits amkey;      /* "easy" attributes */
  s16    sa_known;
} sym_attr_block;

#define MAX_SYMBOL_WEIGHT 100.0
#define MIN_SYMBOL_WEIGHT 0.0
#define MAX_SYM_NAME_LEN 24

s16 g_addsym_seq;

/* -------------- variables ----------------------------------------------- */

char * g_argv0;    /* Set to argv[0] by main for use by sudden death errors */

double g_levadj;   /* -l option or default DEFAULT_LEV_ADJ */

char *block_base;  /* pointer to current alloc block */
char *freepool;    /*   ... and the beginning of the free portion therein */
size_t freesize;   /*   ... and # of bytes left in the free portion */

expr *lhs_root;    /* binary tree for LHS list */
/* expr *rhs_root;    binary tree for RHS list (not currently used) */

s16 lmax;          /* current complexity maximum limit for LHS */
s16 rmax;          /* current complexity maximum limit for RHS */
s16 lmin;          /* current complexity minimum limit for LHS */
s16 rmin;          /* current complexity minimum limit for RHS */

double tlevel;     /* The maximum level (depth) of the search. The search
                      ends when LHS + RHS > tlevel */

ries_val g_target; /* The value for which we are searching */
ries_tgs g_targ_tags; /* The tags for the target value */
ries_dif g_mag_ulp;/* Magnitude of one base-10 unit in the last place (ulp),
                      used for --mad option */
ries_val exec_x;   /* the value exec() uses for 'x' symbol (differs from
                      g.target during Newton iteration) */

b001 got_exact;
stats_count g_num_matches;
stats_count g_max_matches;
symbol * g_matches = 0;
size_t g_mtch_alloc = 0;
ries_val * g_nr_deltas = 0;

/* PHSYM_xxx are 'phanton symbols' used to handle special cases
   in output formatting.
PHSYM_ATAN1 is for the legend description of the "1-argument arctangent",
which does not exist as a separate function, but is simply an alternate
form of printing in infix, when the second argument (denominator) is 1. */
#define PHSYM_ATAN1 0

sym_attr_block sym_attrs[SYMBOL_RANGE];
s16    weight_base;              /* weight per symbol for expression
                                    complexity score */
b001 x_lhs_only;              /* true if symbol 'x' should only be on
                                    LHS (this is not necessarily the same as
                                    "-Ox" because there can be multiple x's
                                    in an LHS) */
char * g_anagram = 0;            /* A string of digits used for solving
                                    "four 4's" and similar problems. */
b001 g_no_cv_simplify;
b001 g_one_sided;
b001 g_solve_for_x;
#define LINELEFT_INIT (79-2)

int used_trig;                   /* Set if any trig symbol has been used in
                                    a result */
int used_atan1;
int used_atan2;

b001 S_option;
b001 NOS_options;
b001 g_show_ss;
b001 g_reported_exhaustion;

ries_tgs g_restrict_subexpr; /* Set e.g. to TYPE_RAT by -r option */
ries_tgs g_restrict_exponents; /* Set e.g. to TYPE_RAT by -a option */
ries_tgs g_restrict_trig_args;

b001 g_relative_x;  /* true if X values should be given relative to T */
b001 g_wide_output; /* true if wide output mode is selected (perhaps
                          a bitmask in the future, but not yet) */
b001 g_explicit_multiply; /* Always show '*' symbol for multiplication */

/* attribute masks */
#define AM_KK  0x0001 /* K K - */
#define AM_1   0x0002 /* 1 - */
#define AM_2   0x0004 /* 2 - */
#define AM_n   0x0008 /* n - */
#define AM_r   0x0010 /* r - */
#define AM_55  0x0020 /* J K -   where J and K are both 5 or less */
#define AM_jK  0x0040 /* J K -   where J < K */
#define AM_RHS 0x0080 /* set only when filling RHS list */
#define AM_sq  0x0100 /* op -   where op is 's' or 'q' */
#define AM_1K  0x0200 /* 1 K - */
#define AM_l    0x0400 /* l - */
#define AM_E    0x0800 /* E - */
#define AM_pi   0x1000 /* p - */
#define AM_KxK   0x2000 /* K * K - */
#define AM_KpK   0x4000 /* K + K - */
#define AM_plus  0x8000 /* + - */
#define AM_mul  0x10000 /* * - */
#define AM_pow  0x20000 /* ^ - */
#define AM_a1_e 0x40000 /* e <expr> <seft_c_op> */
#define AM_a1_1 0x80000 /* 1 <expr> <seft_c_op> */
#define AM_a1_r 0x100000 /* r <expr> <seft_c_op> */

#define MAX_SEFT_POP 20
symbol g_asym[MAX_SEFT_POP];   /* the valid seft 'a' symbols */
s16 n_asym;
s16 g_a_minw;        /* minimum weight of seft 'a' symbols */
s16 g_a_maxw;        /* maximum weight of seft 'a' symbols */

symbol g_bsym[MAX_SEFT_POP];   /* the valid seft 'b' symbols */
s16 n_bsym;
b001 g_used_identity;
s16 g_b_minw;        /* minimum weight of seft 'b' symbols */
s16 g_b_maxw;        /* maximum weight of seft 'b' symbols */

symbol g_csym[MAX_SEFT_POP];   /* the valid seft 'c' symbols */
s16 n_csym;
s16 g_c_minw;        /* minimum weight of seft 'c' symbols */
s16 g_c_maxw;        /* maximum weight of seft 'c' symbols */

s16 s_minw;        /* minimum weight of any symbol */

s16 g_exhaust_cpx;  /* Maximum weight of an entire expression */

s16 max_flen; /* max length of forms generated thus far */

/* Irrational constants have lots-o-digits just in case this program ever
   gets ported to a C compiler that offers quad-precision floating point. */
ries_val   k_0 = 0.0L;
ries_val k_ern = (ries_val)-0.367879441171442321595523770161460867445811L; /* -1/e */
ries_val   k_1 = 1.0L;
ries_val k_phi = (ries_val)1.61803398874989484820458683436563811772030L;
ries_tgs tg_phi = TYPE_CONS;
ries_val   k_2 = 2.0L;
ries_val   k_e = (ries_val)2.71828182845904523536028747135266249775724L;
ries_tgs tg_e = TYPE_TCEL;
ries_val   k_3 = 3.0L;
ries_val  k_pi = (ries_val)3.14159265358979323846264338327950288419716L;
ries_tgs tg_pi = TYPE_TCEL;
ries_val   k_4 = 4.0L;
ries_val   k_5 = 5.0L;
ries_val   k_6 = 6.0L;
ries_val   k_7 = 7.0L;
ries_val   k_8 = 8.0L;
ries_val   k_9 = 9.0L;

/* Constants that parametrize functions */

ries_val k_sincos_arg_scale = 0;
b001 g_trig_scale_default;
ries_val k_sincos_max_arg = 1.0;

/* These constants are used to set legal limits in various functions */
ries_val k_sin_clip = (ries_val)0.99999L;
ries_val k_2pi = (ries_val)6.28318530717958647692528676655900576839433L;
ries_dif k_eXlim = 690.0;
ries_dif k_d_nan;
ries_dif k_d_inf;
ries_dif k_d_ninf;

ries_dif k_precision_ulp = 0.0;
ries_dif k_min_best_match = 1.0e-15;
ries_dif k_max_match_dist = -0.01; /* Default is to scale by 0.01 of the target
                                    value */
ries_dif g_init_match_dist;
b001 g_match_all_digits = B_FALSE;

ries_dif k_sig_loss = 0.01;

#define DEFAULT_K_VANISHED_DX 1.0e-6
ries_dif k_vanished_dx = DEFAULT_K_VANISHED_DX;
ries_dif k_biggest_safe_target = 10.0 / DEFAULT_K_VANISHED_DX;

ries_dif k_derivative_margin = 0;
ries_dif k_prune_deriv = 1.0e-10;
ries_dif k_newton_settled = 1.0e-15;

ries_dif g_min_matchsize = 0;
b001 g_exact_exit = B_FALSE;
b001 g_refinement = B_TRUE;

ries_dif p_ovr;
ries_dif n_ovr;

ries_val g_min_equ_val = -9.9e99;
ries_val g_max_equ_val = 9.9e99;

/* Format strings and precision constants.

The binary formats, and associated precision/significance values, of the
floating-point types that one is likely to encounter are:

                                          significand
                                          bin.  decimal
  IEEE binary64            double          53    15.95
  8087 80-bit extended     long double     64    19.27
  double-double                           107    31.21
  IEEE binary128           quad           113    34.01

The following variables (k_xxx_digits and fmt_g_xxx) are variables
initialized at runtime because at some point in the future, RIES might
support one of the higher precisions, selectable at runtime -- and also,
there might be a new option to select the number of digits that are
considered "nominal" and "usable", and how many get printed.

*/

/*
significant digits constants, pre-initialized for IEEE binary64.

The number of "nominal" digits is the significand (decimal) value above,
rounded to an integer.

The number of "usable" digits is based on the this and our choice of
k_sig_loss to limit loss of significance. k_sig_loss is 0.01 by default,
which is 10^-2 so we lose 2 digits of significance.

NOTE: If RIES_VAL_LDBL (or some other precision) is defined, these will get
set to different values at runtime in init.formats()
 */

int k_nominal_digits = 17;
int k_usable_digits = 15;
ries_dif k_ulp = 0.0625;
float k_mantissa_bits;

ries_val rv_maxint;  /* Largest integer that we can rely on measuring */

/* Formatting strings, pre-initialized for IEEE binary64. 'nominal'
and 'usable' are as defined above. Note that some binary formats
(anything that is not compiler-native, like double-double) will not
use these format strings, but instead only use the k_xxx_digits values
above, because the library xxprintf() functions only work with
compiler-native formats.

The 'fixed' strings are designed for fitting any value in a fixed
number of characters, so output lines up in neat columns. This has to
be 6 characters wider than the number of significant figures: 1 for
the sign, 1 for the decimal point, and 4 for an exponent like "e+27"
or "e-05". These strings from the output of "ries 2.5063 -DG" serve
as examples:

 |123456789.123456789.1| <- 21 characters wide
 +---------------------+ left-justified printf format: "%-21.15g"
 |748.729680171193     | common case: 1+15 = 16 characters
 |0.470252846911253    | leading zero: 1+1+15 = 17 characters
 |-0.000131760338472132| sign and 4 leading zeros: 1+1+1+3+15 = 21 characters
 |-5.80064296934474e-05| sign, decmial, 'e', sign and exponent: 1+1+15+1+1+2
 +---------------------+

*/
#define FMT_STR_SIZE 32
char fmt_g_nominal[FMT_STR_SIZE];   /* e.g. "%.17g" */
char fmt_g_nom_fixed[FMT_STR_SIZE]; /* e.g. "%-23.17g" */

char fmt_g_usable[FMT_STR_SIZE];    /* e.g. "%.15g" */
char fmt_g_usa_fixed[FMT_STR_SIZE]; /* e.g. "%-21.15g" */

char fmt_g_diff[FMT_STR_SIZE];      /* e.g. "%.7g" */
char fmt_g_dif_fixed[FMT_STR_SIZE]; /* e.g. "%-13.7g" */


/* Variables used by the search algorithm */
ries_val  best_match;

stats_count g_ne;
long insert_count;
stats_count prune_count, lhs_prune, rhs_prune;
long mem_used_KiB;
unsigned long mem_used_bytes;
int out_expr_format;
#define OF_POSTFIX 0
#define OF_CONDENSED 1
#define OF_NORMAL 2
#define OF_FORTH 3

long lhs_insert, rhs_insert;
stats_count lhs_gen, rhs_gen;
stats_count gen_total;

/* Counters used by thrash_check to estimate how long it should have taken
   us to use a given chunk of memory */
long g_exec_calls;
long g_cv_calls;

/* debugging options: */
/* numbers from "ries -l2 2.5063141592653589 -DJ | wc -l"   */
s16 debug_S; /*  try.solve                                    100    */
s16 debug_s; /*  report.match: "show work"                    277    */
             /*       (values of all subexpressions)                 */
s16 debug_N; /*  eval: sym, x and dx at each step             461    */
s16 debug_n; /*  newton: x and dx at each step                136    */
s16 debug_o; /*  check.match entry                         539235    */
s16 debug_p; /*  infix.preproc                                112    */
s16 debug_Q; /*  cv.simplify                                   51    */
s16 debug_q; /*  check.match past 1st-stage test              140    */
s16 debug_m; /*  ms_push, ms_pop, ms_peek, ms_undo       10247603    */
s16 debug_r; /*  exec                                     1806085    */
             /*  ge.2:                           UPPERCASE  lowercase
                                                   for LHS   for rhs */
             /*    exec:                                             */
s16 debug_A; /*      prune partial exec error        42836     87770 */
s16 debug_B; /*      prune partial 0 or dx near 0     3173      2714 */
s16 debug_C; /*      prune partial noninteger        81056    697227 */
             /*              (using command: "ries -l2 1047 -i -DC") */
s16 debug_D; /*      prune partial overflow           1751      4350 */
             /*    full expr:                                        */
s16 debug_E; /*      prune expr already in database 102356    272746 */
s16 debug_F; /*      canon.val                      349368    848882 */
             /*   (use: ries -l2 2.50631415926 --canon-reduction nr25 -DF) */
s16 debug_G; /*      insert                          96112     97337 */
s16 debug_0; /*      dump entire database                    1712490 */
             /*    partial expr:                                     */
s16 debug_H; /*      rules                          409175    816240 */
s16 debug_I; /*      symbols to try                3904331   7759741 */
s16 debug_J; /*        prune complexity            2579116   5102516 */
s16 debug_K; /*        prune rules                  257302    558199 */
s16 debug_L; /*        prune symcount                61994    114453 */
             /*   (use: ries -l2 2.50631415926 '-O-+/^v*qsrlLeEpf' -Dl) */
             /*  ge_1:                                               */
s16 debug_t; /*    entry                                    11017    */
s16 debug_u; /*    rminw and rmaxw calculation              48895    */
s16 debug_v; /*    expressions generated                     5525    */
s16 debug_w; /*  gf_1                                       32922    */
s16 debug_x; /*  add.rule                                      91    */
s16 debug_y; /*  main loop                                    736    */
s16 debug_z; /*    init and miscellaneous                      55    */
s16 debug_M; /*    memory allocation                           46    */

#define DBG_LHS 2
#define DBG_RHS 1

/* -------------- variables used for special commands --------------------- */

s16 g_enable_output;  /* Enable normal ries output */
s16 g_eval_expr;

/* For the --find.expression command */
#define MAX_FIND_EXPR 16
s16 g_num_find_expr;
symbol * g_find_expr[MAX_FIND_EXPR];


/* -------------- prototypes ---------------------------------------------- */

void ieee_paranoia(void);
void init_formats(void);
void show_version(void);
void brief_help(void);

void ries_strncpy(char * to, char * fr, int n);
ries_val ries_intpow(ries_val x, int p);
void ries_to_digits(ries_val x, char *s, int *expn, int *sign, int precision);
void ries_snprinf_int(char * to, int len, int x);
int ries_strlen(char * s);
void ries_bltr0(char * s);
void ries_spfg(char *s1, int length, char sign_flag, int precision,
  ries_val x);
void spfg(int prec, ries_val x);
void spff(int prec, ries_val x);
void ries_spfg_test(char *s1, int length, char sign_flag, int precision,
  ries_val x);
void msal_test_spfg(void);

char * file_read(const char * filename);
void delimit_args(const char *rawbuf, size_t * nargs, char * * * argv);

const char * err_string(s16 err);
const char * tagname(int t);

time_flt gettime(void);
void inittime(void);
int bitcount(unsigned long x);
void thrash_check(long alloced);
void init_mem(void);
void * my_alloc(size_t size);
void purgeall_mem(void);

void ms_init(metastack *ms);
void ms_push(metastack *ms, ries_val x, ries_dif dx, ries_tgs tags);
ries_val ms_pop(metastack *ms, ries_dif *diff, ries_tgs * tags);
ries_val ms_peek(metastack *ms, ries_dif *diff, ries_tgs * tag, s16 *sptr);
void ms_undo(metastack *ms);

double arctan2(double a, double b);
long double arctan2l(long double a, long double b);

s16 exec(metastack *ms, symbol op, s16 *undo_count, s16 do_dx);

s16 infix_1(symbol * expr, char * term, symbol * t_op);
void cv_phantoms(symbol * s);
void infix_preproc(symbol * expr, symbol * out);

symbol * symstrsym(symbol * exp1, symbol sym);
unsigned int symstrlen(symbol * s);
int symstrtrail(symbol * big, symbol * little);
int bothtrail(symbol * a, symbol * b, symbol *tr);
void symstrclip(symbol * s, unsigned int len);
int symstrcmp(symbol * a, symbol * b);
int symstrneq(symbol * a, symbol * b, unsigned int n);
symbol * symstrsymstr(symbol * haystack, symbol * needle);
int symstrncpy0(symbol *to, symbol *from, int len);
void symstrncat(symbol *to, symbol *from, long len);
void str_remap(char *s, char from, char to);

s16 infix_expand(char * input, char * output);
s16 postfix(symbol * expr, char * term);
s16 postfix_formatter(symbol * expr, char * out, s16 maxlen);
s16 complexity(symbol * expr);

int endstack(symbol * expr, int *ending_sp, symbol * * last_sp_1,
  symbol * * last_sym);
int expr_break(symbol * expr, symbol * op, symbol * seft,
  symbol * * arg1, int * a1_len, symbol * * arg2, int * a2_len);
void expr_print_infix(symbol * expr, int justify);
void eqn_print_infix(symbol * lhs, symbol * rhs);

s16 eval(symbol * expr, ries_val * val, ries_dif * dx, ries_tgs * tags,
         s16 * sptr, s16 show_work);
void try_solve(symbol * l, symbol * r,
  symbol * l_out, int l_len, symbol * r_out, int r_len);
s16 newton(symbol * lhs, symbol * rhs, ries_val *root, ries_dif *diff_dx,
  ries_tgs *tags);
s16 cv_simplify(symbol * lhs, symbol * rhs, ries_val *root, ries_dif *diff_dx,
  ries_tgs *tags, int do_newton);
void defsym_used(symbol * expr);
void describe_symbols(void);
char * pf_intfloat_wid(stats_count x, int width);
void print_end(int exit_code);
void check_exit(int is_exact);
void report_match(symbol * lhs, symbol * rhs, symbol * exm,
  ries_val root, ries_dif delta, int did_newton);
int check_match(expr * lhs, expr * rhs);

expr * bt_first(expr * tree);
int bt_depth(expr * it);
expr * bt_prev(expr *it);
expr * bt_next(expr *it);
void check_sides(expr * it);
void check_exact_match(expr * it, ries_dif new_dx, pe *ex);
s16 bt_insert(ries_val x, ries_dif dx, ries_tgs tg, pe *ex, s16 * res1);
s16 canonval(pe * bpe, metastack * ms, ries_val *p_x, ries_dif *p_dx,
  ries_tgs *p_tg, s16 * muc_ptr, s16 using_x);
void decanon(metastack * ms, s16 muc);

stats_count ge_2(form *base, pe *bpe, s16 e_minw, s16 e_maxw,
                 metastack *ms, s16 using_x);

stats_count ge_1(form *base, s16 e_minw, s16 e_maxw, s16 using_x,
  s16 a_minw, s16 a_maxw, s16 b_minw, s16 b_maxw, s16 c_minw, s16 c_maxw);
stats_count gf_1(form *base, s16 minw, s16 maxw, s16 using_x,
  s16 a_minw, s16 a_maxw, s16 b_minw, s16 b_maxw, s16 c_minw, s16 c_maxw);
stats_count gen_forms(s16 minw, s16 maxw, s16 using_x,
  s16 a_minw, s16 a_maxw, s16 b_minw, s16 b_maxw, s16 c_minw, s16 c_maxw);

void def_amkey(const char * syms, attr_bits mask);
void define_amkeys(void);

void add_symbol(symbol sym, const char *name_forth, const char *name_infix,
  symbol type, s16 weight,
  const char * def_terse, const char * def_normal, const char * description);
/* The macro ADDSYM_NAMES is used to pass the three variants of a symbol's
   name, using the macro helps make the calls to add.symbol() a bit more
   readable. */
#define ADDSYM_NAMES(ascii1,FORTH,infix)  (ascii1), (FORTH), (infix)

void setup_abc_mmw(void);

void show_symset(void);
void add_rule(const char * symset, char sym, attr_bits mask);
void init_numerics(void);
void init_symbol_names(void);
void allsyms_set(s16 n, int include_x);
void somesyms_set(symbol * s, s16 n);
void set_anagram(char * anagram);
void init1(void);
void init2(void);
int unique_eqn(symbol * lhs, symbol * rhs, int addit);
int parse_target(char *str);
void set_debug_opts(char * str);
void set_restrict_rat(void);
void set_restrict_alg(int restrict_trig);
char * pa_defaults_path(void);
char * pa_next_peek(void);
int pa_next_isparam(void);
char * pa_get_arg(void);
char * pa_stk_pop(void);
void parse_args(size_t nargs, char *argv[]);
void validate_types(void);
ries_tgs guess_valtype(ries_val v);
int main(int nargs, char *argv[]);

/* -------------- functions ----------------------------------------------- */

char * pa_def_path;

void show_version(void)
{
  printf(
    "ries version of %s, Copyright (C) 2000-2024 Robert P. Munafo\n",
    RIES_VERSION);

  printf(
    "  architecture: %s\n",
#ifdef __POWERPC__
    "PowerPC"
#else
# ifdef __i386__
    "Intel-32"
# else
#   ifdef __x86_64__
    "Intel-64"
#   else
    "Unknown"
#   endif
# endif
#endif
  );

  printf(
    "  precision: %s (%d nominal, %d usable)\n",
#ifdef RIES_VAL_LDBL
    "long double",
#else
    "double",
#endif
    k_nominal_digits, k_usable_digits
  );

  printf(
    "  mathlib: %s\n",
#ifdef RIES_USE_SA_M64
  "stand-alone"
#else
  "standard"
#endif
  );

  printf("  MAX_ELEN == %d\n", MAX_ELEN);

  if (pa_def_path) {
    printf("  profile: %s\n", pa_def_path);
  }

#ifdef RIES_USE_SA_M64
  printf("\n");
  msal_version_info();
#endif

  printf("\n");
  printf("%s",
"RIES is provided under the GPL license v3. Source code at mrob.com/ries\n"
"This is free software; see the source for copying conditions.  There is NO\n"
"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
  );
}

void brief_help(void)
{
  printf("%s",
"Usage:\n"
"  ries [options] target-value\n"
"\n"
"Target value is required and may be any number. Options include:\n"
"  -l3   Search further (``level-3 search'')\n"
"  -x    Show matched values as ``x = value'' rather than ``x = T + epsilon''\n"
"  -s    (Sort of) solve by transforming to ``x = ...'' form\n"
"  -N+-/ Do not use symbols +, - or /\n"
"  -S    Show list of all available symbols\n"
); printf("%s",
"  -l    Restrict to Liouvillian solutions\n"
"  -a    Restrict to algebraic solutions\n"
"  -c    Restrict to ``constructible'' (straightedge and compass) solutions\n"
"  -r    Restrict to rational solutions (-re for exact match)\n"
"  -i    Restrict to integer solutions (-ie for exact match)\n"
"\n"
"There are many more options; get the full manual at mrob.com/ries\n"
);
} /* End of brief.help */

/* -------------- formatting and conversion ------------------------------- */

/* Copy a string as if by strncpy */
void ries_strncpy(char * to, char * fr, int n)
{
  int i;
  for(i=0; (i<n) && fr[i]; i++) {
    to[i] = fr[i];
  }
  /* copy the final 0 if we have room */
  if (i<n) {
    to[i] = fr[i];
  }
}

/* Raise any value to an integer power. */
ries_val ries_intpow(ries_val x, int p)
{
  int i;
  ries_val rv;
  ries_val pow2;
  int recip = 0;

  pow2 = x;

  if (p < 0) {
    p = -p;
    recip = 1;
    if (p < 0) {
      /* p was MININT */
      p = p / 2;
      p = -p;
      pow2 = x * x;
    }
  }

  rv = 1.0;
  pow2 = x;
  i = p;
  while(i > 0) {
    if (i & 1) {
      rv = rv * pow2;
    }
    i >>= 1;
    if (i) {
      /* We could do better here by a special-case for x near 1.0 */
      pow2 = pow2 * pow2;
    }
  }

  if (recip) {
    return(1.0 / rv);
  }
  return(rv);
} /* End of ries_intpow */

/* This routine returns the first N significant digits of an ries_val
   value. It goes to fairly thorough measures to ensure that rounding
   is done properly. You supply a string with a little bit more than
   the needed amount of space, and (if desired) an integer in which to
   store the exponent. */
void ries_to_digits(ries_val x, char *s, int *expn, int *sign, int precision)
{
  int dig1 = precision + 1;  /* number of digits to compute */

  ries_val r; /* "remainder", the portion not yet turned into digits. */
  ries_val pw;
  int e;  /* exponent */
  int i, d;
  int sgn;

  if (x <= k_d_ninf) {
    ries_strncpy(s, (char *) "-inf", precision);
    return;
  } else if (x >= k_d_inf) {
    ries_strncpy(s, (char *) "+inf", precision);
    return;
  } else if (!((x > k_d_ninf) && (x < k_d_inf))) {
    ries_strncpy(s, (char *) "NaN", precision);
    return;
  }

  sgn = 0;
  if (x < 0.0) {
    x = - x;
    sgn = 1;
  }
  if (sign) {
    *sign = sgn;
  }

  r = (x<0) ? (-x) : x;

  if (x == 0.0) {
    /* x == 0.0 */
    if (expn) {
      *expn = 0;
    }
    for (i = 0; i < precision; i++)
      s[i] = '0';
    return;
  }

  /* First determine the (approximate) exponent. */
  e = (int) (FLOOR(LOG10(FABS(x))));

  if (e < -300) {
    r = r * ries_intpow(10.0, 300);
    pw = ries_intpow(10.0, (e + 300));
    r = r / pw;
  } else if (e > 0) {
    pw = ries_intpow(10.0, e);
    r = r / pw;
  } else if (e < 0) {
    pw = ries_intpow(10.0, -e);
    r = r * pw;
  }

  /* Fix exponent if we are off by one */
  if (r >= 10.0) {
    r = r / 10.0;
    e++;
  } else if (r < 1.0) {
    r = r * 10.0;
    e--;
  }

  if (r >= 10.0 || r < 1.0) {
    fprintf(stderr, "ries_to_digits: exponent adjust failed (x=%g, r=%g).\n",
      ((double) x), ((double) r));
    return;
  }

  /* Extract the digits */
  for (i = 0; i < dig1; i++) {
    d = ((int) r);
    r = r - ((ries_val) d);
    r = r * 10.0;

    s[i] = ((char)(((int)'0') + d));
  }

  /* Fix negative digits. */
  for (i = dig1-1; i > 0; i--) {
    if (s[i] < '0') {
      s[i-1]--;
      s[i] = ((char)(((int)(s[i])) + 10));
    }
  }

  if (s[0] <= '0') {
    fprintf(stderr, "ries_to_digits: non-positive leading digit.\n");
    return;
  }

  /* Round, handle carry */
  if (s[dig1-1] >= '5') {
    s[dig1-2]++;

    i = dig1-2;
    while (i > 0 && s[i] > '9') {
      s[i] = ((char)(((int)(s[i])) - 10));
      i--;
      s[i] = ((char)( ((int)(s[i])) + 1));
    }
  }

  /* If first digit is 10, shift everything. */
  if (s[0] > '9') {
    e++;
    for (i = precision; i >= 2; i--) s[i] = s[i-1];
    s[0] = '1';
    s[1] = '0';
  }

  s[precision] = 0;
  if (expn) *expn = e;
} /* End of ries_to_digits */

/* Format an int into a string as if by snprintf */
void ries_snprinf_int(char * to, int len, int x)
{
  char * s;
  int x2, i;
  char c;

  s = to;
  if(len <= 0) {
    return;
  }
  /* Reserve space for trailing null */
  len--;
  if (len <= 0) {
    /* If that's all we have, leave now */
    *s++ = 0;
    return;
  }
  if (x<0) {
    *s++ = '-'; len--;
    x = -x;
  }
  if (len <= 0) { *s++ = 0; return; }

  /* Handle 0 */
  if (x == 0) {
    *s++ = '0';
    *s = 0;
    return;
  }

  /* Find out if int will fit */
  x2 = x;
  while(x2>0) {
    len--;
    x2 = x2 / 10;
  }
  if (len <= 0) { *s++ = 0; return; }

  /* Write digits in reverse order */
  len = 0; /* len now counts number of digits output */
  x2 = x;
  while(x2 > 0) {
    s[len] = ((char)(((int)'0') + (x2 % 10)));
    len++;
    x2 = x2 / 10;
  }
  /* Trailing null */
  s[len] = 0;

  /* Reverse the string in place */
  for(i=0; i<len/2; i++) {
    c = s[i]; s[i] = s[len-1-i]; s[len-1-i] = c;
  }

  /* We're done */
} /* End of ries_snprinf_int */

/* Measure the length of a string as if by strlen */
int ries_strlen(char * s)
{
  int i;
  for(i=0; s[i]; i++) {
  }
  return i;
}

/* Blank out trailing '0' digits in a string */
void ries_bltr0(char * s)
{
  char *t;

  /* Return right away on null inputs */
  if ((s == 0) || (!(*s))) {
    return;
  }

  /* Find the end of s */
  for(t=s; *t; t++) { }

  /* Back up one character */
  t--;

  /* Keep backing up and nulling out chars as long as they're 0's */
  while((t >= s) && (*t == '0')) {
    *t = 0;
    t--;
  }
}

/* ries_spfg prints an ries_val into a string, using format similar to
   the standard C library printf("%10.3g", val). There is a length field,
   controlling the maximum length of output and a precision field
   specifying how many digits you want. For best results the length should
   be at least precision+8, to allow for the worst case of the sign,
   decimal point, "e", exponent sign, and 3-digit exponent plus the trailing
   null. For example, the string "-1.23e-123" has "precision" 3, but is
   7 characters longer than the precision.

   The parameters are:

   char * s1       - output string. Must have at least as many bytes as the
                     parameter 'length'
   length          - length of the buffer s1. No more than length-1 printable
                     chars will be emitted, followed by a null.
   sign_flag       - pass in '+' if you want explicit + signs; pass anything
                     else to have no '+' signs.
   precision       - number of significant digits to generate.
   ries_val x      - the number to format.

   For the opposite conversion, use ries_sscan.
 */
void ries_spfg(char *s1, int length, char sign_flag, int precision,
  ries_val x)
{
  char *s;
  int sign;
  int exponent;
  int i;
  char s2[40];
  char sexp[8]; /* Signed exponent as a string */

  s = s1;

  /* Handle unreasonably short output buffers */
  if (length <= 0) {
    return;
  } else if (length < 8) {
    for(i=0; i<(length-1); i++) {
      s[i] = '!';
    }
    s[i] = 0;
    return;
  }

  if (x <= k_d_ninf) {
    ries_strncpy(s, (char *) "-inf", precision);
    return;
  } else if (x >= k_d_inf) {
    ries_strncpy(s, (char *) "+inf", precision);
    return;
  } else if (!((x > k_d_ninf) && (x < k_d_inf))) {
    ries_strncpy(s, (char *) "NaN", precision);
    return;
  }

  if (precision > 35) {
    precision = 35;
  } else if (precision < 1) {
    precision = 1;
  }

  ries_to_digits(x, s2, &exponent, &sign, precision);

  if (sign) {
    *s++ = '-'; length--;
  } else if (sign_flag == '+') {
    *s++ = '+'; length--;
  }

  if ((exponent > 0) && (exponent < precision)) {
    /* It can be formatted as a normal number without an exponent field */
    /* length needs to be at least enough for lead digits and trailing null */
    if (length < exponent+2) {
      ries_strncpy(s, (char *) "fmt-err", length);
      return;
    }
    i = 0;
    /* Emit lead digit */
    *s++ = s2[i++]; length--;
    /* Emit the rest of the digits before the decimal point */
    while (exponent) {
      *s++ = s2[i++]; length--;
      exponent--;
    }

    /* Check the rest of the digits for trailing 0's */
    ries_bltr0(s2);
    if (*s2 == 0) {
      *s++ = 0;
      return;
    }

    if (length > 1) {
      *s++ = '.'; length--;
    }
    for(; s2[i] && (i<precision) && (length > 1); i++) {
      *s++ = s2[i]; length--;
    }
    *s++ = 0;
    return;

  } else if (exponent == 0) {
    /* Values from 1.00000 to 9.99999 */
    /* length needs to be at least enough for lead digit, decimal point
       and trailing null */
    if (length < 3) {
      ries_strncpy(s, (char *) "!!!", length);
      return;
    }
    *s++ = s2[0]; length--; /* Lead digit */

    /* Check the rest of the digits for trailing 0's */
    ries_bltr0(s2+1);
    if (s2[1] == 0) {
      *s++ = 0;
      return;
    }

    *s++ = '.'; length--;
    for(i=1; s2[i] && (i<precision) && (length > 1); i++) {
      *s++ = s2[i]; length--;
    }
    *s++ = 0;
    return;

  } else if ((exponent < 0) && (exponent > -5)) {
    /* Values like 0.12345, 0.012345, etc. */
    /* Length needs to be enough for leading 0's, decimal point, one
       significant digit, and trailing null */
    if (length+exponent < 3) {
      ries_strncpy(s, (char *) "fmt-err", length);
      return;
    }

    /* Add a suitable number of zeros */
    *s++ = '0'; length--;
    *s++ = '.'; length--;
    exponent++;
    while ((exponent < 0) && (length > 1)) {
      *s++ = '0'; length--;
      exponent++;
    }

    /* Remove extra trailing 0's */
    ries_bltr0(s2);
    if (*s2 == 0) {
      *s++ = 0;
      return;
    }

    for(i=0; s2[i] && (i<precision) && (length > 1); i++) {
      *s++ = s2[i]; length--;
    }
    *s++ = 0;
    return;

  }

  /* General case: use scientific notation */

  /* Get the exponent as a string */
  if (exponent >= 0) {
    sexp[0] = '+';
    ries_snprinf_int(sexp+1, sizeof(sexp)-1, exponent);
  } else {
    /* ries_snprinf_int will print the '-' sign */
    ries_snprinf_int(sexp, sizeof(sexp), exponent);
  }

  /* Here we are a little forgiving about the length: We'll deduct the
     space we need for the signed exponent and 'e', then emit as many
     digits as we can. Note that s2[] already has the requested number
     of digits. So we just need to make sure the length can accommodate
     the lead digit, decimal point and exponent. */
  if (length - (1 + 1 + 1 + ries_strlen(sexp)) < 1) {
    ries_strncpy(s, (char *) "fmt-err", length);
    return;
  }

  /* Reserve space for 'e' and exponent and null */
  length = length - (1 + ries_strlen(sexp) + 1);

  *s++ = s2[0]; length--; /* Lead digit */

  ries_bltr0(s2+1);
  if (s2[1]) {
    /* We have some nonzero digits to print */
    *s++ = '.'; length--;
    for(i=1; (i<precision) && s2[i] && (length > 0); i++) {
      *s++ = s2[i]; length--;
    }
  }
  *s++ = 'e'; length--;
  /* We now have just enough room for the exponent */
  for(i=0; sexp[i]; i++) {
    *s++ = sexp[i];
  }
  *s++ = 0;
} /* End of ries_spfg */

/* Print a ries_val as if by printf("%.23g", x) where the precision is
given by the parameter 'prec'. */
void spfg(int prec, ries_val x)
{
  char tmp[100];
  ries_spfg(tmp, 100, 0, prec, x);
  printf("%s", tmp);
}

/* Print a ries_val as if by printf("%29.23g", x) where the precision is
given by the parameter 'prec'. */
void spff(int prec, ries_val x)
{
  char tmp[100]; char fmt[FMT_STR_SIZE];
  ries_spfg(tmp, 100, 0, prec, x);
  snprintf(fmt, FMT_STR_SIZE, "%%-%ds", prec+6);
  printf(fmt, tmp);
}

#ifdef RIES_USE_SA_M64

float g_min_spfg_mbits = 256.0;
ries_val g_worst_spfg_inpt;

/* Wrapper for ries_spfg that converts the result string back to a ries_val
and watches for the worst result */
void ries_spfg_test(char *s1, int length, char sign_flag, int precision,
  ries_val x)
{
  ries_val xc;
  int nv;

  ries_spfg(s1, length, sign_flag, precision, x);
  nv = sscanf(s1, RV_SS_FMT, &xc);
  if ((nv) && (x != 0)) {
    ries_val rat, diff;
    float db;
    rat = xc / x;
    diff = rat - 1.0;
    if (diff < 0) {
      diff = 0 - diff;
    }
    if (diff > 0) {
      db = 0.0;
      while (diff < 1.0) {
        db = db + 1.0f;
        diff *= 2.0;
      }

      if (db < g_min_spfg_mbits) {
        g_min_spfg_mbits = db;
        g_worst_spfg_inpt = x;
        if (debug_z) {
          printf("New poorest conversion (%g bits)"
            " from input %23.17g -> '%s'\n", db, ((double)x), s1);
        }
      }
    }
  }
} /* end of ries_spfg_test */

/* Generate a huge number of values and test conversion via ries_spfg */
void msal_test_spfg(void)
{
  int ei, n;
  ries_val eps; /* epsilon */
  ries_val k8, k10;

  printf("msal_test_spfg: starting...\n"); n = 0;
  eps = 0.5;
  k8 = 8.0; k10 = 10.0;
  /* Test each epsilon */
  for(ei=0; ei<100; ei++) {
    ries_val b8, b10; /* "big" test values */
    ries_val s8, s10; /* "big" test values */
    int di;

    for (di=0; di<2; di++) {
      int si;
      if (di == 0) {
        b8 = b10 = s8 = s10 = 1.0 - eps;
      } else {
        b8 = b10 = s8 = s10 = 1.0 + eps;
      }
      /* Run through range of large and small values */
      for(si=0; si<100; si++) {
        char tmp[100];
        int si;
        /* Make values bigger/smaller */
        b8 = b8 * k8; b10 = b10 * k10;
        s8 = s8 / k8; s10 = s10 / k10;
        for(si=0; si<2; si++) {
          ries_spfg_test(tmp, 100, 0, k_nominal_digits, b8); n++;
          ries_spfg_test(tmp, 100, 0, k_nominal_digits, b10); n++;
          ries_spfg_test(tmp, 100, 0, k_nominal_digits, s8); n++;
          ries_spfg_test(tmp, 100, 0, k_nominal_digits, s10); n++;
          /* Negate everything */
          b8 = -b8; b10 = -b10; s8 = -s8; s10 = -s10;
        }
      }
      /* Go to the next epsilon */
      eps = eps * 0.375;
    }
  }
  printf("msal_test_spfg: %d tests completed.\n", n);
}

#endif /* RIES_USE_SA_M64 */

/* -------------- profiles and argument parsing --------------------------- */

#define FILE_READ_SIZE 1024
#define FILE_READ_MAX 10*1024*1024
char * file_read(const char * filename)
{
  FILE * in;
  size_t buf_sz;
  char * base_ptr;
  size_t t_len;

  /* We read in binary mode to avoid having the OS change anything. In
     particular, by definition the RIES profile format is not a "text" file
     format, it is "a sequence of text-like tokens separated by
     non-text bytes". Right now we treat all control characters as
     token-delimiters (and additionally as an end-of-comment delimiter in
     most cases if after a '#') but I may want to change that in the future.
   */
  in = fopen(filename, "rb");
  if (in == NULL) {
    char * name_ext;
    /* Try appending ".ries"
       (Allocation and UNIX compatibility by Markus Milleder, 20120428) */
    /* The typecast to "(char *)" avoids the warning "request for implicit
       conversion from 'void *' to 'char *' not permitted in C++" given by
       the option -Wc++-compat */
    t_len = strlen(filename) + 6; /* + ".ries\0" */
    name_ext = (char *) malloc(t_len);
    if (name_ext) {
      snprintf(name_ext, t_len, "%s.ries", filename);
      in = fopen(name_ext, "rb");
      free(name_ext);
    }
  }

  if (in == NULL) {
    fprintf(stderr, "%s: Could not open '%s' or '%s.ries' for reading.\n"
      "\n\n", g_argv0, filename, filename);
    brief_help();
    print_end(-1);
  }

  buf_sz = FILE_READ_SIZE;
  base_ptr = (char *) malloc(buf_sz);
  t_len = 0;

  while (!feof(in) && !ferror(in)) {
    if (t_len + FILE_READ_SIZE > buf_sz) {
      /* The next read might go beyond the allocated memory, so we need to
         reallocate */
      if (buf_sz > FILE_READ_MAX) break;
      buf_sz = buf_sz * 2;
      base_ptr = (char *) realloc(base_ptr, buf_sz);
    }
    {
      /* Now read a little bit more */
      char * p = base_ptr + t_len;
      t_len += fread(p, 1, FILE_READ_SIZE, in);
    }
  }

  fclose(in);

  /* Reallocate again to free up the memory we didn't use. */
  base_ptr = (char *) realloc(base_ptr, t_len + 1);

  /* It needs a trailing null byte */
  base_ptr[t_len] = 0;

  return base_ptr;
} /* End of file_read */

/* "A RIES argument-file is a sequence of one or more non-blank words
   separated by blanks." */
void delimit_args(const char *rawbuf, size_t * nargs, char * * * argv)
{
  size_t n;
  char ** av;

  n = 0;
  av = 0;

  if (nargs) { *nargs = n; }
  if (argv) { *argv = av; }

  if (rawbuf) {
    unsigned char * p;

    n = 0;

    /* Scan for and canonicize whitespace. */
    p = (unsigned char *) rawbuf;
    while(*p) {
      /* Skip any leading space */
      while(*p && ((*p <= ' ') || (*p == '\177'))) { *p = ' '; p++; }
      if (*p == '#') {
        /* Comment delimiter. Change everything to space until we get to
           a non-tab control character, which we assume is an end of line */
        while(*p && ((*p == '\t') || (*p >= ' '))) { *p = ' '; p++; }
      } else if (*p && (*p > ' ')) {
        /* A word that doesn't start with '#' is an arg; skip it */
        while(*p && (*p > ' ')) { p++; }
      }

      /* We are now at a null or a delimiter; loop can now continue. */
    }

    /* Count up the strings */
    p = (unsigned char *) rawbuf;
    while(*p) {
      /* Skip any leading space */
      while(*p == ' ') { p++; }

      /* If there is an arg, count it */
      if (*p) {
        n++;
        /* Skip the nonspace */
        while(*p && (*p != ' ')) { p++; }
      }

      /* We are now at a null or a blank space; loop can now continue. */
    }

    /* Now n is the number of args, and we can allocate the argv */
    av = (char **) malloc(n * sizeof(char *));
    if (av == 0) {
      fprintf(stderr, "%s: Cannot alloate argv block.\n", g_argv0);
      print_end(-1);
    }
    if (nargs) { *nargs = n; }
    if (argv) { *argv = av; }

    /* Set pointers, marking with nulls as we go */
    p = (unsigned char *) rawbuf;

    while(*p) {
      /* Skip any leading space and turn it into nulls */
      while(*p == ' ') { *p = 0; p++; }

      /* If there is an arg, count it */
      if (*p) {
        /* Save a pointer to this string, with (paranoid) check to avoid
           overwriting the argv */
        if (n > 0) {
          *av = (char *) p;
          av++;
          n--;
        }
        /* Skip the nonspace */
        while(*p && (*p != ' ')) { p++; }
      }

      /* We are now at a null or a blank space; loop can now continue. */
    }
  }
} /* End of delimit_args */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  Error codes and strings
-----------------------------------------------------------------------------*/
#define ERR_EXEC_DIV_ZERO -1
#define ERR_EXEC_ROOT_NEG -2
#define ERR_EXEC_LOG_NEG -3
#define ERR_EXEC_OVERFLOW -4
#define ERR_EXEC_TRIG_RANGE -5
#define ERR_EXEC_TRIG_LOW_DX -6
#define ERR_EXEC_SIG_LOSS -7
#define ERR_EXEC_POW_NEG_BASE -8
#define ERR_EXEC_LOG_BAD_BASE -9
#define ERR_EXEC_ILLEGAL_SYMBOL -10
#define ERR_NEWTON_ZERO_DX -11
#define ERR_NEWTON_NO_CONVERGE -12
#define ERR_EVAL_TOO_LONG -13
#define ERR_EVAL_METASTACK_OVERFLOW -14
#define ERR_EVAL_UNKNOWN_SEFT -15
#define ERR_EVAL_STACK_OVERFLOW -16
#define ERR_EVAL_STACK_UNDERFLOW -17
#define ERR_ES_NULL_EXPRESSION -18
#define ERR_EC_INCOMPLETE_EXPR -19
#define ERR_EXEC_ILLEGAL_EXPONENT -20
#define ERR_EXEC_TRIG_ARGTYPE -21
#define ERR_EXEC_ILLEGAL_DERIV -22
#define ERR_EXEC_ZERO_DERIV -23
/* Add any new ones here */
#define EXIT_NO_ERROR -9998
#define ERR_UNKNOWN -9999

typedef struct err_str {
  s16 val;
  const char * str;
} err_str;

err_str error_strings[] = {
  {0, "No Error"},
  {ERR_EXEC_DIV_ZERO, "Divide by zero"},
  {ERR_EXEC_ROOT_NEG, "Root of a negative value"},
  {ERR_EXEC_LOG_NEG, "Logarithm of a negative value"},
  {ERR_EXEC_OVERFLOW, "Overflow"},
  {ERR_EXEC_TRIG_RANGE, "Trigonometric argument out of range"},
  {ERR_EXEC_TRIG_LOW_DX, "Trigonometric argument generates near-constant"},
  {ERR_EXEC_SIG_LOSS, "Loss of significance"},
  {ERR_EXEC_POW_NEG_BASE, "Power of a negative base"},
  {ERR_EXEC_LOG_BAD_BASE, "Logarithm to base 1 or negative base"},
  {ERR_EXEC_ILLEGAL_SYMBOL, "Illegal symbol"},
  {ERR_NEWTON_ZERO_DX, "Zero derivative in Newton iteration"},
  {ERR_NEWTON_NO_CONVERGE, "Newton iteration did not converge"},
  {ERR_EVAL_TOO_LONG, "Expression is too long"},
  {ERR_EVAL_METASTACK_OVERFLOW, "Metastack overflow"},
  {ERR_EVAL_UNKNOWN_SEFT, "Symbol of unknown seft"},
  {ERR_EVAL_STACK_OVERFLOW, "Stack overflow"},
  {ERR_EVAL_STACK_UNDERFLOW, "Stack underflow"},
  {ERR_ES_NULL_EXPRESSION, "Null expression"},
  {ERR_EC_INCOMPLETE_EXPR, "Incomplete expression"},
  {ERR_EXEC_ILLEGAL_EXPONENT, "Disallowed exponent"},
  {ERR_EXEC_TRIG_ARGTYPE, "Disallowed trigonometric argument"},
  {ERR_EXEC_ILLEGAL_DERIV, "Overflow or NaN in derivative"},
  {ERR_EXEC_ZERO_DERIV, "Underflow in derivative"},
  /* Add any new ones here */
  {EXIT_NO_ERROR, "No error"},
  {ERR_UNKNOWN, "Unknown error"},
};

const size_t g_num_errors = sizeof(error_strings) / sizeof(err_str);

const char * err_string(s16 err)
{
  unsigned int j;

  for(j=0; j<g_num_errors; j++) {
    if (error_strings[j].val == err) {
      return (error_strings[j].str);
    }
  }
  return(error_strings[g_num_errors-1].str);
}

const char * tagname(int t)
{
  switch(t) {
    case TYPE_NONE: default: return "none";
    case TYPE_TRAN: return "tran";
    case TYPE_TCEL: return "elem";
    case TYPE_ALG:  return "alg";
    case TYPE_CONS: return "cons";
    case TYPE_RAT:  return "rat";
    case TYPE_INT:  return "int";
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  Time measurement, memory allocation, and "thrashing" detection
-----------------------------------------------------------------------------*/

time_flt tod_start; /* gettime() value at program start */

/* gettime is the only really OS-specific routine in this whole program.
   It returns how long the program has been running, measured in tenths
   of a second. For example, it returns 14 if the program has run for
   1.4 seconds. This is actual elapsed "clock-on-the-wall" time, not
   necessarily a measure of how much time the CPU has spent working on
   RIES. For example, if you close your laptop while RIES is running it
   will include the time the system was "asleep" in its final printout
   of time used. */
time_flt gettime(void)
{
  struct timeval tod_record;
  time_flt t_now;

  /* There is a block of code inside an #ifdef above (search for
     "---gettimeofday---") that sets up the appropriate include files
     and/or defines a gettimeofday() function based on compile-time flags
     like _WIN32. See the block-comment there for more details. */

  gettimeofday(&tod_record, 0);

  t_now = ((time_flt) (tod_record.tv_sec))
        + ((time_flt) (tod_record.tv_usec)) / 1.0e6;

  return(t_now - tod_start);
}

time_flt g_min_memory;
time_flt g_max_memory;
time_flt g_avg_alloc_rate, g_good_alloc_rate;
time_flt memstat_when;
long memstat_where, rate_where;
time_flt my_alloc_when, g_ttl_elapsed;
long tc_alloced;
long g_rate_increase_run, g_max_rir;
time_flt g_decay_slug;
unsigned long g_last_thrash;

void inittime(void)
{
  time_flt now;

  /* Init the global so gettime() has a valid value to subtract from
     its new measurement */
  tod_start = 0;

  /* With tod_start set to 0 gettime will return the absolute current time */
  now = gettime();

  /* Set the global to this value. */
  tod_start = now;

  /* From now on, calls to gettime() will measure time from the moment we
     made the preceding gettime() call. */

  g_avg_alloc_rate = g_good_alloc_rate = 0.0;
  my_alloc_when = memstat_when = gettime();
  tc_alloced = 0; memstat_where = rate_where = 0;
  g_cv_calls = 0; g_exec_calls = 0;
  g_ttl_elapsed = 0; g_rate_increase_run = g_max_rir = 0;
  g_last_thrash = 0;
}

int bitcount(unsigned long x)
{
  x = ((x & 0xAAAAAAAAL) >> 1) + (x & 0x55555555L);
  x = ((x & 0xCCCCCCCCL) >> 2) + (x & 0x33333333L);
  x = ((x & 0xF0F0F0F0L) >> 4) + (x & 0x0F0F0F0FL);
  x = ((x & 0xFF00FF00L) >> 8) + (x & 0x00FF00FFL);
  x = (x >> 16) + (x & 0x0000FFFFL);
  return((int) x);
}

time_flt g_mem_bad_ratio = 2.0;

/* Try to detect if the system is "thrashing" (swapping memory pages in
and out from the hard drive) by measuring the elapsed time and comparing to
an estimate of how much time our computation should have taken. */
void thrash_check(long alloced)
{
  time_flt now, alloc_elapsed, predicted_rate;
  time_flt this_alloc_rate;
  time_flt decay_numer, decay_denom;
  time_flt sluggish_ratio;

  tc_alloced += alloced;
  if (tc_alloced < (1024L*1024L)) {
    /* It's not time to benchmark memory yet... */
    return;
  }

  /* Look at how much time has passed since the last time we were here */
  now = gettime();
  alloc_elapsed = now - my_alloc_when;
  /* Reset the timer for use next time */
  my_alloc_when = now;

  /* Convert to units of seconds per megabyte */
  this_alloc_rate = alloc_elapsed * 1.0e6 / ((time_flt) tc_alloced);
  g_ttl_elapsed += this_alloc_rate;

  /* Determine the timebase we should use for our decaying averages: Short
     halflife at first, then longer. */
  if (mem_used_KiB < 20480) {
    decay_numer = 0.9;
  } else {
    decay_numer = 0.993;
  }
  decay_denom = 1.0 - decay_numer;


  /* Decaying average with half-life of roughly 100 samples:
     0.5 ^ 0.01 = 0.993092... */
  if (g_avg_alloc_rate == 0) {
    g_avg_alloc_rate = this_alloc_rate;
  } else {
    g_avg_alloc_rate = (decay_numer * g_avg_alloc_rate)
                     + (decay_denom * this_alloc_rate);
  }
  if (this_alloc_rate < g_avg_alloc_rate) {
    /* This sample brought down the average, so it should be counted
       towards the "good performance level" statistic. */
    if (g_good_alloc_rate == 0) {
      g_good_alloc_rate = this_alloc_rate;
    } else {
      g_good_alloc_rate = (decay_numer * g_good_alloc_rate)
                        + (decay_denom * this_alloc_rate);
    }
  }

  rate_where = mem_used_KiB;

  /* Find out how many times exec() was called, which is a good measure of
     how much computation we've done and therefore how much time "should
     have" elapsed */
  predicted_rate = (((time_flt) g_exec_calls) + 1.0) / 4.0e6
                 + (((time_flt) g_cv_calls) + 1.0)   / 2.5e6;

  /* From the prediction and the actual elapsed time, make a measure of
     sluggishness. */
  sluggish_ratio = g_ttl_elapsed / predicted_rate;

  if (debug_M) {
    printf(
      "%4ld MiB, ex%8ld, cv%7ld: %6.3g/%6.3g = %6.3g",
      ((mem_used_KiB >> 9) + 1) >> 1,
      g_exec_calls, g_cv_calls,
      g_ttl_elapsed, predicted_rate, sluggish_ratio);
  }

  /* Reset the raw counters for the next sample */
  g_cv_calls = 0;
  g_exec_calls = 0;
  g_ttl_elapsed = 0;

  /* Count how many times in a row the new sample is bigger than the
     decaying average */
  if (sluggish_ratio > g_decay_slug * g_mem_bad_ratio) {
    if (debug_M) { printf(" >%6.3g", g_decay_slug); }
    g_rate_increase_run++;
  } else {
    if (debug_M) { printf("  %6s", ""); }
    g_rate_increase_run = 0;
  }
  /* g_max_rir keeps track of the longest "run" since the last time
     g_max_rir was reset */
  if (g_rate_increase_run > g_max_rir) {
    g_max_rir = g_rate_increase_run;
  }
  /* Keep a bitmap of the history of recent extreme incidents */
  if (sluggish_ratio > g_decay_slug * g_mem_bad_ratio) {
    g_last_thrash |= 1;
  }
  g_last_thrash <<= 1;
  g_last_thrash &= ((1 << 10) - 1);

  /* Decay-average the slug ratio */
  if (g_decay_slug == 0) {
    g_decay_slug = sluggish_ratio;
  } else {
    g_decay_slug = (decay_numer * g_decay_slug)
                 + (decay_denom * sluggish_ratio);
  }

  if (mem_used_KiB >= memstat_where + 10240) {
    /* It's time to look at the stats and make a guess */

    if (debug_M) {
      /* To evaluate these measurements, run RIES in different memory
      /  environments using a command like:
      /      ries 2.50631415926535897932 -l8 -DM                       */
      printf(" avg %6.3g", g_decay_slug);
      printf(", run %ld", g_max_rir); g_max_rir = 0;
      printf(" (%6.3g ; %6.3g sec/MB)", g_avg_alloc_rate, g_good_alloc_rate);

      if (((time_flt)mem_used_KiB) * 1024.0 > g_min_memory) {
        /* Here we would check for a "getting really slow" trend and quit
           if it is exceeded */
        if (bitcount(g_last_thrash) >= 3) {
          /* Okay, pull the plug. */
          printf("\nExiting now because memory has gotten very slow.\n");
          print_end(EXIT_NO_ERROR);
        }
      }
    }
    fflush(stdout);
    memstat_when = now;
    memstat_where = mem_used_KiB;
  }
  if (debug_M) { printf("\n"); }

  tc_alloced = 0;
} /* End of thrash.check */

void init_mem(void)
{
  freepool = 0;
  freesize = 0;
  block_base = 0;
  mem_used_KiB = 0;
  mem_used_bytes = 0;
}

/* All allocation is done in fixed-size blocks, and nothing is ever
 * deallocated, so it's very easy to manage. This is your typical
 * cached blocked allocate funtion */
void * my_alloc(size_t size)
{
  s16 do_alloc;
  char * rv;

  /* assume the worst */
  rv = 0;

  /* first, see if there's a block with enough space */
  do_alloc = 0;
  if (freepool) {
    if (freesize >= size) {
      /* we're okay */
    } else {
      do_alloc = 1;
    }
  } else {
    do_alloc = 1;
  }

  /* if we need to allocate, do so */
  if (do_alloc) {
    freesize = 0;
    freepool = (char *) malloc(ALLOC_SIZE);

    /* did we get any? */
    if (freepool) {
      freesize = ALLOC_SIZE;
      mem_used_KiB += (ALLOC_SIZE / 1024L);
    }

    thrash_check(ALLOC_SIZE);

    if (((time_flt)mem_used_KiB) * 1024.0 > g_max_memory) {
      printf("Stopping now because %g bytes of memory have been used.\n",
        ((time_flt)mem_used_KiB) * 1024.0);
      print_end(EXIT_NO_ERROR);
    }

    /* Immediately make our new block point to the previous block, if there
       was any. This is to support purgeall_mem(). */
    if (freepool) {
      *((char * *)freepool) = block_base; /* Store backlink ptr */
      block_base = freepool; /* This is the new base ptr */
      freepool += sizeof(char *); /* This is where the user's data will
                                     be allocated */
      freesize -= sizeof(char *); /* We used up some space */
      /* Carefully track memory usage */
      mem_used_bytes += sizeof(char *);
    }
  }

  /* if we have a block now, we can allocate from it */
  if (freepool) {
    rv = freepool;
    freepool += size;  /* %%% how do I find out the alignment
                          requirement for pointers to structs? */
    freesize -= size;
    /* Carefully track memory usage */
    mem_used_bytes += size;
  }

  return((void *) rv);
} /* End of my.alloc() */

/* I said above that "nothing is ever deallocated", but here you can
   deallocate everything all at once if you wish.
   %%% NOTE: As of 20160131, this is still not used; the plan is to facilitate
   running searches against multiple targets and avoid some of the duplicated
   recalculation. */
void purgeall_mem(void)
{
  char * prev_block;
  while(block_base) {
    prev_block = *((char * *) block_base);
    free((void *) block_base);
    block_base = prev_block;
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   Metastack routines (see the "M E T A S T A C K !" block comment above)

   The following illustrate the contents of the stack and undo
   list for some sample expressions. The expressions read from top
   to bottom, and there is one line for each push or pop. (There
   are no undo's illustrated here; an undo would be just moving up
   to a previous line). At the end of each example is the total
   number of steps: This is the number of items in the undo list.

   expression: 1+(2+(4+8))           expression: sqrt(sqrt((2^2)^2)^2)^2
   postfix: 1248+++                  postfix: 2ssqsqs
   op stack       --undo-stack--     op stack  --undo-stack--
   1  1                              2  2
   2  1 2                            s  -      2
   4  1 2 4                             4      2
   8  1 2 4 8                        s  -      2 4
   +  1 2 4       8                     16     2 4
      1 2         8 4                q  -      2 4 16
      1 2 12      8 4                   4      2 4 16
   +  1 2         8 4 12             s  -      2 4 16 4
      1           8 4 12 2              16     2 4 16 4
      1 14        8 4 12 2           q  -      2 4 16 4 16
   +  1           8 4 12 2 14           4      2 4 16 4 16
      -           8 4 12 2 14 1      s  -      2 4 16 4 16 4
      15          8 4 12 2 14 1         16     2 4 16 4 16 4
   total steps: 13                   total steps: 13
   max stack: 4                      max stack: 1
   max undo stack: 6                 max undo stack: 6

   expression: 1/(3-1/(-2))
   postfix: 11nr-r
   op stack       --undo-stack--
   3  3
   2  3 2
   n  3           2
      3 -2        2
   r  3           2 -2
      3 -0.5      2 -2
   -  3           2 -2 -0.5
      -           2 -2 -0.5 3
      3.5         2 -2 -0.5 3
   r  -           2 -2 -0.5 3 3.5
      0.286       2 -2 -0.5 3 3.5
   total steps: 11
   max stack: 2
   max undo stack: 5
-----------------------------------------------------------------------------*/

void ms_init(metastack *ms)
{
  ms->uvp = 0;
  ms->msp = 0;
  ms->sp = 0;
}

#define dbl(x) ((double)(x))

/* ms.push does a standard PUSH operation. */
void ms_push(metastack *ms, ries_val x, ries_dif dx, ries_tgs tags)
{
  s16 sp, msp;

  /* push x */
  sp = ms->sp;

  (ms->ds)[sp] = dx;
  (ms->s)[sp] = x;
  (ms->tg)[sp] = tags;

  /* remember */
  msp = ms->msp;
  ms->ms[msp] = MSO_PUSH;

  if (debug_m) {
    printf("push %d %g (%g)%x '%d'\n", sp, dbl(x), dx, tags, msp);
  }

  /* for a push, we don't need to add any undo values */

  sp++;
  ms->sp = sp;
  msp++;
  ms->msp = msp;
}

/* ms_pop does a standard POP operation. */
ries_val ms_pop(metastack *ms, ries_dif *diff, ries_tgs * tags)
{
  s16 sp, msp, uvp;
  ries_val rv;
  ries_dif drv;
  ries_tgs tg;

  /* pop a value */
  sp = ms->sp;
  sp--;
  rv = (ms->s)[sp];
  drv = (ms->ds)[sp];
  if (diff) {
    *diff = drv;
  }
  tg = (ms->tg)[sp];
  if (tags) {
    *tags = tg;
  }
  ms->sp = sp;

  /* remember the action and the data */
  msp = ms->msp;
  ms->ms[msp] = MSO_POP;

  /* save the popped values in the undo list */
  uvp = ms->uvp;
  if (debug_m) {
    printf("pop %d %g (%g)%x '%d' .%d.\n", sp, dbl(rv), drv, tg, msp, uvp);
  }
  ms->udv[uvp] = drv;
  ms->uv[uvp] = rv;
  ms->utg[uvp] = tg;

  msp++;
  ms->msp = msp;
  uvp++;
  ms->uvp = uvp;

  return rv;
}

/* ms_peek just lets you see what's on the top of the stack. */
ries_val ms_peek(metastack *ms, ries_dif *diff, ries_tgs * tag, s16 *sptr)
{
  s16 sp;
  ries_val rv;
  ries_dif drv;
  ries_tgs tg;

  sp = (s16)((ms->sp)-1);
  rv = ms->s[sp];
  drv = ms->ds[sp];
  if (diff) {
    *diff = drv;
  }
  tg = ms->tg[sp];
  if (tag) {
    *tag = tg;
  }
  if (sptr) {
    *sptr = sp;
  }
  if (debug_m) {
    printf("peek %d %g (%g)%x\n", sp, dbl(rv), drv, tg);
  }
  return (rv);
}

/* ms_undo performs a metastack UNDO operation -- it returns the stack to the
   state it was in before the most recent PUSH or POP. */
void ms_undo(metastack *ms)
{
  s16 msp, opcode;

  /* pop the opcode */
  msp = ms->msp;
  opcode = ms->ms[--msp];
  ms->msp = msp;

  if (opcode == MSO_PUSH) {
    /* to undo a PUSH is easy -- just pop the SP. */
    ms->sp--;
    if (debug_m) { printf("undo-push\n"); }
  } else {
    s16 uvp, sp;
    ries_val uv;
    ries_dif udv;
    ries_tgs tg;
    /* undo a POP. This is like a PUSH except we have to retrieve the value
       from the undo data. */
    uvp = ms->uvp;
    uvp--;
    uv = ms->uv[uvp];
    udv = ms->udv[uvp];
    tg = ms->utg[uvp];
    ms->uvp = uvp;

    sp = ms->sp;
    ms->ds[sp] = udv;
    ms->s[sp] = uv;
    ms->tg[sp] = tg;
    sp++;
    ms->sp = sp;
    if (debug_m) { printf("undo-pop\n"); }
  }
} /* End of ms.undo */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sources and notes related to special functions and their algorithms.
%%% Most of this will/should be moved into msal_math64.c, f107_o.cpp
and/or msal_math128.c

Sources:
   msal_math64.c already contains sin/cos/tan from Sun via Netlib,
   and my own sinl/cosl/tanl.

   I have the openlibm sources, which include a fairly readable 80-bit ldbl
   mathlib, at ~/devt/julia/openlibm on SB4

   .../libs/f107_o/f107_o.cpp already contains all the "normal" functions
   in high-precision: sqrt, sin, cos, exp, log, sinh, etc.

   Lambert W is already in msal_math64.c (derived from Veberic, "Having
   Fun with Lambert W(x) Function", 2009
   A much more thorough approach, with about 20 different polynomial fits
   and both branches, is in .../ries/LambertW-darko

   .../zeta/zeta.cpp contains the high-precision Gamma function (but no
   corresponding digamma, however the single-precision digamma will probably
   be good enough)

   I have the Boost C++ Libraries (from www.boost.org); the C source code
   is in "boost_1_53_0.tar.bz2". Most of the code of interest is in
   "boost/math/special_functions", e.g. their Lanczos Gamma is in files
   .../special_functions/gamma.hpp and .../special_functions/lanczos.hpp

   The GNU Scientific Library (GSL) has a lot of special functions that
   look reasonably easy to adapt. See notes in .../ries/0-notes.txt ; and
   source code in devt/gcc45/tars/gsl-1.15/specfunc and in gsl-1.16.tar.gz

   I also have the GNU version of the C libraries (which includes the more
   common functions like exp and tan) in glibc-2.20.tar.gz but a lot of
   the code is in assembler. See libm_sincosl.S for example. There is also
   some code that is in C, such as sysdeps/ieee754/ldbl-96/k_cosl.c and
   .../ldbl-96/t_sincosl.c for the "96-bit" long double version of cosine.
   And oddly there is "multi-precision" code, e.g. see sincos32.c and
   mpa.c in .../ieee754/dbl-64 which uses radix-2^24 maths and apparently
   a Taylor series (see .../ieee754/dbl-64/dosincos.c) and some double-double
   arithmetic in the style of Dekker 1971 (see .../ieee754/dbl-64/dla.h)
   all for the purpose of getting a correctly-rounded result (see
   .../ieee754/dbl-64/s_sin.c)

   Cody, William J., Jr., and Waite, William. Software Manual for the
   Elementary Functions. Prentice-Hall (Englewood Cliffs, New Jersey, 1980).

Constants that may also be useful (these could be provided to users as
a built-in profile, or at least listed on the website). I am giving
only 35 decimal digits, as that's all we'd need for f107 precision:

0.56714329040978387299996866221035554
  The "omega" constant W(1), with the property : omega e^omega = 1.
See OEIS[A30178].

0.57721566490153286060651209008240243
  Euler's constant "gamma", -Digamma(1), OEIS[A1620].

1.2020569031595942853997381615114499
  Apery's constant Zeta(3), OEIS[A2117]. e^gamma is also somewhat important.

2.5029078750958928222839028732182157
  Feigenbaum reduction parameter, OEIS[6891].

4.6692016091029906718532038204662016
  Feigenbaum bifurcation velocity, OEIS[6890].

-----------------------------------------------------------------------------*/

/* Native double and long double atan2 functions.

The man page for atan2 doesn't specify its behaviour when both arguments
are zero, so just to be safe I am defining it myself. */
double arctan2(double a, double b)
{
  if ((a == 0) && (b == 0)) { return 0; }
  return atan2(a,b);
}

long double arctan2l(long double a, long double b)
{
  if ((a == 0) && (b == 0)) { return 0; }
  return atan2l(a,b);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Notes on transcendental functions:

   sinh
   sinh(x) = (e^x - e^-x) / 2

   complex natural logarithm
   cln(z) = ln(cabs(z)) + phase(z) i

   complex exponential
   cexp(z) = exp(a) (cos b + i sin b)

   complex sine (and sinh)
   csin(z) = (cexp(i z) - cexp(-i z)) / 2i
   csin(i z) == i csinh(z)

   csin(z) = (cexp(i z) - cexp(-i z)) / 2i
           = (cexp(ai - b) - cexp(b - ai)) / 2i
           = (e^-b (cos a + i sin a) - e^b (cos a - i sin a)) / 2i
           = (e^-b cos a + i e^-b sin a - e^b cos a + i e^b sin a) / 2i
           = i 1/2 e^-b cos a - 1/2 e^-b sin a - i 1/2 e^b cos a - 1/2 e^b sin a
   a + bi  = i 1/2 e^-b cos a - 1/2 e^-b sin a - i 1/2 e^b cos a - 1/2 e^b sin a

   a       =  - 1/2 e^-b sin a - 1/2 e^b sin a
        bi = i 1/2 e^-b cos a - i 1/2 e^b cos a

   a = - 1/2 e^-b sin a - 1/2 e^b sin a = - sin a cosh b
   b =   1/2 e^-b cos a - 1/2 e^b cos a = - cos a sinh b

   b = - cos a (e^b - e^-b)/2
   b 2 / (e^-b - e^b) = - cos a
   -2 b / (e^-b - e^b) = cos a
   2 b / (e^b - e^-b) = cos a
   a = 2 pi K + cos' (2 b / (e^b - e^-b))

   a = - sin a (e^b + e^-b)/2
   -1 = sin(a)/a (e^b + e^-b)/2
   -2/(e^b + e^-b) = sin(a)/a
   - bell(b) = sin(a)/a
   b = +/- bell' (-sin(a)/a)   [defined only when sin(a)/a < 0]

   +/- bell'(-sin(a)/a) = - cos a sinh(+/- bell'(sin(a)/a))

   2 pi K + cos' (2 b / (e^b - e^-b)) = - cosh b sqrt(1 - (2 b / (e^b - e^-b))^2)



   More elaborate functions follow. These are the ones that do not
   come directly out of the closure of the simple operators + - * / ^ v
   (as is the case for log, exp, sin and cos and their related functions)
   but from new concepts such as the integral of a function, etc.
   This includes the standard distribution, the Gamma function,
   Bessel functions, etc.


Gamma function and generalized factorial: most notes and source
have now been moved to msal_math64.c


   binomial coefficients
   The binomial coefficient "n over k" is:

   bincoef(n,k) = n! / ((n-k)! k!)
     = exp(lngamma(n) - (lngamma(n-k) + lngamma(k)))


Riemann Zeta function: see the implementation in .../zeta/ken-takusagawa


Jacobi elliptic functions:
    Jacobi amplitude am(u,k)  inverse of the Elliptic Integral of the First Kind
                              (in MMa: JacobiAmplitude[u, k^2])
    sn(u, k) = sin(am(u, k))
    cn(u, k) = cos(am(u, k))
    dn(u, k) = = sqrt(1-(k sn(u,k))^2) = d/du am(u, k)

  Two versions are in function-sources.txt; see also:
    http://en.wikipedia.org/wiki/Theta_function
    http://code.google.com/p/elliptic/source/browse/trunk/ellipj.m
    http://sourceforge.net/projects/asymptote/forums/forum/409349/topic/4725092

 */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  exec() contains all the code that actually performs computations on
  subexpressions during the RIES search.

-----------------------------------------------------------------------------*/

/* exec actually executes an opcode, using a metastack. It returns a
   nonzero value if there was an error, e.g. divide-by-zero. It also
   sets undo_count to a number indicating the number of times you have to
   call ms_undo to put the stack back to the state it was in before calling
   exec on this opcode.

There is plenty of error checking for zero and negative arguments, but
no checking for overflow. The reason is that only a few expressions,
(the simplest is 445^^) are capable of overflowing -- so it is not
much of an optimization. There is no danger of the program generating
an exception from overflow.

If the symbol is 'x', the value of exec.x is placed on the stack.
Normally (as when exec() is called by ge.2 or canon.val) this is the
target value, but when called by eval() for newton() or the
--eval-expression command it might be set to another value.
 */
s16 exec(metastack *ms, symbol op, s16 *undo_count, s16 do_dx)
{
  ries_val a = 0; /* Argument 1 */
  ries_val b = 0; /* Argument 2 */
  ries_val lb;    /* Log of b */
  ries_val rv;    /* Return value */
  ries_dif da = 0;/* Derivatives */
  ries_dif db = 0;
  ries_dif drv;
  ries_tgs tga;   /* Tags of arg 1 */
  ries_tgs tgb;   /* Tags of arg 2 */
  ries_tgs trv;   /* Tags of result */
  int f1;         /* flag */

  /* set default for derivative (overridden if we compute it) */
  drv = (ries_dif)k_0;
  tga = tgb = trv = TYPE_NONE;

  switch(op) {
    /* seft '0' ( -- ) symbols. These do nothing. */
  case ' ' :
    rv = 0; *undo_count = 0; break;  /* ' ' is a no-op */

    /* Roll '(' or ')' operators might go here */

    /* seft 'a' ( -- K ) symbols. For all constants the derivative is zero;
       for X the derivative is 1.0 */
  case '1' :
    rv = k_1; ms_push(ms, rv, (ries_dif) k_0, TYPE_INT); *undo_count = 1; break;
  case 'f' :
    rv = k_phi; ms_push(ms, rv, (ries_dif) k_0, tg_phi); *undo_count = 1; break;
  case '2' :
    rv = k_2; ms_push(ms, rv, (ries_dif) k_0, TYPE_INT); *undo_count = 1; break;
  case 'e' :
    rv = k_e; ms_push(ms, rv, (ries_dif) k_0, tg_e); *undo_count = 1; break;
  case '3' :
    rv = k_3; ms_push(ms, rv, (ries_dif) k_0, TYPE_INT); *undo_count = 1; break;
  case 'p' :
    rv = k_pi; ms_push(ms, rv, (ries_dif) k_0, tg_pi); *undo_count = 1; break;
  case '4' :
    rv = k_4; ms_push(ms, rv, (ries_dif) k_0, TYPE_INT); *undo_count = 1; break;
  case '5' :
    rv = k_5; ms_push(ms, rv, (ries_dif) k_0, TYPE_INT); *undo_count = 1; break;
  case '6' :
    rv = k_6; ms_push(ms, rv, (ries_dif) k_0, TYPE_INT); *undo_count = 1; break;
  case '7' :
    rv = k_7; ms_push(ms, rv, (ries_dif) k_0, TYPE_INT); *undo_count = 1; break;
  case '8' :
    rv = k_8; ms_push(ms, rv, (ries_dif) k_0, TYPE_INT); *undo_count = 1; break;
  case '9' :
    rv = k_9; ms_push(ms, rv, (ries_dif) k_0, TYPE_INT); *undo_count = 1; break;

  case 'x' :
    rv = exec_x; drv = (ries_dif) k_1;
    ms_push(ms, rv, drv, g_targ_tags); *undo_count = 1; break;

    /* seft 'b' ( arg -- val ) symbols */
  case 'I' :        /* Identity: used only when all other seft-b symbols
                       have been excluded */
    a = ms_pop(ms, &da, &tga);
    rv = a;
    drv = da;
    trv = tga;
    ms_push(ms, rv, drv, trv); *undo_count = 2;
    break;

  case 'n' :        /* negate */
    /* n(a+bi) = -a + -bi */
    a = ms_pop(ms, &da, &tga);
    if (do_dx) {
      drv = - da;
    }
    rv = -a;
    trv = tga;
    ms_push(ms, rv, drv, trv); *undo_count = 2;
    break;

  case 'r' :         /* reciprocal */
    /* r(a+bi) = (a-bi) / (a^2 + b^2) */
    a = ms_pop(ms, &da, &tga); *undo_count = 1;
    if (a == k_0) {
      return ERR_EXEC_DIV_ZERO;
    }
    if (do_dx) {
      drv = (ries_dif) (( - da) / (a * a));
    }
    if (!((drv < k_d_inf) && (drv > k_d_ninf))) {
      return ERR_EXEC_ILLEGAL_DERIV;
    }
    rv = k_1 / a;
    /* Int become rational, everything else stays the same */
    trv = TGMIN(tga, TYPE_RAT); /* tgs-manip */
    ms_push(ms, rv, drv, trv); *undo_count = 2;
    break;

  case 's' :        /* squared */
    a = ms_pop(ms, &da, &tga);
    if (do_dx) {
      drv = (ries_dif) (k_2 * a * da);
    }
    rv = a * a;
    trv = tga;
    ms_push(ms, rv, drv, trv); *undo_count = 2;
    break;

  case 'q' :         /* square root */
    /* q(a+bi) is the conjugate with the same imaginary sign as bi.
       for example, q(i) = (1+i)/sqrt(2); q(-i) = (1-i)/sqrt(2) */
    a = ms_pop(ms, &da, &tga); *undo_count = 1;
    if (a < k_0) {
      return ERR_EXEC_ROOT_NEG;
    }
    rv = SQRT(a);
    if (do_dx) {
      drv = (ries_dif) (da / (k_2 * rv));
    }
    /* tgs-manip: Try to determine if it was a perfect square and tag result
       accordingly. This happens if target is 143 and we're computing [x1-q] */
    if (   (tga == TYPE_INT)
        && (rv < 1.0e6)
        && (rv == FLOOR(rv))) {
      trv = TYPE_INT;
    } else {
      trv = TGMIN(TYPE_CONS, tga);
    }
    ms_push(ms, rv, drv, trv); *undo_count = 2;
    break;

  case 'l' :         /* ln */
    a = ms_pop(ms, &da, &tga); *undo_count = 1;
    if (g_restrict_exponents) {
      if ((da != 0) /* a is a subexpression containing x */
       || (tga < g_restrict_exponents)) {
        return ERR_EXEC_ILLEGAL_EXPONENT;
      }
    }
    if (a <= k_0) {
      return ERR_EXEC_LOG_NEG;
    }
    if (FABS(a - 1.0) <= k_sig_loss) {
      /* Loss-of-significance error. For example ln(1.00023)=2.3e-4.
         The input 1.00023 has 6 significant figures but the value
         2.3e-4 only has 2. */
      return ERR_EXEC_SIG_LOSS;
    }
    if (do_dx) {
      drv = (ries_dif) (da / a);
    }
    if (!((drv < k_d_inf) && (drv > k_d_ninf))) {
      return ERR_EXEC_ILLEGAL_DERIV;
    }
    rv = LOG(a);
    trv = TGMIN(tga, TYPE_TCEL); /* tgs-manip */
    ms_push(ms, rv, drv, trv); *undo_count = 2;
    break;

  case 'E' :         /* e ^ X */
    /* E(a+bi) = e^x (cos b + sin b i) */
    a = ms_pop(ms, &da, &tga); *undo_count = 1;
    if (g_restrict_exponents) {
      if ((da != 0) /* a is a subexpression containing x */
       || (tga < g_restrict_exponents)) {
        return ERR_EXEC_ILLEGAL_EXPONENT;
      }
    }
    if (a > k_eXlim) {
      return ERR_EXEC_OVERFLOW;
    }
    if (a < k_sig_loss) {
      /* Loss-of-significance error, e.g. "e^0.0001" */
      return ERR_EXEC_SIG_LOSS;
    }
    rv = EXP(a);
    if (do_dx) {
      drv = (ries_dif) (rv * da);
    }
    trv = TGMIN(tga, TYPE_TCEL); /* tgs-manip */
    ms_push(ms, rv, drv, trv); *undo_count = 2;
    break;

  case 'S' :        /* sine */
    /* d/dx sin(u) = cos(u) */
    a = ms_pop(ms, &da, &tga); *undo_count = 1;
    if (g_restrict_trig_args) {
      if ((da != 0) /* a is a subexpression containing x */
       || (tga < g_restrict_trig_args)) {
        return ERR_EXEC_TRIG_ARGTYPE;
      }
    }
    a *= k_sincos_arg_scale;
    /* This is to eliminate nonsense "solutions" like "sin(X^9) = 1/4"
       Note that if we're comparing to pi, the "equal" case doesn't matter
       because sin(pi)=0 and cos(pi)=-1, etc. so we can generate the error
       when a==pi without losing ny useful solutions. However if the max_arg
       option is not 1, then the foregoing no longer applies. */
    if ((a > (k_pi * k_sincos_max_arg))
     || (-a > (k_pi * k_sincos_max_arg)))
    {
      return ERR_EXEC_TRIG_RANGE;
    }
    rv = SIN(a);
    if (FABS(rv) > k_sin_clip) {
      /* This is to eliminate stuff like "sin(pi/2 + 0.00001) = 1"
         %%% This is partly redundant with testing dx < k_vanished.dx.
         If x=pi/2+0.00001 then the derivative will be really small and
         it's a meaningless solution. On the other hand, sin(pi/2 + 0.00001)
         itself is sufficiently precise (there is no loss of significant
         figures) and might be of interest. */
      return ERR_EXEC_TRIG_LOW_DX;
    }
    if (FABS(COS(a)) < k_sig_loss) {
      /* Significance is lost when the value of sin is near +- 1 */
      return ERR_EXEC_SIG_LOSS;
    }
    if (FABS(rv) < (FABS(a) * k_sig_loss)) {
      /* Loss-of-significance error, e.g. "sin(pi+0.0001)" */
      return ERR_EXEC_SIG_LOSS;
    }
    if (do_dx) {
      drv = (ries_dif) (COS(a) * k_sincos_arg_scale * da);
      if (FABS(drv) < (FABS(da) * k_sig_loss)) {
        return ERR_EXEC_SIG_LOSS;
      }
    }
    /* tgs-manip: if k_sincos_arg_scale is pi, rational arg -> algebraic */
    if (g_trig_scale_default) {
      trv = (tga >= TYPE_RAT) ? TYPE_ALG : TYPE_NONE;
    } else {
      trv = TYPE_NONE;
    }
    ms_push(ms, rv, drv, trv); *undo_count = 2;
    break;

  case 'C' :        /* cosine */
    /* d/dx cos(u) = - sin(u) */
    a = ms_pop(ms, &da, &tga); *undo_count = 1;
    if (g_restrict_trig_args) {
      if ((da != 0) /* a is a subexpression containing x */
       || (tga < g_restrict_trig_args)) {
        return ERR_EXEC_TRIG_ARGTYPE;
      }
    }
    a *= k_sincos_arg_scale;
    /* Same comment as above (in SIN function section) */
    if ((a > (k_pi * k_sincos_max_arg))
     || (-a > (k_pi * k_sincos_max_arg)))
    {
      return ERR_EXEC_TRIG_RANGE;
    }
    rv = COS(a);
    if (FABS(rv) < (FABS(a) * k_sig_loss)) {
      /* Loss-of-significance error, e.g. "cos(pi/2.0001)" */
      return ERR_EXEC_SIG_LOSS;
    }
    if (FABS(SIN(a)) < k_sig_loss) {
      /* Significance is lost when the value of cos is near +- 1 */
      return ERR_EXEC_SIG_LOSS;
    }
    if (FABS(rv) > k_sin_clip) {
      /* This is to eliminate stuff like "cos(0.00001) = 1" */
      return ERR_EXEC_TRIG_LOW_DX;
    }
    if (do_dx) {
      drv = (ries_dif) (k_0 - (SIN(a) * k_sincos_arg_scale * da));
    }
    /* tgs-manip: if k_sincos_arg_scale is pi, rational arg -> algebraic */
    if (g_trig_scale_default) {
      trv = (tga >= TYPE_RAT) ? TYPE_ALG : TYPE_NONE;
    } else {
      trv = TYPE_NONE;
    }
    ms_push(ms, rv, drv, trv); *undo_count = 2;
    break;

  case 'T' :        /* tangent */
    /* d/dx tan(u) = (1 + tan^2(u)) du */
    a = ms_pop(ms, &da, &tga); *undo_count = 1;
    if (g_restrict_trig_args) {
      if ((da != 0) /* a is a subexpression containing x */
       || (tga < g_restrict_trig_args)) {
        return ERR_EXEC_TRIG_ARGTYPE;
      }
    }
    a *= k_sincos_arg_scale;
    /* Same comment as above (in SIN function section) */
    if ((a > (k_pi * k_sincos_max_arg))
     || (-a > (k_pi * k_sincos_max_arg)))
    {
      return ERR_EXEC_TRIG_RANGE;
    }
    rv = TAN(a);
    if (FABS(rv) > (1 / k_sig_loss)) {
      /* Loss-of-significance error, e.g. "tan(pi/2.0001)" */
      return ERR_EXEC_SIG_LOSS;
    }
    if ((a!=0) && (FABS(rv/a) < k_sig_loss)) {
      /* Significance is lost when the value of tangent is near zero
         (except when the argument is also near zero) */
      return ERR_EXEC_SIG_LOSS;
    }
    if (do_dx) {
      drv = (ries_dif) (k_sincos_arg_scale * (1.0 + rv*rv) * da);
    }
    /* tgs-manip: if k_sincos_arg_scale is pi, rational arg -> algebraic */
    if (g_trig_scale_default) {
      trv = (tga >= TYPE_RAT) ? TYPE_ALG : TYPE_NONE;
    } else {
      trv = TYPE_NONE;
    }
    ms_push(ms, rv, drv, trv); *undo_count = 2;
    break;

  /* 'G' Gamma function would go here */

  case 'W':  /* Lambert W function */
    a = ms_pop(ms, &da, &tga); *undo_count = 1;
    if (a < k_ern) {
      /* If x is less than -1/e, W(x) has complex values. This is kind of like
         taking the logarithm of a negative number, so we'll use that error. */
      return ERR_EXEC_LOG_NEG;
    }
    rv = LAMBERTW(a);
    if (do_dx) {
      if (rv <= -1.0) {
        /* At W(x) = -1 the derivative is undefined (infinite) and if we get
           W(x) < -1 there was roundoff error in evaluating W(x) */
        return ERR_EXEC_ILLEGAL_DERIV;
      } else if (FABS(a) < 1.0e-30) {
        /* The derivative calculation involves "W(x)/x" which approaches 1 as x
           approaches 0 and involves loss of precision, but that's not a
           problem. However when x gets very small we might have underflow,
           and when it is exactly 0 we need to avoid the division. We do this
           by removing "rv" from the numerator and "a" from the denominator. */
        drv = (ries_dif) (da / (rv + 1.0));
      } else {
        drv = (ries_dif) (da * rv / (a * (rv + 1.0)));
      }
    }
    if (!((drv < k_d_inf) && (drv > k_d_ninf))) {
      return ERR_EXEC_ILLEGAL_DERIV;
    }
    /* Values of W(x) are considered "transcendental" */
    trv = TGMIN(tga, TYPE_TRAN); /* tgs-manip */
    ms_push(ms, rv, drv, trv); *undo_count = 2;
    break;

    /* seft 'c' ( arg1 arg2 -- val ) symbols */
  case '-' :
    b = ms_pop(ms, &db, &tgb);
    a = ms_pop(ms, &da, &tga); *undo_count = 2;
    /* this operator shares the loss-of-significance tests with '+' */
    b = - b; db = - db;
    goto add_common;

  case '+' :
    b = ms_pop(ms, &db, &tgb);
    a = ms_pop(ms, &da, &tga); *undo_count = 2;
  add_common: ;
    rv = a + b;
    if ((FABS(a) < (FABS(b) * k_sig_loss))
        || (FABS(b) < (FABS(a) * k_sig_loss)) ) {
      /* loss-of-significance error, e.g. "1 + e^(5^2)"
         In addition to the risk of meaningless tautologies like
         "x^2 = 2" followed by "x^2+e^(e^pi) = e^(e^pi)+2",
         this test is also useful for pruning: the expression
         "1 + e^(5^2)" is useless to us because there is no way to
         distinguish it from the simpler expression "e^(5^2)" */
      /* 20120505: If (a+b)-b == a, then no information has been lost,
         so we allow the operation to be performed. This is very important
         for -i (integer subexpressions) when x>100. */
      if (((rv - b) != a) || ((rv - a) != b)) {
        return ERR_EXEC_SIG_LOSS;
      }
    }
    if ((FABS(rv) < (FABS(a) * k_sig_loss))
        || (FABS(rv) < (FABS(b) * k_sig_loss)) ) {
      /* another loss-of-significance error, e.g. "1e40 - (1e40+1)" */
      /* 20120505: Similarly, when a sum is of lesser magnitude
         than both of the addends, it is sometimes an exact result. This
         is particularly common if the smaller addend was an integer,
         as in 143 + (-1). */
      if (((rv - b) != a) || ((rv - a) != b)) {
        return ERR_EXEC_SIG_LOSS;
      }
    }
    if (do_dx) {
      drv = da + db;
    }
    trv = TGMIN(tga, tgb); /* tgs-manip: Only rational if both terms were rational;
                        likewise for integer */
    ms_push(ms, rv, drv, trv); *undo_count = 3;
    break;

  case '*' :
    /* d/dx u v = v du + u dv */
    b = ms_pop(ms, &db, &tgb);
    a = ms_pop(ms, &da, &tga);
    if (do_dx) {
      drv = (ries_dif) ((b * da) + (a * db));
    }
    rv = a * b;
    trv = TGMIN(tga, tgb); /* tgs-manip: INT*INT->INT; INT*RAT -> RAT; RAT*RAT->RAT;
                            (anything)*NONE->NONE */
    ms_push(ms, rv, drv, trv); *undo_count = 3;
    break;

  case '/' :
    /* d/dx u/v = (v du - u dv) / v^2 */
    b = ms_pop(ms, &db, &tgb); *undo_count = 1;
    if (b == k_0) {
      return ERR_EXEC_DIV_ZERO;
    }
    a = ms_pop(ms, &da, &tga); *undo_count = 2;
    if (do_dx) {
      drv = (ries_dif) (((b * da) - (a * db)) / (b * b));
    }
    if (!((drv < k_d_inf) && (drv > k_d_ninf))) {
      return ERR_EXEC_ILLEGAL_DERIV;
    }
    rv = a / b;
    if ((tga >= TYPE_RAT) && (tgb >= TYPE_RAT)) {
      /* tgs-manip: We have INT/INT, RAT/INT, or RAT/RAT any of which may cause
         cancellation and result in an integer */
      trv = (rv == FLOOR(rv)) ? TYPE_INT : TYPE_RAT;
    } else {
      trv = TGMIN(tga, tgb);
    }
    /*printf("a %f tg %x  b %f tg %x  ans %f tg %x\n",a,tga,b,tgb,rv,trv);*/
    ms_push(ms, rv, drv, trv); *undo_count = 3;
    break;

  case '^' :        /* a to the power of b */
    /* d/dx u^v = v u^(v-1) du + ln(u) u^v dv */
    b = ms_pop(ms, &db, &tgb); *undo_count = 1;
    if (g_restrict_exponents) {
      if ((db != 0) /* b is a subexpression containing x */
       || (tgb < g_restrict_exponents)) {
        return ERR_EXEC_ILLEGAL_EXPONENT;
      }
    }
    a = ms_pop(ms, &da, &tga); *undo_count = 2;
    if (a <= k_0) {
      /* This would give a complex answer except when b is an integer,
         but in that case there will always be another expression that
         gives the same value without needing to raise a negative number
         to a power. */
      return ERR_EXEC_POW_NEG_BASE;
    }
    if (FABS(b) < k_sig_loss) {
      /* loss-of-significance error, e.g. "2^0.001" */
      return ERR_EXEC_SIG_LOSS;
    }
    rv = POW(a, b);
    if (do_dx) {
      drv = (ries_dif) (rv * ( (b * da / a) + (LOG(a) * db) ));
      if (!((drv < k_d_inf) && (drv > k_d_ninf))) {
        return ERR_EXEC_ILLEGAL_DERIV;
      }
      if ((da || db) && (drv == k_0)) {
        return ERR_EXEC_ZERO_DERIV;
      }
    }
    if (tgb == TYPE_INT) { /* tgs-manip */
      if (b >= 0) {
        trv = tga;                   /* INT^+INT->INT; RAT^+INT->RAT */
      } else {
        trv = TGMIN(tga, TYPE_RAT);  /* INT^-INT->RAT; RAT^-INT->RAT */
      }
    } else if (tgb == TYPE_RAT) {
      /* tgs-manip: if b is 1/2 or 1/4 or 1/8, treat as sqrt */
      lb=FABS(b);
      if ((lb == 0.5) || (lb == 0.25) || (lb == 0.125)) {
        trv = TGMIN(tga, TYPE_CONS); /* We have sqrt(a) or 1/sqrt(a), etc. */
      } else {
        trv = TGMIN(tga, TYPE_ALG); /* Some other rational power/root */
      }
    } else if ((tgb == TYPE_CONS) || (tgb == TYPE_ALG)) {
      trv = TGMIN(tga, TYPE_TCEL);
    } else {
      /* Exponent isn't even algebraic! */
      trv = TGMIN(tga, tgb);
    }
    ms_push(ms, rv, drv, trv); *undo_count = 3;
    break;

  case 'v' :       /* the bth root of a, that is, a^(1/b) */
    b = ms_pop(ms, &db, &tgb); *undo_count = 1;
    if (g_restrict_exponents) {
      if ((db != 0) /* b is a subexpression containing x */
       || (tgb < g_restrict_exponents)) {
        return ERR_EXEC_ILLEGAL_EXPONENT;
      }
    }
    if (b == k_0) {
      return ERR_EXEC_OVERFLOW;
    }
    if (FABS(b) > (1.0 / k_sig_loss)) {
      /* loss-of-significance error, e.g. "100000,/2" */
      return ERR_EXEC_SIG_LOSS;
    }
    a = ms_pop(ms, &da, &tga); *undo_count = 2;
    f1 = 0;
    if (b == k_3) {
      /* Cube root can handle any argument */
      if (a < k_0) {
        a = -a;
        f1 = 1;
      }
    } else if (a < k_0) {
      return ERR_EXEC_ROOT_NEG;
    }
    rv = POW(a, k_1 / b);
    if (do_dx) {
      drv = (ries_dif) (rv * ( (da / (a * b)) - (LOG(a) * db / (b*b)) ));
      if ((da || db) && (drv == k_0)) {
        return ERR_EXEC_ZERO_DERIV;
      }
    }
    if (f1) {
      rv = -rv;
    }
    if (!((drv < k_d_inf) && (drv > k_d_ninf))) {
      return ERR_EXEC_ILLEGAL_DERIV;
    }
    if (tgb >= TYPE_RAT) { /* tgs-manip */
      lb=FABS(b);
      if ((lb==2.0) || (lb==4.0) || (lb==8.0)) {
        trv = TGMIN(tga, TYPE_CONS); /* sqrt(a) or 1/sqrt(a) */
      } else {
        trv = TGMIN(tga, TYPE_ALG); /* Some other rational power/root */
        /* NOTE: If b is an integer reciprocal then this would be like
           raising to an integer power, but the search will have found
           that first anyway */
      }
    } else if ((tgb == TYPE_CONS) || (tgb == TYPE_ALG)) {
      trv = TGMIN(tga, TYPE_TCEL);
    } else {
      /* Radix isn't even algebraic! */
      trv = TGMIN(tga, tgb);
    }
    ms_push(ms, rv, drv, trv); *undo_count = 3;
    break;

  case 'L' :       /* [<a><b>L] is the log base b of a, that is, ln(a)/ln(b) */
    b = ms_pop(ms, &db, &tgb); *undo_count = 1;
    if (g_restrict_exponents) {
      if ((db != 0) /* b is a subexpression containing x */
       || (tgb < g_restrict_exponents)) {
        return ERR_EXEC_ILLEGAL_EXPONENT;
      }
    }
    if ((b <= k_0) || (b == k_1)) {
      return ERR_EXEC_LOG_BAD_BASE;
    }
    if (FABS(1.0 - b) < k_sig_loss) {
      /* loss-of-significance error, e.g. "log_0.999(2)" or "log_1.001(2)" */
      return ERR_EXEC_SIG_LOSS;
    }
    a = ms_pop(ms, &da, &tga); *undo_count = 2;
    if (g_restrict_exponents) {
      if ((da != 0) /* a is a subexpression containing x */
       || (tga < g_restrict_exponents)) {
        return ERR_EXEC_ILLEGAL_EXPONENT;
      }
    }
    if (a <= k_0) {
      return ERR_EXEC_LOG_NEG;
    }
    if (FABS(a - 1.0) <= k_sig_loss) {
      /* loss-of-significance error, e.g. "log_2(1.00001)" */
      return ERR_EXEC_SIG_LOSS;
    }
    lb = LOG(b);
    rv = LOG(a) / lb;
    if (do_dx) {
      drv = (ries_dif) ((da / (a * lb)) - (rv * db / (b * lb)));
      if (!((drv < k_d_inf) && (drv > k_d_ninf))) {
        return ERR_EXEC_ILLEGAL_DERIV;
      }
      if ((da || db) && (drv == k_0)) {
        return ERR_EXEC_ZERO_DERIV;
      }
    }
    trv = TGMIN(tga, TGMIN(tgb, TYPE_TCEL)); /* tgs-manip */
    ms_push(ms, rv, drv, trv); *undo_count = 3;
    break;

  case 'A' :       /* atan2 function (quadrant-correct arctangent) */
    b = ms_pop(ms, &db, 0);
    a = ms_pop(ms, &da, 0);
    rv = ARCTAN2(a,b);
    if (do_dx) {
      /* %%% I need to verify this. It also needs to be transformed into
         a form that won't lose accuracy when computed (consider b near 0)
         probably need to use two forms: one for when fabs(a) > fabs(b),
         and one for the other case.
            (b * da - a * db) / (k_1 + (a*a / b*b)) * b*b);  */
      drv = (((ries_dif)b)*da - ((ries_dif)a)*db) / ((ries_dif)(b*b + a*a));
    }
    trv = TYPE_NONE; /* tgs-manip */
    ms_push(ms, rv, drv, trv); *undo_count = 3;
    break;

  default:
    return ERR_EXEC_ILLEGAL_SYMBOL;
  }

  if (debug_r) {
    if (do_dx && sym_attrs[op].seft == 'c') {
      printf("exec %8.5g", dbl(a));
      printf(" (%8.5g)%x", da, tga);
      printf(" '%c'", op);
      printf(" %8.5g", dbl(b));
      printf(" (%8.5g)%x", db, tgb);
      printf(" -> %10.6g", dbl(rv));
      printf(" d/dx=%10.6g", drv);
    } else {
      if (sym_attrs[op].seft != 'a') {
        printf("exec %14.10g", dbl(a));
        if (do_dx) {
          printf(" (%14.10g)", da);
        }
        printf("%x", tga);
      }
      printf(" '%c'", op);
      if (sym_attrs[op].seft == 'c') {
        printf(" %14.10g", dbl(b));
        if (do_dx) {
          printf(" (%14.10g)", db);
        }
        printf("%x", tgb);
      }
      printf(" -> %14.10g", dbl(rv));
      if (do_dx) {
        printf(" d/dx=%14.10g", drv);
      }
      printf(" tag %x", trv);
    }
    printf("\n");
  }

  return 0;
} /* End of exec() */


/* F_ALLOC is the size used for fscratch, suitable for single-character infix
   format */
#define F_ALLOC (MAX_ELEN * 4)

/* infix.1 pulls one full subexpression off the end of an expression,
   converts it to infix, and writes the result into the supplied "char * term".
   It is a recursive function, calling itself to infix-ize subexpressions.
   The result is still in one-character-per-symbol format.
   It also modifies the input expression by moving its terminating 0 backwards.
   It writes the term's final operator into t_op, which is to aid the caller
   in determining whether precedence will require the use of parentheses.
   infix.1 does not put parentheses around the term, however it
   does parenthesize subterms within the term. */
s16 infix_1(
  symbol * expr,  /* The input */
  char * term,    /* The output */
  symbol * t_op   /* Output: final symbol from input ("top-level" symbol
                  /  of resulting infix) */
)
{
  s16    iptr, optr;  /* input, output pointers */
  symbol op;
  /* st_a and st_b are allocated twice as large in case we're processing
     an RHS after try.solve */
  char   st_a[F_ALLOC*2];  /* subterm A */
  symbol op_a;             /* and its operator */
  char   st_b[F_ALLOC*2];  /* subterm B */
  symbol op_b;
  s16 swap;
  char * term_a; /* for exchanging the two terms if we decide to do so */
  char * term_b; /* for exchanging the two terms if we decide to do so */
  symbol op_t;   /* for swapping op_a and op_b */
  char * s;      /* for copying subterms to output */
  s16 paren_a, paren_b; /* precedence flags for seft 'c' operators */
  s16 atan2_show_denom;

  optr = 0;
  /* go to the end */
  iptr = 0;
  while(expr[iptr]) {
    iptr++;
  }
  /* check for underrun */
  if (iptr == 0) {
    return 1;
  }
  /* get the operator */
  --iptr; op = expr[iptr]; expr[iptr] = 0;
  *t_op = op;
  /* check its type */
  switch(sym_attrs[op].seft) {
  case 'a':
    /* oooh, this is easy */
    term[optr++] = (char) op;
    /* we terminate the input string here. */
    expr[iptr] = 0;
    break;
  case 'b':
    /* we've got the op, get the term */
    if(infix_1(expr, st_a, &op_a)) {
      return 1;
    }
    swap = 0;
    if (op == 's') {
      swap = 1;
    }
    paren_a = 1;
    if ((op == 'n') && (sym_attrs[op_a].seft != 'c')) {
      paren_a = 0;
    }

    /* %%% what happens next should depend on op and op_a */
    if (swap == 0) {
      term[optr++] = (char) op;
    }
    if (paren_a) {
      term[optr++] = '(';
    }
    for(s = st_a; *s; s++) {
      term[optr++] = *s;
    }
    if (paren_a) {
      term[optr++] = ')';
    }
    if (swap) {
      term[optr++] = (char) op;
    }
    break;
  case 'c':
    /* get the terms */
    if(infix_1(expr, st_b, &op_b)) {
      return 1;
    }
    if(infix_1(expr, st_a, &op_a)) {
      return 1;
    }

    /* default is to keep them in order */
    term_a = st_a; term_b = st_b; swap = 0;

    /* optional swaps for the commutative operators
       if adding, and 1st arg is constant, put that argument 2nd */
    if ((op == '+') && (sym_attrs[op_a].seft == 'a') && (op_a != 'x')) {
      swap = 1;
    }
    /* rules for multiplying */
    if (op == '*') {
      int a_has_x, b_has_x;

      a_has_x = (strchr(term_a, 'x') != 0);
      b_has_x = (strchr(term_b, 'x') != 0);
      /* a bare seft-a symbol always precedes an expression
         %%% this swap should not be done if the baresymbol is 'x' and
         the expression has no 'x' */
      if ((! a_has_x) && (op_b == 'x')) {
        /* no swappy */
      } else if ((sym_attrs[op_a].seft != 'a') && (sym_attrs[op_b].seft == 'a')) {
        swap = 1;
      } else if ((op_a == 'x') && (! b_has_x)) {
        swap = 1;
      }
      /* with two bare symbols multiplied by each other, swap if the
         latter is an integer. Note: the integer test of op_b implicitly
         tests for the second term being seft-a. */
      if ((sym_attrs[op_a].seft == 'a') && (op_b >= '0') && (op_b <= '9')) {
        swap = 1;
      } else if ((op_a == 'x') &&
          ( (op_b == 'e') || (op_b == 'f') || (op_b == 'p')) ) {
        /* "x pi" -> "pi x" */
        swap = 1;
      }
    }
    /* Always swap for operators that are inherently swapped. */
    if (op == 'v') {
      swap = 1;
    } else if (op == 'L') {
      swap = 1;
    }
    /* Check for phantom symbols */
    if (op == PS_REVSUB) {
      swap = 1;
      op = '-';
      *t_op = '-';
    } else if (op == PS_REVDIV) {
      swap = 1;
      op = '/';
      *t_op = '/';
    } else if (op == PS_REVPOW) {
      swap = 1;
      op = '^';
      *t_op = '^';
    }
    /* %%% Identity involving arctan2: negate first arg and negate result
       (does not work if y=0 and x<=0, but atan2(0,x) should be excluded)
       %%% Must *not* negate both arguments: atan2(1,-1) != atan2(-1,1) */

    /* okay, do the swap */
    if (swap) {
      term_a = st_b; term_b = st_a;
      op_t = op_a; op_a = op_b; op_b = op_t;
    }

    /* determine precedence */
    paren_a = paren_b = 0; /* default don't use parens */

    atan2_show_denom = 1;
    if (op == 'L') {
      /* Latter part is always in parens; first part (base) is
         in parens only if it's an expression */
      paren_b = 1;
      if (sym_attrs[op_a].seft != 'a') {
        paren_a = 1;
      }
    } else if (op == 'A') {
      if (op_b == '1') {
        atan2_show_denom = 0;
        used_atan1 = 1;
      } else {
        used_atan2 = 1;
      }
    }
    if (sym_attrs[op_a].seft == 'c') {
      /* We have (s1 op_a s2) op s3 */
      paren_a = 1; /* default use parens */
      if (strchr("+*/^vA", op_a) && strchr("+-", op)) {
        /* anything followed by + or - doesn't need parens */
        paren_a = 0;
      } else if (strchr("*^vA", op_a) && strchr("*/A", op)) {
        /* * ^ or v followed by * or / doesn't need parens */
        paren_a = 0;
      }
    }
    if (sym_attrs[op_b].seft == 'c') {
      /* We have s1 op (s2 op_b s3) */
      paren_b = 1; /* default use parens */
      if ((op == '+') && strchr("+-*/^vA", op_b)) {
        /* + followed by anything doesn't need parens */
        paren_b = 0;
      } else if ((op == '-') && strchr("*/^vA", op_b)) {
        /* - followed by mult or higher operators doesn't need parens */
        paren_b = 0;
      } else if ((op == '*') && strchr("*/^vA", op_b)) {
        /* * followed by mult or higher operators doesn't need parens */
        paren_b = 0;
      } else if (strchr("/A", op) && strchr("^vA", op_b)) {
        /* / followed by ^ or v doesn't need parens */
        paren_b = 0;
      }
    }

    /* We're all set to generate output. */

    /* Emit leading operator for two-argument custom functions, 'A', and 'L' */
    if (strchr("LA", op)) {
      /* This operator goes in front of both arguments */
      term[optr++] = (char) op;
    }

    if (op == 'A') {
      term[optr++] = '(';
    }

    /* Emit first argument */
    if (paren_a) {
      term[optr++] = '(';
    }
    for(s = term_a; *s; s++) {
      term[optr++] = *s;
    }
    if (paren_a) {
      term[optr++] = ')';
    }

    /* Emit infix operator */
    if (op == '*') {
      if (g_explicit_multiply) {
        term[optr++] = (char) op; /* emit an actual "*" */
      } else if ((op_a <= '9') && paren_b) {
        /* digit times anything starting with parentheses */
        /* Emit no symbol at all */
      } else if ((op_a <= '9') && (term_b[0] <= '9')) {
        /* digit times anything starting with a bare digit */
        term[optr++] = (char) op; /* emit an actual "*" */
      } else if (op_b == 'n') {
        /* A times negative B, require explicit '*' */
        term[optr++] = (char) op;
      } else if ((sym_attrs[op_a].seft != 'a') || (sym_attrs[op_b].seft != 'a')
                        || (term_b[0] > '9')) {
        /* printf("op_a '%c' op_b '%c' mult '.'\n", op_a, op_b);  */
        term[optr++] = '.'; /* This becomes a blank space " " */
      } else {
        term[optr++] = (char) op;
      }
    } else if (op == 'A') {
      /* Add the comma between the arguments */
      if (atan2_show_denom) {
        term[optr++] = ',';
      }
    } else if (op == 'L') {
      /* We already emitted it */
    } else {
      term[optr++] = (char) op;
    }

    /* Emit second argument */
    if (atan2_show_denom) {
      if (paren_b) {
        term[optr++] = '(';
      }
      for(s = term_b; *s; s++) {
        term[optr++] = *s;
      }
      if (paren_b) {
        term[optr++] = ')';
      }
    }

    if (op == 'A') {
      term[optr++] = ')';
    }
    break;
  }

  /* terminate input and output strings */
  term[optr] = 0;

  return 0;
} /* End of infix.1 */

/* cv.phantoms takes a symstr and converts any phantom symbols to
their ASCII equivalents. symstrs with phantom symbols are fairly
short-lived, because they are created only by infix.preproc and then
immediately converted by infix.1 */
void cv_phantoms(symbol * s)
{
  if (s) {
    while(*s) {
      if (*s == PS_REVSUB) {
        *s = '_';
      } else if (*s == PS_REVDIV) {
        *s = '\\';
      } else if (*s == PS_REVPOW) {
        *s = '`';
      }
      s++;
    }
  }
}

/* infix.preproc copies "expr" to "out", and performs manipulations on
expressions that make the infix conversion easier.
  For example, the "squared" operator [s] gets turned into "^2" [2^].
That gives the infix conversion less special cases to check for when
it figures out where to put the parentheses. We can also do some
simplification here, for example [xn2+] which is "-x+2" in infix,
becomes [x2_] "2-x". */
void infix_preproc(symbol * expr, symbol * out)
{
  symbol * s;
  symbol * o;
  s = expr;
  o = out;

  if (debug_p) { printf("infix.preproc input [%s]\n", expr); }

  while(*s) {
    if (0) {

    } else if ((*s == '-') && (*(s+1) == 'n')) {
       /* [-n] is seen when --canon-reduction is used. Here we convert
       [-n] "-(a-b)" into [_] "b-a", where '_' represents the reversed
       subtract phantom op. This makes e.g. "1/-(x-pi)" into "pi-x". */
      if (debug_p) { printf("  [-n]->[_]\n"); }
      *o++ = PS_REVSUB;
      s ++;

    } else if ((*s == '/') && (*(s+1) == 'r')) {
       /* [/r] is also seen when --canon-reduction is enabled, e.g. [27/r]
       in the output of "ries 2.4284920346331 --canon-reduction nr25".
       Here we convert [/r] "1/(a/b)" into [\] "b/a", where '\' is
       reversed division. */
      if (debug_p) { printf("  [/r]->[\\]\n"); }
      *o++ = PS_REVDIV;
      s ++;

    } else if (*s == ' ') {
      /* ' ' is a no-op */
    } else if ((*s == 'n') && (sym_attrs[*(s+1)].seft == 'a') && (*(s+2) == '+')) {
      /* converting [n2+] into [2_]. Test case: 1.3204 gives
         "-(x)+2 = e/4" without this conversion and "2-x = e/4" with. */
      if (debug_p) { printf("  [nA+]->[A_]\n"); }
      *o++ = *(s+1);
      *o++ = PS_REVSUB;
      s += 2;
    } else if (*s == 's') {
      /* [s] -> [2^] */
      if (debug_p) { printf("  [s]->[2^]\n"); }
      *o++ = '2';
      *o++ = '^';
    } else if (*s == 'r') {
      /* [r] -> [1\] */
      if (debug_p) { printf("  [r]->[1\\]\n"); }
      *o++ = '1';
      *o++ = PS_REVDIV;
    } else if (*s == 'E') {
      /* [E] -> [e`] where ` is reverse power */
      if (debug_p) { printf("  [E]->[e`]\n"); }
      *o++ = 'e';
      *o++ = PS_REVPOW;
    } else {
      *o++ = *s;
    }
    s++;
  }
  *o++ = 0;
  if (debug_p) {
    symbol prtmp[EXPR_ALLOC];
    symstrncpy0(prtmp, out, EXPR_ALLOC);
    cv_phantoms(prtmp);
    printf("  result: [%s]\n", prtmp);
  }
}

/* symstrsym is like strchr for symbol strings. */
symbol * symstrsym(symbol * exp1, symbol sym)
{
  while((*exp1) && (*exp1 != sym)) {
    exp1++;
  }
  if (*exp1) {
    return exp1;
  }
  return 0;
}

/* symstrlen is just like strlen, for symbol strings. */
unsigned int symstrlen(symbol * s)
{
  unsigned int l;
  if (s == 0) {
    return 0;
  }
  for(l=0; s[l]; l++) { }
  return l;
}

/* symstrtrail detects a literal trailing pattern: it examines "big"
to see if it ends with the same characters as "little". In most odd
cases (e.g. if either string is null or 'little' is longer than 'big')
it returns 0. */
int symstrtrail(symbol * big, symbol * little)
{
  unsigned int i, l1, l2;
  symbol * s;
  l1 = symstrlen(big);
  l2 = symstrlen(little);
  if ((l1 == 0) || (l2 == 0)) {
    return 0;
  }
  if (l1 < l2) {
    return 0;
  }
  s = big + (((int)l1) - ((int)l2));
  for(i=0; i<l2; i++) {
    if (s[i] != little[i]) {
      return 0;
    }
  }
  return 1;
}

int bothtrail(symbol * a, symbol * b, symbol *tr)
{
  int rv;
  rv = symstrtrail(a, tr) && symstrtrail(b, tr);
  return rv;
}

/* symstrclip removes len symbols from the end of a symbol string (but does
nothing if the symbol string doesn't have that many symbols to start with) */
void symstrclip(symbol * s, unsigned int len)
{
  unsigned int l;
  l = symstrlen(s);
  if (l >= len) {
    l -= len;
    s[l] = 0;
  }
}

/* symstrcmp is like strcmp for symbol strings. */
int symstrcmp(symbol * a, symbol * b)
{
  while((*a) && (*b) && (*a == *b)) {
    a++;
    b++;
  }
  if ((*a == 0) && (*b == 0)) {
    return 0;
  }
  if (*a == 0) {
    /* a ended, so b is lexicographically later */
    return -1;
  }
  if (*b == 0) {
    /* b ended, so a is later */
    return 1;
  }
  if ((*a) > (*b)) {
    return 1;
  }
  return -1;
}

/* symstrneq compares the first n symbols in two strings; it returns 1 only
   of that many symbols exist in both strings and all are equal. */
int symstrneq(symbol * a, symbol * b, unsigned int n)
{
  unsigned int i;
  i = 0;
  while((*a) && (*b) && (*a == *b) && (i<n)) {
    a++;
    b++;
    i++;
  }
  if (i== n) {
    return 1;
  }
  return 0;
}

/* sym.strsymstr is like strstr for symbol strings. */
symbol * symstrsymstr(symbol * haystack, symbol * needle)
{
  symbol * hs = haystack;
  symbol c1;
  unsigned int l, n;

  n = 0;
  if ((hs == 0) || (needle == 0)) {
    return 0;
  }
  l = symstrlen(needle);
  /* The null string occurs at the beginning of every string */
  if (l == 0) {
    /* printf("symstrsymstr: r1\n"); */
    return haystack;
  }
  c1 = *needle;
  /* General case */
  while(hs && (*hs)) {
    hs = symstrsym(hs, c1);
    n++;
    if (hs == 0) {
      /* printf("symstrsymstr: r0a\n"); */
      return 0;
    }
    if (symstrneq(hs, needle, l)) {
      /* printf("symstrsymstr: r2\n"); */
      return hs;
    }
    hs++;
  }
  /* printf("symstrsymstr: r0b '%s' n=%d '%s'\n",
      (char*)needle,n,(char*)haystack); */
  return 0;
}

/* symstrncpy0 is kind of like strncpy for symbol strings, except
   that it ensures the last symbol copied will be 0. */
int symstrncpy0(symbol *to, symbol *from, int len)
{
  int l = 0;
  if ((to == 0) || (len <=0)) {
    return 0;
  }
  if (from == 0) {
    *to = 0;
    return 0;
  }
  while ((from[l]) && (l+1 < len)) {
    to[l] = from[l];
    l++;
  }
  to[l] = 0; l++;
  return(l);
}

/* sym.strncat is kind of like strncat for symbol strings */
void symstrncat(symbol *to, symbol *from, long len)
{
  int l = 0;

  if ((to == 0) || (len <=0)) {
    return;
  }
  /* Now get length of existing dest string */
  while(to[l]) {
    l++;
  }
  if (from == 0) {
    return;
  }
  while ((*from) && (l+1 < len)) {
    to[l++] = *from++;
  }
  to[l++] = 0;

  return;
}

void str_remap(char *s, char from, char to)
{
  while(s && (*s)) {
    if (*s == from) {
      *s = to;
    }
    s++;
  }
}

/* infix_expand expands the output of infix.1 into something more
   human-readable. It returns the length of the output. */
s16 infix_expand(char * input, char * output)
{
  s16 i, l;
  size_t j, l2;
  symbol c;

  l = (s16) strlen(input);
  j = 0;
  for(i=0; i<l; i++) {
    c = (symbol) input[i];
    /* printf("sym '%c', name '%s'\n", c, symbol_names[c]); */
    l2 = strlen(sym_attrs[c].sa_name);
    ries_strncpy(output+j, (char *) (sym_attrs[c].sa_name), (int)l2);
    j = j + l2;
  }
  output[j] = 0;
  return((s16) j);
}

/* postfix converts an expression (in its native postfix format, i.e. a
/  null-terminated array of symbol) to a character string in the old RIES
/  "compact postfix format". This is currently just one character per symbol,
/  so it does little more than strcpy would. However, if the symbolspace is
/  expanded to more than 256, or when custom-definable symbols are possible,
/  the simple one byte to one byte mapping will no longer apply. */
s16 postfix(symbol * expr, char * out)
{
  symbol * s;
  char * o;

  for(s = expr, o = out; (*o++ = (char)(*s++));) {
  }
  return 0;
}

/* Convert a compact postfix expression into a more human-readable form */
s16 postfix_formatter(symbol * expr, char * out, s16 maxlen)
{
  char cv_scratch[EXPR_ALLOC];
  s16 i, len;
  symbol c;

  len = 0;
  maxlen--;
  if (postfix(expr, cv_scratch)) {
    /* error */
    return 0;
  } else {
    const char * s;

    for(i=0; cv_scratch[i] && (len < maxlen); i++) {
      c = (symbol) cv_scratch[i];
      if(sym_attrs[c].name_forth) {
        s = sym_attrs[c].name_forth;
        while(*s && (len < maxlen)) {
          out[len++] = *s++;
        }
      } else if (len < maxlen) {
        out[len++] = (char) c;
      }
      if (len < maxlen) {
        out[len++] = ' ';
      }
    }
  }

  /* Ensure the output ends in a null, and remove any final spaces */
  out[len] = 0;
  while ((len > 0) && (out[len-1] == ' ')) {
    len--;
    out[len] = 0;
  }

  return len;
}

/* complexity simply adds up the complexity scores (weights) in an
   expression. */
s16 complexity(symbol * expr)
{
  symbol sym;
  s16 comp;

  comp = 0;
  while((sym = *expr++)) {
    comp = (s16) (comp + sym_attrs[sym].sa_wgt);
  }
  return comp;
}

/* endstack goes through an expression and determines how many things
   it leaves on the stack when it is done. It also returns pointers to
   the last symbol in the expression, and the last symbol that left the stack
   with one item on it. These are all useful in expr_break(). */
int endstack(symbol * expr, int *ending_sp, symbol * * last_sp_1,
  symbol * * last_sym)
{
  symbol sym, sf;
  int sp;
  symbol * ex;
  symbol * lsp1;

  ex = expr;
  sp = 0; lsp1 = 0;
  while((sym = *ex)) {
    /* Look at the symbol's seft */
    sf = sym_attrs[sym].seft;
    if (sf == 'a') {
      /* Push a constant onto the stack */
      sp++;
    } else if (sf == 'b') {
      /* Single-argument function */
    } else if (sf == 'c') {
      /* Two-argument function */
      sp--;
    }

    /* Check for trouble */
    if (sp <= 0) {
      /* Nothing on the stack -- this means we underflowed */
      printf("endstack underflow\n");
      return ERR_EVAL_STACK_UNDERFLOW;
    }

    /* Keep track of the last place where the stack was 1. */
    if ((sp == 1) && (*(ex+1))) {
      lsp1 = ex;
    }

    ex++;
  }
  /* Go back to the final symbol */
  ex--;
  if (ex < expr) {
    printf("endstack null\n");
    return ERR_ES_NULL_EXPRESSION;
  }

  if (ending_sp) {
    *ending_sp = sp;
  }
  if (last_sp_1) {
    *last_sp_1 = lsp1;
  }
  if (last_sym) {
    *last_sym = ex;
  }

  return 0; /* No error */
} /* End of endstack */

/* Break an expression up into its root operator and the argument(s) if any. */
int expr_break(symbol * expr, symbol * op, symbol * seft,
  symbol * * arg1, int * a1_len, symbol * * arg2, int * a2_len)
{
  int ending_sp, err;
  symbol * last_sp_1;
  symbol * last_sym;
  symbol sf;

  /* Set some default return values */
  if (arg1) { *arg1 = 0; }      if (arg2) { *arg2 = 0; }
  if (a1_len) { *a1_len = 0; }  if (a2_len) { *a2_len = 0; }

  err = endstack(expr, &ending_sp, &last_sp_1, &last_sym);
  if (err) {
    printf("endstack err %d\n", err);
    return err;
  }
  if (ending_sp > 1) {
    printf("expr_break incomplete (sp=%d)\n", ending_sp);
    return ERR_EC_INCOMPLETE_EXPR;
  }
  sf = sym_attrs[*last_sym].seft;
  if (op) { *op = *last_sym; }
  if (seft) { *seft = sf; }
  if (sf == 'a') {
    /* It's a constant, and should be just one symbol long. There
       are no arguments. */
  } else if (sf == 'b') {
    /* Operator is a single-argument function, and the entire rest of the
       expression is its argument */
    if (arg1) { *arg1 = expr; }
    if (a1_len) { *a1_len = ((int) ((last_sym - expr))); }
  } else if (sf == 'c') {
    /* Operator is a two-argument function, and the two arguments are
       divided at the point marked by last_sp_1
         23+r   2s3r+
         ^ ^^   ^ ^ ^ */
    if (arg1) { *arg1 = expr; }
    if (a1_len) { *a1_len = ((int) (last_sp_1 - expr) + 1); }
    if (arg2) { *arg2 = last_sp_1+1; }
    if (a2_len) { *a2_len = ((int) (last_sym - last_sp_1) - 1); }
  }
  return 0;
}

/*
 expr_print_infix does everything needed to format an expression
(which must be complete) into an infix string and print it.
  To right-justify the output, pass a positive field width in the justify
parameter. */
void expr_print_infix(symbol * expr, int justify)
{
  symbol escratch[EXPR_ALLOC];
  char fscratch[F_ALLOC];
  char gscratch[MAX_ELEN * MAX_SYM_NAME_LEN];
  symbol ss;

  infix_preproc(expr, escratch);
  infix_1(escratch, fscratch, &ss);
  infix_expand(fscratch, gscratch);
  if (justify > 0) {
    int j;
    for(j=((int)strlen(gscratch)); j<justify; j++) {
      printf(" ");
    }
  }
  printf("%s", gscratch);
}

void eqn_print_infix(symbol * lhs, symbol * rhs)
{
  expr_print_infix(lhs, 0);
  printf(" = ");
  expr_print_infix(rhs, 0);
}

/* eval evaluates an expression; useful for retrieving the values of both
   sides after a match, or for iterating Newton's method. It returns
   an error if it doesn't evaluate for some reason.

  expr  - The expression to evaluate
  val   - Pointer to place to store the value (may be 0)
  dx    - Pointer to place to put the derivative (may be 0)
  tag   - Pointer to place to put the tags (may be 0)
  sptr  - Pointer to place to put the final stack pointer value
  show_work - Nonzero to display work (for -Ds option)

Return value is an error code like ERR_EVAL_TOO_LONG
 */
s16 eval(symbol * expr, ries_val * val, ries_dif * dx, ries_tgs * tags,
         s16 * sptr, s16 show_work)
{
  s16 contains_x;
  metastack ms;
  symbol * s;
  symbol dbg_scratch[EXPR_ALLOC];
  s16 err, i;
  s16 undo_count;

  contains_x = (symstrsym(expr, 'x') != 0);

  /* default return values */
  if (val) { *val = k_0; }
  if (dx) { *dx = (ries_dif) k_0; }
  if (tags) { *tags = 0; }

  ms_init(&ms);
  for(s = expr, i=0; s[i]; i++) {
    if (i >= MAX_ELEN) {
      return ERR_EVAL_TOO_LONG;
    } else {
      /* Determine if the metastack will overflow */
      int sp1 = 0; int sp2 = 0; int ms1 = 0;
      switch (sym_attrs[s[i]].seft) {
      /* number of:  pops    pushes   actions */
        case '0': ;                            break;
        case 'a': ;          sp2 = 1; ms1 = 1; break;
        case 'b': ; sp1 = 1;          ms1 = 2; break;
        case 'c': ; sp1 = 2;          ms1 = 3; break;
        default: ; return ERR_EVAL_UNKNOWN_SEFT;
      }
      if (ms.sp < sp1) {
        return ERR_EVAL_STACK_UNDERFLOW;
      }
      if (ms.sp + sp2 > MS_STK_MAX) {
        return ERR_EVAL_STACK_OVERFLOW;
      }
      if (ms.msp + ms1 > MS_UNDO_MAX) {
        return ERR_EVAL_METASTACK_OVERFLOW;
      }

      err = exec(&ms, s[i], &undo_count, contains_x);
      if (err) {
        return err;
      }
      if (debug_N || show_work) {
        symbol escratch[EXPR_ALLOC];
        char fscratch[F_ALLOC];
        char gscratch[MAX_ELEN * MAX_SYM_NAME_LEN];
        symbol ss;
        ries_val v;
        ries_dif dv;
        ries_tgs tg;
        s16 j;

        dbg_scratch[i] = s[i];
        dbg_scratch[i+1] = 0;
        infix_preproc(dbg_scratch, escratch);
        err = infix_1(escratch, fscratch, &ss);
        infix_expand(fscratch, gscratch);
        v = ms_peek(&ms, &dv, &tg, 0);
        if (debug_N) { printf("eval "); }
        for(j=(s16)strlen(gscratch); j<27; j++) {
          printf(" ");
        }
        printf("%s = ", gscratch);
        spfg(k_nominal_digits, v); /* printf(fmt_g_nom_fixed, v); */
        if (dv != k_0) {
          printf(" (d/dx = ");
          printf(fmt_g_diff, dv);
          printf(")");
        }
        printf(" %s\n", tagname(tg));
      }
    }
  }

  { /* Store any requested results in the pointers passed to us */
    ries_val v;
    v = ms_peek(&ms, dx, tags, sptr);
    if (val) { *val = v; }
  }

  return 0;
} /* End of eval */

/* try.solve does most of the algebraic manipulation to convert an
equation into the form "x = ...". If there are multiple x's in the
input, it doesn't notice and simply pushes the extra ones to the RHS.
  There are a few special things to note:
  * The handling of [<l>s]=[<r>] deals with a branch case: when
transforming this to [<l>]=[<r>q], the signs might disagree. An
example is shown by "ries 0.8183431428522" which finds [x3-s]=[pf+].
After removing 's' and adding 'q' it calls eval() on the resulting
expressions and if their signs disagree, it negates the RHS to give
[<l>]=[<r>qn].
  * The sqrt sign check and others like it are subject to inaccuracies
if eval() is using g_target instead of the root of the current
equation as found by newton(). We could improve this by changing
exec.x temporarily, and restoring it afterwards the way newton() does;
or by making the exec.x an explicit parameter of eval().
  * Most seft-c operators can be solved by appending arg2 to the RHS,
by manipulating the lhs and rhs strings in-place. [L] is an exception:
[<a><b>L]=[<c>] must be converted into [<a>]=[<b><c>^], and it uses an
extra temp buffer "rtmp[]".
 */
#define TS_ALLOC_L EXPR_ALLOC
#define TS_ALLOC_R (EXPR_ALLOC*4)
void try_solve(symbol * l, symbol * r,
  symbol * l_out, int l_len, symbol * r_out, int r_len)
{
  symbol op, seft;
  symbol * arg1; int a1_len;
  symbol * arg2; int a2_len;

  symbol lhs[TS_ALLOC_L];
  symbol part1[TS_ALLOC_L];
  symbol part2[TS_ALLOC_L];
  symbol rhs[TS_ALLOC_R];
  symbol tmpc[TS_ALLOC_R];

  unsigned int gg, l1;

  symstrncpy0(lhs, l, TS_ALLOC_L);
  symstrncpy0(rhs, r, TS_ALLOC_R);
  /* printf("try.solve [%s]:[%s]\n", lhs, rhs); */

  gg = 1;
  while(gg) {
    op = 0;
    expr_break(lhs, &op, &seft, &arg1, &a1_len, &arg2, &a2_len);

    symstrncpy0(part1, arg1, a1_len+1);
    symstrncpy0(part2, arg2, a2_len+1);
    /* printf("try.solve: [%s] [%s] [%c] %c\n", part1, part2, op, seft); */

    if (0) {
    /* -------------------- Seft c symbols -------------------- */
    } else if (op == '+') {
      if (debug_S) {
        printf("transforming: ");
        eqn_print_infix(lhs, rhs);
        printf("\n  by subtracting '");
        expr_print_infix(part2, 0);
        printf("' from both sides\n");
      }
      /* printf("construct [%s]:['%s'%s'-']\n", part1, rhs, part2); */
      symstrncat(rhs, part2, TS_ALLOC_R);
      symstrncat(rhs, ((symbol *) "-"), TS_ALLOC_R);
      symstrncpy0(lhs, part1, TS_ALLOC_L);
      /* printf("now [%s]:[%s]\n", lhs, rhs); */
    } else if (op == '-') {
      if (debug_S) {
        printf("transforming: ");
        eqn_print_infix(lhs, rhs);
        printf("\n  by adding '");
        expr_print_infix(part2, 0);
        printf("' to both sides\n");
      }
      symstrncat(rhs, part2, TS_ALLOC_R);
      symstrncat(rhs, ((symbol *) "+"), TS_ALLOC_R);
      symstrncpy0(lhs, part1, TS_ALLOC_L);
    } else if (op == '*') {
      if (debug_S) {
        printf("transforming: ");
        eqn_print_infix(lhs, rhs);
        printf("\n  by dividing both sides by '");
        expr_print_infix(part2, 0);
        printf("'\n");
      }
      symstrncat(rhs, part2, TS_ALLOC_R);
      symstrncat(rhs, ((symbol *) "/"), TS_ALLOC_R);
      symstrncpy0(lhs, part1, TS_ALLOC_L);
    } else if (op == '/') {
      if (debug_S) {
        printf("transforming: ");
        eqn_print_infix(lhs, rhs);
        printf("\n  by multiplying both sides by '");
        expr_print_infix(part2, 0);
        printf("'\n");
      }
      symstrncat(rhs, part2, TS_ALLOC_R);
      symstrncat(rhs, ((symbol *) "*"), TS_ALLOC_R);
      symstrncpy0(lhs, part1, TS_ALLOC_L);
    } else if (op == '^') {
      if (debug_S) {
        printf("transforming: ");
        eqn_print_infix(lhs, rhs);
        printf("\n  by taking the '");
        expr_print_infix(part2, 0);
        printf("' root of both sides\n");
      }
      symstrncat(rhs, part2, TS_ALLOC_R);
      symstrncat(rhs, ((symbol *) "v"), TS_ALLOC_R);
      symstrncpy0(lhs, part1, TS_ALLOC_L);
    } else if (op == 'L') {
      /* [<p1><p2>L] = [<r>]  log to base <part2> of <part1> = <rhs>
         [<p1>] = [<p2><r>^]  <part1> = <part2> ^ <rhs> */
      symbol rtmp[TS_ALLOC_R];

      if (debug_S) {
        printf("transforming: ");
        eqn_print_infix(lhs, rhs);
        printf("\n  by raising '");
        expr_print_infix(part2, 0);
        printf("' to the power of both sides\n");
      }
      symstrncpy0(rtmp, part2, TS_ALLOC_R);
      symstrncat(rtmp, rhs, TS_ALLOC_R);
      symstrncat(rtmp, ((symbol *) "^"), TS_ALLOC_R);
      symstrncpy0(rhs, rtmp, TS_ALLOC_R);
      symstrncpy0(lhs, part1, TS_ALLOC_L);
    } else if (op == 'v') {
      if (debug_S) {
        printf("transforming: ");
        eqn_print_infix(lhs, rhs);
        printf("\n  by raising both sides to the '");
        expr_print_infix(part2, 0);
        printf("' power\n");
      }
      symstrncat(rhs, part2, TS_ALLOC_R);
      symstrncat(rhs, ((symbol *) "^"), TS_ALLOC_R);
      symstrncpy0(lhs, part1, TS_ALLOC_L);

    /* -------------------- Seft b symbols -------------------- */

    } else if (op == 'E') {
      if (debug_S) {
        printf("transforming: ");
        eqn_print_infix(lhs, rhs);
        printf("\n  by taking the natural logarithm of both sides\n");
      }
      /* printf("construct [%s]:['%s'l']\n", part1, rhs); */
      symstrncat(rhs, ((symbol *) "l"), TS_ALLOC_R);
      symstrncpy0(lhs, part1, TS_ALLOC_L);
      /* printf("now [%s]:[%s]\n", lhs, rhs); */
    } else if (op == 'l') {
      if (debug_S) {
        printf("transforming: ");
        eqn_print_infix(lhs, rhs);
        printf("\n  by raising e to the power of both sides\n");
      }
      symstrncat(rhs, ((symbol *) "E"), TS_ALLOC_R);
      symstrncpy0(lhs, part1, TS_ALLOC_L);
    } else if (op == 'n') {
      if (debug_S) {
        printf("transforming: ");
        eqn_print_infix(lhs, rhs);
        printf("\n  by negating both sides\n");
      }
      symstrncat(rhs, ((symbol *) "n"), TS_ALLOC_R);
      symstrncpy0(lhs, part1, TS_ALLOC_L);
    } else if (op == 'q') {
      if (debug_S) {
        printf("transforming: ");
        eqn_print_infix(lhs, rhs);
        printf("\n  by squaring both sides\n");
      }
      symstrncat(rhs, ((symbol *) "s"), TS_ALLOC_R);
      symstrncpy0(lhs, part1, TS_ALLOC_L);
    } else if (op == 'r') {
      if (debug_S) {
        printf("transforming: ");
        eqn_print_infix(lhs, rhs);
        printf("\n  by taking the reciprocal of both sides\n");
      }
      symstrncat(rhs, ((symbol *) "r"), TS_ALLOC_R);
      symstrncpy0(lhs, part1, TS_ALLOC_L);
    } else if (op == 's') {
      ries_val lsqrt, rsqrt;
      if (debug_S) {
        printf("transforming: ");
        eqn_print_infix(lhs, rhs);
        printf("\n  by taking the square root of both sides\n");
      }
      symstrncat(rhs, ((symbol *) "q"), TS_ALLOC_R);
      symstrncpy0(lhs, part1, TS_ALLOC_L);
      /* Since we took a square root, we now have to ensure that both
         sides of the equation agree as to which square root (positive
         or negative) generates the match. For example, one answer
         given for 0.8183431428522 is "(x-3)^2 = phi+pi"; when solving
         we'd get "x = sqrt(phi+pi)+3" unless we do these tests. */
      /* %%% These calls to eval() would benefit from using the Newton root,
         which will have already been computed when try.solve is called */
      eval(lhs, &lsqrt, 0, 0, 0, 0);
      eval(rhs, &rsqrt, 0, 0, 0, 0);
      /* printf("post-sqrt %17.15g = %s ?= %s = %17.15g\n", lsqrt, lhs, rhs, rsqrt); */
      if (lsqrt * rsqrt < 0) {
        if (debug_S) {
          printf("  then negating the right side so the signs agree\n");
        }
        symstrncat(rhs, ((symbol *) "n"), TS_ALLOC_R);
      }

    } else if (op == 'W') {

      /* The inverse of y=W(x) is x=w e^y, thus we solve this one by
      /  changing "W(x)=RHS" by "x = RHS*e^RHS". This causes the RHS
      /  to become more than twice as long. In theory if the LHS is of
      /  the form [xWWWWW], TS_ALLOC_R would need to be something like
      /  TS_ALLOC_L*2^(TS_ALLOC_L+1). We're not going to do that so
      /  instead we declare TS_ALLOC_R to be something like 4*EXPR_ALLOC,
      /  and we check to see if there is enough room to generate a
      /  "RHS*e^RHS" here. */
      l1 = symstrlen(rhs) * 2 + 2;
      if (l1 >= TS_ALLOC_R) {
        /* We can't handle this 'W', so we're done solving. */
        gg = 0;
      } else {
        if (debug_S) {
          printf("transforming: ");
          eqn_print_infix(lhs, rhs);
          printf("\n  by replacing 'W(a) = b' with 'a = b e^b'\n");
        }
        /* if (debug_S){printf(">> start: lhs=[%s], rhs=[%s]\n",lhs,rhs);} */
        /* Keep a copy of the RHS prior to manipulation */
        symstrncpy0(tmpc, rhs, TS_ALLOC_R);
        /* if (debug_S) { printf(">> step1 tmpc=[%s]\n", tmpc); } */
        /* Add the entire thing to the end, e.g. [1pq+] becomes [1pq+1pq+] */
        symstrncat(rhs, tmpc, TS_ALLOC_R);
        /* if (debug_S) { printf(">> step2 rhs=[%s]\n", rhs); } */
        /* Add an 'E' and a '*', so we have [1pq+1pq+E*] */
        symstrncat(rhs, (symbol *) "E*", TS_ALLOC_R);
        /* if (debug_S) { printf(">> step3 rhs=[%s]\n", rhs); } */
        /* Replace LHS with the argument, i.e. remove 'W' from the end */
        symstrncpy0(lhs, part1, TS_ALLOC_L);
        /* if (debug_S) { printf(">> result: lhs=[%s], rhs=[%s] len %d\n",
                         lhs, rhs, l1); } */
      }

    /* ------------------ Unhandled symbols ------------------- */
    /* Some symbols, e.g. sine and tangent, are not tested for at
       all. These functions have no inverse in RIES, so they cannot
       be moved to the right-hand-side. */

    } else {
      /* We get here if the symbol 'op' is not one of the handled symbols */
      gg = 0;
    }
  }

  if (l_out && r_out) {
    symstrncpy0(l_out, lhs, l_len);
    symstrncpy0(r_out, rhs, r_len);
  }
} /* End of try.solve */

#define NEWTON_MAX_ITER 12

/* newton performs Newton's method to determine the precise value of X
   for a given LHS and RHS to match. If the method does not converge
   or if there is an error the target value is returned instead in
   place of a root.

 lhs      - left-hand-side expression
 rhs      - right-hand-side expression
 root     - place to store the root
 diff_dx  - place to store the derivative
 tags     - place to put the tags (may be 0)

 */
s16 newton(symbol * lhs, symbol * rhs, ries_val *root, ries_dif *diff_dx,
  ries_tgs *tags)
{
  ries_val rhs_val; ries_dif rhs_dx; ries_tgs rhs_tg;
  ries_val lhs_val; ries_dif lhs_dx; ries_tgs lhs_tg;
  ries_val curr, prev, prv2;
  s16 i, err, rhs_has_x;

  /* Set default return values. */
  *root = g_target;
  /* If we return before we can compute a valid diff_dx, then we should
     treat it as a nonconverging case. We express that by returning a
     diff_dx of 0 */
  *diff_dx = 0;
  if (tags) { *tags = TYPE_NONE; }

  rhs_has_x = (symstrsym(rhs, 'x') != 0);

  /* first get the RHS value. */
  rhs_dx = 0; rhs_tg = TYPE_NONE; /* Assumed defaults */
  err = eval(rhs, &rhs_val, &rhs_dx, &rhs_tg, 0, 0); /* In newton() */
  if (err) {
    return err;
  }
  if (debug_n) {
    printf("newton RHS [%s] = ", rhs);
    spfg(k_nominal_digits, rhs_val); /* printf(fmt_g_nominal, rhs_val); */
    printf("  tag %x", rhs_tg);
    printf("\n");
    printf("       iterating LHS [%s]", lhs);
    if (rhs_has_x) {
      printf(" and RHS");
    }
    printf("...\n");
  }

  curr = g_target; prev = curr - k_1; prv2 = prev - k_1;
  for(i=0; i<NEWTON_MAX_ITER; i++) {
    prv2 = prev; prev = curr;
    exec_x = curr;
    /* Get new LHS value */
    err = eval(lhs, &lhs_val, &lhs_dx, &lhs_tg, 0, 0); /* In newton() */
    if (err) {
      if (debug_n) { printf("       step %d, error from LHS eval.\n", i); }
      exec_x = g_target;
      return err;
    }
    if (lhs_dx == k_0) {
      if (debug_n) { printf("       step %d, zero LHS derivative.\n", i); }
      exec_x = g_target;
      return ERR_NEWTON_ZERO_DX;
    }
    if (debug_n) {
      printf("       step %d l=", i);
      spff(k_nominal_digits, lhs_val); /* printf(fmt_g_nom_fixed, lhs_val); */
      printf("  dl=");
      spfg(k_nominal_digits, lhs_dx); /* printf(fmt_g_nominal, lhs_dx); */
      printf("  tag %x", lhs_tg);
      printf("\n");
    }
    /* If there is an X on the RHS, we need to re-eval to get new RHS value */
    if (rhs_has_x) {
      err = eval(rhs, &rhs_val, &rhs_dx, &rhs_tg, 0, 0); /* In newton() */
      if (err) {
        if (debug_n) { printf("       step %d, error from RHS eval.\n", i); }
        exec_x = g_target;
        return err;
      }
      if (lhs_dx == k_0) {
        if (debug_n) { printf("       step %d, zero RHS derivative.\n", i); }
        exec_x = g_target;
        return ERR_NEWTON_ZERO_DX;
      }
      if (debug_n) {
        printf("          RHS r=");
        spff(k_nominal_digits, rhs_val); /* printf(fmt_g_nom_fixed, rhs_val); */
        printf("  dr=");
        spfg(k_nominal_digits, rhs_dx); /* printf(fmt_g_nominal, rhs_dx); */
        printf("  tag %x", rhs_tg);
        printf("\n");
      }
    }
    /* Note: To demonstrate that the derivatives don't need to be very
       accurate, you can substitute the following with:

          curr = curr + ((rhs_val - lhs_val) * 1.001 / (lhs_dx - rhs_dx));

       and RIES will report all the same results (but the -Dn option will
       show that it takes a few more iterations to converge on each root)
    */
    curr = curr + ((rhs_val - lhs_val) / (lhs_dx - rhs_dx));
    if (curr == prev) {
      /* we be done */
      i = NEWTON_MAX_ITER + 1;
    } else if (curr == prv2) {
      ries_dif margin;
      margin = (ries_dif) (FABS(curr-prev) * 9007199254740992.0 /* 2^53 */
                                                  / FABS(curr));
      if (debug_n) {
        printf("newton cycle-2 loop, i == %d,  margin == %g\n", i, margin);
      }
      if (margin <= 2.0) {
        /* Period-2 loop with amplitude 1 ULP (unit in the last place) */
        curr = prev; i = NEWTON_MAX_ITER + 1; /* Force success, exit loop */
      } else if ((i >= 6) && (margin <= 4.0)) {
        curr = prev; i = NEWTON_MAX_ITER + 1; /* Force success, exit loop */
      }
    }
  }

  /* restore exec's target value. very important!! (-:*/
  exec_x = g_target;

  /* Check for acceptably close convergence
  /    older versions were:
  /      if (fabs(curr - prev)  == k_0)
  /      if (fabs(curr - prev) < fabs(g_target * k_prune_deriv))      */
  if (curr == prev) {
    *root = curr;
    *diff_dx = lhs_dx - rhs_dx;
    if (tags) { *tags = rhs_tg; }
    if (debug_n) {
      printf("       converged precisely on ");
      spfg(k_nominal_digits, curr); /* printf(fmt_g_nominal, curr); */
      printf("\n");
    }
    return 0;
  } else if (FABS(curr - prev) < FABS(g_target * k_newton_settled)) {
    *root = curr;
    *diff_dx = lhs_dx - rhs_dx;
    if (tags) { *tags = rhs_tg; }
    if (debug_n) {
      printf("       converged on ");
      spfg(k_nominal_digits, curr); /* printf(fmt_g_nominal, curr); */
      printf(" by margin of ");
      spfg(k_nominal_digits, FABS(curr - prev)); /* printf(fmt_g_nominal, FABS(curr - prev)); */
      printf("\n");
    }
    return 0;
  }

  if (debug_n) {
    printf("       did not converge (last delta = ");
      spfg(k_nominal_digits, curr - prev); /* printf(fmt_g_nominal, curr - prev); */
    printf(").\n");
  }
  return ERR_NEWTON_NO_CONVERGE;
} /* End of newton() */

/* cv.simplify takes the same arguments as newton(). It looks at an
lhs and rhs that is suspected to be a new match.
  First it tries to remove redundant trailing operations added by
canon.val; for example if you run "ries 2.141592653 --canon-reduction
nr25 -l0" it will transform [x1+2/]=[p2/] into [x1+]=[p]. It does not try to
reorder things, so for example [xn2*]=[p2*n] would be left untouched.
  Then it runs newton() on the result, passing all the parameters
through to it.
 */
s16 cv_simplify(symbol * lhs, symbol * rhs, ries_val *root, ries_dif *diff_dx,
  ries_tgs *tags, int do_newton)
{
  s16 rv;
  int gg;
  s16 err;

  if (debug_Q) {
    printf("cv.simplify: [%s]=[%s]\n", lhs, rhs);
  }

  gg = 1;
  if (g_no_cv_simplify) {
    gg = 0;
  }
  while(gg) {
    gg = 0;

    /* Bilateral cancellation */
    if (bothtrail(lhs, rhs, (symbol *) "2/")) {
      if (debug_Q) {
        printf("  remove shared trailing [2/]\n");
      }
      symstrclip(lhs, 2); symstrclip(rhs, 2); gg = 1;
    }
    if (bothtrail(lhs, rhs, (symbol *) "2*")) {
      if (debug_Q) {
        printf("  remove shared trailing [2*]\n");
      }
      symstrclip(lhs, 2); symstrclip(rhs, 2); gg = 1;
    }
    if (bothtrail(lhs, rhs, (symbol *) "r")) {
      if (debug_Q) {
        printf("  remove shared trailing [r]\n");
      }
      symstrclip(lhs, 1); symstrclip(rhs, 1); gg = 1;
    }
    if (bothtrail(lhs, rhs, (symbol *) "n")) {
      if (debug_Q) {
        printf("  remove shared trailing [n]\n");
      }
      symstrclip(lhs, 1); symstrclip(rhs, 1); gg = 1;
    }

    /* Asymmetrical simplifications */
    if (symstrtrail(lhs,(symbol*)"nr") && symstrtrail(rhs,(symbol*)"n")) {
      /* [Anr]=[Bn] -> [Ar]=[B]
        This shows up in "ries 2.4284920346331 --canon-reduction nr25"
        which gives "1/-(ln(x)-1) = -(1-pi^2)" without this transformation,
        and "1/(ln(x)-1) = 1-pi^2" with it. */
      if (debug_Q) {
        printf("  replace [<a>nr]=[<b>n] with [<a>r]=[<b>]\n");
      }
      symstrclip(lhs, 2); symstrncat(lhs, (symbol *) "r", TS_ALLOC_L);
      symstrclip(rhs, 1); gg = 1;
    }
    if (symstrtrail(lhs,(symbol*)"n") && symstrtrail(rhs,(symbol*)"nr")) {
      /* [An]=[Bnr] -> [A]=[Br] */
      if (debug_Q) {
        printf("  replace [<a>n]=[<b>nr] with [<a>]=[<b>r]\n");
      }
      symstrclip(lhs, 1); symstrclip(rhs, 2);
      symstrncat(rhs, (symbol *) "r", TS_ALLOC_L); gg = 1;
    }
    if (symstrtrail(lhs,(symbol*)"rr")) {
      /* [Arr]=[B] -> [A]=[B]
        This shows up in "ries 2.50618 --canon-reduction r"
        which gives "1/(1/x^2) = 2 pi" without this transformation,
        and "x^2 = 2 pi" with it. */
      if (debug_Q) {
        printf("  replace [<a>rr]=[<b>] with [<a>]=[<b>]\n");
      }
      symstrclip(lhs, 2); gg = 1;
    }
    if (symstrtrail(rhs,(symbol*)"rr")) {
      /* [A]=[Brr] -> [A]=[B] */
      if (debug_Q) {
        printf("  replace [<a>]=[<b>rr] with [<a>]=[<b>]\n");
      }
      symstrclip(rhs, 2); gg = 1;
    }
    if (symstrtrail(lhs,(symbol*)"r2/") && symstrtrail(rhs,(symbol*)"r")) {
      /* [Ar2/]=[Br] -> [A2*]=[B]
        This shows up in "ries 2.4284920346331 --canon-reduction nr25" which
        gives "(1/((x/e)^pi)^2)/2 = 1/cospi(1/(4-1/pi))" without this
        transformation, and "2*((x/e)^pi)^2 = cospi(1/(4-1/pi))" with it. */
      if (debug_Q) {
        printf("  replace [<a>r2/]=[<b>r] with [<a>2*]=[<b>]\n");
      }
      symstrclip(lhs, 3); symstrncat(lhs, (symbol *) "2*", TS_ALLOC_L);
      symstrclip(rhs, 1); gg = 1;
    }
    if (symstrtrail(lhs,(symbol*)"r") && symstrtrail(rhs,(symbol*)"r2/")) {
      /* [Ar]=[Br2/] -> [A]=[B2*]
        This shows up in "ries 2.4284920346331 --canon-reduction nr25" which
        gives "1/-tanpi(sinpi(x-2)) = (1/cospi(2/e)^2)/2" without this
        transformation, and "-tanpi(sinpi(x-2)) = 2 cospi(2/e)^2" with it. */
      if (debug_Q) {
        printf("  replace [<a>r]=[<b>r2/] with [<a>]=[<b>2*]\n");
      }
      symstrclip(lhs, 1); symstrclip(rhs, 3);
      symstrncat(rhs, (symbol *) "2*", TS_ALLOC_L); gg = 1;
    }
  }

  if (do_newton) {
    rv = newton(lhs, rhs, root, diff_dx, tags);
  } else {
    /* Do not perform Newton, but instead assume that lhs and rhs are already
       equal. This isn't currently used, but might be used by
       check.exact_match in the future. */
    *root = g_target;
    /* diff_dx will have been already set by caller */
    /* Call eval on the RHS to get the tags, and to discover any eval error. */
    err = eval(rhs, 0, 0, tags, 0, 0);
    return err;
  }
  return rv;
} /* End of cv.simplify */

void defsym_used(symbol * expr)
{
  symbol * sym;

  sym = expr;
  while(*sym) {
    if (sym_attrs[*sym].defn) {
      sym_attrs[*sym].def_needed = 1;
    }
    if (symstrsym(((symbol *) "CST"), *sym)) {
      used_trig = 1;
    }
    sym++;
  }
}

/* AKA define_symbols, show_legend */
void describe_symbols(void)
{
  s16 sym;
  s16 lineleft, sl, clen;
  s16 going;
  const char * candidate;
  symbol csym = 0;
  int need_lf = 1;

  if (sym_attrs['A'].def_needed) {
    if (used_atan1) {
      sym_attrs[PHSYM_ATAN1].def_needed = 1;
    }
    if (!(used_atan2)) {
      sym_attrs['A'].def_needed = 0;
    }
  }
  sym_attrs[PHSYM_ATAN1].defn =
    "atan2(s) = Angle of ray from origin through point (1,s)";

  going = 1; lineleft = LINELEFT_INIT;
  while (going) {
    going = 0;
    for(sym=0; sym<SYMBOL_RANGE; sym++) {
      if (sym_attrs[sym].def_needed) {
        if (sym_attrs[sym].def_given == 0) {
          going = 1;
          if (need_lf) {
            printf("\n");
            need_lf = 0;
          }
        }
      }
    }

    if (going) {
      /* there's still one to print */
      candidate = 0; clen = 0;
      for(sym=0; sym<SYMBOL_RANGE; sym++) {
        if (sym_attrs[sym].def_needed) {
          if (sym_attrs[sym].def_given == 0) {
            sl = (s16) strlen(sym_attrs[sym].defn);
            if ( (sl > clen) && (sl < lineleft) ) {
              csym = (symbol) sym;
              candidate = sym_attrs[csym].defn;
              clen = (s16) strlen(candidate);
            }
          }
        }
      }

      /* did we actually find one that fits? */
      if (candidate) {
        printf("  %s", sym_attrs[csym].defn);
        sym_attrs[csym].def_given = 1;
        lineleft = (s16) (lineleft - clen - 2);
      } else {
        printf("\n");
        lineleft = LINELEFT_INIT;
      }
    }
  }

  if (lineleft < LINELEFT_INIT) {
    printf("\n");
  }

  if (used_trig) {
    if (g_trig_scale_default) {
      /* In this case the functions are called "sinpi", etc. and are defined
         by the sym_defn[] strings */
    } else {
      /* Tell what units were used, special-case radians */
      printf("  For the trig functions, ");
      if (k_sincos_arg_scale == 1.0) {
        printf("2 pi");
      } else {
        printf("%g", dbl(2.0 * k_pi / k_sincos_arg_scale));
      }
      printf(" units are a full circle.\n");
    }
  }
}


/* This routine is meant to be used for *integer* values given as a double,
but formatted as if with the "%ld" format specifier. If the integer won't fit
in the given width, it then prints it using using a %e format instead. The
result is a char * pointing to a private buffer, which you need to use
before calling pf_intfloat_wid again.
  This example demonstrates its handling of width limits, rounding, etc.

  #define TESTWIDMAX 14
  double x; int w;
  for(x=143.0; x<1.0e14; x*=-7.0) {
    for(w=7; w<TESTWIDMAX; w++) {
      printf("%*s ", w, pf_intfloat_wid(x, w));
    }
    printf("\n");
  }

As you can see, a width of 7 characters only gives two significant digits
when the value is bigger than 9999999, or one significant digit for values
less than -999999. The routine is not designed for smaller widths. On the
high end, most C libraries will let you print as many digits as you want,
but widths bigger than about 20 don't give you much because "double" usually
only provides about 16 decimal digits of precision. */
char pfw_buf[40];
char * pf_intfloat_wid(stats_count x, int width)
{
  int i, w;
  char fmt1[FMT_STR_SIZE];
  char fmt2[FMT_STR_SIZE];

  w = (x < 0) ? (width-1) : width;
  snprintf(fmt1, FMT_STR_SIZE, "%%.%dg", w);
  snprintf(fmt2, FMT_STR_SIZE, "%%.%de", w-6);

  snprintf(pfw_buf, 40, fmt1, x);
  if (strchr(pfw_buf, 'e') || strchr(pfw_buf, 'E')) {
    snprintf(pfw_buf, 40, fmt2, x);
  } else if (strchr(pfw_buf, '.')) {
    for (i=0; pfw_buf[i]; i++) { }
    i--;
    while ((i > 0) && (pfw_buf[i] == '0')) {
      pfw_buf[i] = 0;
      i--;
    }
    if ((i > 0) && (pfw_buf[i] == '.')) {
      pfw_buf[i] = 0;
    }
  }
  return pfw_buf;
}

int g_allow_slow_message;

/* print.end generates the summary statistics that get printed at the
   end. Note that if the g_enable_output flag is not set, most of this
   does not get printed (such as when --find.expression is being used). */
void print_end(int exit_code)
{
  stats_count combos;
  long total_insert;
  time_flt tsec = gettime();

  if (got_exact && (g_restrict_subexpr < TYPE_INT)) {
    printf(
      "\n"
      "  NOTE: 'exact' match may result from floating-point roundoff error.\n"
    );
  }

  if (debug_s) {
    printf(
      "\n"
      "  NOTE: Some values will have lost significance in the last one or\n"
      "  two digits due to round-off during intermediate calculations.\n"
    );
  }

  if (g_enable_output && (out_expr_format <= OF_NORMAL)) {
    describe_symbols();
  }

  total_insert = lhs_insert + rhs_insert;
  if (total_insert && g_enable_output) {
    printf("\n");
    printf("                         --LHS--      --RHS--      -Total-\n");
    printf("     max complexity: %11d  %11d  %11d\n", lmax, rmax, lmax+rmax);
    printf("          dead-ends: %11s", pf_intfloat_wid(lhs_prune, 11));
                       printf("  %11s", pf_intfloat_wid(rhs_prune, 11));
                 printf("  %11s", pf_intfloat_wid(lhs_prune + rhs_prune, 11));
                       printf("  Time: %.3f\n", tsec);
    printf("        expressions: %11s", pf_intfloat_wid(lhs_gen, 11));
                       printf("  %11s", pf_intfloat_wid(rhs_gen, 11));
                      printf("  %11s\n", pf_intfloat_wid(gen_total, 11));
    printf("           distinct: %11ld  %11ld  %11ld",
                                        lhs_insert, rhs_insert, total_insert);
    if (mem_used_KiB > 1024L) {
      printf("  Memory: %ldKiB\n", mem_used_KiB);
    } else {
      printf("  Memory: %ld B\n", mem_used_bytes);
    }

    /* tell them how much work we did. */
    printf("\n");
    combos = ((stats_count) lhs_insert) * ((stats_count) rhs_insert);
    printf("        Total equations tested:    %20s",
                                                 pf_intfloat_wid(combos, 20));
    if (combos > 9999) {
      printf(" (%.4g)", (stats_count) combos);
    }
    printf("\n");
  }

  if (exit_code) {
    exit(exit_code);
  }
} /* End of print.end */

/* Cross-reference to the initialization functions:
/
/ validate_types: verifies that s16 is actually 16-bit, etc.
/ init_formats (aka check_precision, check_types): measures the precision
/   of the ries_val floating-point type
/ init_numerics: calculates the values of pi, e, phi, etc.
/
*/

/* ieee.paranoia tries to check for IEEE-compliant compile options. Some optiona
   (such as those mentioned in the error printfs), make it hard to detect
   overflow, roundoff, etc. For background see:

     www.gnu.org/software/libc/manual/html_node/Infinity-and-NaN.html
     gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
 */
void ieee_paranoia(void)
{
  ries_dif a1;

#ifdef __FAST_MATH__
  fprintf(stderr,
    "RIES does not work when compiled with the --ffast-math option.\n");
  exit(-1);
#endif

  /*
  Detect IEEE NaN compliance, by generating a NaN and trying to detect it in
  the manner supported by IEEE 754: a NaN is defined to be "unordered", i.e. it
  is not equal to, greater than, or less than anything, including even itself.

  Note that in most GCC implementations, the C99 isnan() function exists but
  is merely defined by a macro, e.g.:

    static __inline__ int __inline_isnanf( float __x ) { return __x != __x; }

  We create a NaN by generating an oveflow value and subtracting it from
  itself.
  */

  /* Generate an overflow value by starting with 2 and self-squaring 100
     times. This overflows if the exponent field is less than 100 bits. */
  for(k_d_inf=2.0,a1=1.0; a1<100.0; k_d_inf*=k_d_inf,a1++) { }
  k_d_ninf = 0.0 - k_d_inf;

  /* Subtract this overflowed value from itself: inf-inf is NaN. */
  k_d_nan = k_d_inf - k_d_inf;

  /* By definition, a NaN is not equal to itself. */
  if (k_d_nan != k_d_nan) {
    /* printf("detected NaN\n"); */
  } else {
    /* printf("did not detect NaN\n"); */
    fprintf(stderr,
"RIES does not work when compiled with options that prevent full IEEE 754\n"
"compliance, such as the GCC compiler options --ffast-math, -ffinite-math-only,\n"
"and -funsafe-math-optimizations.\n");
    exit(-1);
  }
}

/* init.formats measures available precision and sets the formatting strings
   and other values used to format and print numbers.
     For initialization of constants like pi and e, see init_numerics */
void init_formats(void)
{
  float digits_usable, digits_nominal;
  ries_val a1, a2, k0, k1, k2;
  const char * gstr;

  k0 = 0.0; k1 = 1.0; k2 = 2.0;
  /* The following loop looks for overflow and roundoff error, and counts
     the number of significant bits. There are several ways computer
     arithmetic can fail; this code tests three of them:
   - Normal signed integers will "wrap around" and become negative, failing
     the "a1>k0" test.
   - Floating-point numbers overflow to Infinity; this is caught by the
     "a1-a1==0" test (because Infinity-Infinity is a NaN).
   - Normal floating-point numbers eventually reach a point where they are
     so large that adding 1 to X then subtracting X rounds off to 0 (or
     perhaps 2 or a larger power of 2). The "a2-a1==k1" test catches this.
   - Arbitrary-precision types might be able to continue intil we fill
     available memory, or there might be some numerical oddity that our
     other tests miss. The "k_mantissa_bits<256" test catches these cases.
   - Dekker-style "double-double" arithmetic manages to pass all of the
     above tests when working with powers of 2. Therefore, rather than
     just doubling a1 each time through, every other time we add 2 so that
     when expressed in binary the mantissa has a "10101010..." pattern. */
  a1 = k2;           /*  2 */
  a2 = a1 + k1;      /*  3 */
  k_mantissa_bits = 1;
  rv_maxint = 1.0;
  while ((a2-a1 == k1) && (a1>k0) && (a1-a1==0) && (k_mantissa_bits < 666)) {
    a1 = a1 + a1;    /*  4  20   84  340 */
    a2 = a1 + 1;     /*  5  21   85  341 */
    k_mantissa_bits++;
    if ((a2-a1 == k1) && (a1>k0) && (a1-a1==0) && (k_mantissa_bits < 666)) {
      a1 = a1+a1+k2; /* 10  42  170  682 */
      a2 = a1 + 1;   /* 11  43  171  683 */
      k_mantissa_bits++;
    }
    rv_maxint *= 2.0;
  }
  rv_maxint /= 4.0;

  /* NOTE: Cannot use test debug_z flag here because args have not
     been parsed yet */
#ifdef RIES_DEBUG_FORMATS
  printf("init.formats: got %d mantissa bits\n", (int) k_mantissa_bits);
#endif
  /* Make sure the answer is "sane" */
  if ((int)k_mantissa_bits < 20) {
    printf("init.formats: Precision seems to be only %d binary bits; RIES cannot proceed.\n", (int) k_mantissa_bits);
    print_end(-1);
  }
  if (k_mantissa_bits > (float)(sizeof(ries_val))*9.0) {
    printf("init.formats: ries_val seems to have %d binary bits, but only uses %d bits!\n",
      (int) k_mantissa_bits, (int) (sizeof(ries_val) * 8));
    print_end(-1);
  }
  if (k_mantissa_bits > 256.0) {
    k_mantissa_bits = 256.0;
  }
  /* Compute size of the ULP (unit in the last place) for values in
     [0.5..1.0) */
  k_ulp = 0.5;
  for(a1=0.5; a1<k_mantissa_bits; a1+=1.0) {
    k_ulp *= 0.5;
  }
  digits_nominal = (k_mantissa_bits+1.0f) / 3.321928f;
  digits_usable = (k_mantissa_bits-3.0f) / 3.321928f;
  k_nominal_digits = (int)floor(digits_nominal + 1.0);
  k_usable_digits = (int)floor(digits_usable);
#ifdef RIES_DEBUG_FORMATS
  printf("  nominal: %f -> %d\n", digits_nominal, k_nominal_digits);
  printf("   usable: %f -> %d\n", digits_usable, k_usable_digits);
#endif

  if (sizeof(long double) == sizeof(ries_val)) {
    gstr = "Lg";
  } else {
    gstr = "g";
  }

  snprintf(fmt_g_nominal, FMT_STR_SIZE, "%%.%d%s", k_nominal_digits, gstr);
  snprintf(fmt_g_nom_fixed, FMT_STR_SIZE,
                     "%%-%d.%d%s", k_nominal_digits+6, k_nominal_digits, gstr);
  snprintf(fmt_g_usable, FMT_STR_SIZE,
                                             "%%.%d%s", k_usable_digits, gstr);
  snprintf(fmt_g_usa_fixed, FMT_STR_SIZE,
                       "%%-%d.%d%s", k_usable_digits+6, k_usable_digits, gstr);
  snprintf(fmt_g_diff, FMT_STR_SIZE, "%%.%dg", k_usable_digits-8);
  snprintf(fmt_g_dif_fixed, FMT_STR_SIZE,
                            "%%-%d.%dg", k_usable_digits-2, k_usable_digits-8);

#ifdef RIES_DEBUG_FORMATS
  printf("formats: fmt_g_nominal '%s', fmt_g_nom_fixed '%s'\n",
    fmt_g_nominal, fmt_g_nom_fixed);
  printf("   fmt_g_usable '%s', fmt_g_usa_fixed '%s'\n",
    fmt_g_usable, fmt_g_usa_fixed);
  printf("   fmt_g_diff '%s', fmt_g_dif_fixed '%s'\n",
    fmt_g_diff, fmt_g_dif_fixed);
#endif
} /* End of init.formats */

/* check.exit checks to see if it's time to exit after a new match.
   If the g_exact.exit flag is set we exit because we're done. If not,
   then we test to see if best.match is negative. Once best.match becomes
   negative there is no chance of getting any more nonexact matches,
   and therefore this new match is the last useful output we'll get. */
void check_exit(int is_exact)
{
  if (is_exact && (g_restrict_subexpr == TYPE_INT) && g_exact_exit) {
    printf("  (Stopping now because -ie option was given.)\n");
  } else if (is_exact && (g_restrict_subexpr == TYPE_RAT) && g_exact_exit) {
    printf("  (Stopping now because -re option was given.)\n");
  } else if (is_exact && (g_restrict_subexpr == TYPE_CONS) && g_exact_exit) {
    printf("  (Stopping now because -ce option was given.)\n");
  } else if (is_exact && (k_max_match_dist == 0)) {
    printf("  (Stopping now because an exact match was found.)\n");
  } else if (is_exact && g_exact_exit) {
    printf(
"  (Stopping now because --min-match-distance 0 or --max-match-distance 0\n"
"  was given.)\n");
  } else if (best_match < g_min_matchsize) {
    /* Decide what number to print in the following message. If the
       --min.match-distance option was not given then g_min.matchsize will
       be zero; but k_min_best.match is positive in all cases.
       g_min.matchsize is zero then we're exiting because of
       subtracting k_min_best.match above. */
    ries_dif t = (k_min_best_match > g_min_matchsize) ?
                  k_min_best_match : g_min_matchsize;
    printf(
      "  (Stopping now because best match is within %7.3g of target value.)\n",
      t);
  } else if (g_num_matches >= g_max_matches) {
    if (g_num_matches == 1) {
      printf("  (Stopping now because 1 match was found.)\n");
    } else {
      printf("  (Stopping now because %ld matches were found.)\n",
                                                       (long) g_num_matches);
    }
  } else {
    /* No exit condition was matched. */
    return;
  }

  print_end(EXIT_NO_ERROR);
}

/* unique.eqn takes an equation (in the form of LHS and RHS) and searches the
   list g.matches (which gets filled with all equations that we have decided
   to report as a match) */
int unique_eqn(symbol * lhs, symbol * rhs, int addit)
{
  symbol te[TS_ALLOC_R];
  symbol *where;

  te[0] = 0;
  symstrncat(te, ((symbol *) " "), TS_ALLOC_R);
  symstrncat(te, lhs, TS_ALLOC_R);
  symstrncat(te, ((symbol *) "="), TS_ALLOC_R);
  symstrncat(te, rhs, TS_ALLOC_R);
  symstrncat(te, ((symbol *) " "), TS_ALLOC_R);

  if ((where = symstrsymstr(g_matches, te)), where) {
    /* printf("unique.eqn: found '%s' at position %ld in '%s'\n",
                (char *) te, (long)(where-g_matches), (char *) g_matches); */
    return 0;
  }
  if (addit) {
    symstrncat(g_matches, te+1, (long) g_mtch_alloc);
    /* Carefully track memory usage */
    mem_used_bytes += (sizeof(symbol) * (symstrlen(te+1)+1));
    /* printf("unique.eqn: added '%s', result '%s'\n",
       (char *) te+1, (char *) g_matches); */
  }
  return 1;
}

/* This field size is big enough for certain parts of a result output,
   including the "delta" part of an answer like "1.23456789 = T + 0.1234"
   and the "(1 part in 1023)" bit of a wide output */
#define REPORT_FIELD_SIZE 30

/* Must be big enough for largest value of k_usable_digits */
#define FROOT_SIZE 40

/* report.match does the formatting to print out a match. You can either
   supply an LHS and RHS that are already in the tree, or just one tree
   member and a pe (which you would do if reporting an exact match and
   don't want to insert the item) */
void report_match(symbol * lhs, symbol * rhs, symbol * exm,
       ries_val root, ries_dif delta, int did_newton)
{
  symbol * le;
  symbol * re;
  s16 i;
  char fval[REPORT_FIELD_SIZE];  /* formatted numerical value (before manual left-justify) */
  char froot[FROOT_SIZE]; /* formatted root of equation (value of X, not delta from T) */
  char * x_T_intro;
  s16  posn = 0;  /* "cursor position" for column padding */
  int width;      /* A column width */
  int justify_overflow = 0;
  symbol l_solved[TS_ALLOC_L];       symbol r_solved[TS_ALLOC_R];
  symbol l_escratch[TS_ALLOC_L];     symbol r_escratch[TS_ALLOC_R];
  char l_fscratch[F_ALLOC];          char r_fscratch[F_ALLOC * 2];
  char l_gscratch[MAX_ELEN * MAX_SYM_NAME_LEN];
    char r_gscratch[MAX_ELEN * MAX_SYM_NAME_LEN * 2]; /* *2 for try.solve */
  char * l_fmt;
  char * r_fmt;
  symbol ss;
  s16 err;
  s16 lf_len, rf_len;
  ries_val l_val, r_val; /* NOTE: None of these is used, not even in */
  ries_dif l_dx, r_dx;   /* debug messages, so we could just pass 0's */
  ries_tgs l_tg, r_tg;   /* to eval() */

  /* ignore second and subsequent exact matches. */
  if ((delta == k_0) && got_exact) {
    return;
  }

  /* If not doing refinement, prune based on the delta */
  if (!(g_refinement)) {
    stats_count i;
    ries_val * closeness;
    closeness = g_nr_deltas;
    for(i=0; i<g_num_matches; i++) {
      if (*closeness == delta) {
        if (debug_o) {
          printf("reject4 [%s]=[%s], duplicte delta value\n",
                                                   (char *)lhs, (char *)rhs);
        }
        return;
      }
      closeness++;
    }
    /* It's a new delta value, save it. NOTE: other conditions later in
    /  this routine may cause us to exit, in which case g_num_matches
    /  won't get incremented and this saved delta will get overwritten
    /  later, but that's okay because we only care about saving the
    /  deltas of results that actually get reported. */
    *closeness = delta;

    /* Carefully track memory usage */
    mem_used_bytes += sizeof(ries_val);
  }

  /* If we are not doing refinement, then we should prune duplicate eqns */
  if(!(g_refinement)) {
    if (!(unique_eqn(lhs, rhs, 1))) {
      if (debug_o) {
        printf("reject1 [%s]=[%s], already reported\n",
                                                   (char *)lhs, (char *)rhs);
      }
      return;
    }
  }

  /* Set up le and re to point to the two parts of the equation. */
  if (lhs) {
    le = lhs;
  } else {
    le = exm;
  }

  if (rhs) {
    re = rhs;
  } else {
    re = exm;
  }

  if (g_solve_for_x) {
    if (debug_S || debug_s) {
      /* With either of these options we should print an extra blank line
         for readability */
      printf("\n");
    }
  }

  /* Evaluate both sides, and show work if that option was given */
  if (debug_s) {
    printf("based on:\n");
  }
  /* These calls to eval() use the target value, thus displaying the
     discrepancy that existed before newton solving. */
  eval(le, &l_val, &l_dx, &l_tg, 0, debug_s); /* In report.match() */
  eval(re, &r_val, &r_dx, &r_tg, 0, debug_s);
 /* %%% is re_has_x used? */

  if (debug_s) {
    printf("then equating ");
    expr_print_infix(le, 0);
    printf(" to ");
    expr_print_infix(re, 0);
    printf(",\n");
    printf("  ");
    if (did_newton) {
      printf("and solving for x by the Newton-Raphson method, ");
    }
    printf("I got\n");
    printf("  x = ");
    spfg(k_usable_digits, root); /* printf(fmt_g_usable, root); */
    printf(" = T ");
    if (delta < k_0) {
      printf("- %.6g\n", -delta);
    } else {
      printf("+ %.6g\n", delta);
    }
  }

  if (g_solve_for_x) {
    /* Solve for X and point to resulting expressions */
    try_solve(le, re, l_solved, TS_ALLOC_L, r_solved, TS_ALLOC_R);
    le = l_solved; re = r_solved;

    /* If not doing refinement, do the duplicate pruning again. This catches
       duplicates that were not duplicates in their pre-solved state. */
    if(!(g_refinement)) {
      if (!(unique_eqn(le, re, 1))) {
        if (debug_o) {
          printf("reject2 [%s]=[%s], already reported\n",
                                                   (char *)lhs, (char *)rhs);
        } else if (debug_s) {
          printf("result rejected by unique_eqn() test.\n");
        }
        return;
      }
    }
  }

  /* Format the LHS expression */
  lf_len = 0; err = 0; l_fmt = l_fscratch; l_fmt[0] = 0;
  if (out_expr_format == OF_POSTFIX) {
    err = postfix(le, (char *) l_escratch);
    l_fmt = (char *) l_escratch; lf_len = (s16) strlen(l_fmt);
  } else if ((out_expr_format == OF_CONDENSED)
          || (out_expr_format == OF_NORMAL)) {
    infix_preproc(le, l_escratch);
    err = infix_1(l_escratch, l_fscratch, &ss);
    if (out_expr_format == OF_CONDENSED) {
      l_fmt = l_fscratch; lf_len = (s16) strlen(l_fmt);
    } else {
      lf_len = infix_expand(l_fscratch, l_gscratch);
      l_fmt = l_gscratch;
    }
  } else if (out_expr_format == OF_FORTH) {
    lf_len = postfix_formatter(le, l_gscratch, MAX_ELEN * MAX_SYM_NAME_LEN);
    err = 0; l_fmt = l_gscratch;
  }
  if (err) {
    l_fmt[0] = 0;
  }

  /* Format the RHS expression */
  rf_len = 0; err = 0; r_fmt = r_fscratch; r_fmt[0] = 0;
  if (out_expr_format == OF_POSTFIX) {
    err = postfix(re, (char *) r_escratch);
    r_fmt = (char *) r_escratch; rf_len = (s16) strlen(r_fmt);
  } else if ((out_expr_format == OF_CONDENSED)
          || (out_expr_format == OF_NORMAL)) {
    infix_preproc(re, r_escratch);
    err = infix_1(r_escratch, r_fscratch, &ss);

    /* Now we have both sides reformatted to infix. If we are not doing
       refinement, we should add this to the list. */
    if(!(g_refinement)) {
      if (!(unique_eqn((symbol *)l_fscratch, (symbol *)r_fscratch, 1))) {
        if (debug_o) {
          printf("reject3 [%s]=[%s], already reported\n",
                                (char *) l_fscratch, (char *) r_fscratch);
        } else if (debug_s) {
          printf("result rejected by unique_eqn() test.\n");
        }
        return;
      }
    }

    if (out_expr_format == OF_CONDENSED) {
      r_fmt = r_fscratch; rf_len = (s16) strlen(r_fmt);
    } else {
      rf_len = infix_expand(r_fscratch, r_gscratch);
      r_fmt = r_gscratch;
    }
  } else if (out_expr_format == OF_FORTH) {
    rf_len = postfix_formatter(re, r_gscratch, MAX_ELEN * MAX_SYM_NAME_LEN * 2);
    err = 0; r_fmt = r_gscratch;
  }
  if (err) {
    r_fmt[0] = 0;
  }

  if (debug_s) {
    printf("therefore:\n");
  }

  /* Display the LHS and RHS as an equation, nicely centered. If either side
     can't fit we steal space from the other side. The amount of space we can
     use depends on the g.relative_x option (which affects the width of the
     "x+..." column). The default width values (44 and 40) are chosen to fit
     in 78 and 80 columns, respectively. If we go to a higher precision
     this will need to change significantly. */
  if (g_enable_output) {
    int l_pad, r_pad;
    int t_width, equ_width;

#ifdef RIES_VAL_LDBL
    width = g_relative_x ? 44 : 37; /* Space allocated for LHS and RHS */
#else
    width = g_relative_x ? 44 : 40; /* Space allocated for LHS and RHS */
#endif
    equ_width = 5; /* one space on each end, " = " in the middle */
    t_width = width + equ_width; /* total width of our output */

    /* compute padding (blank space) needed to make LHS and RHS fill their
       allotted space */
    /* In "solve for x" mode we give more to RHS */
    l_pad = (g_one_sided || g_solve_for_x) ? width/8 : width/2;
    r_pad = width - l_pad;
    l_pad -= lf_len; if (l_pad < 0) { l_pad = 0; }
    r_pad -= rf_len; if (r_pad < 0) { r_pad = 0; }

    /* See how wide the result will be; if too wide, steal padding from
       the left, then from the right */
    width = l_pad + lf_len + equ_width + rf_len + r_pad;
    while ((width > t_width) && (l_pad > 0)) {
      l_pad--; width--;
    }
    while ((width > t_width) && (r_pad > 0)) {
      r_pad--; width--;
    }
    /* The result is bigger than t_width only when the equation genuinely won't
       fit in the space we want. Note this for use later. */
    justify_overflow = (width - t_width);
    if (justify_overflow < 0) {
      justify_overflow = 0;
    }

    for(i=0; i<l_pad; i++) {
      printf(" ");
    }
    printf(" %s", l_fmt);

    printf(" %s ", sym_attrs['='].sa_name);

    printf("%s ", r_fmt);
    for(i=0; i<r_pad; i++) {
      printf(" ");
    }
  }

  ries_spfg(froot, FROOT_SIZE, 0, k_usable_digits, root); /* sprintf(froot, fmt_g_usable, root); */

  x_T_intro = (char *) "for x ";
  if (g_enable_output && g_wide_output) {
    /* Wide output mode: display the root first. This mode will have also
       set g.relative_x */
    printf("x = %-20s  ", froot);
    x_T_intro = (char *) "";
  }

  /* We put the "for x = " part in a fixed-size column doing the left-justify
     manually to avoid relying on an extension ("%*" parametrized-width
     format specifier) that is not available in old C compilers. */

  if (delta == k_0) {
    if (g_enable_output) {
      printf("('exact' match)");
    }
    fval[0] = 0;
    posn = 15;
    got_exact = B_TRUE;
  } else if (g_enable_output && (!(g_relative_x))) {
    printf("for x = ");
    ries_spfg(fval, REPORT_FIELD_SIZE, 0, k_usable_digits, root); /* sprintf(fval, fmt_g_usable, root); */
    posn = (s16) (8 + strlen(fval));
  } else if (g_enable_output && (delta < k_0)) {
    /* Delta is negative */
    delta = - delta;
    printf("%s= T - ", x_T_intro);
    snprintf(fval, REPORT_FIELD_SIZE, "%.6g", delta);
    posn = (s16)(6 + strlen(x_T_intro) + strlen(fval) );
  } else if (g_enable_output) {
    /* Delta is positive */
    printf("%s= T + ", x_T_intro);
    snprintf(fval, REPORT_FIELD_SIZE, "%.6g", delta);
    posn = (s16)(6 + strlen(x_T_intro) + strlen(fval) );
  }

  if (g_enable_output) {
    if (g_relative_x) {
      width = 12 + 6 + 5;  /* "for x = T + ", 6 digits, decimal, "e-12" */
    } else {
      width = 8 + k_usable_digits + 6; /* "for x = ", digits, sign/dec/expon */
    }
    printf("%s", fval);

    /* Add any extra chars that came from the equation being larger than its
       allocated space */
    posn = (s16)(posn + justify_overflow);
    /* Add blank space to fill the width */
    for(i=posn; i<width; i++) {
      printf(" ");
    }

    /* Now we're past the printing of (exact match) and/or x, T, delta values
       and can proceed with additional columns */
    if (g_enable_output && g_wide_output) {
      /* Show delta as ratio with respect to T */
      ries_dif ratio;
      char temp[REPORT_FIELD_SIZE];
      ratio = 0;
      if (delta != 0) {
        ratio = (ries_dif) (FABS(g_target / delta));
      }
      if (delta == 0) {
        snprintf(temp, REPORT_FIELD_SIZE, "%s", "(1 part in infinity)");
      } else if (ratio < 100) {
        int r10, r1;
        r10 = (int) ((ratio * 10) + 0.5);
        r1 = r10 % 10; r10 = (r10-r1) / 10;
        snprintf(temp, REPORT_FIELD_SIZE, "(1 part in %d.%d)", r10, r1);
      } else if (ratio < 100000000) {
        snprintf(temp, REPORT_FIELD_SIZE,
                                       "(1 part in %d)", ((int) (ratio+0.5)));
      } else {
        snprintf(temp, REPORT_FIELD_SIZE, "(1 part in %.3e)", ratio);
      }
      printf("%-22s", temp);
    }

    printf(" {%d}\n", complexity(le) + complexity(re));
  }

  g_num_matches++;

  defsym_used(le);
  defsym_used(re);

  check_exit(delta==k_0);
} /* End of report.match() */

/* matchscore = diff / dx */
/* diff = matchscore * dx */

/* check.match reports a match if the RHS is within best.match * dx, where
   dx is the LHS's derivative. */
int check_match(expr * lhs, expr * rhs)
{
  ries_dif delta;  /* The ratio diff / dx. This is how much bigger X
                    would have to be to get a perfect match. */
  ries_dif score;  /* Absolute value of delta */
  ries_val root;
  ries_dif total_deriv;
  s16 err;

  if (debug_o) {
    printf("check_match [%s] ?= [%s]\n", lhs->sym, rhs->sym);
  }

  /* start with a single step of Newton, which we can do easily because
     the x and dx values are already computed */
  total_deriv = lhs->der - rhs->der;
  if (total_deriv == k_0) {
    fprintf(stderr, "check_match got dx = 0!\n");
    return 0;
  }
  delta = (ries_dif) ((rhs->val - lhs->val) / total_deriv);
  score = fabs(delta);
  if (debug_o) {
    printf("check_match score %g, best_match %g, g_mms %g\n",
      score, dbl(best_match), g_min_matchsize);
  }

  if ((score > best_match) || (score < g_min_matchsize)) {
    if (debug_o) {
      printf("            first score %g not good enough\n", score);
    }
    return 0;
  }

  /* If we are not doing refinement, we can check this equation against
     those that have been reported.
     %%% This might actually slow things down! */
  if(!(g_refinement)) {
    if (!(unique_eqn(lhs->sym, rhs->sym, 0))) {
      if (debug_o) {
        printf("            already reported\n");
      }
      return 0;
    }
  }

  /* We have a good candidate for a new match. Now we use Newton's
     method to get a more accurate score. From this point forward we only
     need the symstr part of the expression */
  {
    symbol lexpr[TS_ALLOC_L];
    symbol rexpr[TS_ALLOC_L];

    symstrncpy0(lexpr, lhs->sym, TS_ALLOC_L);
    symstrncpy0(rexpr, rhs->sym, TS_ALLOC_L);

    if (debug_q) {
      printf("            [%s] ~= [%s] (score %g), calling newton\n",
        lexpr, rexpr, score);
    }

    /* cv.simplify removes things from both sides where possible, then
       calls newton */
    err = cv_simplify(lexpr, rexpr, &root, &total_deriv, 0, 1);
    if (err) {
      /* Eval got an error, or Newton did not converge: in either case
         we don't accept, because a failed Newton converge is probably
         a pathological case like sin(1/a) near a=0 */
      if (debug_q) {
        printf("            newton returned %d (%s)\n", err, err_string(err));
      }
      return 0;
    } else {
      if (debug_q) {
        printf("root: ");
        spfg(k_nominal_digits, root); /* printf(fmt_g_nominal, root); */
        printf("\n");
      }
    }

    if (FABS(total_deriv) < k_vanished_dx) {
      if (debug_q) {
        printf("            derivative = %g is too small\n", total_deriv);
      }
    }

    delta = (ries_dif) (root - g_target);
    if (debug_q) {
      printf("newton: target %g root %g delta %g\n", dbl(g_target),
        dbl(root), delta);
    }
    score = fabs(delta);

    if (score == k_0) {
      /* Newton's method revealed that we have an exact match. We'll
         report it (in case it's our first exact match) but not adjust our
         report threshold. */
      if (debug_q) {
        printf("            exact match\n");
      }

      report_match(lexpr, rexpr, 0, root, delta, 1);
      return 1;
    } else if (score < best_match) {
      if (debug_q) {
        printf("            new record %g\n", score);
      }
      report_match(lexpr, rexpr, 0, root, delta, 1);
      if(g_refinement) {
        /* The minus k_min_best.match is to avoid having lots of matches
           that beat each other only because of roundoff in the score
           calculation. This happens if you invoke e.g.
           "ries 0.434294481903252" */
        best_match = (score * 0.999) - k_min_best_match;
        if (debug_q) {
          printf("            lowering match threshold to %g\n",
                   dbl(best_match));
        }
      }
      check_exit(0);
      return 1;
    } else if (debug_q) {
      printf("            post-newton score %g not good enough\n", score);
    }
  }
  return 0;
} /* End of check_match */


/* bt_first gives a pointer to the first node in the tree. Pass in the
   tree's root pointer. */
expr * bt_first(expr * it)
{
  while (it && it->left) {
    it = it->left;
  }
  return it;
}

/* bt_depth returns the depth (number of links down from root) of an item. */
int bt_depth(expr * it)
{
  int rv;

  /* Start at zero */
  rv = 0;

  while(it) {
    rv++;
    it = it->up;
  }

  return(rv);
}

void bt_stats(void);
void bt_stats(void)
{
  long n;
  int tdepth;

  expr * it;
  it = bt_first(lhs_root);
  n = 0; tdepth = 0;
  while(it) {
    if (debug_0) {
      printf("%8ld %10s {%3d} = ", n, it->sym, complexity(it->sym));
      spff(k_nominal_digits, it->val); /* printf(fmt_g_nom_fixed, it->val); */
      if (it->der) {
        printf(", dx = ");
        printf(fmt_g_diff, it->der); /* printf(fmt_g_nominal, it->der); */
      }
      printf("\n");
    }

    /* Accumulate stats */
    tdepth += bt_depth(it);

    it = bt_next(it);
    n++;
  }

  printf("Current tree stats:\n");
  printf("       Nodes: %ld\n", n);
  printf("  Avg. Depth: %f\n", ((stats_count) tdepth) / ((stats_count) n));
}

/* bt_prev traverses the list "backwards" to the next-smaller expression. */
expr * bt_prev(expr *it)
{
  expr *old;

  /* if it has a left child it's relatively easy */
  if (it->left) {
    /* go down left, then down right to dead end */
    it = it->left;
    while (it->right) {
      it = it->right;
    }
  } else {
    /* here we have to traverse up until we get to a node from which we
       were the right. We also need to worry about going all the way off
       the top, which would mean we're done. */
    old = 0;
    while(it && (old == it->left)) {
      old = it;
      it = it->up;
    }
  }

  return it;
}

/* bt_next traverses the list to the next-greater expression. */
expr * bt_next(expr *it)
{
  expr *old;

  /* if it has a right child it's relatively easy */
  if (it->right) {
    /* go down right, then down left to dead end */
    it = it->right;
    while (it->left) {
      it = it->left;
    }
  } else {
    /* here we have to traverse up until we get to a node from which we
       were the left. We also need to worry about going all the way off
       the top, which would mean we're done. */
    old = 0;
    while(it && (old == it->right)) {
      old = it;
      it = it->up;
    }
  }

  return it;
}

/*
check.sides looks for a match for a given expression. It checks
expressions of the opposite type (LHS or RHS) on either side (lower-
and higher-valued) of the supplied expression, and for each, calls
check.match.
 */
void check_sides(expr *it)
{
  expr * other;
  int cm_result;

  /* "it" is the expression that is just now being added to the database.
     Check if it is an RHS or an LHS. */

  if (it->der == k_0) {
    /* New way */
    /* we've got a RHS. Look on both sides for an LHS. If we find another
       RHS, we can stop because that RHS, by definition, will be a
       closer match to any LHS that lies beyond. */
    other = bt_prev(it);
    if (other && (other->der != k_0)) {
      /* "other" is an LHS, test it. */
      cm_result = check_match(other, it);
      if (cm_result && debug_q) {
        printf("(1st) LHS (left) = New RHS\n");
      }
    }

    /* Now we do the same thing again, to the right this time. */
    other = bt_next(it);
    if (other && (other->der != k_0)) {
      cm_result = check_match(other, it);
      if (cm_result && debug_q) {
        printf("(1st) LHS (left) = New RHS\n");
      }
    }

  } else {
    /* we've got an LHS */
    other = bt_prev(it);
    /* Check for an RHS, which always has a zero derivative term */
    if (other && (other->der == k_0)) {
      /* We have an RHS, check it */
      cm_result = check_match(it, other);
      if (cm_result && debug_q) {
        printf("New LHS = first RHS (left)\n");
      }
    }
    /* do the same thing again, to the right this time. */
    other = bt_next(it);
    if (other && (other->der == k_0)) {
      cm_result = check_match(it, other);
      if (cm_result && debug_q) {
        printf("New LHS = first RHS (right)\n");
      }
    }
  }
} /* end of check.sides */

void check_exact_match(expr * it, ries_dif new_dx, pe *ex)
{
  symbol * lhs;
  symbol * rhs;
  ries_val x, xpe;
  ries_dif eps, total_deriv, delta, score;

  lhs = rhs = 0;

  if (new_dx == k_0) {
    /* new item is RHS. */
    if (it->der == k_0) {
      /* Tree item is RHS, too. We just discard in this case. */
      return;
    } else {
      /* Tree item is LHS... */
      if (fabs(it->der) < k_vanished_dx) {
        if (debug_q) {
          printf("chk_ex_match it->dx = %g is too small\n", it->der);
        }
        return;
      } else {
        lhs = it->sym;
        rhs = ex->sym;
        total_deriv = it->der;
      }
    }
  } else {
    /* new item is LHS. */
    if (it->der == k_0) {
      /* tree item is RHS... */
      if (fabs(new_dx) < k_vanished_dx) {
        if (debug_q) {
          printf("chk_ex_match dx = %g is too small\n", new_dx);
        }
        return;
      } else {
        lhs = ex->sym;
        rhs = it->sym;
        total_deriv = 0 - new_dx;
      }
    } else {
      /* Tree item is LHS too, discard. */
      return;
      /* %%% For x-on-both-sides, in this instance we will want to do
         a normal check.match, subtracting the two derivatives and
         rejecting if the result is too small (indicating an excessively
         high correlation or a tautology).
           Even if we get a match, we would still discard the new
         node afterwards because the existing node is more likely
         to yield equations with a lesser combined complexity. */
    }
  }

  x = it->val; eps = ((ries_dif)x) * 1.0e-14; xpe = x + eps;
  if (x == xpe) {
    /* Answer is so small that we underflowed trying to compute epsilon */
    return;
  }
  if (total_deriv == 0) {
    return;
  }
  delta = eps / total_deriv;
  score = fabs(delta);
  if ((best_match > 0) && (score > best_match)) {
    return;
  }
  if (score < g_min_matchsize) {
    return;
  }
  if(!(g_refinement)) {
    if (!(unique_eqn(lhs, rhs, 0))) {
      return;
    }
  }

  report_match(lhs, rhs, ex->sym, g_target, (ries_dif) k_0, 0);
}

/* bt.insert adds an expression to the tree. The dx parameter
   is used to determine if it's an RHS or an LHS expression. */
s16 bt_insert(ries_val x, ries_dif dx, ries_tgs tg, pe *ex, s16 * res1)
{
  expr * it;
  s16  going, insert, fillin, i;

  fillin = 0; going = 0; insert = 0; *res1 = 0;
  it = lhs_root;
  /* If there's a tree to descend, descend it. */
  if (it) {
    going = 1;
    insert = 1;
  } else {
    /* insert and copy first node */
    lhs_root = (expr *) my_alloc(sizeof(expr));
    if (lhs_root == 0) {
      return 1;
    }
    *res1 = 1;
    insert_count++;
    it = lhs_root;
    it->up = 0;
    going = 0; /* no descending to do */
    insert = 0;  /* and we just inserted */
    fillin = 1; /* but we need to fill it in */
  }
  while(going) {
    if (it->val == x) {
      /* Exact match: there is already a node with the exact same value.
         We never insert another node with the same value, because by
         definition (due to the way we generate simpler expressions first)
         any equation made with the newly inserted node would be more complex
         than the existing equation we can get with the existing, simpler
         node.
           However, we do take the opportunity to report an exact match,
         checking the derivative of the side that contains X to avoid
         reporting a tautology. */
      going = 0;
      insert = 0;

      check_exact_match(it, dx, ex); /* BT_CODE_MATCH */
    } else {
      /* no match yet: descend. */
      if (x < it->val) { /* BT_CODE_CMP */
        /* go to left child */
        if (it->left) {
          it = it->left;
        } else {
          /* no left: that means we insert here. */
          insert = -1;
          going = 0;
        }
      } else {
        /* go to right child */
        if (it->right) {
          it = it->right;
        } else {
          /* no right: that means we insert here. */
          insert = 1;
          going = 0;
        }
      }
    }
  }

  if (insert) {
    expr *n;

    n = (expr *) my_alloc(sizeof(expr));
    if (n == 0) {
      return 1;
    }
    *res1 = 1;
    insert_count++;
    if (insert > 0) {
      it->right = n;
      n->up = it;
    } else {
      it->left = n;
      n->up = it;
    }
    it = n;
    fillin = 1;
  }
  if (fillin) {
    /* copy the expression into the new node (BT_CODE_FILLIN) */
    it->val = x;
    it->der = dx;
    it->tags = tg;
    it->left = 0;
    it->right = 0;
    it->elen = ex->elen;
    for(i=0; i<=it->elen; i++) {
      it->sym[i] = ex->sym[i];
    }
  }

  /* Last thing to do is to check for a new match. */
  if (insert) {
    check_sides(it); /* BT_CODE_INSERTED */
  }

  return 0;
}

s16    g_dbg_side;

#define CANONVAL_NEGATE     1
#define CANONVAL_RECIPROCAL 2
#define CANONVAL_DIV2       4
#define CANONVAL_MUL2       8
int g_canon_ops;

/*

canon.val takes an expression which should be complete, and tries to
append additional operators to make its value fall within the range
[1.0,2.0).

There are 4 types of transformations that we try to make, and they are
partly redundant: [r] and [2*] are only used if the value is in the
range (-1.0,1.0), but if [r] is used then the value will not be in
that range anymore, and [2*] will not trigger. This redundancy is
there to maximize the effectiveness of canon.val when the symbolset has
been restricted via the -S/-O/-N options.

We check sym_attrs[*].count vs. sym_attrs[*].allowed, but don't bother
to update the counts because these are the last symbols that will be
added and we don't add more than one of any symbol.

All parameters except muc_ptr are both inputs and return values.
*/
s16 canonval(
  pe * bpe,       /* The expression to operate on */
  metastack * ms, /* The expression's metastack */
  ries_val *p_x,    /* Value of the expression */
  ries_dif *p_dx,   /* Derivative */
  ries_tgs *p_tg,   /* tags */
  s16 *muc_ptr,   /* Metastack undo count */
  s16 using_x     /* Nonzero if we're currently generating expressions
                     for the LHS tree. */
 )
{
  s16 muc = 0;
  s16 uc;
  s16 ip = bpe->elen;
  s16 sp;
  s16 exec_err;
  ries_val x;
  ries_dif dx;
  ries_tgs tg;

  g_cv_calls++;
  if (g_canon_ops == 0) {
    *muc_ptr = muc;
    return 0;
  }

  x = *p_x; dx = *p_dx; tg = *p_tg;

  if (debug_F & g_dbg_side) {
    printf("canonval: ip %d, el %d [%s] val=%g, dv=%g\n",
      ip, bpe->elen, bpe->sym, dbl(x), dx);
  }

  if ((g_canon_ops & CANONVAL_NEGATE)
    && (sym_attrs['n'].sa_ct < sym_attrs['n'].sa_alwd)
    && (x < 0.0) && (ip < MAX_ELEN))
  {
    /* Negate */
    bpe->sym[ip++] = 'n';
    exec_err = exec(ms, bpe->sym[ip-1], &uc, using_x); muc=(s16)(muc+uc);
    if (exec_err) {
      *muc_ptr = muc;
      return exec_err;
    }
    x = ms_peek(ms, &dx, &tg, &sp);
    if (debug_F & g_dbg_side) {
      bpe->sym[ip] = 0;
      printf("   neg -> ip %d, el %d [%s] val=%g, dv=%g, tg %x\n",
       ip, bpe->elen, bpe->sym, dbl(x), dx, tg);
    }
  }

  if ((g_canon_ops & CANONVAL_RECIPROCAL)
    && (sym_attrs['r'].sa_ct < sym_attrs['r'].sa_alwd)
    && (x*x < 1.0) /* faster way to test if (fabs(x) < 1.0) */
    && (ip < MAX_ELEN))
  {
    /* Take the reciprocal */
    bpe->sym[ip++] = 'r';
    exec_err = exec(ms, bpe->sym[ip-1], &uc, using_x); muc=(s16)(muc+uc);
    if (exec_err) {
      *muc_ptr = muc;
      return exec_err;
    }
    x = ms_peek(ms, &dx, &tg, &sp);
    if (debug_F & g_dbg_side) {
      bpe->sym[ip] = 0;
      printf(" recip -> ip %d, el %d [%s] val=%g, dv=%g, tg %x\n",
        ip, bpe->elen, bpe->sym, dbl(x), dx, tg);
    }
  }

  if ((g_canon_ops & CANONVAL_DIV2)
    && (sym_attrs['/'].sa_ct < sym_attrs['/'].sa_alwd)
    && (sym_attrs['2'].sa_ct < sym_attrs['2'].sa_alwd)
    && (x*x >= 4.0) /* faster way to test if (fabs(x) >= 2.0) */
    && (ip+1 < MAX_ELEN))
  {
    /* Divide by 2 */
    bpe->sym[ip++] = '2';
    exec_err = exec(ms, bpe->sym[ip-1], &uc, using_x); muc=(s16)(muc+uc);
    if (exec_err) {
      *muc_ptr = muc;
      return exec_err;
    }
    bpe->sym[ip++] = '/';
    exec_err = exec(ms, bpe->sym[ip-1], &uc, using_x); muc=(s16)(muc+uc);
    if (exec_err) {
      *muc_ptr = muc;
      return exec_err;
    }
    x = ms_peek(ms, &dx, &sp, &tg);
    if (debug_F & g_dbg_side) {
      bpe->sym[ip] = 0;
      printf("    2/ -> ip %d, el %d [%s] val=%g, dv=%g, tg %x\n",
        ip, bpe->elen, bpe->sym, dbl(x), dx, tg);
    }
  }

  if ((g_canon_ops & CANONVAL_MUL2)
    && (sym_attrs['*'].sa_ct < sym_attrs['*'].sa_alwd)
    && (sym_attrs['2'].sa_ct < sym_attrs['2'].sa_alwd)
    && (x*x < 1.0) /* faster way to test if (fabs(x) < 1.0) */
    && (ip+1 < MAX_ELEN))
  {
    /* Multiply by 2 */
    bpe->sym[ip++] = '2';
    exec_err = exec(ms, bpe->sym[ip-1], &uc, using_x); muc=(s16)(muc+uc);
    if (exec_err) {
      *muc_ptr = muc;
      return exec_err;
    }
    bpe->sym[ip++] = '*';
    exec_err = exec(ms, bpe->sym[ip-1], &uc, using_x); muc=(s16)(muc+uc);
    if (exec_err) {
      *muc_ptr = muc;
      return exec_err;
    }
    x = ms_peek(ms, &dx, &sp, &tg);
    if (debug_F & g_dbg_side) {
      bpe->sym[ip] = 0;
      printf("    2* -> ip %d, el %d [%s] val=%g, dv=%g, tg %x\n",
        ip, bpe->elen, bpe->sym, dbl(x), dx, tg);
    }
  }

  /* We made it through the exec()s without error, so now we'll save the
     results of our calculations. */
  bpe->elen = ip;
  *p_x = x; *p_dx = dx; *p_tg = tg;
  *muc_ptr = muc;
  return 0;
} /* End of canon.val */

void decanon(metastack * ms, s16 muc)
{
  while(muc) {
    ms_undo(ms);
    muc--;
  }
}

/* ge_2 is the core code for generating expressions from a form.

   base -----bpe-----
        comp elen syms
   ab   0    0    -
   ab   10   1    1
   ab   17   2    1l
   ab   17   2    1n
   ab   13   1    2
   ab   20   2    2r
   ab   20   2    2q
   ab   20   2    2l
*/
stats_count ge_2(
         form *base,
         pe *bpe,
         s16 e_minw, s16 e_maxw,
         metastack *ms,
         s16 using_x  /* Nonzero if we're currently generating expressions
                         for the LHS tree. */
 )
{
  symbol seft, sym;
  symbol *syms;
  s16    ns;
  s16    in_cpx;  /* Complexity of input partial expression */
  s16    rminw, rmaxw;
  s16    ip;
  stats_count n;
  s16    recurse;
  attr_bits atts;
  s16    muc;     /* metastack undo count */
  ries_val curtop; ries_dif ctdx; ries_tgs cttg;
  s16    err;
  s16    cur_sp;

  n = 0; muc = 0;

  ip = bpe->elen;

  /* ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' "
                         Exec symbol and handle errors
   On entry, ge.2() usually has a symbol that was just added by the parent
   instance of ge.2(). We start by exec'ing this symbol, then return if any
   type of error happened.
   ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " */

  if (ip > 0) {
    g_exec_calls++;
    /* if there are errors like [0/] or [1nq], we'll return right away,
       thereby pruning */
    err = exec(ms, bpe->sym[ip-1], &muc, using_x);
    if (err) {
      if (debug_A & g_dbg_side) {
        bpe->sym[ip] = 0;
        printf("prune partial exec error [%s) got %d\n", bpe->sym, err);
      }
      while(muc) { ms_undo(ms); muc--; }
      return 0;
    }

    /* Now find out what the current value on the stack is */
    curtop = ms_peek(ms, &ctdx, &cttg, &cur_sp);

    /* We can always prune zero subexpressions. Any solution that
       contains a zero subexpression can be reduced to a simpler
       solution that does not contain a zero subexpression. */
    if (curtop == k_0) {
      if (debug_B & g_dbg_side) {
        bpe->sym[ip] = 0;
        printf("prune partial zero [%s) = ", bpe->sym);
        spfg(k_usable_digits, curtop); /* printf(fmt_g_usable, curtop); */
        printf("\n");
      }
      while(muc) { ms_undo(ms); muc--; }
      prune_count += 1.0;
      return 0;
    }

    /* Prune non-integer subexpressions if the -i option was given. */
    if (g_restrict_subexpr == TYPE_INT) {
      if (!TAG_INT_P(cttg)) {
        if (debug_C & g_dbg_side) {
          bpe->sym[ip] = 0;
          printf("prune partial noninteger [%s) = ", bpe->sym);
          spfg(k_nominal_digits, curtop); /* printf(fmt_g_nominal, curtop); */
          printf(" %s", tagname(cttg));
          printf("\n");
        }
        while(muc) { ms_undo(ms); muc--; }
        prune_count += 1.0;
        return 0;
      }
    }

    /* Similar pruning for less-restrictive values of g_restrict_subexpr.
       If an option like "-a" is used without modification, this test is
       redundant because the other restrictions e.g. g_restrict_exponents
       and the symbolset ensure that functions results remain within the
       restricted class. */
    if (cttg < g_restrict_subexpr) {
      if (debug_C & g_dbg_side) {
        bpe->sym[ip] = 0;
        printf("prune partial g_restr_sub [%s) = ", bpe->sym);
        spfg(k_nominal_digits, curtop);
        printf(" %s", tagname(cttg));
        printf("\n");
      }
      while(muc) { ms_undo(ms); muc--; }
      prune_count += 1.0;
      return 0;
    }

    /* %%% here we could do a bt_find to search for the value curtop, and
       if an exact match is found, confirm that the derivative is also equal,
       then check if the trailing N opcodes of our PE are equal to that
       found (if any). If not, our PE is a redundant and possibly more
       complex way to generate a value, and can be pruned. This should be
       a significant optimization. It covers the same ground as the AM_
       rules, but catches many cases they miss (like multiple equivalent
       sums: 37+, 28+, 19+, 136++, 136n-+, and on and on... */

    /* %%% this could be done more efficiently inside exec() */
    /* Any subexpression that overflows either in value or in the
       derivative causes pruning. */
    if ((curtop >= p_ovr) || (curtop <= n_ovr)) {
      if (debug_D & g_dbg_side) {
        bpe->sym[ip] = 0; /* Do not display not-yet-exec'd symbols */
        printf("prune partial overflow [%s) = ", bpe->sym);
        spfg(k_nominal_digits, curtop); /* printf(fmt_g_nominal, curtop); */
        printf("\n");
      }
      while(muc) { ms_undo(ms); muc--; }
      prune_count += 1.0;
      return 0;
    }

    /* ignore any LHS partial-expressions with nonzero but very small
       derivative (tautology problem from roundoff error) */
    if (using_x) {
      /* formerly tested "(fabs(ctdx) > k_0) && (fabs(ctdx) < k_prune_deriv)"
         pp. 20111228 was "(fabs(ctdx)/(1.0 + fabs(curtop)) < k_prune_deriv)"
         Test the current top of stack for bogus derivative. We have to
         test ctdx != 0 to ensure we don't prune constants, like pruning the
         "2" in "x2+" before the "+" has been executed. The check for
         full-expression tautologies, e.g. pruning "x^2-x*x", happens
         in the next block. */
      if ( (ctdx != k_0) && (FABS(ctdx)/(1.0 + FABS(curtop)) < k_vanished_dx) )
      {
        bpe->sym[ip] = 0; /* Do not display not-yet-exec'd symbols */
        if (debug_B & g_dbg_side) {
          printf("prune partial dx~=0 [%s) = ", bpe->sym);
          spfg(k_usable_digits, curtop); /* printf(fmt_g_usable, curtop); */
          printf(", d/dx = ");
          printf(fmt_g_diff, ctdx);
          printf(" %s", tagname(cttg));
          printf("\n");
        }
        while(muc) { ms_undo(ms); muc--; }
        prune_count += 1.0;
        return 0;
      }

      /* Ignore complete LHS expressions with zero derivative (tautology
         problem). This catches all normal non-roundoff tautologies, like
         "-x/x" and "(x-4)-(x-1)" */
      if ( (cur_sp == 0) && (FABS(ctdx)/(1.0 + FABS(curtop)) < k_vanished_dx) )
      {
        bpe->sym[ip] = 0; /* Do not display not-yet-exec'd symbols */
        if (debug_B & g_dbg_side) {
          printf("prune full.1 dx~=0 [%s] = ", bpe->sym);
          spfg(k_usable_digits, curtop); /* printf(fmt_g_usable, curtop); */
          printf(", d/dx = ");
          printf(fmt_g_diff, ctdx);
          printf(" %s", tagname(cttg));
          printf("\n");
        }
        while(muc) { ms_undo(ms); muc--; }
        prune_count += 1.0;
        return 0;
      }
    } /* End of "if (using_x)" */
  } /* End of "if (ip > 0)" */

  /* ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' "
                         Handling complete expressions
   If the just-exec'd symbol makes the expression complete, we try to insert
   it in the database.
   ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " */

  if (ip >= base->flen) {
    s16 res1;
    /* it's now as long as it can get. */

    /* We still have values of curtop, dx, and cur_sp from calling
       ms_peek() above */
    bpe->sym[ip] = 0; /* Needed by bt.insert */

    /* Implement the --find.expression option */
    if (g_num_find_expr) {
      s16 i;
      for(i=0; i<g_num_find_expr; i++) {
        if(symstrcmp(bpe->sym, g_find_expr[i]) == 0) {
          printf(" [%s] = ", bpe->sym);
          spfg(k_usable_digits, curtop); /* printf(fmt_g_usable, curtop); */
          if (ctdx != 0) {
            printf(", d/dx = %g", ctdx);
          }
          printf(" %s", tagname(cttg));
          printf(", complexity = {%d}\n", bpe->cplx);
        }
      }
    }

    if (using_x && (fabs(ctdx) < k_vanished_dx)) {
      if (debug_B & g_dbg_side) {
        printf("prune full.2 dx~=0 [%s) = ", bpe->sym);
        spfg(k_usable_digits, curtop); /* printf(fmt_g_usable, curtop); */
        printf(", d/dx = ");
        printf(fmt_g_diff, ctdx);
        printf(" %s", tagname(cttg));
        printf("\n");
      }
      while(muc) { ms_undo(ms); muc--; }
      prune_count += 1.0;
      return 0;
    } else if (curtop < g_min_equ_val) {
      if (debug_B & g_dbg_side) {
        printf("prune equval [%s) = ", bpe->sym);
        spfg(k_usable_digits, curtop);
        printf(" too low\n");
      }
      while(muc) { ms_undo(ms); muc--; }
      prune_count += 1.0;
      return 0;
    } else if (curtop > g_max_equ_val) {
      if (debug_B & g_dbg_side) {
        printf("prune equval [%s) = ", bpe->sym);
        spfg(k_usable_digits, curtop);
        printf(" too high\n");
      }
      while(muc) { ms_undo(ms); muc--; }
      prune_count += 1.0;
      return 0;
    } else {
      s16 uc; /* Metastack undo count for canonval operation */
      s16 oip;
      oip = bpe->elen;
      /* Bring value into canonical range */
      err = canonval(bpe, ms, &curtop, &ctdx, &cttg, &uc, using_x);
      if (err) {
        if (debug_G & g_dbg_side) {
          bpe->sym[ip] = 0;
          printf("prune canonval exec error [%s) got %d\n", bpe->sym, err);
        }
        while(uc) { ms_undo(ms); uc--; }
        return 0; /* Don't count this as a valid generated expression. */
      }
      /* Looks good so far, now try to insert in tree */
      bpe->sym[bpe->elen] = 0; /* Needed by bt.insert */
      if (bt_insert(curtop, ctdx, cttg, bpe, &res1)) { /* this is in ge_2 */
        fprintf(stderr, "%s: Out of memory\n", g_argv0);
        print_end(1);
      }

      if (res1 == 0) {
        if (debug_E & g_dbg_side) {
          printf("reject [%s] = ", bpe->sym);
          spfg(k_usable_digits, curtop); /* printf(fmt_g_usable, curtop); */
          printf(" (duplicate value)\n");
        }
        /* We do not undo and return 0 here, because we want to count this as a
           "generated value". So we fall through to the "return 1;" below. */
      } else if (debug_G & g_dbg_side) {
        /* We do this debug print in infix, because it's so useful for things
           like OEIS sequence A005245. */
        printf("ge_2 inserted ");
        expr_print_infix(bpe->sym, 0);
        printf(" = ");
        spfg(k_usable_digits, curtop); /* printf(fmt_g_usable, curtop); */
        if (ctdx != 0) {
          printf(", d/dx = ");
          printf(fmt_g_diff, ctdx);
        }
        printf(" %s", tagname(cttg));
        printf(" {%d}\n", bpe->cplx);
      }

      /* Undo the canonval ops */
      decanon(ms, uc);
      bpe->elen = oip;
    }

    /* undo stack manipulation and return */
    while(muc) { ms_undo(ms); muc--; }
    return 1; /* We generated 1 item */
  } /* End of "if (ip >= base->flen)" */

  /* ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' "
                         Generate and add the next symbol(s)
   We reach here if we have an incomplete expression. This is also the first
   code executed in the case where there is no expression yet (initial call
   from ge.1())
   ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " */

  /* get seft and comp */
  seft = base->sym[ip];
  in_cpx = bpe->cplx;

  /* set up our variables for generation.
   Here we get a pointer to the list of symbols of the seft for this
   position in the form.
     %%% solve-for-x: If we're generating LHS's, this list will be either
         the LHS symlist or the RHS list, depending on the value of cur_sp */
  syms = 0; ns = 0;
  switch(seft) {
  case 'a':
    syms = g_asym; ns = n_asym;
    break;
  case 'b':
    syms = g_bsym; ns = n_bsym;
    break;
  case 'c':
    syms = g_csym; ns = n_csym;
    break;
  }

  /* find out how much weight might be added to complete this expression.
  these stats were pre-computed at the start of the form (by ge.1) */
  rminw = bpe->pe_rminw[ip+1];
  rmaxw = bpe->pe_rmaxw[ip+1];

  /* ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' "
                            Prune pattern recognition
   Here we look at the recentmost few symbols and match them against all
   the special patterns used for pruning.
   ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " */

  /* calculate the attributes for the symbols we have so far */
  if (ip > 0) {
    if (debug_H & g_dbg_side) {
      bpe->sym[ip] = 0;
      printf("attributes for [%s)%c: ", bpe->sym, seft);
    }
  }
  atts = using_x ? 0 : AM_RHS;
  /* no debug print for using_x because it's pretty obvious */

  if (ip > 2) {
    /* Three-symbol patterns */
    if (   (base->sym[ip-1] == 'a')
        && (base->sym[ip-2] == 'c')
        && (base->sym[ip-3] == 'a') ) {
      /* We have ...aca */
      if (debug_H & g_dbg_side) {
        printf(" (%c%c%c)", bpe->sym[ip-3], bpe->sym[ip-2], bpe->sym[ip-1]);
      }
      /* KxK rule is true if we do K * K with the same constant both times. */
      if (bpe->sym[ip-1] == bpe->sym[ip-3]) {
        if (bpe->sym[ip-2] == '*') {
          atts |= AM_KxK;
          if (debug_H & g_dbg_side) { printf(" K*K"); }
        } else if (bpe->sym[ip-2] == '+') {
          atts |= AM_KpK;
          if (debug_H & g_dbg_side) { printf(" K+K"); }
        }
      }
    }
  }

  if (ip > 1) {
    /* two-symbol patterns */

    if (seft == 'c') {
      s16 a1i;

      /* next symbol will be a binary operator; look at arg1 */
      a1i = base->arg1[ip];
      if (a1i >= 0) {
        if (bpe->sym[a1i] == 'e') {
          atts |= AM_a1_e;
          if (debug_H & g_dbg_side) { printf(" e..<op>"); }
        } else if (bpe->sym[a1i] == '1') {
          atts |= AM_a1_1;
          if (debug_H & g_dbg_side) { printf(" 1..<op>"); }
        } else if (bpe->sym[a1i] == 'r') {
          atts |= AM_a1_r;
          if (debug_H & g_dbg_side) { printf(" r..<op>"); }
        }
      }
    }

    /* currently, all two-symbol patterns are for (aa) forms */
    if ((base->sym[ip-1] == 'a')
        && (base->sym[ip-2] == 'a')) {
      if (debug_H & g_dbg_side) {
        printf(" (%c%c)", bpe->sym[ip-2], bpe->sym[ip-1]);
      }

      /* KK rule is true if the same constant occurs twice in a row. */
      if (bpe->sym[ip-1] == bpe->sym[ip-2]) {
        atts |= AM_KK;
        if (debug_H & g_dbg_side) { printf(" KK"); }
      }

      /* 55 rule is true if both constants are integers less than or equal
       * to 5. */
      if ((bpe->sym[ip-1] < '6')
          && (bpe->sym[ip-2] < '6')) {
        atts |= AM_55;
        if (debug_H & g_dbg_side) { printf(" 55"); }
      }

      /* 1K rule is true if first constant is 1 */
      if (bpe->sym[ip-2] == '1') {
        atts |= AM_1K;
        if (debug_H & g_dbg_side) { printf(" 1K"); }
      }

      /* jK rule: true if smaller constant is followed by larger constant.
       * For these purposes, the noninteger constants are considered
       * larger than all the integers. (This can be changed if necessary
       * by adding an indirection array defining symbol sequence, or by
       * using the symbol weights as a sequencing measure -- but for now,
       * it works fine this way.) */
      if (bpe->sym[ip-1] > bpe->sym[ip-2]) {
        atts |= AM_jK;
        if (debug_H & g_dbg_side) { printf(" jK"); }
      }
    }
  }

  if (ip > 0) {
    /* one-symbol patterns */

#if 0
    if (debug_H & g_dbg_side) {
      printf(" (%c)", bpe->sym[ip-1]);
    }
    if (bpe->sym[ip-1] == '1') {
      atts |= AM_1;
      if (debug_H & g_dbg_side) { printf(" 1"); }
    } else if (bpe->sym[ip-1] == '2') {
      atts |= AM_2;
      if (debug_H & g_dbg_side) { printf(" 2"); }
    } else if (bpe->sym[ip-1] == 'n') {
      atts |= AM_n;
      if (debug_H & g_dbg_side) { printf(" n"); }
    } else if (bpe->sym[ip-1] == 'r') {
      atts |= AM_r;
      if (debug_H & g_dbg_side) { printf(" r"); }
    } else if (bpe->sym[ip-1] == 'l') {
      atts |= AM_l;
      if (debug_H & g_dbg_side) { printf(" l"); }
    } else if (bpe->sym[ip-1] == 'E') {
      atts |= AM_E;
      if (debug_H & g_dbg_side) { printf(" E"); }
    } else if (bpe->sym[ip-1] == 'p') {
      atts |= AM_pi;
      if (debug_H & g_dbg_side) { printf(" p"); }
    } else if ((bpe->sym[ip-1] == 's')
               || (bpe->sym[ip-1] == 'q')) {
      atts |= AM_sq;
      if (debug_H & g_dbg_side) { printf(" sq"); }
    }
    if (debug_H & g_dbg_side) { printf("\n"); }
#else
    atts |= sym_attrs[bpe->sym[ip-1]].amkey;
    if (debug_H & g_dbg_side) {
      printf(" (%c)", bpe->sym[ip-1]);
      if (atts & AM_1) { printf(" 1"); }
      if (atts & AM_2) { printf(" 2"); }
      if (atts & AM_n) { printf(" n"); }
      if (atts & AM_r) { printf(" r"); }
      if (atts & AM_l) { printf(" l"); }
      if (atts & AM_E) { printf(" E"); }
      if (atts & AM_pi) { printf(" p"); }
      if (atts & AM_sq) { printf(" sq"); }
      if (atts & AM_plus) { printf(" +"); }
      if (atts & AM_mul) { printf(" *"); }
      if (atts & AM_pow) { printf(" ^"); }
      printf("\n");
    }
#endif
  }

  /* ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' "
                            Symbol-append loop
   Here we look at all the eligible symbols, and for each one we check the
   complexity limits to see if the symbol can be added to our partial
   expression.
   ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " ' " */

  bpe->elen = (s16)(ip + 1);
  if (debug_I & g_dbg_side) { printf("%d symbols to try.\n", ns); }
  while (ns > 0) {
    s16  t_cpx;  /* Test complexity with each candidate symbol */

    /* get next symbol and see what this does to our complexity score */
    sym = *syms++;
    t_cpx = (s16)(in_cpx + sym_attrs[sym].sa_wgt);
    if (debug_I & g_dbg_side) {
      bpe->sym[ip] = 0;
      printf("trying [%s) . '%c' {%d}:\n", bpe->sym, sym, t_cpx);
    }

    /* begin pruning */
    recurse = 1;
    if (0) {

    /* Can the remaining symbols make a full expression that is
       within the global complexity limits? */
    } else if (t_cpx + rminw > e_maxw) {
      if (debug_J & g_dbg_side) {
        printf("prune complexity {%d} + rminw[%d] {%d} > e_maxw {%d}\n",
               t_cpx, ip+1, rminw, e_maxw);
      }
      recurse = 0;
    } else if (t_cpx + rmaxw < e_minw) {
      if (debug_J & g_dbg_side) {
        printf("prune complexity {%d} + rmaxw[%d] {%d} < e_minw {%d}\n",
               t_cpx, ip+1, rmaxw, e_minw);
      }
      recurse = 0;

    /* does this symbol generate a stupid combination? */
    } else if (atts & sym_attrs[sym].sa_mask) {
      if (debug_K & g_dbg_side) {
        printf("prune on symbol rules: ");
        if (ip > 2) putchar(bpe->sym[ip-3]);
        if (ip > 1) putchar(bpe->sym[ip-2]);
        if (ip > 0) putchar(bpe->sym[ip-1]);
        printf(":%c atts %x & mask %x == %x reject\n", sym, atts,
          sym_attrs[sym].sa_mask, atts & sym_attrs[sym].sa_mask);
      }
      recurse = 0;

    /* LHS expressions always start with 'x' */
    } else if (using_x && (ip == 0) && (sym != 'x')) {
      if (debug_K & g_dbg_side) {
        printf("prune LHS must start with 'x'\n");
      }
      recurse = 0;

    /* Check symbol count for this symbol */
    } else if (sym_attrs[sym].sa_ct >= sym_attrs[sym].sa_alwd) {
      if (debug_L & g_dbg_side) {
        printf("prune symcount[%c]\n", sym);
      }
      recurse = 0;
    }

    if (recurse) {
      /* we're going to use this symbol: write it into the BPE */
      bpe->sym[ip] = sym;
      sym_attrs[sym].sa_ct = (s16)(sym_attrs[sym].sa_ct + 1);
      /* update complexity */
      bpe->cplx = t_cpx;

      /* okay, it's all set to recurse */
      n += ge_2(base, bpe, e_minw, e_maxw,
                                  ms, using_x); /* ge_2 self-recursive call */

      /* undo the writing of this symbol. */
      sym_attrs[sym].sa_ct = (s16)(sym_attrs[sym].sa_ct - 1);
    } else {
      prune_count += 1.0;
    }

    ns--;
  } /* End of "while (ns > 0)" */

  /* undo the damage */
  bpe->elen = ip;
  bpe->cplx = in_cpx;

  /* we're done with the IR's of this opcode */
  while(muc) { ms_undo(ms); muc--; }

  return n;
} /* End of ge.2 */

/* ge.1() takes a complete form like "aabacbc" and sets up the data
   structures needed by ge.2() to generate and execute all valid
   expressions (like [32s1+s*]) that fit that form. This includes a
   metastack, lists of complexity ranges, etc. */
stats_count ge_1(form *base, s16 e_minw, s16 e_maxw, s16 using_x,
  s16 a_minw, s16 a_maxw, s16 b_minw, s16 b_maxw, s16 c_minw, s16 c_maxw)
  /* %%% For proper solve-for-x with restricted symbol sets, the
  {a|b|c}_{min|max}w variables will have been generated from two sets of
  {a|b|c}_{min|max}w variables depending on whether we are generating an
  RHS or an LHS  */
{
  pe  bpe;
  metastack ms;
  s16 blen;
  stats_count n;
  s16 i, rminw, rmaxw;
  symbol sym;
  long start_ins_count;

  blen = base->flen;

  /* put a null at the end, used by the debug prints */
  (*base).sym[blen] = 0;

  if (debug_t) {
    printf("ge_1 on form {%s}\n", (char *) (&((*base).sym[0])));
    printf("%3d >= %3d <= {%s} <= %3d >= %3d\n", e_maxw, base->min_weight,
           (char *) (&((*base).sym[0])), base->max_weight, e_minw);
  }

  n = 0;

  /* set up the pe struct */
  bpe.cplx = 0;
  bpe.elen = 0;

  /* calculate and fill in the rmimw (remaining minimum weight) and
  rmaxw (remaining maximum weight) fields. For example, if the form
  is [abac] (length 4), the pe_rminw and pe_rmaxw arrays get set up
  like this:
               i    0   1   2   3   4
      symbol seft   a   b   a   c   -
      pe_rminw[i]  31  21  14   4   0
      pe_rmaxw[i]  63  44  28   9   0

  Each element of the array tells how much complexity might be added
  by the symbols at that position and later in the expression. For example,
  position 3 is seft c, and the minw and maxw values are 4 and 9. This
  is the symbol weight range for seft c (ranging from 4 points for [*]
  to 9 points for [L])
    %%% For proper solve-for-x with restricted symbol sets, the
  {a|b|c}_{min|max}w variables will have been generated from two sets of
  {a|b|c}_{min|max}w variables depending on whether we are generating an
  RHS or an LHS  */
  if (debug_u) {
    printf("setting up rminw and rmaxw arrays for form [%s] (length %d):\n",
      (*base).sym, blen);
  }
  rminw = rmaxw = 0;
  i=blen;
  if (debug_u) {
    printf("  position %d (no sym): rminw[%d] = %d, rmaxw[%d] = %d.\n",
      i, i, rminw, i,rmaxw);
  }
  bpe.pe_rminw[i] = rminw;
  bpe.pe_rmaxw[i] = rmaxw;
  while(i>0) {
    i--;
    sym = (*base).sym[i];
    switch(sym) {
    case 'a':
      rminw = (s16)(rminw + a_minw);
      rmaxw = (s16)(rmaxw + a_maxw);
      break;
    case 'b':
      rminw = (s16)(rminw + b_minw);
      rmaxw = (s16)(rmaxw + b_maxw);
      break;
    case 'c':
      rminw = (s16)(rminw + c_minw);
      rmaxw = (s16)(rmaxw + c_maxw);
      break;
    }
    if (debug_u) {
      printf("  position %d, seft %c:  rminw[%d] = %d, rmaxw[%d] = %d.\n",
        i, sym, i, rminw, i,rmaxw);
    }
    bpe.pe_rminw[i] = rminw;
    bpe.pe_rmaxw[i] = rmaxw;
  }

  /* set up the metastack */
  ms_init(&ms);

  /* generate! */
  start_ins_count = insert_count;
  n = ge_2(base, &bpe, e_minw, e_maxw, &ms, using_x); /* this is in ge_1 */

  if(debug_v) {
    printf("form %s generated %ld expressions", base->sym, (long) n);
    if (insert_count > start_ins_count) {
      printf(" and inserted %ld", insert_count - start_ins_count);
    }
    printf(".\n");
  }
  return n;
}

/* long g_dstats[MAX_ELEN]; */

/* generate forms by simple recursive algorithm.

   ------base------  ------next------
   flen sym-- stack  flen sym-- stack  Comments
   0    -     0                        Initial call
   0    -     0      1    a     1      Setting up call to myself
   1    a     1                        Entering recursive invocation
   1    a     1      2    aa    2      Setting up call to myself
    . . .                              (. . .)
   1    a     1                        Entering recursive invocation
   1    a     1      2    ab    1      Setting up another call to myself
*/
stats_count gf_1(form *base, s16 e_minw, s16 e_maxw, s16 using_x,
  s16 a_minw, s16 a_maxw, s16 b_minw, s16 b_maxw, s16 c_minw, s16 c_maxw)
  /* %%% For proper solve-for-x with restricted symbol sets, the
  {a|b|c}_{min|max}w variables will have been generated from two sets of
  {a|b|c}_{min|max}w variables depending on whether we are generating an
  RHS or an LHS  */
{
  s16 blen;
  form next;
  s16 i, recurse_forms, gen_expr;
  symbol s, slim;
  stats_count n;

/* The "No identity optimization" skips the use of the 'I' operator and
   instead avoids using the 'b' seft in gf_1. */
#define NO_IDENTITY_OPTIMIZATION /* enable when brave */

#ifdef NO_IDENTITY_OPTIMIZATION
  int sincr;
#endif

  n = 0;

  /* copy base form */
  blen = base->flen;
  for(i=0; i<blen; i++) {
    next.sym[i] = (*base).sym[i];
    next.stk[i] = (*base).stk[i];
    next.arg1[i] = (*base).arg1[i];
  }
  next.sym[i] = 0;

  if (debug_w) {
    printf("gf_1 [%s)\n", next.sym);
  }

  /* set up and generate forms elaborating by one symbol on the base */
  next.flen = (s16)(blen + 1);
  /* Find out if any seft-c symbols are enabled */
  if (n_csym == 0) {
    /* No: Use just 'a' and 'b' */
    slim = (symbol)'b';
  } else {
    /* Yes: Use all three sefts */
    slim = (symbol)'c';
  }
#ifdef NO_IDENTITY_OPTIMIZATION
  /* We have no seft-b symbols, not even the identity 'I', thus we can
     loop on just seft 'a' and 'c'. */
  sincr = 1;
  if (n_bsym == 0) {
    sincr = 2;
  }
  for ( s=((symbol)'a'); s <= slim; s=((symbol)(s+sincr)) ) {
#else
  for(s=((symbol)'a'); s <= slim; s++) {
#endif
    next.sym[blen] = s;
    if (debug_w) {
      next.sym[blen+1] = 0;
    }
    /* Stack changes by 1 for a, 0 for b, -1 for c */
    next.stack = (s16)(base->stack + (((symbol)'b') - s));

    /* Compute the minimum and maximum possible weights, including the
       symbols we got from base plus the symbol we just added */
    if (s == 'a') {
      next.min_weight = (s16)(base->min_weight + a_minw);
      next.max_weight = (s16)(base->max_weight + a_maxw);
    } else if (s == 'b') {
      next.min_weight = (s16)(base->min_weight + b_minw);
      next.max_weight = (s16)(base->max_weight + b_maxw);
    } else {
      next.min_weight = (s16)(base->min_weight + c_minw);
      next.max_weight = (s16)(base->max_weight + c_maxw);
    }

    /* check form symtax */
    recurse_forms = 1;
    if (next.stack < 1) {
      /* stack can't be zero or underflow */
      if (debug_w) {
        printf("gf_1 prune [%s) stack would underflow\n", next.sym);
      }
      recurse_forms = 0;
    } else if (next.min_weight + s_minw > e_maxw) {
      /* adding this symbol would make a form that can't possibly generate
         any expressions within the given complexity range. */
      if (debug_w) {
        printf("gf_1 prune [%s) complexity\n", next.sym);
      }
      recurse_forms = 0;
    } else if (blen > MAX_ELEN) {
      /* limits expression length to our physical allocation size. If this
         actually triggers at runtime, it implies that the weights are too
         spread out or that MAX_ELEN is just too darn small. */
      if (debug_w) {
        printf("gf_1 prune [%s) length\n", next.sym);
      }
      recurse_forms = 0;
    } else if ((next.flen + next.stack) > MAX_ELEN + 1) {
      /* in this case it would have no way of getting the stack down to one
         item before exceeding MAX_ELEN */
      if (debug_w) {
        printf("gf_1 prune [%s) stack too high\n", next.sym);
      }
      recurse_forms = 0;
    }

    /* Fill in the stack-height and arg1 arrays. Here are some examples:
    expr  xsE1Exs^-   xsEexs^-   34+
    ix    012345678   01234567   012
    form  abbababcc   abbaabcc   aac
    stk   111223321   11123321   121
    arg1  .......42   ......32   ..0
     */
    next.stk[blen] = next.stack;
    next.arg1[blen] = ARG1_NA; /* Default value is "not applicable" */
    if (s == ((symbol)'c')) {
      /* Find where the first argument is */
      for (i=(s16)(blen-2); i>=0; i--) {
        if (next.stk[i] == next.stack) {
          next.arg1[blen] = i;
          i = -1; /* we found it, make the loop exit */
        }
      }
    }

    /* check viability for expressions */
    gen_expr = 0;
    if (next.stack == 1) {
      gen_expr = 1;
      /* if (next.flen < MAX_ELEN) { g_dstats[next.flen]++; } */
      if (next.min_weight > e_maxw) {
        if (debug_w) {
          printf("gf_1 [%s] min weight too big for expressions\n", next.sym);
        }
        gen_expr = 0;
      } else if (next.max_weight < e_minw) {
        if (debug_w) {
          printf("gf_1 [%s] max weight too small for expressions\n", next.sym);
        }
        gen_expr = 0;
      }
    }

    if (gen_expr) {
      if (debug_w) {
        printf("gf_1 generating expressions on form [%s]\n", next.sym);
      }

      g_ne += ge_1(&next, e_minw, e_maxw, using_x,
        a_minw, a_maxw, b_minw, b_maxw, c_minw, c_maxw); /* this is in gf_1 */
    }

    /* recurse, if appropriate */
    if (recurse_forms) {
      n += gf_1(&next, e_minw, e_maxw, using_x,
        a_minw, a_maxw, b_minw, b_maxw, c_minw, c_maxw); /* gf_1 self-recursive */
    }

    /* count the leaf nodes... leaf. huh-huh. heh huh heh-heh. */
    if (gen_expr) {
      n++;
    }
  }

  return(n);
} /* End of gf.1 */

/* generate forms on-the-fly, given a minimum and maxmum complexity score.
 * It will generate all expressions with valid forms that lie within the
 * complexity limits. */
stats_count gen_forms(s16 e_minw, s16 e_maxw, s16 using_x,
  s16 a_minw, s16 a_maxw, s16 b_minw, s16 b_maxw, s16 c_minw, s16 c_maxw)
  /* %%% For proper solve-for-x with restricted symbol sets, the
  {a|b|c}_{min|max}w variables will have been generated from two sets of
  {a|b|c}_{min|max}w variables depending on whether we are generating an
  RHS or an LHS  */
{
  form base;
  stats_count n;
/* int i; for(i=0; i<MAX_ELEN; i++) { g_dstats[i] = 0; } */

  n = 0;
  base.flen = 0;
  base.stack = 0;
  base.min_weight = 0;
  base.max_weight = 0;
  n = gf_1(&base, e_minw, e_maxw, using_x,
                          a_minw, a_maxw, b_minw, b_maxw, c_minw, c_maxw);

/*  if (debug_y) {
    printf("%s", "nforms[i] >= {");
    for(i=0; i<MAX_ELEN; i++) {
      if (i > 0) { printf(", "); }
      printf("%ld", g_dstats[i]);
    }
    printf(", ...}\n");
  } */

  return(n);
} /* End of gen.forms */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  Routines for initializing the data structures and variables used for the
  search.
*/

/* The amkey is a bitmask of all the single-symbol pattern flag bits
 * for a symbol. For example, the symbol 'n' has a single bit, AM_n. This
 * is all just a lookup-table optimization for ge.2(), which would
 * otherwise have to compare the current symbol against all the
 * single-symbol flags. */
void def_amkey(const char * syms, attr_bits mask)
{
  symbol * s = (symbol *) syms;
  while(*s) {
    sym_attrs[*s].amkey |= mask;
    s++;
  }
}

void define_amkeys(void)
{
  def_amkey("1", AM_1);
  def_amkey("2", AM_2);
  def_amkey("n", AM_n);
  def_amkey("r", AM_r);
  def_amkey("sq", AM_sq);
  def_amkey("l", AM_l);
  def_amkey("E", AM_E);
  def_amkey("p", AM_pi);
  def_amkey("+", AM_plus);
  def_amkey("*", AM_mul);
  def_amkey("^", AM_pow);
}

void add_symbol(symbol sym, const char * name_forth, const char * name_infix,
  symbol seft, s16 weight,
  const char * def_terse, const char * def_normal, const char * description)
{
  if (out_expr_format == OF_NORMAL) {
    sym_attrs[sym].defn = def_normal;
  } else {
    sym_attrs[sym].defn = def_terse;
  }
  if (sym_attrs[sym].defn) {
    if(strlen(sym_attrs[sym].defn) > LINELEFT_INIT) {
      printf("Symbol definition for '%c' too long:\n%s\n", sym,
              sym_attrs[sym].defn);
    }
  }
  sym_attrs[sym].desc = description;

  /* Only set the names if the user didn't assign a non-null name */
  if ((sym_attrs[sym].sa_name==0) || (strlen(sym_attrs[sym].sa_name) == 0)) {
    if (name_infix) {
      sym_attrs[sym].sa_name = name_infix;
    }
    if (name_forth) {
      sym_attrs[sym].name_forth = name_forth;
    }
  }

  sym_attrs[sym].seft = seft;

  if (IS_PHANTOM(sym)) {
    return;
  }

  /* All symbols have a base complexity of 10 points, plus the individual
     per-symbol weight */
  if (sym_attrs[sym].preempt_weight >= 0) {
    weight = sym_attrs[sym].preempt_weight;
  } else {
    weight = (s16)(weight + weight_base);
  }

  sym_attrs[sym].sa_wgt = weight;
  sym_attrs[sym].sa_mask = 0;

  if (sym_attrs[sym].sa_alwd == 0) {
    return;
  }

  /* Set the sa_known flag to keep track of which symbols made it this far;
     this is for use by setup_abc_mmw(). */
  g_addsym_seq++;
  sym_attrs[sym].sa_known = g_addsym_seq;
  /* printf("add_symbol: '%c'\n", (char) sym); */

  if (seft == 'a') {
    if (n_asym >= MAX_SEFT_POP) {
      printf("add_symbol: Too many seft 'a' symbols.\n");
      print_end(-1);
    }
    g_asym[n_asym++] = sym;
  } else if (seft == 'b') {
    if (n_bsym >= MAX_SEFT_POP) {
      printf("add_symbol: Too many seft 'b' symbols.\n");
      print_end(-1);
    }
    g_bsym[n_bsym++] = sym;
  } else if (seft == 'c') {
    if (n_csym >= MAX_SEFT_POP) {
      printf("add_symbol: Too many seft 'c' symbols.\n");
      print_end(-1);
    }
    g_csym[n_csym++] = sym;
  }
} /* End of add.symbol */

/* Calculate, or re-calculate, the global g_{a|b|c}_{min|max}w
variables and s_minw. This is needed for proper solve-for-x with restricted
symbol sets. */
void setup_abc_mmw(void)
{
  int i;
  symbol seft;
  s16 weight;

  /* %%% For proper solve-for-x with restricted symbol sets, we'll probably
     need to recalculate the number of symbols in n_{a|b|c}sym and rebuild the
     g_{a|b|c}sym arrays. If needed, I can preserve the order of the arrays
     (which affects the order symbols are tried in ge_2) by using the
     g_addsym_seq values in sa_known. */
  g_a_minw = g_b_minw = g_c_minw = (int) MAX_SYMBOL_WEIGHT;
  g_a_maxw = g_b_maxw = g_c_maxw = (int) MIN_SYMBOL_WEIGHT;

  for (i=0; i<SYMBOL_RANGE; i++) {
    if (sym_attrs[i].sa_known) {
      /* This symbol was enabled at init.2() time */
      if (sym_attrs[i].sa_alwd) {
        /* We want this symbol for this petit cycle. %%% For proper solve-for-x
           with restricted symbol sets, this part of the test will
           depend on whether we're about to generate LHS or RHS */

        seft = sym_attrs[i].seft;
        weight = sym_attrs[i].sa_wgt;

        if (seft == 'a') {
          if (weight < g_a_minw) { g_a_minw = weight; }
          if (weight > g_a_maxw) { g_a_maxw = weight; }
        } else if (seft == 'b') {
          if (weight < g_b_minw) { g_b_minw = weight; }
          if (weight > g_b_maxw) { g_b_maxw = weight; }
        } else if (seft == 'c') {
          if (weight < g_c_minw) { g_c_minw = weight; }
          if (weight > g_c_maxw) { g_c_maxw = weight; }
        }
  } } }

  s_minw = g_a_minw;
  if (g_b_minw < s_minw) {
    s_minw = g_b_minw;
  }
  if (g_c_minw < s_minw) {
    s_minw = g_c_minw;
  }
} /* End of setup.abc_mmw */

const char * seft_names[3] = {
  "Explicit values",
  "Functions of one argument",
  "Functions of two arguments",
};

/* Show the set of symbols that is defined, along with seft, weight,
   definition, etc. */
void show_symset(void)
{
  int i, seft;
  const char * def;

  for(seft = 'a'; seft <= 'c'; seft++) {
    printf("%s:\n", seft_names[seft-'a']);
    printf(" sym seft wght name description\n");
    for (i=0; i<SYMBOL_RANGE; i++) {
      if (!(IS_PHANTOM(i)) && (sym_attrs[i].seft == seft) && (sym_attrs[i].sa_alwd)) {
        printf("  %c    %c   %2d   %-4s", i, sym_attrs[i].seft,
          sym_attrs[i].sa_wgt, sym_attrs[i].sa_name);
        def = sym_attrs[i].defn;
        if (def == 0) {
          def = sym_attrs[i].desc;
        }
        if (def) {
          printf(" %s", def);
        }
        printf("\n");
      }
    }
    printf("\n");
  }
} /* End of show.symset */

/* add.rule compares a pruning rule's attributes to the list of valid
symbols; if the rule matches (i.e. if the set of permitted symbols
include the necessary substitute symbols), the rule's trigger-bitmask
gets added to the bitmask for the symbol that this rule blocks.

For example, executing the rule ("-", '+', AM_n) blocks the '+' symbol
and is triggered by the bitmask AM_n. If this rule is enabled, '+'
will never get appended to any partial expression ending in 'n'. This
is because the sequence [n+] is equivalent to [-]. Therefore, this
rule can only be used if the permitted symbols include '-'. So this
function adds the bit AM_n to the rule bitmask for '+' only if the
sym_allowed list contains '-'.

The parameters are:

  ss       Substitute symbols
  sy       Symbol to prohibit
  mask     Flag bits that trigger the rule
*/
void add_rule(const char * ss, char sy, attr_bits mask)
{
  symbol * symset = (symbol *) ss;
  symbol sym = (symbol) sy;
  symbol sreq;
  s16 allowed;
  symbol * s;    /* Copy of 'symset' for loop */

  /* The rule is allowed unless one or more in its symbolset are restricted
     in number by our symbol frequency options. */

  /* %%% solve-for-x: The ss (substitute symbols) need to be compared
     against the INTERSECTION of the LHS and RHS symbol sets */
  allowed = 1;
  s = symset;
  while((sreq = *s++)) {
    if (sym_attrs[sreq].sa_alwd < MAX_ELEN) {
      allowed = 0;
    }
  }
  if (allowed) {
    if (debug_x) {
      printf("Using rule %8s %c %4x\n", symset, sym, mask);
    }

    /*  */
    sym_attrs[sym].sa_mask |= mask;
  }
} /* End of add.rule */

/* Initialize numerics (values of constants)
   For initialization of printf format strings, see init_formats */
void init_numerics(void)
{
  if (debug_z) {
    printf("init_num: nominal %d digits, usable %d\n",
      k_nominal_digits, k_usable_digits);
  }

  {
#ifdef REDUNDANT_ULP_INIT
    /* Figure out the size of the ULP (unit in least position), used to
       auto-compute some of the constants used for precision, rounding,
       overflow, and tautology handling. */
    ries_val k14 = 181.0 / 128.0; /* Approximately sqrt(2) */
    ries_val ulp = 1.0;  /* Unit in the Last Place */
    ries_val sum = k14 + ulp;
    while (sum != k14) {
      ulp = ulp / 2.0;
      sum = k14 + ulp;
    }
    k_precision_ulp = (ries_dif) ulp;
#endif
    k_precision_ulp = 2.0 * k_ulp;
    /* Before 20120102, RIES was setting k_min_best.match to 1.0e-15,
       which is 9.0072 times the ULP of IEEE binary64. */
    k_min_best_match = 9.0072 * (ries_dif) k_precision_ulp;
    if (debug_z) {
      printf("init_num: ulps are %g, %g ; kmbm=%g\n",
               k_precision_ulp, k_ulp, k_min_best_match);
    }
  }


  /* Initialize the transcendental constants */

  {
    ries_val check_phi
                  = (ries_val) 1.61803398874989484820458683436563811772030L;

    k_phi = (SQRT(5.0) + 1.0) / 2.0;
    if (debug_z) {
      printf("init_num: k_phi ");
      spfg(k_nominal_digits, k_phi);
      printf(" chk ");
      spfg(k_nominal_digits, check_phi);
      printf(" diff %g\n", dbl(check_phi - k_phi));
    }
  }

  {
    ries_val check_e
                  = (ries_val) 2.71828182845904523536028747135266249775724L;
    ries_val i;
    /* This is just the Taylor series for e^1:

         e = SIGMA_(k=0..inf) [ 1 / k! ]
           = 1 + 1 + 1/2 + 1/3*2 + 1/4*3*2 + 1/5+4+3+2 + ...
           = 1 + 1/1 * (1 + 1/2 * (1 + 1/3 * (1 + 1/4 * (1 + ...))))
           = 1 + (1 + (1 + (1 + (1 + ... )/4 )/3 )/2 )/1

       which can be readily unrolled into the loop shown here.
    */
    i = 31.0; /* 19 for IEEE binary64; 23 For 64-bit mantissa;
                 31 for PowerPC long double  */
    k_e = 1.0 / i;
    while(i > 1.0) {
      i -= 1.0;
         /* 1.0 + 1/i * (k_e) */
      k_e = 1.0 + k_e / i;
    }
    if (debug_z) {
      printf("init_num: k_e ");
      spfg(k_nominal_digits, k_e);
      printf(" chk ");
      spfg(k_nominal_digits, check_e);
      printf(" diff %g\n", dbl(check_e - k_e));
    }
  }

  {
    ries_val check_pi
                  = (ries_val) 3.14159265358979323846264338327950288419716L;
    ries_val i;

    /* This pi algorithm is based on the infinite sum attributed to
       Isaac Newton:

         pi/2 = SIGMA_(k=0..inf) [ k! / (2k+1)!! ]
              = 1 + 1/3 + 2/5*3 + 3*2/7*5*3 + 4*3*2/9*7*5*3 + ...
              = 1 + 1/3 * (1 + 2/5 * (1 + 3/7 * (1 + 4/9 * (1 + ...))))

       which can be readily unrolled into the loop shown here. It is slow
       (requiring lots of divisions by distinct primes) but surpasses
       faster-converging methods like Gauss-Legendre and Borwein-Borwein
       in that it produces the most precise possible answer in IEEE binary64.
    */
    i = 103.0; /* 50 for IEEE binary64; 61 for long double;
                  103 for PowerPC long double */
    k_pi = 1.57;
    while(i > 1.0) {
      i -= 1.0;
      k_pi = 1.0 + (i / (1.0 + (2.0 * i))) * k_pi;
    }
    k_pi = 2.0 * k_pi;
    if (debug_z) {
      printf("init_num: k_pi ");
      spfg(k_nominal_digits, k_pi);
      printf(" chk ");
      spfg(k_nominal_digits, check_pi);
      printf(" diff %g\n", dbl(check_pi - k_pi));
    }

    /* If the trig scale hasn't been set, use the default value */
    if (k_sincos_arg_scale <= 0) {
      k_sincos_arg_scale = k_pi;
      g_trig_scale_default = B_TRUE;
    }

    k_2pi = 2.0 * k_pi;
  }
} /* End of init.numerics */

/* init_symbol_names does nothing now, but will be needed when
/  user-defined functions and constants are implemented. The symbol names
/  table is updated as we parse through the arguments, and once defined,
/  a symbol name can be used by another function definition. This routine
/  will set up this database in a suitable initial state. */
void init_symbol_names(void)
{
  int i;

  for(i=0; i<SYMBOL_RANGE; i++) {
    sym_attrs[i].sa_name = "";
    sym_attrs[i].name_forth = 0;
  }
}

void allsyms_set(s16 n, int include_x)
{
  int i;

  for(i=0; i<SYMBOL_RANGE; i++) {
    if (include_x || ((char) i != 'x')) {
      sym_attrs[i].sa_alwd = n;
    }
  }
}

void somesyms_set(symbol * s, s16 n)
{
  while (s && *s) {
    sym_attrs[*s].sa_alwd = n;
    /* if (*s == 'W') { printf("Set W to %d\n", n); } */
    s++;
  }
}

void set_anagram(char * anagram)
{
  int i;

  if (anagram) {
    const char * ana_syms = "123456789efprs";
    symbol s;
    /* --numeric.anagram option specifies a set of digits which can be used
       (in any order) in the RHS. They can give zeros or other symbols
       if they want, but we only pay attention to the digits 1-9, constants
       efp, and 'r' and 's' which represent a '1' or '2' respectively. */
    for(i=0; ana_syms[i]; i++) {
      s = (symbol)(ana_syms[i]);
      sym_attrs[s].sa_alwd = 0;
    }
    for(i=0; anagram[i]; i++) {
      s = (symbol)(anagram[i]);
      if (strchr(ana_syms, ((char) s))) {
        sym_attrs[s].sa_alwd++;
      }
    }
  }
} /* End of set.anagram */

/* init1() sets defaults (anything that can be overridden or changed by
   command-line arguments) */
void init1()
{
  s16 i;

  inittime();
  init_mem();

  g_enable_output = 1;

  for(i=0; i<MAX_FIND_EXPR; i++) {
    g_find_expr[i] = 0;
  }
  g_num_find_expr = 0;
  g_eval_expr = 0;

  /* There are four command-line options for specifying the symbolset:
     -S, -O, -E, and -N. -S means "use only these symbols"; -O means "use
     at most one of these per expression"; -E and -N enable and disable
     certain symbols without affecting any others.

     Initially all symbols are enabled except 'W'. While scanning the
     command-line, when any of these symbolset-selection arguments is
     encountered they cause the sa.alwd fields (FKA an array "sym_allowed")
     to be modified. sa.alwd speficies how many of each symbol is
     allowed in each expression. -N changes the sa.alwd value to 0,
     -O sets it to 1, -E and -S set it to MAX_ELEN, and -S also sets all
     other symbols' sa.alwd value to 0.

     These are done in the order the options are given. (In earlier versions of
     RIES, there were only the options -S, -N and -O; their parameters
     were remembered until all command-line options were parsed, and then
     they were handled as if they had been given in the order: -N, then
     -O, then -S).
  */
  for(i=0; i<SYMBOL_RANGE; i++) {
    sym_attrs[i].preempt_weight = -1;
  }
  allsyms_set(MAX_ELEN, 1);
  somesyms_set((symbol *) "W", 0);
  S_option = B_FALSE;
  NOS_options = B_FALSE;
  g_show_ss = B_FALSE;
  x_lhs_only = B_FALSE;
  g_no_cv_simplify = B_FALSE;
  g_one_sided = B_FALSE;
  g_solve_for_x = B_FALSE;
  g_reported_exhaustion = B_FALSE;
  g_refinement = B_TRUE;
  g_max_matches = DEFAULT_MAX_MATCHES;

  /* Set default serach level */
  g_levadj = DEFAULT_LEV_ADJ;
  tlevel = DEFAULT_LEV_BASE + g_levadj;

  g_restrict_subexpr = TYPE_NONE;
  g_restrict_exponents = TYPE_NONE;
  g_restrict_trig_args = TYPE_NONE;
  g_relative_x = B_TRUE;
  g_wide_output = B_FALSE;
  g_explicit_multiply = B_FALSE;
  out_expr_format = OF_NORMAL;
  init_symbol_names();

  /* 20120105: without cv.simplify, canon reduction is a bit of a nuisance,
  /  so it is disabled by default right now.
  /  In the future we might set it back to:
  /    CANONVAL_NEGATE | CANONVAL_RECIPROCAL | CANONVAL_DIV2 | CANONVAL_MUL2 */
  g_canon_ops = 0;

  debug_m = 0; debug_M = 0; debug_n = 0; debug_N = 0; debug_o = 0; debug_p = 0;
  debug_Q = 0; debug_q = 0; debug_r = 0; debug_S = 0; debug_s = 0; debug_t = 0;
  debug_u = 0; debug_v = 0; debug_w = 0; debug_x = 0; debug_y = 0;
  debug_A = 0; debug_B = 0; debug_C = 0; debug_D = 0; debug_E = 0;
  debug_F = 0; debug_G = 0; debug_H = 0; debug_I = 0; debug_J = 0;
  debug_K = 0; debug_L = 0; debug_z = 0; debug_0 = 0;

  g_allow_slow_message = 1;

  g_min_memory = 0;
  g_max_memory = 1.0e20;

  g_target = 1.0;
} /* End of init1 */

/* Post-arguments initialization: All the command-line arguments have been
   detected; now we proceed to initialize globals that depend on arguments */
void init2()
{
  s16 i;

  if (debug_z) {
    printf("Struct sizes:\n");
    printf("  ALLOC_SIZE == %ld\n", ((long) ALLOC_SIZE));
    printf("  MAX_ELEN == %d, TS_ALLOC_L == %d, TS_ALLOC_R == %d\n",
      ((int) MAX_ELEN), ((int) TS_ALLOC_L), ((int) TS_ALLOC_R));
    printf("  sizeof(sym_attr_block) == %d\n", (int) sizeof(sym_attr_block));
    printf("  sizeof(form) == %d\n", (int) sizeof(form));
    printf("  sizeof(pe) == %d\n", (int) sizeof(pe));
    printf("  sizeof(metastack) == %d\n", (int) sizeof(metastack));
    printf("  sizeof(expr) == %d\n", (int) sizeof(expr));
  }

  init_numerics();

  max_flen = 0;
  lhs_root = 0;

  /* Prohibit -Nx (act as if -Sx was given unless they specified -Ox) */
  if (sym_attrs['x'].sa_alwd == 1) {
    x_lhs_only = B_TRUE;
  } else {
    sym_attrs['x'].sa_alwd = MAX_ELEN;
  }

#ifdef DUMMY_LAMBERT
  if (sym_attrs['W'].sa_alwd) {
    printf("%s: The Lambert W function 'W' requires the stand-alone maths library.\n",
      g_argv0);
    brief_help();
    print_end(1);
  }
#endif

  for(i=0; i<SYMBOL_RANGE; i++) {
    sym_attrs[i].seft = 0;
    sym_attrs[i].defn = 0;
    sym_attrs[i].desc = 0;
    sym_attrs[i].def_given = 0;
    sym_attrs[i].def_needed = 0;
    sym_attrs[i].amkey = 0;
    sym_attrs[i].sa_known = 0;
  }
  used_trig = used_atan1 = used_atan2 = 0;
  n_asym = n_bsym = n_csym = 0;
  g_used_identity = B_FALSE;
  weight_base = 10;
  g_addsym_seq = 0;

  /* Now we fill in the table with all the specific values for the symbols
     we actually know about. %%% This will include custom constants from the
     queue built up during argument scanning. */

  add_symbol(ADDSYM_NAMES(' ', 0,       "NOP"),
    '0', 0,   0, 0, "no operation");

  /* seft 'a' symbols are constants.
     For most of these, the weight is close to 10.0*ln(x)/ln(10) */
  add_symbol(ADDSYM_NAMES('1', 0,       "1"),
    'a', 0,     0, 0, "integer");
  add_symbol(ADDSYM_NAMES('f', "phi",   "phi"),
    'a', 8,  "f = phi, the golden ratio, (1+sqrt(5))/2",
                                           "phi = the golden ratio, (1+sqrt(5))/2", "");
  add_symbol(ADDSYM_NAMES('2', 0,       "2"),
    'a', 3,     0, 0, "integer");
  add_symbol(ADDSYM_NAMES('e', "e",     "e"),
    'a', 6,    "e = base of natural logarithms, 2.71828...",
                                           "e = base of natural logarithms, 2.71828...", "");
  add_symbol(ADDSYM_NAMES('3', 0,       "3"),
    'a', 5,     0, 0, "integer");
  add_symbol(ADDSYM_NAMES('p', "pi",    "pi"),
    'a', 4,   "p = pi, 3.14159...", "pi = 3.14159...", "");
  add_symbol(ADDSYM_NAMES('4', 0,       "4"),
    'a', 6,     0, 0, "integer");
  add_symbol(ADDSYM_NAMES('5', 0,       "5"),
    'a', 7,     0, 0, "integer");
  add_symbol(ADDSYM_NAMES('6', 0,       "6"),
    'a', 8,     0, 0, "integer");
  add_symbol(ADDSYM_NAMES('7', 0,       "7"),
    'a', 8,     0, 0, "integer");
  add_symbol(ADDSYM_NAMES('8', 0,       "8"),
    'a', 9,     0, 0, "integer");
  add_symbol(ADDSYM_NAMES('9', 0,       "9"),
    'a', 9,     0, 0, "integer");
  add_symbol(ADDSYM_NAMES('x', 0,       "x"),
    'a', 5,     0, 0, "the variable of the equation");

  /* seft 'b' symbols */
  add_symbol(ADDSYM_NAMES('n', "neg",  "-"),
    'b', -3,    "n = negative", 0, "negate");
  add_symbol(ADDSYM_NAMES('r', "recip","1/"),
    'b', -3,    0, 0, "reciprocal");
  add_symbol(ADDSYM_NAMES('s', "dup*", "^2"),
    'b', -1,    0, 0, "square");
  add_symbol(ADDSYM_NAMES('q', "sqrt", "sqrt"),
    'b', -1, "q = square root", "sqrt(x) = square root", "");
  add_symbol(ADDSYM_NAMES('l', "ln",   "ln"),
    'b',  3,   "l = ln, natural logarithm or log base e",
                                           "ln(x) = natural logarithm or log base e", "");
  add_symbol(ADDSYM_NAMES('E', "exp",  "e^"),
    'b',  3,    0, 0, "natural exponent function");

  if (k_sincos_arg_scale == k_pi) {
    /* With a scale factor of pi we'll call the functions "sinpi", etc. */
    add_symbol(ADDSYM_NAMES('S', "sinpi", "sinpi"),
      'b',  3, "S(x) = sinpi(x) = sin(pi * x)",
                                           "sinpi(X) = sin(pi * x)", "sinpi");
    add_symbol(ADDSYM_NAMES('C', "cospi", "cospi"),
      'b',  3, "C(x) = cospi(x) = cos(pi * x)",
                                           "cospi(X) = cos(pi * x)", "cospi");
    add_symbol(ADDSYM_NAMES('T', "tanpi", "tanpi"),
      'b',  6, "T(x) = tanpi(x) = tan(pi * x)",
                                           "tanpi(X) = tan(pi * x)", "tanpi");
  } else {
    /* With any other scale factor we just call it "sin", etc. and let
       the user fend for himself (there are too many possibilities to
       test for: degrees, grads, and of course natural units, plus all
       the nonstandard units). Presumably if the user gave a scale factor,
       she knows what the definition is. */
    add_symbol(ADDSYM_NAMES('S', "sin",  "sin"),
      'b',  3,  "S(x) = sine", 0, "sine");
    add_symbol(ADDSYM_NAMES('C', "cos",  "cos"),
      'b',  3,  "C(x) = cosine", 0, "cosine");
    add_symbol(ADDSYM_NAMES('T', "tan",  "tan"),
      'b',  6,  "T(x) = tangent", 0, "tangent");
  }

  add_symbol(ADDSYM_NAMES('W', "W", "W"),
      'b', 5, "W(x) = LambertW(x) = inverse(x=w*e^w)",
                               "W(x) = LambertW(x) = inverse(x=w*e^w)", "W");


  /* seft 'c' symbols */
  add_symbol(ADDSYM_NAMES('+', "+",    "+"),
    'c', -6,    0, 0, "add");
  add_symbol(ADDSYM_NAMES('-', "-",    "-"),
    'c', -5,    0, 0, "subtract");
  add_symbol(ADDSYM_NAMES('*', "*",    "*"),
    'c', -6,    0, 0, "multiply");
  add_symbol(ADDSYM_NAMES('/', "/",    "/"),
    'c', -5,    0, 0, "divide");
  add_symbol(ADDSYM_NAMES('^', "**",   "^"),
    'c', -4,    0, 0, "A ^ B = A to the power of B");
  add_symbol(ADDSYM_NAMES('v', "root", "\"/"),
    'c', -3,  "A v B = Ath root of B", "A\"/B = Ath root of B", ""); /* Power[B, Rational[1, A]] */
  add_symbol(ADDSYM_NAMES('L', "logN", "log_"),
    'c', -1, "A L B = logarithm to base A of B = ln(B) / ln(A)",
                                           "log_A(B) = logarithm to base A of B = ln(B) / ln(A)", ""); /* Log_x(y) */
  add_symbol(ADDSYM_NAMES('A', "atan2", "atan2"),
    'c', -1,  /* Weight normally -1, use 11 to disable arctan2 */
           "s A c = arctan2(s,c)",
           "atan2(s,c) = Angle of ray from origin through point (c,s)", "");

  /* phantom symbols -- used only for postfix to infix translation */
  add_symbol(ADDSYM_NAMES(PS_REVDIV, 0, "!/"),
    'c', 0,   0, 0, 0);
  add_symbol(ADDSYM_NAMES(PS_REVSUB, 0, "!-"),
     'c', 0,   0, 0, 0);
  add_symbol(ADDSYM_NAMES(PS_cross, 0, "><"),
     'c', 0,   0, 0, 0);
  add_symbol(ADDSYM_NAMES(PS_REVPOW, 0, "!^"),
    'c', 0,   0, 0, 0);

  /* These are used for infix formatting */
  /* sym_attrs['('].sa_name = "("; sym_attrs[')'].sa_name = ")"; */
  add_symbol(ADDSYM_NAMES('(', 0, "("), 0, 0, 0, 0, 0);
  add_symbol(ADDSYM_NAMES(',', 0, ","), 0, 0, 0, 0, 0); /* Arg sep for atan2 */
  add_symbol(ADDSYM_NAMES(')', 0, ")"), 0, 0, 0, 0, 0);
  add_symbol(ADDSYM_NAMES('=', 0, "="), 0, 0, 0, 0, 0);

  /* This symbol is a temporary placeholder for infix multiplication.
     %%% Figure out if I need this at all, or use PS_cross instead */
  sym_attrs['.'].sa_name = " ";

  /* Report error and abort if there is only one type A symbol
     (namely X, which is always included) */
  if (n_asym < 2) {
    printf("%s: You must allow at least one constant symbol.\n",
      g_argv0);
    if (S_option) {
      printf("  (Note that the -S option disables all other symbols)\n");
    }
    brief_help();
    print_end(1);
  }

  if (n_bsym == 0) {
    if (n_csym == 0) {
      printf("%s: You must allow at least one operator symbol.\n",
        g_argv0);
      brief_help();
      print_end(1);
    }
#ifdef NO_IDENTITY_OPTIMIZATION
    /* With the "no identity" optimization we don't need this kludge. */
#else
    /* Add "identity" operator if there are no type B symbols */
    sym_attrs['I'].sa_alwd = MAX_ELEN;
    add_symbol(ADDSYM_NAMES('I', "nop", "I"),
      'b', 10, "I = identity", 0, "identity");
    g_used_identity = B_TRUE;
#endif
  }

  /* Setup the g_{a|b|c}_{min|max}w variables */
  setup_abc_mmw();

  /* If there are no type C symbols, gf_1 will notice n_csym and
     will skip generating forms with 'c's */

  define_amkeys();

  /* operators have masks for pruning (optimization). These are commented
   * with reasons, given as "transformation" equations. A transformation
   * looks like this: [KK*] => [Ks], and represents two sequences of operators
   * that have the same value or have the same effect in an expression. The
   * form to the left of the '=>' is the form being eliminated by the rule,
   * and the form to the right is shown to demonstrate why the form on the
   * left is eliminated. When the transformation has a shorter form on the
   * right than on the left, as with [1r] => [1], the reason for the rule
   * is obvious. In other cases it is important to eliminate only the form
   * that has a higher complexity score. If the scores are equal, such as
   * [rn] => [nr], we pick one that interacts favorably with other rules.
   * The form on the right is said to be "forced", meaning that any expressions
   * involving this type of calculation are "forced" to do it in the right-hand
   * form.
   *    You must not "force" a form that is also eliminated by another rule!
   * For example, the following two rules are OK by themselves, but together
   * cause a problem:
   *          [sr] => [rs]
   *          [rs] => [sr]
   * However, you can also "force" a form that is also "forced" into another
   * form by another rule. The following two rules exemplify this:
   *          [rn] => [nr]
   *          [rq] => [qr]
   * These two rules, combined, make sure that 'r' never comes before 'n'
   * or 'q'; this will cause the combinations of 'r', 'n', and 'q' to be
   * reduced from six {[rnq], [rqn], [nrq], [nqr], [qrn], [qnr]} to
   * two {[nqr], [qnr]}
   *    Each rule has a symbolset which must be present in order for that
   * rule to be used (in a few cases this symset is null).
   *      symset   sym  mask  mval          */
  add_rule("",     'x', AM_RHS);
  add_rule("nr",   'n', AM_r);  /* [rn] => [nr]             */
  add_rule("",     'n', AM_n);  /* [nn] => []               */
  add_rule("1",    'r', AM_1);  /* [1r] => [1]              */
  add_rule("",     'r', AM_r);  /* [rr] => []               */
  add_rule("1",    's', AM_1);  /* [1s] => [1]              */
  add_rule("4",    's', AM_2);  /* [2s] => [4]              */
  add_rule("s",    's', AM_n);  /* [ns] => [s]              */
  add_rule("rs",   's', AM_r);  /* [rs] => [sr]             */
  add_rule("4^",   's', AM_sq); /* [qs] => []; [ss] => [4^] */
  add_rule("1",    'q', AM_1);  /* [1q] => [1]              */
  add_rule("rq",   'q', AM_r);  /* [rq] => [qr]             */
  add_rule("4v",   'q', AM_sq); /* [sq] => []; [qq] => [4v] */
  add_rule("",     'l', AM_1);  /* [1l] => 0                */
  add_rule("ln",   'l', AM_r);  /* [rl] => [ln]             */
  add_rule("",     'l', AM_E);  /* [El] => []               */
  add_rule("",     'E', AM_l);  /* [lE] => []               */
  add_rule("Er",   'E', AM_n);  /* [nE] => [Er]             */
  add_rule("Sn",   'S', AM_n);  /* [nS] => [Sn]             */
  add_rule("C",    'C', AM_n);  /* [nC] => [C]              */

  /* The operators are not included in their own require-symset string
     unless they are also used in the target of the forced transformation.
     This is important with the -O option. For example, if they specify
     -O+, the '+' rules will have the effect of "saving" the '+' for
     "a more important", i.e. irreducible, use. Of course, it doesn't
     actually prevent solutions from being found, it just makes them get
     found sooner. */
  add_rule("2*",   '+', AM_KK); /* [KK+] => [K2*]           */
  add_rule("*23456789", /* If our integers maxed out at an even number we
                           wouldn't need the "*" here */
                   '+', AM_55); /* [25+]=>[7]; [55+]=>[52*] */
  add_rule("+",    '+', AM_jK); /* [jK+] => [Kj+]           */
  add_rule("-",    '+', AM_n);  /* [n+] => [-]              */
  add_rule("",     '-', AM_KK); /* [KK-] => 0               */
  add_rule("1234n",'-', AM_55); /* [JK-] => [L] or [Ln]     */
  add_rule("12345678n",
                   '-', AM_jK); /* [35-] => [2n]            */
  add_rule("+",    '-', AM_n);  /* [n-] => [+]              */
  add_rule("s",    '*', AM_KK); /* [KK*] => [Ks]            */
  add_rule("*",    '*', AM_jK); /* [jK*] => [Kj*]           */
  add_rule("*",    '*', AM_1);  /* [1*] => []               */
  add_rule("*n",   '*', AM_n);  /* [n*] => [*n]             */
  add_rule("/",    '*', AM_r);  /* [r*] => [/]              */
  add_rule("1",    '/', AM_KK); /* [KK/] => [1]             */
  add_rule("r",    '/', AM_1K); /* [1K/] => [Kr]            */
  add_rule("",     '/', AM_1);  /* [1/] => []               */
  add_rule("/n",   '/', AM_n);  /* [n/] => [/n]             */
  add_rule("*",    '/', AM_r);  /* [r/] => [*]              */
  add_rule("",     '^', AM_1);  /* [1^] => []               */
  add_rule("s",    '^', AM_2);  /* [2^] => [s]              */
  add_rule("^r",   '^', AM_n);  /* [n^] => [^r]             */
  add_rule("1",    '^', AM_1K); /* [1K^] => [1]             */
  add_rule("v",    '^', AM_r);  /* [r^] => [v]              */
  add_rule("",     'v', AM_1);  /* [1v] => []               */
  add_rule("q",    'v', AM_2);  /* [2v] => [q]              */
  add_rule("vr",   'v', AM_n);  /* [nv] => [vr]             */
  add_rule("1",    'v', AM_1K); /* [1Kv] => [1]             */
  add_rule("^",    'v', AM_r);  /* [rv] => [^]              */
  add_rule("1",    'L', AM_KK); /* [KKL] => [1]             */
  add_rule("",     'L', AM_1);  /* [1L] => undefined        */
  add_rule("Ln",   'L', AM_r);  /* [rL] => [Ln]             */
  add_rule("",     'L', AM_1K); /* [1KL] => 0               */

  if (k_sincos_arg_scale == 1.0) {
    /* Added on 20070511 */
    add_rule("",     'S', AM_pi); /* [pS] => 0              */
    /* Added on 20090513 */
    add_rule("1n",   'C', AM_pi); /* [pC] => [1n]           */
  }

  /* Added on 20090513 */
  add_rule("",     '/', AM_KxK);/* [K*K/] -> []             */

  /* Added on 20111230 */
  add_rule("",     '-', AM_KpK);/* [K+K-] -> []             */

  /* 20130130: The commutative operators + and * have the property
     that A+(B+C) = (A+B)+C. Thus, we can add a rule that forces
     one or the other of these two forms; the easier one to force
     is left-hand associative, i.e. do each operation as soon as
     possible. */
  add_rule("",     '+', AM_plus); /* [ABC++] -> [AB+C+]     */
  add_rule("",     '*', AM_mul);  /* [ABC**] -> [AB*C*]     */

  /* 20130130: Here we force A^(B*C) into the equivalent form
     (A^B)^C, which is only available if more than one ^ symbol
     is allowed. */
  add_rule("^",    '^', AM_mul);  /* [ABC*^] -> [AB^C^]     */

  /* 20130130: [AB^q] = sqrt(A^B) is the same as [AqB^] = sqrt(A)^B */
  add_rule("",     'q', AM_pow);  /* [AB^q] -> [AqB^]       */

  /* 20141212: [2E]->[es], e.g. 1.07822380518236 finds xfLr = 2E1- */
  add_rule("es",   'E', AM_2);    /* [2E] => [es]           */
  add_rule("E",    '^', AM_a1_e); /* [e..^] => [..E]        */

  /* 20141213 If I disable these I can find examples with the
     #search1# script, e.g. the command ./search1 es '1ab/'
     found that "ries 3.31130856083748 -F0 -n999 -l3 --no-refinement --max-match-distance 1e-6" gave the result "x4xr-+ = 31pq/v" */
  add_rule("",     '*', AM_a1_1); /* [1..*] => [..]         */
  add_rule("r",    '/', AM_a1_1); /* [1../] => [..r]        */

  /* 20141215 More redundancy found via ./search1 es '[1r]ab*[v^]' */
  add_rule("",     'v', AM_a1_r); /* [r..v] => [..vr]       */
  add_rule("",     '^', AM_a1_r); /* [r..^] => [..^r]       */
  add_rule("e",    'E', AM_1);    /* [1E] => [e]            */

  /* Compute the weight of the most complex expression that could possibly
     fit in the available MAX_ELEN symbols */
  {
    s16 el_tmp = MAX_ELEN >> 1; /* Half, rounded down */
    if (MAX_ELEN & 1) {
      /* Odd max length: most cplx is of the form [aaacc] */
      g_exhaust_cpx = ((s16) ((el_tmp+1)*g_a_maxw + el_tmp*g_c_maxw));
    } else {
      /* Even max length: most cplx is of the form [aaabcc] */
      g_exhaust_cpx
             = ((s16) (el_tmp*g_a_maxw + g_b_maxw + (el_tmp-1)*g_c_maxw));
    }
    /* Compute the alternative using all b's i.e. [abbbbb] */
    el_tmp = ((s16) (g_a_maxw + (MAX_ELEN-1)*g_b_maxw));
    /* See if the [abbbbb] complexity is bigger */
    if (el_tmp > g_exhaust_cpx) {
      g_exhaust_cpx = el_tmp;
    }
  }

  /* This init is for ge_2. */
  for(i=0; i<SYMBOL_RANGE; i++) {
    sym_attrs[i].sa_ct = 0;
  }

  /* calculate "overflow" (really roundoff error) limits */
  p_ovr = fabs((ries_dif)g_target) / k_prune_deriv;
  n_ovr = - p_ovr;

  g_matches = 0;
  if(!(g_refinement)) {
    size_t n_exprs_space; /* Number of symbols for g_max_matches eqns */

    /* Allocate a block to hold the "delta" values of all reported eqns */
    g_mtch_alloc = ((size_t)g_max_matches) * sizeof(ries_val);
    g_nr_deltas = (ries_val *) malloc(g_mtch_alloc);
    if (g_nr_deltas == 0) {
      fprintf(stderr, "%s: Could not allocate %ld bytes for %ld matches.\n"
               "\n\n", g_argv0, (long)g_mtch_alloc, (long)g_max_matches);
      exit(-1);
    }
    /* g_nr_deltas never gets reallocated, because we never store more
       than g_max_matches into it. This global defaults to DEFAULT_MAX_MATCHES
       and can be altered by the --max-matches or -n option. */

    /* Allocate a block to hold a list of all matched equations. */
    n_exprs_space = ((size_t)g_max_matches) * ((size_t) (MAX_ELEN+1));
    /* Figure out how many symbols worth of space we want */
    g_mtch_alloc = n_exprs_space;
    if (g_solve_for_x) {
      /* We need to store every match twice, once to prune before doing
         newton, and again to prune after doing try.solve */
      g_mtch_alloc += n_exprs_space;
    }
    if ((out_expr_format == OF_CONDENSED) || (out_expr_format == OF_NORMAL)) {
      /* In this situation we also add every match in infix.1 format, which
         can be 4 times as long as internal format */
      g_mtch_alloc += (n_exprs_space * 4);
    }

    g_matches = (symbol *) malloc(g_mtch_alloc * sizeof(symbol));
    if (g_matches == 0) {
      fprintf(stderr, "%s: Could not allocate %ld bytes for %ld matches.\n"
               "\n\n", g_argv0, (long)(g_mtch_alloc * sizeof(symbol)),
               (long)g_max_matches);
      exit(-1);
    }
    symstrncat(g_matches, ((symbol *) " "), 2);
    mem_used_KiB = mem_used_KiB + (long)((g_mtch_alloc * sizeof(symbol))>>10);
    /* Carefully track memory usage */
    mem_used_bytes += (2 * sizeof(symbol));
  }

  if (g_target == 0) {
    fprintf(stderr, "%s: Target number cannot be zero.\n", g_argv0);
    exit(-1);
  }
} /* End of init.2() */

int g_got_target;

/* Parse out a number from a string and set g_target and g_targ_tags based
   on the result.

This is a fancy version of sscanf. The additional functions it
provides over sscanf are:
     * Comma and period are both accepted as a decimal point
     * Blank spaces are allowed between digits
     * It computes the magnitude of the ULP (unit in the last place)
       of the mantissa, which is needed for the --mad option
 */
int parse_target(char *str)
{
  int nv;
  size_t slen;
  char * s;
  char * last_digit;
  ries_val targ;

  nv = 0; /* Default return value is "failure" */
  g_mag_ulp = 0; /* By default, the ULP measurement is zero, which will
                 /  prevent use of --mad                                */

  /* Copy the string, leaving out any blank spaces and changing ',' to '.' */
  slen = strlen(str);
  s = (char *) malloc(slen+1);
  {
    unsigned int i, j;
    for(i=0, j=0; i<slen; i++) {
      if (str[i] == ' ') {
        /* skip */
      } else if (str[i] == ',') {
        /* treat ',' as a decimal point */
        s[j++] = '.';
      } else {
        s[j++] = str[i];
      }
      s[j] = 0;
    }
  }

  /* Now we painstakingly match against legal numeric format, making note of
     where the final mantissa digit lies. */
  {
    int i, going;
    int num_dots, init_sig;
    char section;
    last_digit = 0;
    section = 'b'; /* beginning */
    num_dots = 0; init_sig = 1;
    i = 0; going = 1;
    while(s[i] && going) {
      if (section == 'b') {
        /* We're at the beginning */
        if ((s[i] == '+') || (s[i] == '-')) {
          section = 'm'; /* mantissa */
        } else if (init_sig && (s[i] == '0')) {
          section = 'm'; /* mantissa, initial zero */
          last_digit = &(s[i]);
        } else if ((s[i] >= '0') && (s[i] <= '9')) {
          section = 'm'; /* mantissa */
          last_digit = &(s[i]);
          init_sig = 0;
        } else if (s[i] == '.') {
          section = 'm'; /* initial decimal point, mantissa */
          num_dots++;
        } else {
          s[i] = '0';
          going = 0;
        }
      } else if (section == 'm') {
        /* Previous char was in the mantissa */
        if ((s[i] == 'e') || (s[i] == 'E')) {
          section = 'e'; /* exponent */
        } else if (init_sig && (s[i] == '0')) {
          /* still in manitssa, a(nother) leading zero */
          last_digit = &(s[i]);
        } else if ((s[i] >= '0') && (s[i] <= '9')) {
          /* still in manitssa, another digit */
          last_digit = &(s[i]);
          init_sig = 0;
        } else if ((num_dots == 0) && (s[i] == '.')) {
          /* still in mantissa, decimal point */
          num_dots++;
        } else {
          /* This would be an error */
          s[i] = '0';
          going = 0;
        }
      } else {
        /* We're in the exponent now, so we can stop scanning */
        going = 0;
      }
      i++;
    }
  }

  if (last_digit) {
    ries_val inc_ulp;  /* value altered by one Unit in the Last Place (ulp) */
    /* We successfully located a last digit, so we can use the converted
       string to get the target number. */
    nv = sscanf(s, RV_SS_FMT, &targ);
    if (nv) {
      /* So far, so good. Now try altering the last digit. Whenever possible
         we want to diminish this digit, to avoid bumping up the base-2
         exponent. */
      if (*last_digit == '0') {
        *last_digit = '1';
      } else {
        *last_digit = (char)((*last_digit) - 1);
      }
      if (sscanf(s, RV_SS_FMT, &inc_ulp)) {
        if (inc_ulp > targ) {
          g_mag_ulp = (ries_dif) (inc_ulp - targ);
        } else {
          g_mag_ulp = (ries_dif) (targ - inc_ulp);
        }
        if (g_mag_ulp == 0) {
          /* This happens when they give more decimal digits than RIES'
             native float format can represent. This case should be used to
             report that the --mad option is not realistic.
               We could handle this as "--max-match.distance 0" meaning that
             only an exact solution is accepted, but it's safer to assume we
             don't know the exact value of the last digit(s) and therefore
             cannot match them. */
        }
      } else {
        printf("%s: could not parse target value (as altered for ULP check)\n",
          g_argv0);
        nv = 0;
      }

      g_target = targ;
    }
  }

  free(s);

  /* Tell the caller whether we got something */
  return nv;
} /* End of parse.target */

void set_debug_opts(char * str)
{
  char d;
  while((d = *str)) {
    switch(d) {
    case 'a': debug_A |= DBG_RHS; break;
    case 'b': debug_B |= DBG_RHS; break;
    case 'c': debug_C |= DBG_RHS; break;
    case 'd': debug_D |= DBG_RHS; break;
    case 'e': debug_E |= DBG_RHS; break;
    case 'f': debug_F |= DBG_RHS; break;
    case 'g': debug_G |= DBG_RHS; break;
    case 'h': debug_H |= DBG_RHS; break;
    case 'i': debug_I |= DBG_RHS; break;
    case 'j': debug_J |= DBG_RHS; break;
    case 'k': debug_K |= DBG_RHS; break;
    case 'l': debug_L |= DBG_RHS; break;
    case 'm': debug_m = 1; break;
    case 'M': debug_M = 1; break;
    case 'n': debug_n = 1; break;
    case 'N': debug_N = 1; break;
    case 'o': debug_o = 1; break;
    case 'p': debug_p = 1; break;
    case 'q': debug_q = 1; break;
    case 'r': debug_r = 1; break;
    case 's': debug_s = 1; break;
    case 't': debug_t = 1; break;
    case 'u': debug_u = 1; break;
    case 'v': debug_v = 1; break;
    case 'w': debug_w = 1; break;
    case 'x': debug_x = 1; break;
    case 'y': debug_y = 1; break;
    case 'z': debug_z = 1; break;
    case 'A': debug_A |= DBG_LHS; break;
    case 'B': debug_B |= DBG_LHS; break;
    case 'C': debug_C |= DBG_LHS; break;
    case 'D': debug_D |= DBG_LHS; break;
    case 'E': debug_E |= DBG_LHS; break;
    case 'F': debug_F |= DBG_LHS; break;
    case 'G': debug_G |= DBG_LHS; break;
    case 'H': debug_H |= DBG_LHS; break;
    case 'I': debug_I |= DBG_LHS; break;
    case 'J': debug_J |= DBG_LHS; break;
    case 'K': debug_K |= DBG_LHS; break;
    case 'L': debug_L |= DBG_LHS; break;
    case 'Q': debug_Q = 1; break;
    case 'S': debug_S = 1; break;
    case '0': debug_0 = 1; break;
    default: break;
    }
    str++;
  }
} /* End of set.debug_opts */

/* Summary of how each of the restricted class options is implemented

  Option                            -i   -r   -c   -a   -l
  g_restrict_subexpr                INT  RAT CONS  ALG  TRAN
  g_restrict_exponents                             RAT  ELEM
  g_restrict_trig_args                             RAT  ELEM
  set_restrict_rat                        *
  set_restrict_alg                                  1    0
  somesyms_set:
    digits                           *    *    *    *    *
    +-* /nr                          *    *    *    *    *
    sqf                                        *    *    *
    x                                1    1    1    *    *
    ^vASCT                                          *    *
    eplEL                                                *
 */

/* Restrict to rational subexpressions. This is invoked by the -r
option and when we switch to -r after getting -i with a non-integer
target. */
void set_restrict_rat(void)
{
  g_restrict_subexpr = TYPE_RAT;
  somesyms_set((symbol *) "pefqASCTlvLEW", 0);
  somesyms_set((symbol *) "+-*/nr", MAX_ELEN);
  somesyms_set((symbol *) "x", 1);
  somesyms_set((symbol *) "s^", 0); /* %%% Once I improve LHS vs RHS
                         symbolset handling, I can enable [s] on RHS and on
                         non-x-containing subexpressions in LHS, and I can
                         allow q to be appended to LHS if and only if the
                         stack is 1. I can do the same thing for ^ and v
                         if I also implement an "integer arguments" option. */
}

/* Restrict to algebraic roots. This is invoked by the -a and -l options. */
void set_restrict_alg(int restrict_trig)
{
  g_restrict_subexpr = TYPE_ALG;
  /* This option is mostly achieved by turning off transcendental
     functions. */
  somesyms_set((symbol *) "pelLEWA", 0);
  /* Unlike with the smaller classes (constructible, rational) we allow
     more than one x in the solution */
  somesyms_set((symbol *) "+-*/nrsqfx", MAX_ELEN);
  /* Exponents are okay as long as we use only rational exponents.
     The implementation of g_restrict_exponents also disallows x
     in an exponent. */
  /* %%% It might be nice to have a restriction permitting integer exponents
     of x if the exponent is 5 or less, which would let us offer an
     "algebraic closed-form" class. For now, -Ox is the only way to get
     a guarantee of closed-form roots. */
  somesyms_set((symbol *) "^v", MAX_ELEN);
  g_restrict_exponents = TYPE_RAT;
  /* Trig functions are okay as long as we restrict their arguments
     to rational multiples of pi (and g_restrict_trig_args also
     disallows x inside a trig function). */
  somesyms_set((symbol *) "SCT", MAX_ELEN);
  if (restrict_trig) {
    g_restrict_trig_args = TYPE_RAT;
    k_sincos_arg_scale = k_pi;
    g_trig_scale_default = B_TRUE;
  }
} /* End of set.restrict_alg */

# define MAX_FILE_DEPTH 27
size_t stk_nargs[MAX_FILE_DEPTH];
char * * stk_argv[MAX_FILE_DEPTH];
int pa_sp;
char * pa_this_arg;
int pa_argnum;

/* Look for a default setting file, and if one exists, return a pointer
   to its pathname with "-p" in front. This allocates a block of memory,
   and is only called once.  */
char * pa_defaults_path(void)
{
  char * hd;
  char * pdp;
  char sep;
  FILE * f;

#ifdef _WIN32
  hd = getenv("USERPROFILE");
  sep = '\\';
#else
  hd = getenv("HOME");
  sep = '/';
#endif
  if (hd) {
    size_t plen;
    /* Allocate enough space to copy the directory, plus a leading "-p",
       a directory separator character '/' or '\\', and filename e.g.
       "ries_profile.txt" plus trailing null */
    plen = sizeof(char) * (strlen(hd) + 100);
    pdp = (char *) malloc(plen);
    if (pdp) {
      /* Copy the environment variable while adding all the rest */
      snprintf(pdp, plen, "-p%s%c%s", hd, sep, "ries_profile.txt");
      /* Try to open it */
      if (f = fopen(pdp+2,"r"), f) {
        /* Successful: close the file and return the string pointer */
        fclose(f); return pdp;
      }
      /* That filename did not work; try again with ".ries_profile" */
      snprintf(pdp, plen, "-p%s%c%s", hd, sep, ".ries_profile");
      if (f = fopen(pdp+2,"r"), f) {
        fclose(f); return pdp;
      }
    }
  }
  return 0;
} /* End of pa.defaults_path */

/* Return a pointer to the next argument, or a null string if there
   are no more arguments on the present stack. */
char * pa_next_peek(void)
{
  if (pa_sp < 0) {
    return ((char *) "");
  }
  if (stk_nargs[pa_sp] <= 0) {
    /* No more arguments at this level. Note in particular we do not allow
       an argument in a profile to take a parameter from the argument list
       that included it. */
    return ((char *) "");
  }
  return(*(stk_argv[pa_sp]));
}

/* Returns true if the next argument (as given by pa_next_peek) is a non-null
   string beginning with '-' */
int pa_next_isparam(void)
{
  char * p;

  p = pa_next_peek();
  if (p && p[0] && (p[0] != '-')) {
    return 1;
  }
  return 0;
}

char * pa_get_arg(void)
{
  char * rv;
  if (pa_sp < 0) {
    rv = 0;
  } else if (stk_nargs[pa_sp] <= 0) {
    rv = 0;
  } else {
    rv = *(stk_argv[pa_sp]);
    (stk_argv[pa_sp])++;
    (stk_nargs[pa_sp])--;
  }
  pa_this_arg = rv;
  return rv;
}

char * pa_stk_pop(void)
{
  pa_sp--;
  if (pa_sp < 0) {
    pa_this_arg = 0;
  } else if (stk_nargs[pa_sp] <= 0) {
    pa_this_arg = 0;
  } else {
    pa_this_arg = *(stk_argv[pa_sp]);
  }
  return pa_this_arg;
} /* End of pa_stk_pop */

/* Scan argv, parsing and executing arguments. When a --include/-p argument
   is encountered, open and scan the indicated file. Recursion is implemented
   by an explicit stack; this function does not call itself recursively. */
void parse_args(size_t nargs, char *argv[])
{
  int nv; /* Number of values returned by a sscanf */
  double tmp_dbl;
  int do_getarg;

  {
    /* Skip our program name/path */
    argv++;
    nargs--;

    /* Initialize the stack */
    pa_sp = 0;
    pa_argnum = 0;
    stk_nargs[pa_sp] = nargs;
    stk_argv[pa_sp] = argv;
  }

  pa_def_path = pa_defaults_path();

  while(pa_sp >= 0) {
    /* First figure out if we're going to auto-load the defaults */
    do_getarg = 1;
    if (pa_argnum == 0) {
      if (strcmp(pa_next_peek(), "-p") != 0) {
        /* They did *not* give an initial '-p' */
        if (pa_def_path) {
          pa_this_arg = pa_def_path;
          /* printf("Starting with defaults: %s\n", pa_this_arg); */
          do_getarg = 0;
        }
      }
    }

    /* Get the next argument at the present stacklevel */
    if (do_getarg) {
      pa_get_arg(); /* Sets pa_this_arg */
    }
    pa_argnum++;

    if (pa_this_arg == 0) {
      /* This happens if pa_get_arg has run out of args or if it gets an
         arg that is a null string. Either case means we should pop */
      pa_stk_pop();
    } else if (strcmp(pa_this_arg, "--ries-arguments-end") == 0) {
      /* Ignore any more arguments at this level */
      pa_stk_pop();

    } else if ((strncmp(pa_this_arg, "-p", 2) == 0)
            || (strcmp(pa_this_arg, "--include") == 0)) {
      /* load Parameters (or "Profile") from a file */

      /* First check for a bare '-p' and not on the first arg; if so and
         if there is a defaults file, we use it. */
      if (strcmp(pa_this_arg, "-p") == 0) {
        if (pa_argnum <= 1) {
          /* -p on the first arg means do not load the defaults; this will
             have already been handled so all we need to do now is ignore
             the option. */
          pa_this_arg = 0;
        } else {
          if (pa_def_path) {
            pa_this_arg = pa_def_path;
          } else {
            /* They gave a bare '-p' but there is no profile */
            printf(
                "%s: got -p option, but could not find .ries_profile or"
                                                       " ries_profile.txt\n"
                "in home directory.\n",
                g_argv0);
            brief_help();
            print_end(-1);
          }
        }
      }

      if (pa_this_arg && strcmp(pa_this_arg, "--include") == 0) {
        pa_get_arg();
        if (pa_this_arg == 0) {
          printf(
            "%s: --include requires a filename, e.g. '--include trig.ries'\n"
            "\n",
            g_argv0);
          brief_help();
          print_end(-1);
        }
      } else if (pa_this_arg) {
        pa_this_arg += 2;   /* skip the "-p" */
      }
      if (pa_this_arg && *pa_this_arg) {
        char * filebuf;
        size_t n;
        char * * av;
        filebuf = file_read(pa_this_arg);
        delimit_args(filebuf, &n, &av);
        if (n) {
          if ((pa_sp+1) < MAX_FILE_DEPTH) {
            /* Push new set of args onto the stack */
            pa_sp++;
            stk_nargs[pa_sp] = n;
            stk_argv[pa_sp] = av;
          } else {
            printf("%s: -p parameters nested too deep (max %d levels).\n"
              "\n", g_argv0, MAX_FILE_DEPTH);
            brief_help();
            print_end(-1);
          }
        } else {
          /* File had no tokens; we could complain but we let it pass. */
        }
      } else if (pa_this_arg) {
        printf(
            "%s: -p parameter requires a filename, e.g. '-ptrig.ries'\n"
            "\n", g_argv0);
          brief_help();
          print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "-") == 0) {
        /* Null option, useful for ending a string of numeric arguments,
           as e.g. after the --eval-expression option, before giving the
           target number. */

      /* First we check the "--foo-bar VAL" type options, in which the
         "opcode" and its "arguments" are each separate elements of argv[].
         These are used for special or rarely-used commands, like the
         command that gives an expression's complexity score */


    } else if (strcmp(pa_this_arg, "--any-exponents") == 0) {
      g_restrict_exponents = TYPE_NONE;

    } else if (strcmp(pa_this_arg, "--any-subexpressions") == 0) {
      g_restrict_subexpr = TYPE_NONE;

    } else if (strcmp(pa_this_arg, "--any-trig-args") == 0) {
      g_restrict_trig_args = TYPE_NONE;

    } else if (strcmp(pa_this_arg, "--canon-reduction") == 0) {
      if (pa_next_isparam()) {
        /* Set which types of canonval reduction to use. */
        char * s;
        s = pa_get_arg();
        if (s) {
          g_canon_ops = 0;
          while(*s) {
            switch(*s) {
              case 'n': ; g_canon_ops |= CANONVAL_NEGATE; break;
              case 'r': ; g_canon_ops |= CANONVAL_RECIPROCAL; break;
              case '2': ; g_canon_ops |= CANONVAL_MUL2; break;
              case '5': ; g_canon_ops |= CANONVAL_DIV2; break;
              default: ; break;
            }
            s++;
          }
        }
      } else {
        printf("%s: --canon-reduction requires a set of reduction types, "
            "e.g. 'nr25'\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--canon-simplify") == 0) {
      g_no_cv_simplify = B_FALSE;

    } else if (strcmp(pa_this_arg, "--derivative-margin") == 0) {
      /* Override default value of k_vanished.dx */
      ries_dif t;
      pa_get_arg();
      if (pa_this_arg && sscanf(pa_this_arg, "%lf", &t)) {
        if (t < 0) {
          printf("%s: --derivative-margin must be positive (I got %g).\n"
              "\n", g_argv0, t);
          brief_help();
          print_end(-1);
        } else if (t > 1.0e-4) {
          /* Too large */
          printf("%s: --derivative-margin value should be at most 1.0e-4.\n"
              "\n", g_argv0);
          brief_help();
          print_end(-1);
        } else {
          k_derivative_margin = t;
          printf("Allowing d/dx to be as small as value times %g.\n",
              k_derivative_margin);
        }
      } else {
        printf("%s: --derivative-margin should be followed by a numeric "
            "argument.\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--eval-expression") == 0) {
      if (pa_next_isparam()) {
        /* This is used to check FORTH expression syntax; it is a complement
           to --find-expression and some of the other command options
           like -F0 and -DGg */
        while(pa_next_isparam()) {
          if (g_num_find_expr > MAX_FIND_EXPR) {
            printf("%s: --eval-expression takes at most %d arguments.\n",
                g_argv0, MAX_FIND_EXPR);
            brief_help();
            print_end(-1);
          }
          g_find_expr[g_num_find_expr] = (symbol *) pa_get_arg();
          g_num_find_expr++;
        }
        g_enable_output = 0;
        g_eval_expr = 1;
      } else {
        printf("%s: --eval-expression should be followed by compact "
            "postfix expression(s).\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--explicit-multiply") == 0) {
      g_explicit_multiply = B_TRUE;

    } else if (strcmp(pa_this_arg, "--find-expression") == 0) {
      if (pa_next_isparam()) {
        /* This is used to scan for one of more expression(s) and print
           out their stats, in whatever order they happen to be found. This
           is an easier-to-use replacement for the -DGg option filtered
           through grep. */
        while(pa_next_isparam()) {
          if (g_num_find_expr > MAX_FIND_EXPR) {
            printf("%s: --find-expression takes at most %d arguments.\n",
                g_argv0, MAX_FIND_EXPR);
            brief_help();
            print_end(-1);
          }
          g_find_expr[g_num_find_expr] = (symbol *) pa_get_arg();
          g_num_find_expr++;
        }
        g_enable_output = 0;
      } else {
        printf("%s: --find-expression should be followed by compact "
            "postfix expression(s).\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if ((strcmp(pa_this_arg, "--match-all-digits") == 0)
            || (strcmp(pa_this_arg, "--mad") == 0)
    ) {
      g_match_all_digits = B_TRUE;
      k_max_match_dist = -0.01; /* This will be calculated from the target
                                   by init.2 */

    } else if (strcmp(pa_this_arg, "--max-equate-value") == 0) {
      ries_val t;
      pa_get_arg();
      if (pa_this_arg && sscanf(pa_this_arg, RV_SS_FMT, &t)) {
        if (t <= g_min_equ_val) {
          printf("%s: --max-equate-value argument cannot be greater than "
              "--min-equate-value argument.\n", g_argv0);
          brief_help();
          print_end(-1);
        }
        g_max_equ_val = t;
        printf("Equations will have both sides at most ");
        spfg(k_usable_digits, g_max_equ_val);
        printf("\n");
      } else {
        printf("%s: --max-equate-value should be followed by a numeric "
            "argument.\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--max-match-distance") == 0) {
      ries_dif t;
      pa_get_arg();
      if (pa_this_arg && sscanf(pa_this_arg, "%lf", &t)) {
        k_max_match_dist = t;
        g_match_all_digits = B_FALSE; /* Cannot use both options together */
        if (k_max_match_dist < 0) {
          printf(
              "First match must be closer than %g times your target value.\n",
              -k_max_match_dist);
        } else if (k_max_match_dist == 0) {
          printf("Only give an 'exact' match (if any) then exit.\n");
        } else {
          printf("First match must be closer than %g.\n", k_max_match_dist);
        }
      } else {
        printf("%s: --max-match-distance should be followed by a numeric "
            "argument.\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if ((strcmp(pa_this_arg, "--max-matches") == 0)
           || (strncmp(pa_this_arg, "-n", 2) == 0)) {
      double t;
      if (pa_this_arg[1] == 'n') {
        pa_this_arg += 2; /* Skip the '-n' */
      } else {
        pa_get_arg(); /* Get the next arg */
      }
      if (pa_this_arg && sscanf(pa_this_arg, "%lf", &t)) {
        if (t >= 1) {
          g_max_matches = (stats_count) t;
        } else {
          printf("%s: -n or --max-matches argument should be 1 or more.\n"
              "examples:  -n7  or  --max-matches 7\n"
              "\n", g_argv0);
          brief_help();
          print_end(-1);
        }
      } else {
        printf("%s: --max-matches should be followed by a numeric "
            "argument.\n"
            "examples:  -n7  or  --max-matches 7\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--max-memory") == 0) {
      time_flt t;
      pa_get_arg();
      if (pa_this_arg && sscanf(pa_this_arg, "%lf", &t)) {
        g_max_memory = fabs(t);
        printf("Will not use more than %g bytes of memory.\n",
            g_max_memory);
      } else {
        printf("%s: --max-memory should be followed by a numeric argument.\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--memory-abort-threshold") == 0) {
      time_flt t;
      pa_get_arg();
      if (pa_this_arg && sscanf(pa_this_arg, "%lf", &t)) {
        if (t > 1.0) {
          g_mem_bad_ratio = t;
          printf("Memory slowness abort ratio: %g.\n",
              g_mem_bad_ratio);
        } else {
          printf("%s: --memory-abort-threshold should be at least 1.0,"
              " and values less than about 1.5 are unlikely to be of"
              " much use.\n"
              "\n", g_argv0);
          brief_help();
          print_end(-1);
        }
      } else {
        printf(
            "%s: --memory-abort-threshold should be followed by a numeric\n"
            "argument larger than 1.0 (larger than 1.5 is recommended).\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--min-equate-value") == 0) {
      ries_val t;
      pa_get_arg();
      if (pa_this_arg && sscanf(pa_this_arg, RV_SS_FMT, &t)) {
        if (t >= g_max_equ_val) {
          printf("%s: --min-equate-value argument cannot be greater than "
              "--max-equate-value argument.\n", g_argv0);
          brief_help();
          print_end(-1);
        }
        g_min_equ_val = t;
        printf("Equations will have both sides at least ");
        spfg(k_usable_digits, g_min_equ_val);
        printf("\n");
      } else {
        printf("%s: --min-equate-value should be followed by a numeric "
            "argument.\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--min-match-distance") == 0) {
      /* %%% I might want to have "-ee" be a synonym for
             "--max-match-distance 0" */
      ries_dif t;
      pa_get_arg();
      if (pa_this_arg && sscanf(pa_this_arg, "%lf", &t)) {
        g_min_matchsize = t;
        if (g_min_matchsize == 0.0) {
          printf("Will exit if an 'exact' match is found.\n");
          g_exact_exit = B_TRUE;
        } else if (g_min_matchsize < -0.1) {
          printf("%s: --min-match-distance argument can be negative but not "
              "less than -0.1\n"
              "\n", g_argv0);
          brief_help();
          print_end(-1);
        } else if (g_min_matchsize < 0.0) {
          printf("Using a minimum match distance of %g times your "
              "target value.\n", g_min_matchsize);
        } else {
          printf("Using minimum match distance: %g\n", g_min_matchsize);
        }
      } else {
        printf("%s: --min-match-distance should be followed by a numeric "
            "argument.\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--max-trig-cycles") == 0) {
      ries_val t;
      pa_get_arg();
      if (pa_this_arg && sscanf(pa_this_arg, RV_SS_FMT, &t)) {
        if ((t > 0.0) && (t <= 100.0)) {
          k_sincos_max_arg = (ries_val) t;
          printf("Arguments of trig functions will be restricted to ");
          spfg(k_usable_digits, k_sincos_max_arg * 0.5);
          printf(" cycles on either side of zero.\n");
        } else {
          printf("%s: --max-trig-cycles must be between 0.0 and 100.0\n\n",
                                                                    g_argv0);
          brief_help();
          print_end(-1);
        }
      } else {
        printf("%s: --max-trig-cycles should be followed by a numeric "
          "argument.\n"
          "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--min-memory") == 0) {
      time_flt t;
      pa_get_arg();
      if (pa_this_arg && sscanf(pa_this_arg, "%lf", &t)) {
        g_min_memory = fabs(t);
        printf("Memory-hogging safeguard disabled for the first %g bytes.\n",
            g_min_memory);
      } else {
        printf("%s: --min-memory should be followed by a numeric "
            "argument.\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--no-canon-simplify") == 0) {
      g_no_cv_simplify = B_TRUE;

    } else if (strcmp(pa_this_arg, "--no-refinement") == 0) {
      g_refinement = 0;

    } else if (strcmp(pa_this_arg, "--no-slow-messages") == 0) {
      g_allow_slow_message = 0;

    } else if (strcmp(pa_this_arg, "--no-solve-for-x") == 0) {
      g_solve_for_x = B_FALSE;

    } else if (strcmp(pa_this_arg, "--numeric-anagram") == 0) {
      if (pa_next_isparam()) {
        /* Remember the selected anagram string, we test this later */
        g_anagram = (char *) pa_get_arg();
        /* Set the sym_allowed values */
        set_anagram(g_anagram);
      } else {
        printf(
            "%s: --numeric-anagram should be followed by a string of digits.\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--one-sided") == 0) {
      g_one_sided = B_TRUE;

    } else if (strcmp(pa_this_arg, "--rational-exponents") == 0) {
      g_restrict_exponents = TYPE_RAT;

    } else if (strcmp(pa_this_arg, "--rational-trig-args") == 0) {
      g_restrict_trig_args = TYPE_RAT;

    } else if (strcmp(pa_this_arg, "--relative-roots") == 0) {
      /* Show "x = T + epsilon" rather than absolute values  */
      if (g_match_all_digits) {
        printf("Note: Ignoring '%s' because '--match-all-digits' is set.\n",
            pa_this_arg);
      } else {
        g_relative_x = B_TRUE;
      }

    } else if (strcmp(pa_this_arg, "--show-work") == 0) {
      /* Same as -Ds (sets debug_s flag) */
      set_debug_opts((char *) "s");

    } else if (strcmp(pa_this_arg, "--significance-loss-margin") == 0) {
      ries_dif t;
      pa_get_arg();
      if (pa_this_arg && sscanf(pa_this_arg, "%lf", &t)) {
        if ((t >= 0.0) && (t < 100.0)) {
          t = pow(10.0, -t);
          k_sig_loss = t;
          printf("Using significance loss margin: %g\n", k_sig_loss);
        } else {
          printf("%s: --significance-loss-margin must be between 0.0 "
              "and 100.0\n"
              "\n", g_argv0);
          brief_help();
          print_end(-1);
        }
      } else {
        printf(
            "%s: --significance-loss-margin should be followed by a numeric "
            "argument.\n"
            "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--symbol-names") == 0) {
      if (pa_next_isparam()) {
        char space_sym = ' ';
        /* Override the standard symbol names. Multiple arguments may
           be given. */
        while(pa_next_isparam()) {
          char * a;
          symbol sym;
          a = pa_get_arg();
          if (  (a[0] == ':') && a[1]
             && (a[2] == ':') && (a[3]==a[1]) && (a[4] == 0)
             && (space_sym == ' ')) {
            /* This syntax is used to define a symbol that stands in for
               blank space. */
            space_sym = a[1];
          } else if ((a[0] == ':') && a[1] && (a[2] == ':')) {
            sym = (symbol) a[1];
            if (strlen(a+3) <= MAX_SYM_NAME_LEN) {
              str_remap(a+3, space_sym, ' ');
              sym_attrs[sym].sa_name =
              sym_attrs[sym].name_forth = a+3;
              /* printf("setsym %c:%s\n", sym, a+3); */
            } else {
              printf("%s: Symbol name can be at most %d characters\n"
                  "(I got '%s')\n", g_argv0, MAX_SYM_NAME_LEN, a+3);
              print_end(-1);
            }
          } else {
            printf("%s: --symbol-names argument syntax is :<sym>:name,"
                                                              " for example\n"
                "  :-:deme   to set name of '-' to 'deme'\n"
                "  Instead I got '%s'\n", g_argv0, a);
            print_end(-1);
          }
        }
      } else {
        printf(
            "%s: --symbol-names should be followed by one or more tuples\n"
            "  of the form <sym>:name, for example:\n"
            "  :-:deme   to set name of '-' to 'deme'\n", g_argv0);
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--symbol-weights") == 0) {
      if (pa_next_isparam()) {
        /* Override the standard symbol weights. Multiple arguments may
           be given. */
        while(pa_next_isparam()) {
          char * a;
          unsigned char argtmp[20];
          int i; ries_dif w;
          a = pa_get_arg();
          ries_strncpy((char *) argtmp, a, 20);
          /* Skip the numeric portion */
          for(i=0;
              (argtmp[i]=='.') || ((argtmp[i]>='0') && (argtmp[i]<='9'));
              i++ ) {  }
          if ((argtmp[i] == ':') && (argtmp[i+1])) {
            argtmp[i++] = 0; /* Null-terminate the numeric portion by
                                overwriting the ':' */
            w = strtod((char *) argtmp, 0);
            if (w <= MIN_SYMBOL_WEIGHT) {
              printf("%s: Symbol weight may not be %f or less.\n",
                g_argv0, MIN_SYMBOL_WEIGHT);
              print_end(-1);
            }
            if (w > MAX_SYMBOL_WEIGHT) { w = MAX_SYMBOL_WEIGHT; }
            /* printf("set weight of '%c' to %d\n", argtmp[i], (int) w); */
            sym_attrs[argtmp[i]].preempt_weight = (s16) floor(w + 0.5);
          } else {
            printf("%s: --symbol-weights argument syntax is NUMBER:<sym>,"
                                                              " for example\n"
                "  12:^   to set weight of '^' to 12\n"
                "  Instead I got '%s'\n", g_argv0, a);
            print_end(-1);
          }
        }
      } else {
        printf(
            "%s: --symbol-weights should be followed by one or more tuples\n"
            "  of the form NUMBER:<sym>, for example"
                             " 12:^   to set weight of '^' to 12\n", g_argv0);
        print_end(-1);
      }

    } else if (strcmp(pa_this_arg, "--trig-argument-scale") == 0) {
      ries_val t;
      pa_get_arg();
      if (pa_this_arg && sscanf(pa_this_arg, RV_SS_FMT, &t)) {
        if ((t >= 0.0) && (t < 100.0)) {
          k_sincos_arg_scale = (ries_val) t;
          g_trig_scale_default = (k_sincos_arg_scale == k_pi);
          printf("Argument of trig functions will be scaled by ");
          spfg(k_usable_digits, k_sincos_arg_scale); /* printf(fmt_g_usable, k_sincos_arg_scale); */
          printf("\n");
        } else {
          printf("%s: --trig-argument-scale must be between 0.0 "
            "and 100.0\n"
            "\n", g_argv0);
          brief_help();
          print_end(-1);
        }
      } else {
        printf("%s: --trig-argument-scale should be followed by a numeric "
          "argument.\n"
          "\n", g_argv0);
        brief_help();
        print_end(-1);
      }

    } else if ((strncmp(pa_this_arg, "-s", 2) == 0)
            || (strcmp(pa_this_arg, "--try-solve-for-x") == 0)) {
      g_solve_for_x = B_TRUE;

    } else if (strcmp(pa_this_arg, "--version") == 0) {
      show_version();
      exit(0);

    } else if ((strcmp(pa_this_arg, "--wide-output") == 0)
            || (strcmp(pa_this_arg, "--wide") == 0)
    ) {
      g_wide_output = B_TRUE;
      g_relative_x = B_TRUE;

    /* Single-character arguments:
     { %% those in braces are not yet implemented }
       -0 to -9    target number with leading - sign
       -a          Algebraic class
       -c          Constructible class
             -D    Debug
     { -e          Elementary class (between algebraic and Liouvillian) }
     { -ee         Exact exit (might be confused with -e plus 'exact') }
             -F    Format
       -i          Integer class (and -ie)
       -l          Liouvillian (if bare); level (if with digit)
             -N    Not these symbols
             -O    Once-only symbols
       -p          profile/parameters (parsed above)
       -r          Rational class (and -re)
       -s          (try to) solve for x
             -S    Symbolset
     { -t          Transcendental class (enable [W], [G], etc.) }
       -x          Show X, not T+epsilon
     */
    /* Next we check the "-xN" type options, in which the "opcode" is a
       single letter and its "arguments" follow it without a space in
       between. These are used for the options you'll commonly want to
       give when invoking ries to solve a problem. */


    } else if ((strncmp(pa_this_arg, "-a", 2) == 0)
           || (strcmp(pa_this_arg, "--algebraic-subexpressions") == 0)) {
      /* Subexpressions must be algebraic numbers */
      set_restrict_alg(1);
      if (pa_this_arg[2] == 'e') {
        /* They gave "-ae"
           %%% This should instead set an epsilon as with
           --min-match-distance with a negative argument proportional
           to the ULP as measured by init.formats */
        g_exact_exit = B_TRUE;
      }

    } else if ((strncmp(pa_this_arg, "-c", 2) == 0)
           || (strcmp(pa_this_arg, "--constructible-subexpressions") == 0)) {
      /* Subexpressions must be constructible numbers */
      g_restrict_subexpr = TYPE_CONS;
      if (pa_this_arg[2] == 'e') {
        /* They gave "-ce": exit on exact match.
           NOTE: We already set k_min_best_match proportionally
           to the target size */
        g_exact_exit = B_TRUE;
      }
      /* This option is just shorthand for turning off a bunch of
         functions. */
      somesyms_set((symbol *) "peASCTl^vLEW", 0);
      somesyms_set((symbol *) "+-*/nrsqf", MAX_ELEN);
      somesyms_set((symbol *) "x", 1);
      /* %%% Once I add an integer arguments option for ^, I can enable it
        on RHS and in non-x-containing subexpressions in LHS. */

    } else if (strncmp(pa_this_arg, "-D", 2) == 0) {
      /* Debugging options */
      set_debug_opts(pa_this_arg+2); /* +2 to skip the "-D" */

    } else if (strncmp(pa_this_arg, "-E", 2) == 0) {
      /* Enable these symbols: Like -S but doesn't clear everything else
         out */
      NOS_options = B_TRUE;
      somesyms_set((symbol *) (pa_this_arg+2), MAX_ELEN); /* +2 skips "-E" */

    } else if (strncmp(pa_this_arg, "-F", 2) == 0) {
      /* Select expression display format */
      pa_this_arg += 2;   /* skip the "-F" */
      if (*pa_this_arg == 0) {
        out_expr_format = OF_FORTH; /* default */
      } else {
        nv = sscanf(pa_this_arg, "%d", &out_expr_format);
        if (nv == 0) {
          printf("%s: -F parameter requires a number, e.g. '-F0'\n"
              "\n", g_argv0);
          brief_help();
          print_end(-1);
        }
      }

    } else if ((strncmp(pa_this_arg, "-i", 2) == 0)
            || (strcmp(pa_this_arg, "--integer-subexpressions") == 0)) {
      /* Integer subexpressions */
      g_restrict_subexpr = TYPE_INT;
      somesyms_set((symbol *) "pefqASCTlvLEW", 0);
      if (pa_this_arg[2] == 'e') {
        /* They gave "-ie" */
        g_exact_exit = B_TRUE;
      }

    } else if (nv=sscanf(pa_this_arg, "-l%lf", &tmp_dbl), nv>0) {
      /* They gave a level eg. "-l-1" or "-l5.5" */
      g_levadj = tmp_dbl;
      tlevel = DEFAULT_LEV_BASE + g_levadj;
    } else if ((strcmp(pa_this_arg, "-el") == 0)
            || (strcmp(pa_this_arg, "-l") == 0)
            || (strcmp(pa_this_arg, "--liouvillian-subexpressions") == 0)
            || (strcmp(pa_this_arg, "-le") == 0)
    ) {
      /* We have "-el", a bare "-l", or "--liou..." without the "--"; in
         either case this means we want to restrict to Liouvillian roots */
      set_restrict_alg(0);
      /* Enable exponential and logarithmic functions (but not Gamma or
         LambertW) */
      somesyms_set((symbol *) "eplEL", MAX_ELEN);
      /* To disallow x within an exponent, but still allow anything
         else in an exponent, we set g_restrict.subexpr to TYPE_TRAN
         which causes g_target to be tagged as transcendental. Then
         we restrict exponents and trigonometric arguments to elementary,
         allowing even something like e^(2^(1/phi)). This allows
         sqrt(2)^sqrt(2) to be found, but prevents finding the root of
         x^x=7. */
      g_restrict_subexpr = TYPE_TRAN;
      g_restrict_exponents = TYPE_TCEL;
      g_restrict_trig_args = TYPE_TCEL;
      if (pa_this_arg[2] == 'e') {
        /* They gave "-le"
           %%% This should instead set an epsilon as with
           --min-match-distance with a negative argument proportional
           to the ULP as measured by init.formats */
        g_exact_exit = B_TRUE;
      }
    } else if (strncmp(pa_this_arg, "-l", 2) == 0) {
      printf("%s: -l parameter requires a number, e.g. '-l3'\n"
          "\n", g_argv0);
      brief_help();
      print_end(-1);

    } else if (strncmp(pa_this_arg, "-N", 2) == 0) {
      /* Not these symbols */
      NOS_options = B_TRUE;
      somesyms_set((symbol *) (pa_this_arg+2), 0); /* +2 skips "-N" */

    } else if (strncmp(pa_this_arg, "-O", 2) == 0) {
      /* Once-only symbols */
      NOS_options = B_TRUE;
      somesyms_set((symbol *) (pa_this_arg+2), 1); /* +2 skips "-O" */

    } else if ((strncmp(pa_this_arg, "-r", 2) == 0)
             || (strcmp(pa_this_arg, "--rational-subexpressions") == 0)) {
      /* Rational subexpressions */
      set_restrict_rat();
      if (pa_this_arg[2] == 'e') {
        /* They gave "-re" */
        g_exact_exit = B_TRUE;
      }

    } else if (strncmp(pa_this_arg, "-S", 2) == 0) {
      /* Only these symbols */
      pa_this_arg += 2;   /* skip the "-S" */
      if (*pa_this_arg == 0) {
        /* Without args, show the symbols in use and their definitions */
        g_show_ss = B_TRUE;
      } else {
        S_option = B_TRUE;
        NOS_options = B_TRUE;
        allsyms_set(0, 0);
        somesyms_set((symbol *) pa_this_arg, MAX_ELEN);
      }

    } else if ((strcmp(pa_this_arg, "-x") == 0)
            || (strcmp(pa_this_arg, "--absolute-roots") == 0)) {
      /* Show values of x rather than "x = T + epsilon" */
      if (g_wide_output) {
        /* -x is incompatible with wide mode (which shows both types of x
           value output) */
        printf("Note: '%s' option with '--wide' is redundant.\n",
            pa_this_arg);
      } else {
        g_relative_x = B_FALSE;
      }

    /* test for number must be last, because it might have a leading '-' */
    } else if (((pa_this_arg[0] >= '0') && (pa_this_arg[0] <= '9'))
             || (pa_this_arg[0] == '-') || (pa_this_arg[0] == '.')) {
      /* This would be the target number */
      nv = parse_target(pa_this_arg);
      if (nv == 1) {
        g_got_target = 1;
      } else {
        printf("%s: Unknown option '%s'\n\n", g_argv0, pa_this_arg);
        brief_help();
        print_end(-1);
      }
    } else {
      printf("%s: Unknown option '%s'\n\n", g_argv0, pa_this_arg);
      brief_help();
      print_end(-1);
    }
  }
  /* At this point pa_sp <= 0 and stk_nargs[pa_sp] == 0, so we're out of
     args */

} /* End of parse.args */

/* We use precise and sized datatypes (e.g. 16-bit integer), and
/  in some cases (old compilers) we need to define these with a custom
/  typedef. This function makes sure the definitions work the way we need
/  and complains if not. We can also tell the user which ifdef flag to
/  use when re-compiling.                                                   */
void validate_types(void)
{
#ifdef VT_TEST_CMP1
  {
    int i; unsigned u;
    i = 1; u = ((unsigned) -2);
    printf("i %08x  u %08x\n", i, u);
    printf("cmp1 %s\n", (u<i) ? "T" : "nil");
    printf("cmp1 %s\n", (((signed)u)<i) ? "T" : "nil");
    printf("cmp1 %s\n", (u<((unsigned)i)) ? "T" : "nil");
  }
#endif

  b001 flag1, flag2;
  flag1 = B_TRUE;
  flag2 = (1==1);

  if (flag1 == flag2) {
    /* All is well */
  } else {
    /* Non-standard compiler and/or non-standard definition of B_TRUE: This
    /  happens if "b001" is int, "B_TRUE" is 1, and "(1==1)" is __INT_MIN__,
    /  or something similar. */
    printf("validate_types: B_TRUE does not match (1==1).\n");
    print_end(-1);
  }

  if (sizeof(s16) != 2) {
    printf("validate_types: s16 is not 2 bytes (got %d).\n",
      (int) sizeof(s16));
    if (sizeof(short) == 2) {
      printf(
        "  To fix this error, compile RIES with the flag -DSHORT_IS_S16\n");
    } else if (sizeof(int) == 2) {
      printf(
        "  To fix this error, compile RIES with the flag -DINT_IS_S16\n");
    } else {
      printf(
        "  (presently, sizeof(short)==%d and sizeof(int)==%d)\n",
        (int) sizeof(short), (int) sizeof(int));
    }
    print_end(-1);
  }

  if (sizeof(s32) != 4) {
    printf("validate_types: s32 is not 4 bytes (got %d).\n",
      (int) sizeof(s32));
    if (sizeof(int) == 4) {
      printf(
        "  To fix this error, compile RIES with the flag -DINT_IS_S32\n");
    } else if (sizeof(long) == 4) {
      printf(
        "  To fix this error, compile RIES with the flag -DLONG_IS_S32\n");
    } else {
      printf(
        "  (presently, sizeof(int)==%d and sizeof(long)==%d)\n",
        (int) sizeof(int), (int) sizeof(long));
    }
    print_end(-1);
  }
} /* End of validate.types */

/* Try to make a guess as to whether a value is integer or rational.
   Due to the main purpose of the rational tags (filtering the types of
   arguments to transcendental functions) we only count it as rational
   if it's a half or quarter-integer.
       %%% tgs-manip: We could really go nuts and run a continued
   fraction series calculation, but that's second-guessing the user. I'd
   rather add a syntax allowing the user to explicitly give a fraction as
   the target number, and that's sort of counter to the purpose of RIES.
   Note that if the user knows what type the target is, she can use an
   option like -a or -c to specify it as such. */
ries_tgs guess_valtype(ries_val v)
{
  if (v == FLOOR(v)) {
    return TYPE_INT;
  } else if (FLOOR(v*4.0) == (v*4.0)) {
    /* Target is a half-integer or quarter-integer */
    return TYPE_RAT;
  }
  /* Set target class based on selected restriction, but not better than
     rational since we know it's not an integer. We are giving it
     the benefit of the doubt: in fact, *every* target value is
     rational because it is specified in terms of a finite number of
     decimal digits.
       If they set --rational-exponents (including the -a option),
     g_restrict_exponents will be set to TYPE_RAT and if this target
     type is also lower than RAT, we'll prohibit x in the exponent. */
  return TGMIN(TYPE_RAT, g_restrict_subexpr);
} /* End of guess.valtype */

int main(int nargs, char *argv[])
{
  stats_count genf;
  ries_val tgt_to_print;
  stats_count lim_gentotal;
  stats_count lim_prune;
  int ml_going;

#ifdef OLD_EXHAUSTION_TESTS
  s16 timeout;
  timeout = 0;
#endif

  validate_types();
  ieee_paranoia();
  init_formats();

  g_argv0 = argv[0];  /* Used in various sudden-death printf's */

  init1();

  g_got_target = 0;

  /* parse arguments. Note that parse.args is a recursive algorithm with
     its own stack, executing nested loops for the --include option. */
  if (nargs > 1) {
    parse_args((size_t) nargs, argv);
  } else {
    printf("%s: Please specify a target number.\n"
      "\n", g_argv0);
    brief_help();
    print_end(1);
  }

  /* init the evaluation system */
  init2();

  /* Execute the '-S' command */
  if (g_show_ss) {
    show_symset();
    /* We exit because the main purpose of a bare -S is to learn the weights.
       However the user may have expected more, so we explain the necessary
       -S syntax. */
    printf("%s: %s", g_argv0,
"Exiting now (to do an equation search, omit '-S' or include\n"
"  symbol names, for example: 'ries 3.14159 -S123456789+/')\n");
    exit(0);
  }

  if (g_got_target) {
    g_targ_tags = guess_valtype(g_target);
  }

  /* printf("g_restrict_subexpr is %s\n", tagname(g_restrict_subexpr)); */

  /* Here we tag the non-integer constants as being whatever type
     is set as the subexpressions restriction. This would allow the
     constants to be used as subexpressions, but the symbols are
     also excluded via e.g sym_attrs['p'].sa_alwd, so %%% I don't
     know why we even bother to do this */
  /* printf("valtype of pi is %s", tagname(tg_pi)); */
  if (g_restrict_subexpr > tg_pi) {
    tg_pi = TGMIN(TYPE_RAT, g_restrict_subexpr);
  }
  /* printf(" -> %s\n", tagname(tg_pi)); */
  if (g_restrict_subexpr > tg_phi) {
    tg_phi = TGMIN(TYPE_RAT, g_restrict_subexpr);
  }
  if (g_restrict_subexpr > tg_e) {
    tg_e = TGMIN(TYPE_RAT, g_restrict_subexpr);
  }

  /* printf("sym_attrs['p'].sa_alwd == %d\n", sym_attrs['p'].sa_alwd); */

  /* Execute the --eval-expression option */
  if (g_eval_expr && g_num_find_expr) {
    s16 i, err, sp, contains_x;
    ries_val x; ries_dif dx; ries_tgs tg;
    symbol * expr;
    exec_x = g_target;
    for(i=0; i<g_num_find_expr; i++) {
      expr = g_find_expr[i];
      printf("Evaluating postfix expression '%s'", expr);
      contains_x = (symstrsym(expr, 'x') != 0);
      if (contains_x) {
        printf(" with x=");
        spff(k_usable_digits, exec_x); /* printf(fmt_g_usa_fixed, exec_x); */
      }
      printf("\n");
      err = eval(expr, &x, &dx, &tg, &sp, 1); /* In main() */
      if (err) {
        printf("Error %d from eval: %s\n", err, err_string(err));
      } else if (sp > 0) {
        /* The other clients of eval() always have complete expressions,
           so eval() does not check for this error */
        printf("Error: incomplete expression (%d item(s) remain on stack)\n",
          sp);
      } else {
        printf("[%s] = ", expr);
        spfg(k_usable_digits, x); /* printf(fmt_g_usable, x); */
        printf("; d/dx = ");
        printf(fmt_g_diff, dx);
        printf(" %s", tagname(tg));
        printf(", complexity = {%d}\n", complexity(expr));
      }
      printf("\n");
    }
    exit(0);
  }

  if (g_got_target == 0) {
    printf("%s: Please specify a target number.\n"
      "\n", g_argv0);
    brief_help();
    print_end(1);
  }

  /* -i option is of no use when X isn't an integer, because X is a
   * subexpression of all LHS's; demote to rational.*/
  if (g_restrict_subexpr == TYPE_INT) {
    if (!(TAG_INT_P(g_targ_tags))) { /* tgs-manip */
      if (g_enable_output) {
        printf("ries: Replacing -i with -r because target isn't an integer.\n");
      }
      set_restrict_rat();
      /* g_exact_exit setting is still relevant */
    } else if (FABS(g_target) > rv_maxint) {
      if (g_enable_output) {
        printf("ries: Replacing -i with -r because target is too large.\n");
      }
      set_restrict_rat();
      /* g_exact_exit setting is still relevant */
    }
  }

  /* Set or adjust target value and match distance cutoffs. */
  tgt_to_print = g_target; /* Save user-specified value for printing */
  if (g_match_all_digits) {
    if (g_mag_ulp > 0) {
      /* Tweak the target and set a max match distance */
      k_max_match_dist = g_mag_ulp * 0.5;
      if (g_target < 0) {
        g_target -= k_max_match_dist;
      } else {
        g_target += k_max_match_dist;
      }
      /* In the case where they gave an integer or rational with
         --match-all-digits, 1/2 of the ULP is added to the target
         which is therefore rational. */
      if (g_targ_tags == TYPE_INT) { g_targ_tags = TYPE_RAT; } /* tgs-manip */

      /* For --match-all-digits to make sense the roots should be displayed
         as absolute numbers, not "x = T + epsilon" */
      if (g_wide_output == B_FALSE) {
        g_relative_x = B_FALSE;
      }
      /* printf("Setting target: %g and k_max_match.dist %g\n",
         g_target, k_max_match_dist); */
    } else {
      /* %%% We could handle this as "--max-match-distance 0", but it's
         safer to assume we don't know the exact value of the last digit(s)
         and therefore cannot match them. */
      printf(
        "Ignoring --match-all-digits option because I could not determine\n"
        "the magnitude of the ULP (unit in the last place).\n"
      );
    }
  }

  if (k_derivative_margin > 0) {
    /* They selected a specific initial k_vanished.dx */
    k_vanished_dx = k_derivative_margin;
  } else if (fabs((ries_dif)g_target) * k_vanished_dx > 0.1) {
    k_vanished_dx = 0.1 / fabs((ries_dif)g_target);
    printf("Auto-setting --derivative-margin %g\n", k_vanished_dx);
  } else if (fabs((ries_dif)g_target) < 1.0e-8) {
    k_vanished_dx = 0.1 * fabs((ries_dif)g_target);
    printf("Auto-setting --derivative-margin %g\n", k_vanished_dx);
  }
  if (debug_z) {
    printf("fabs(g_target) == %g; k_vanished_dx == %g;\n",
            fabs((ries_dif)g_target), k_vanished_dx);
    printf("fabs(g_target) * k_vanished_dx == %g\n",
            fabs((ries_dif)g_target) * k_vanished_dx);
  }

  /* If k_vanished.dx changed, make k_prune.deriv agree */
  k_prune_deriv = k_vanished_dx / 1.0e4;
  p_ovr = fabs((ries_dif)g_target) / k_prune_deriv;
  n_ovr = - p_ovr;
  if (debug_z) {
    printf("k_prune_deriv == %g; p_ovr == %g\n", k_prune_deriv, p_ovr);
  }

  if (k_vanished_dx >= 1.0/FABS(g_target)) {
    printf(
"%s: With this --derivative-margin option (%g) and\n"
"  target value (%g), I cannot compute any expressions.  Either\n"
"  use a smaller target value, or a derivative margin below %g\n\n",
        g_argv0, k_vanished_dx, dbl(g_target), 1.0/fabs((ries_dif)g_target));
    print_end(-1);
  }
  if (FABS((ries_dif)g_target) / k_vanished_dx >= 1.0e15) {
    if (sym_attrs['x'].sa_alwd > 1) {
      if (k_derivative_margin > 0) {
        printf(
"WARNING: Your --derivative-margin option (%g)\n"
"  is so small that RIES might report tautologies like x/x = 1. To avoid\n"
"  this warning, use the option -Ox or give a target value smaller than\n"
"  %g.\n\n",
          k_derivative_margin, k_biggest_safe_target);
      } else {
        printf(
"WARNING: Your target value is so large that RIES might report tautologies\n"
"  like x/(x/3) = 3. To avoid this warning, use the option -Ox or give a\n"
"  target value smaller than %g.\n\n",
          k_biggest_safe_target);
      }
    }
  }

  if (g_min_matchsize < 0.0) {
    /* Variable minimum match threshold */
    g_min_matchsize = fabs((ries_dif)g_target * (-g_min_matchsize));
  }

  if (k_max_match_dist >= 0) {
    /* They have set a fixed starting match threshold */
    g_init_match_dist = k_max_match_dist;
  } else {
    /* Variable starting match threshold */
    g_init_match_dist = fabs((ries_dif)g_target * (-k_max_match_dist));
  }
  best_match = g_init_match_dist;
  if (debug_q) {
    printf("Initial match threshold = %g\n", dbl(best_match));
  }
  /* Adjust k_min_best.match to accomodate precision and large targets.
     %%% We do this for large-magnitude targets and for targets close to 0.
     I still need to look more closely at how I handle SIG_LOSS errors
     and how that affects operation when the target is close to zero. */
  if ((FABS(g_target) > 1.0) || (FABS(g_target) < 0.25)) {
    /* %%% 20170211: The numbers here should be adjusted in such a way that
       there is a smooth transition when the target goes outside the default
       range. To check, do commands like "ries 1.001 -Dz | grep kmbm" and
       "ries 0.249 -Dz | grep kmbm" and note what kmbm is being set to. */
    k_min_best_match = fabs((ries_dif)g_target) * 8.0 * k_precision_ulp;
    if (debug_z) {
      if (FABS(g_target) > 1.0) {
        printf("Large target, setting kmbm=%g\n", k_min_best_match);
      } else {
        printf("Small target, setting kmbm=%g\n", k_min_best_match);
      }
    }
  }
  exec_x = g_target;

  if (g_anagram != 0) {
    g_one_sided = B_TRUE;
  }

  /* --------------------------------------------------------------------------
               Finished adjusting parameters; start algorithm
  -------------------------------------------------------------------------- */

  if (g_enable_output) {
    char fmt1[FMT_STR_SIZE];
    printf("\n");
    printf("   Your target value: T = ");
    spff(k_usable_digits, tgt_to_print); /* printf(fmt_g_usa_fixed, tgt_to_print) */
    /* If robustness were not important we could use the "%*" printf
       extension to get the variable width */
    snprintf(fmt1, FMT_STR_SIZE, "%s%d%s", "  %", 44 - k_usable_digits, "s"); /* "  %27s" */
    printf(fmt1, "mrob.com/ries");
    printf("%s", "\n\n");
  }

  /* printing symbol weight limits for debugging */
  if (debug_z) {
    printf("Symbol weight ranges:\n");
    printf(" seft  num  minw  maxw\n");
    printf("   %c   %3d   %3d   %3d\n", 'a', n_asym, g_a_minw, g_a_maxw);
    printf("   %c   %3d   %3d   %3d\n", 'b', n_bsym, g_b_minw, g_b_maxw);
    printf("   %c   %3d   %3d   %3d\n", 'c', n_csym, g_c_minw, g_c_maxw);
  }

  /* calculate search cutoff. This number is the total number of
   * expressions generated. Note that the total number of possible
   * combinations between LHS and RHS is much more: If N expressions
   * are generated and equally divided between LHS and RHS, the total
   * number of combinations is N^2/4. Thus, we get 4 times as many
   * combinations each time we double this number.
   *   The user interface says that each level will produce a 10-times
   * greater search. This means checking 10 times as many combinations.
   * however, as the number of combinations rises they also get more
   * spread out, because more very large and very small numbers are
   * getting generated. Because of this spreading out, LHS and RHS
   * expressions become slightly less likely to be near each other as
   * the number of combinations goes up. Therefore, in order to get 10
   * times more "relevant" combinations, we multiply by factors of 4,
   * since 4^2 is 16, somewhat more than 10. */
  lim_gentotal = 2000.0;
  if (tlevel >= -2.0) {
    lim_gentotal *= pow(4.0, tlevel);
  }
  lim_prune = lim_gentotal * 15.0;
  gen_total = 0;
  if (debug_y) {
    printf("Will stop after generating searchmax=%g expressions\n",
      lim_gentotal);
  }

#ifdef RIES_USE_SA_M64
  /* Test of new Lambert W function in msal_math64.c */
  if (debug_z) {
# ifdef RIES_VAL_LDBL
    msal_test_lambertl();
# else
    msal_test_lambert();
# endif
    msal_test_gamma();
    msal_test_spfg();
  }
#endif

  /* generate and print solutions */
  lmin = rmin = 1;
  lmax = rmax = PASS_GRAN;
  rhs_gen = lhs_gen = 0;
  rhs_insert = lhs_insert = 0;
  rhs_prune = lhs_prune = 0;
  got_exact = B_FALSE;
  g_num_matches = 0;
  ml_going = 1;
  while (ml_going) {
    /* We increase the complexity limit of whichever side has generated
       the least number of expressions so far. This is to ensure that we
       stay near the optimal point where the search time is O(sqrt(K^N)).
       If we let one side outnumber the other by a lot, the search gets
       to be more like O(K^N), and we don't want that (-:

       Note that we are going by the actual number of LHS and RHS items
       in the database, rather than maintaining an equal complexity limit
       for both types of expressions. This is to adapt gracefully to
       changes in the symbol
       set, which of course can be specified on the command line. For
       example, if they significantly limit the number of constants,
       the LHS will grow at a faster rate than the RHS because its one
       additional type-a symbol ('x') becomes a significant factor in how
       many expressions there are of each complexity.

       Since LHS and RHS are stored in the same struct type, there is no
       memory savings from storing fewer LHS's, however there might be
       a bit of speed cost from evaluating the derivatives.
       storing them. Some kind of benchmarking would be in order.

       For the --one.sided option, the main test is altered so that
       it inserts x as the sole LHS expression and then performs RHS
       passes from then on.
    */

    g_ne = 0; insert_count = 0; prune_count = 0;
    if (
         (g_one_sided && (lhs_insert >= 1))   /* We're in RHS-only mode and
                                                 we have our 'x' */
      || (lhs_insert > rhs_insert))         /* Or normal mode and LHS
                                               outnumbers RHS */
    {
      rmin = (s16)(rmin + PASS_GRAN);
      rmax = (s16)(rmax + PASS_GRAN);

      /* Generate RHS expressions */
      g_dbg_side = DBG_RHS;
      genf = gen_forms(rmin, rmax, 0,
                  g_a_minw, g_a_maxw, g_b_minw, g_b_maxw, g_c_minw, g_c_maxw);
      if (debug_y) {
        printf("finished RHS from {%d} to {%d}: %g forms expanded, %g expressions.\n",
               rmin, rmax, genf, g_ne);
      }
      rhs_gen += g_ne;
      rhs_insert += insert_count;
      rhs_prune += prune_count;
    } else {
      lmin = (s16)(lmin + PASS_GRAN);
      lmax = (s16)(lmax + PASS_GRAN);

      /* Generate LHS expressions */
      g_dbg_side = DBG_LHS;
      genf = gen_forms(lmin, lmax, 1,
                  g_a_minw, g_a_maxw, g_b_minw, g_b_maxw, g_c_minw, g_c_maxw);
      if (debug_y) {
        printf("finished LHS from {%d} to {%d}: %g forms expanded, %g expressions.\n",
               lmin, lmax, genf, g_ne);
        printf("\n");
      }
      lhs_gen += g_ne;
      lhs_insert += insert_count;
      lhs_prune += prune_count;
    }
    gen_total += g_ne;

#ifdef OLD_EXHAUSTION_TESTS
    /* Test the number of new expressions found. Reset the timeout if we
       found new expressions. If we go too many passes and find nothing, the
       search will terminate.
         This test prevents an infinite loop in the case where the
       searchmax is so big (or the symbolset so small) that all possible
       expressions of size MAX_ELEN are generated becore searchmax is
       reached. (If this actually happens, it probably means MAX_ELEN should
       be increased.) */
    if (g_ne > 0) {
      timeout = 0;
    } else {
      timeout++;
    }
    /* %%% with -i and/or -S it's not very reliable. Instead of timing
     * out we should use an explicit test: is complexity minimum
     * larger than max complexity possible in length MAX_ELEN? */
    if (timeout > 40) { }
#endif

    /* Detect exhaustion */
    if ((lmax > g_exhaust_cpx)
       && (g_one_sided || (rmax > g_exhaust_cpx)))
    {
      /* If no actual results have been given, report some reasons why this
         might happen. Otherwise, we'll assume they know what they're doing.
         For example, "ries 17 '-S+*-/' --numeric-anagram 4444 -Ox" prints
         a single result and then triggers this code before the default
         searchlevel is complete. */
      if (g_num_matches == 0) {
        printf(
          "exhaustion timeout (complexity LHS {%d}, RHS {%d}). Possible causes:\n"
          "  MAX_ELEN %d too small\n",
          lmax, rmax, MAX_ELEN);

        if (NOS_options) {
          printf("  too many symbols excluded via -N, -O or -S\n");
        }
        if (g_anagram) {
          printf("  --numeric-anagram too restrictive\n");
        }
        if (k_derivative_margin == 0) {
          printf("  target too big or small for k_vanished_dx (use --derivative-margin)\n");
        }
        g_reported_exhaustion = B_TRUE;
      }
      gen_total = lim_gentotal;
    }

    if (debug_y || debug_0) {
      printf("gen_total %g (lhs %ld rhs %ld) searchmax %g gnm %ld\n",
        gen_total, lhs_insert, rhs_insert, lim_gentotal, (long) g_num_matches);
      bt_stats();
    }

    /* If we have spent more than a couple seconds and still not reported
       a match, it's probably because they made really stringent demands,
       such as a very small k_max.match_dist value. Let them know that this
       might take a while... */
    if (g_num_matches == 0) {
      time_flt tsec = gettime();
      if (debug_y
      || ((tsec > 2.0) && g_allow_slow_message)) {
        printf("  Still searching: %11s expr and %.3f sec so far...     \r",
          pf_intfloat_wid(gen_total, 11), tsec);
        fflush(stdout);
      }
    }

    /* The main loop exits based on how much work we've done, as measured
       by counting how many expressions were generated (or pruned) rather
       than just looking at how high the complexity score has gotten.
         We measure how much work we've done in two different ways: number
       of "dead-ends" that were discarded (pruned), and number of valid
       complete expressions (not necessarily with distinct values). Having
       these two different exit conditions is important so that a "level N"
       search takes about the same amount of time in normal unrestricted
       search (bi-directional, full symbolset) and in uni-directional search
       modes (e.g. for --numeric-anagram). */
    if ((lhs_prune+rhs_prune) >= lim_prune) {
      ml_going = 0;
    }
    if (gen_total >= lim_gentotal) {
      ml_going = 0;
    }
  } /* End of if(ml_going) */

  if (debug_y) {
    printf("Level %.g search complete (gen_total exceeded lim_gentotal).\n",
                                                              g_levadj);
  }

  /* Given appropriate advice about how to get more results */
  if (g_enable_output) {
    if (g_reported_exhaustion) {
      /* If the exhaustion timeout error was reported, increasing the
         searchlevel will not help. */
    } else if (g_num_matches == 0) {
      printf("No solution was found (try using -l%d",
                                                ((int) floor(g_levadj+1.0)));
      if (g_init_match_dist < (0.001 * g_target)) {
        printf(" or a larger --max-match-distance");
      }
      printf(").  \n");
    } else if (g_exact_exit) {
      printf(
           "               No 'exact' solution was found (try using -l%d).\n",
                                                ((int) floor(g_levadj+1.0)));
    } else {
      printf("                  (for more results, use the option '-l%d')\n",
                                                ((int) floor(g_levadj+1.0)));
    }
  }

  print_end(0);

  return 0;
}

/*
    ries.c
    RIES -- Find Algebraic Equations, Given Their Solution
    Copyright (C) 2000-2024 Robert P. Munafo

    See copyright notice at beginning of this file.
 */
