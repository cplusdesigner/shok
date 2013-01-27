lush: language walkthrough
==========================

lush is a an object-oriented language with a strongly-typed prototype-based
inheritance model.

Everything is an object.  The name of an object is just the name of a variable
that holds the object.  An object contains "members": functions and data.

Objects are made by copying or combining existing objects, by extending an
object with new members, or by restricting the domain of some of its members.

The "prototype" (or just "type") of an object is its name, the names and types
of its members, and the names and types of its parents.  The type describes the
set of all possible values for an object.

Comments
--------
        # this is a single-line comment

        (* this is a
            (* multi-line comment *)
           that allows (*nesting*)
        *)

Help operator: `?`
------------------
        ? foo     # describes an object 'foo'
        ? :grep   # man/info/--help integration
        ? ->      # describes language symbols and operators

Variables
---------
        new color = "orange"                  # Declare variable
        new color = "green"                   # Error! 'color' already defined.
        renew color = 3                       # Re-declare (change type)
        del color                             # Delete variable
        isvar color                           # Check if variable exists

`new` makes a new variable (object) by making a copy of an existing object.
The new object's "parent" is the name of the object that was copied; the new
object is a "child".  This parent-assignment is called the object's "type" and
it cannot change for the lifetime of the object.  Its first value is a copy of
its parent, but this value may change over time: either by mutation (changing
any of the object's members' values) or by assignment (copying in another
object entirely -- as long as it matches our type).

        new x = animal    # x is a child of 'animal', and is currently a copy of 'animal'
        x = giraffe       # x is now a copy of 'giraffe', but still has type 'animal'
                          # x may have members found only in 'giraffe' but not 'animal'.
        x = plum          # error: plum is not an animal

`del` deletes a variable.  This can only be done at the original scope of the
variable.  This is primarily meant for interactive use, to clean up unnecessary
objects/names and save memory.

`renew` is similar to deleting a variable and then re-'new'ing it as perhaps a
different type.  There are two places you can do this:

1. At the original scope of the variable, e.g. to re-use a variable name or fix a mistake.
1. When making a child object, to restrict the variable's type to a subset of the parent's.

`isvar` tells you if the name of a variable is defined.

We have seen that `new` sets a variable's type: its initial value must be the
parent of all future values.  When constructing new objects (described later),
it's nice to be able to set the type of a variable to one object, but then also
set its first value to something more specific, as a default.  We can do this
with `new`:

        new x = animal = giraffe      # x's type is 'animal', but it has a giraffe!

This is just a shorthand for

        new x = animal
        x = giraffe

Multiple variables can be defined with the same `new' using a comma:

        new x=animal=giraffe, y=fruit, z=fruit=plum

If `new` is used without assigning a type/value, the default `object` is
assumed:

        new q
        typeof q    # object

Expressions
-----------
Pretty standard.

        new wantPapaya = (isTasty and isJuicy) or (amIStarving() and not isRotten)
        new x = (14 + 12) * 3 / (9 `mod` 4)

`` ` ``backticks`` ` `` let you make your own infix operators.  You get to
specify its evaluation precedence... somehow.

int defines `` `mod` `` because `%` is already taken, as we shall see.

Operators and precedence
------------------------

Special keywords
----------------
These special keywords check meta-information about variables.

        keyword     type    description
        -------     ----    -----------
        typeof      [str]   A list of the immediate parents of a variable
        attribs     [str]   A list of type attributes (e.g. abstract, const)
        whatis      str     The best known name of the variable's current value
        is          bool    Check if a variable has an object in its parent tree
        isvar       bool    Check if a variable is defined in the current scope

They cannot be used in expressions with boolean logic (and/or/not), so that the
interpreter can follow through expressions like:

        each file|dir f in ~/music/ {     # each file or directory in my music folder
            if f is dir {
                f.rmdir()   # Only 'dir' has this method.  The interpreter knows that
                            # this block can only execute if f is really a dir.
            }
            f.rmdir()       # Static error! The interpreter won't even execute a script
                            # with this broken code in it.
        }

Standard types
--------------
lush includes these standard types:

        object      Parent of all objects; automatic default.
        @           Function.  A child of 'object' just like everything else.
        null        Stubborn object that refuses to do anything.
        bool        = true | false
        str         Text string
        num         Abstract parent of numeric types: int, fixed, float
        int         Any integer (unbounded).  Use int32 or int64 if you need to.
        fixed       Fixed-precision decimal-point number.  Literals like 014.3600
        float       IEEE 754-2008 binary64 floating-point number
        <"hi", 12>  Tuple: Collection with fixed length and types
        [4, 1, 17]  List: Ordered collection of any length and one specific type
        table       Data table with typed columns
        path        Filesystem path, with children: file, dir, link, pipe, bdev, cdev
        %           Process,  e.g. %123 refers to process #123
        cmd         A command invocation; its I/O streams, filehandles, exit code...
                    Returned by command-line invocations:  new x = {: wget www.com}

These might be included but require deep thought:

        date        A calendar date
        time        A time of day, independent of date
        datetime    A specific date and time

Good programs will use lush's object-composition system to hand-craft subsets
and combinations of these to describe the exact domain required for any
particular task.

String concatenation
--------------------
`~` is string concatenation

`~~` is sugar for `~' '~`  i.e. "string concat with a space in-between"

        new h = "hello"
        new w = "world"
        print(h ~ w)      # helloworld
        print(h ~~ w)     # hello world

These are the only operators that do an implicit cast, and it's `->str`.

Every object has a `->str` cast.

        print(4 ~~ "is four, and" ~~ someFunction ~~ "is a function")

`format()`: another way to mix variables into strings

        print("one {1} two {2} three {3}".format("cat", 42, someFunction))

Regular expressions
-------------------

Numeric types
-------------
No implicit casts

        fixed x = 1.0 + 5         # error
        fixed x = 1.0 + 5->fixed  # ok

int
---
Integers of arbitrary size.

fixed
-----

float
-----
IEEE 754-2008 binary64 floating-point numbers.  Specific details to come.

Tuples
------
Tuples are finite-length lists where each element has a specific type.  They
are enclosed in `<...>`'s.

Lists
-----
Lists are arbitrary-length lists where the elements all have the same parent,
which is fixed in advance.  They are enclosed in `[...]`'s.

Slicing/splitting/splicing...

Tables
------
Consider: a dictionary is a table with 2 columns, the first of which is
indexed.  Our `table` standard object helps you make tables with typed columns,
and you can declare which ones should be indexed so that you can use them as
keys with which to select rows.

A multi-set is a table with one column that is set 'sorted'.

A set is a table with a single column that is set 'sorted' and 'unique'.

A map is a table with two columns, the first of which is sorted and unique.

A multi-map is a table with two columns, the first of which is sorted.

Paths
-----
The filesystem path is a native object in lush.  It has child types for
different kinds of filesystem nodes: file, dir, link, pipe, bdev, cdev

paths can be constructed from strings, in which case they'll interpret the
'`/`' character to mean a platform-independent path separator.

        new bindir = path.new('/usr/bin/')

In the statement above, the path has not been evaluated, so it needn't exist on
the target system for the variable "bindir" to get assigned.

The previous statement is equivalent to:

        new bindir = /usr/bin/

In this case, we used a "path literal" -- a string directly in the sourcecode
that gets parsed to be a path.  A path literal must either:

        path literal        example           description
        ------------        -------           -----------
        start with a /      /vmlinuz          absolute path
        end in a /          Desktop/          directory relative to working dir
        start with a ./     ./recipe.txt      relative to working dir
        start with a ../    ../recipe.txt     relative to parent dir
        start with a ~/     ~/music/omg.mp3   relative to home dir

A `path` object gives you easy access to all its file attributes and helps you
build command invocations or manipulate the filesystem.

If the path literal ends with a `/` then it is a "dir literal" and gives you
back a dir (directory object).  dir implements "collection" so you can loop
over its elements (nodes in a directory).

Note that `/` has two meanings.  One is an overloadable operator, typically
"division".  The other is as part of a path literal.  The two uses are
unambiguous, but try not to get confused:

        new bin = "I has a bucket"
        new bash = "a monster bash!"
        print(bin/bash)       The '/' operator between str's
        print(/bin/bash)      A path
        print(bin/bash/)      Another path

`str` does not define the '/' operator, but still.

Using variables to extend path literals...

Hidden files (vs. files starting with '.'), and wildcard path literals...

Processes
---------
`%123` is a literal that means "process #123", if it exists.

        print(%123.runtime)     # how long has this process been running?
        %123.signal($SIGKILL)   # kill it!

We just used the  `$` scope operator, which will be described later.  It let us
conveniently refer to `SIGKILL` which is a symbol defined in the
`@process.signal()` function itself.  What we wrote was equivalent to:

        %123.signal(%123.signal.SIGKILL)

Control flow
------------
`if`, `elif`, `else`

        if x > 500 {
            print("big")
        } elif x > 50 {
            print("medium")
        } else {
            print("small")
        }

Compact form of `if`.  The "`,`" is only required for `if` and `elif`.

        if x > 500, print("big")
        elif x > 50, print("medium")
        else print("small")

You could do that on one-line using explicit "`;`"'s to end lines:

        if x > 500, print("big"); elif x > 50, print("medium"); else print("small")

`switch` statement.  The object must have type 'comparable' which gives it `==`

        switch snack {
            case "celery" {
                print("veggie")
            }
            case "cookie" | "pie" {
                print("dessert")
            }
            # "case is" lets you select based on type rather than operator==
            # This is useful for specific types of fall-through
            case is food {
                print("Well, at least it's some kind of food")
            }
            case else {
                print("Don't put that in your mouth!")
            }
        }

Alternatively you can provide your own comparison function

        switch food, food.foodGroupEquals {
            # ...
        }

Easy loops: `loop`, `repeat`, `while`

        new x = int
        loop {
            x += 1
            if x > 7, break
        }

        repeat x times {
            print("Woo!")
        }

        while x < 7 {
            x += 1
        }

`each` loops iterate over a list or tuple

        each x in [5, 10, 15] {
            print(x)
        }

Type filters: you only get objects that match the type:

        each int x in [5, "hello", -12] {
            print("Got int:" ~~ x)          # will only get 5 and -12
        }

The `where` clause lets you be very picky:

        each int x in [5, "hello", -12] where x > 0 {
            print("I could have just used a better type filter")
            print(whatis x)   # int
        }

The `where` clause above could have been done with a type filter instead.
Another use of `$`, the scope operator, here referring to the object being
tested after the `?`.

        each int?($>0) in [5, "hello", -12] {
            print("This puts the positivity constraint right in the type!")
            print(whatis x)   # int?($ > 0)
        }

If you don't specify a loop variable, the automagic variable '_' is used
instead:

        each in [5, 10, 15] {
            print(typeof _)     # int
            print("My number is: ~~ _)
        }

The `each` loop's type filters are very useful with paths:

        each file|dir f in ~/music/ {
            if f is file {
                : mp3blaster {f}
            } else {
                if f.count() == 0, f.rmdir(); else {:mp3blaster {f}}
            }
        }

Every object has a magic member `i` that you're not allowed to change.  It gets
set by the `each` loop, and tells you how far along you are:

        each int n in [5, 10, 15] {
            print(n.i ~ ":" ~~ n)
        }
        # Prints:
        1: 5
        2: 10
        3: 15

        each food f in ["pie", "cookie", "cake"] {
            each size s in ["small", "large"] {
                print(s.i ~'.'~ f.i ~~ ": a" ~~ s ~~ f)
            }
        }

        # Prints:
        1.1: a small pie
        1.2: a large pie
        2.1: a small cookie
        # and so on.


Other control-flow statements include `continue`, `break`, and `return`.

Blocks can be given labels, which the `continue` and `break` can refer to.

        *first* each x in [1 2 3] {
            *second* each y in [4, 6, 8] {
                if x*y >= 12, continue *first*
                print("y:"~~y)
            }
            print("x:"~~x)
        }

Modules
-------
A module is just a script that defines some objects.

Importing a module (or specific members of a module) runs the code in a
sandbox, then brings the symbols into your local namespace, while giving
everything the attribute "const".  Import fails if there are any name
collisions for objects that aren't identical.

        import my_date_lib.date     # import a single object from a module
        import my_date_lib          # ok, just grabs the rest

Creating new objects
--------------------
Every `new` statement creates a new object by copying some existing object, and
maybe adding things to it.

        new x = object
        new y             # same as above; 'object' is the automatic default
        new z = {}        # same as above; 'object' is included by default

        new animal = {
            new name = str
            new age = int
            print("Rawr")
        }
        typeof animal       # object

        new lion = animal   # makes a copy of 'animal', called 'lion'.
                            # "Rawr" gets printed, oh my.
        typeof lion         # animal

When we write `new lion = animal`, this is what happens:

1. A variable called `lion` is made.
1. Its parent is set to 'animal'.
1. The body of the 'animal' block is copied into 'lion', and then run.  

   In this case, first the members inherited from 'object' will be initialized
for 'lion'.  Then lion's 'name' and 'age' members are new'd, and then "Rawr"
gets printed.

`object` comes with some members; for example:

        member          type
        object.i        int                 # Loop increment
        object.env      table(<str,str>)    # Environment variables
        object->str     @()->str            # to-string (cast)

Functions
---------
`@` means "function".  Every function is the `op()` (parenthesis operator) of
an object, though these objects are sometimes made behind-the-scenes.

Consider an enhanced 'animal':

        new animal = {
            new name = str = "(none)"
            new age = int
            new op() = @(str s) {
                name = s
                stdout.print("Rawr! My name is" ~~ name)
            }
        }

'animal' has an `op()`, meaning it can be called like a function.  Its `op()`
takes a str called 's', and has no return value.

Let's try using it:

        new lion = animal
        print(lion.name)      # (none)
        lion()                # error: lion() is not defined
        lion("Simba")         # Rawr! My name is Simba
        print(lion.name)      # Simba

A function can be made on-its-own, seemingly without an object.  Some examples:

        # No args, no return type
        new func1 = @() {
            print("hello")
        }
        func1()         # prints "hello"
        typeof func1    # @()

        # No args, returns an int
        new func2 = @()->int {
            return 14 * 3
        }
        typeof func2    # @()->int

        # takes a str and an int, and returns a 'fixed' (a decimal-point number)
        new func3 = @(str s, int n)->fixed {
            print("s:" ~~ s ~~ "and n:" ~~ n)
            return n->fixed + 0.123
        }
        typeof func3              # @(str,int)->fixed
        new x = func3("hi", 5)    # s: hi and n: 5
        print(x)                  # 5.123

`func3` above is nearly equivalent to:

        new func4 = {
            new op() = @(str s, int n)->fixed {
                print("s:" ~~ s ~~ "and n:" ~~ n)
                return n->fixed + 0.123
            }
        }

        typeof func4    # @(str,int)->fixed

The only difference between a function defined "as a function" (func3) vs. "as
an object" (func4) is the meaning of 'self'.  When you make an object, it gets
a special member variable 'self' that refers to the object itself.  Functions
(made "as a function") do not get this member, so that within a function
defined inside an object, 'self' will still refer to the *object* being defined
(in nearest scope).

If a function returns 'self', then each call of this function will create a
copy of the 'self' object (whatever it is) as if `new` had been used, and then
the function is executed in the context of this new object, which is then
automatically returned when the function is done (or by an explicit call to
`return` within the function).

Recall that every object is its own 'default value' that you get (a copy of)
when you assign it to a new variable.  What about an object that doesn't have a
sensible default -- where some data is required in order to make one?  In this
case, you use a "factory" function:

        # A function that, given a strength, returns to you a lion-like object.
        new lion = @(int strength)->object {
            return {
                new roar = @() {
                    print("ROAR" ~ "!".repeat(strength))
                }
            } with(strength)      # see "closures" section for why we need this
        }

        new simba = lion(2)
        typeof simba        # lion(2)
        simba.roar()        # ROAR!!

        new scar = lion     # what is this?
        typeof scar         # @lion(int)->object
        scar.roar()         # error!  scar doesn't have "roar".
        scar = simba        # error!  scar is a lion-maker function, it can't be a lion
        new mufasa = scar(3)  # ok!
        mufasa.roar()         # ROAR!!!

When you assign a function to a variable, you are just making the first of a
possible set of functions that can take different sets of arguments, and can
even return different types (though this actually means that your variable has
a return type of the "OR" union of all the return types you've mentioned).  You
can append more functions into the set:

        lion = lion & @(str name, int strength)->object {
            return {
                new roar = @() {
                    print(name ~~ "says, ROAR" ~ "!".repeat(strength))
                }
            } with(name, strength)
        }

        lion &= @(str name, int strength, str msg)->object {
            print("I'm making a lion." ~~ msg);
            return lion(name, strength)
        }

        typeof lion     # @[(int strength),
                        #   (str name, int strength),
                        #   (str name, int strength, str msg)]->object

        renew mufasa = lion("Mufasa", 6, "Aw yea")    # I'm making a lion. Aw yea
        mufasa.roar()   # Mufasa says, ROAR!!!!!!

Like with `new`, if the type of a function argument is not specified, it is
assumed to be '`object`':

        new f1 = @(int n, thing) {
            typeof thing      # object
        }

Inside a function, the special variable 'args' is a tuple of the arguments that
were passed in.  An ellipsis (`...`) can be used to make a function that
accepts any number of arguments, and these trailing arguments may be given a
specific type (default: `object`).

        new f2 = @(int, ...) {      # Takes an int and any number of objects
            typeof args             # <int, [object]>
        }

        new f3 = @(int, str...) {   # Takes an int and any number of str's
            typeof args             # <int, [str]>
        }

You can use the `$` scope operator to set values for named parameters.
Consider the '$' to mean "his/her/its" -- it refers to variables that live
within the function being called.

        # Make a lion with "Nala", her message is "Ohai", and her age is 4
        lion("Nala", $message="Ohai", $age=4)

Any positional (unnamed) arguments must precede the ones that are named.  

Specify optional arguments with a prefix `?`:

        new foo = @(str s, num n, ?str msg, ?str another) {
            if isvar msg {
                print("Thanks for this nice message:" ~~ msg)
            }
        }

Functions can call themselves recursively, and expect the "tail-call
optimization" aka "tail recursion".

When you call a function, lush uses "pattern-matching" of the arguments you've
provided to find the best version of the function to call.

        new fib = @(0)->0 { return 0 }
                & @(1)->1 { return 1 }
                & @(int n)->int {
                    return fib(n-1) + fib(n-2)
                }
        typeof fib        # @[(0)->0, (1)->1, int]->int

TODO: can we append code to a function?  Can we replace a function?  Under what
conditions can we remove a specific function signature from a function?

Object conversions and casts
----------------------------
Casts are convenient conversions of an object from one type to another.  In
`lush` these are just normal functions, but we give them an 'arrow' syntax to
help them stand out.

`object` has a ->str ("to string") function that takes no arguments, and is
pre-defined to tell you the variable name of the object.  An object
automatically gets a default cast function to any of its parents, but you can
replace these with your own at anytime.  You can arbitrarily make your own
->whatever functions, too, but a '->foo' function must always return 'foo'.

        new date = {
            int year; int month; int day

            new print = @() {
                stdout.print(self->str)    # just use our ->str cast
            }

            typeof self->str    # @self->str()->str

            # Since we inherit self->str from 'object', we don't need to
            # declare with 'new'.  We just replace the parent version with '='.
            self->str = @()->str {
                # yyyy-MM-dd
                return year->str.pad('0',4) ~'-'~       # beware of bats!  ~'-'~
                       month->str.pad('0',2) ~'-'~
                       day->str.pad('0',2)
            }
            # Append another ->str cast function.  This one takes an argument.
            # format is passed in by-value (copy-on-write) so it's ok to modify.
            self->str &= @(str format)->str {
                format.replace('%YEAR%', year)
                format.replace('%MONTH%', month)
                format.replace('%DAY%', day)
                return format
            }

            # Automatically returns a new 'date' after this code is run
            op() = @(int year, int month, int day)->self {
                self.year = year; self.month = month; self.day = day
            }
        }

Object inheritance and composition
----------------------------------
Assigning a block (`{...}`) implicitly means it inherits from `'object'`

        new date = {          # equivalent:  new date = object & { ... }
            new year = int
            new month = int
            new day = int
        }

`&` is the "append code" operator.  We extend `date` by making a child that
appends some more code:

        new datetime = date & {
            new hour = int
            new minute = int
            new second = int
        }

We can just merge objects together:

        new cool = {
            new wow = @() { print("Woooo!") }
        }
        new cooldate = date & cool

If any members collide (have the same name but different types or values), then
the child can't access them directly.  It must cast itself to a specific parent
in order to access that parent's version of it.  An inherited method would only
access the version of the variable appropriate for the method.

We can use `|` to say an object could be one thing or another.

        new nullableInt = int|null

        new something = cool|date|datetime
        print(typeof something)     # cool|date|datetime
        print(whatis something)     # cool|date|datetime
        something = datetime.new(2012, 01, 20)
        print(typeof something)     # cool|date|datetime
        print(whatis something)     # datetime

`typeof` tells you the type, which is always the full range of possible values.

`whatis` tells you the most specific type that the object is known to be.

We use `is` to discover what functionality we're allowed to access

        new foo = @(cool|date|datetime thing) {
            if thing is cool {
                thing.wow()
            } else {
                print(whatis thing)     # date|datetime
                print(thing.month)      # date and datetime can both do this
                print(thing.minute)     # error!  don't know it's a datetime
            }
        }

        # x is either a positive int, or a null
        new x = positive & int | null

`?` lets you quickly declare an object with some restrictions on its members

        new nonnegative = int?($ >= 0)
        new dateIn2013 = date?($year==2013)
        new dateInQ4of2013 = date?($year==2013 and $month>=10)

These `?` checks will be performed as run-time checks every time the object
is modified.  So they may have a noticeable affect on performance.
Nevertheless they can be very useful, especially for quick scripts and
prototyping.

Note that in some cases the 'default value' of an object will not match the
criteria, and thus the new object cannot be defined.  Instead you'll have to
add extra code to set up your object properly:

        new positive = int?($ > 0)                    # error!  int defaults to 0
        new positive = int?($ > 0) & { value = 1 }    # ok!

Scope and self
--------------
`self` is an object member that always automatically refers to the object
itself.

If you're defining a method (member function of an object), then `self` is
still in scope from the owning object.

Alternatively, `$` is the "scope operator".  On its own it's a reference to the
current scope in which you're operating:

        new x = "outside"
        {
            new w = 42
            print(w)        # as a local variable
            print($w)       # access anything that's in scope
            print($.w)      # as a member of this immediate scope, via $
            print($.x)      # nope, x was not defined by this specific scope
            print($x)       # yes, in this style you get anything visible to this scope
            print($../x)    # yes, finds x via the "scope hierarchy"
        }

`$` is a way to talk about the object "under discussion".  You can understand
it to be similar to "he/she/it" or "his/hers/its".

In a type restriction (`?`), within the `()`'s, the `$` refers to the object in
question:

        new x = int?($ > 12)        # x is an int where itself is greater than 12

        # y is a str where its length is greater than 4 and less than 12
        new y = str?($length > 4 and $length < 12)

Within the `()`'s of a function call, `$` refers to the scope of the function
being called.  This lets you refer to the function's parameters by name, or
access public-facing members of the object/function you're calling into.

`$` lets you use a "filesystem-like" syntax to refer to variables that were
defined in a parent scope:

        new x = "toast"
        {
            new x = "soup"
            new y = "salad"
            if y.length > 2 {
                new x = "burrito"
                print($x)           # burrito
                print($.x)          # burrito
                print($./x)         # burrito
                print($../x)        # soup
                print($../y)        # salad
                print($../../x)     # toast
                print($/x)          # toast  (via "absolute variable path")
                print($.y)          # error! no 'y' is *immediately* here
                print($y)           # salad.  y is, however, visible.
            }
        }

The "variable path hierarchy" root is at the nearest object definition, or the
base command-line scope that started this code block.  Its use is mainly for
user-interactive discovery.  Within a function, the $/ and $../ forms can only
be used with member variables that are explicitly closed over (see the Closure
section).

Abstract objects
----------------
The attribute 'abstract' declares an object that has a function, perhaps with a
specific type, but no code body.

        abstract animal = {
            new name = str
            abstract speak = @()->str
        }

If you just use 'new' instead, it's an error.  The interpreter will be nice and
give you an abstract and a warning, but scripts will fail.

Likewise, if you use 'abstract' to declare an object that has all its functions
filled out, it's an error (but the interpreter will be nice).

        new duck = animal         # WARNING: duck is abstract
        duck.speak()              # static error!
        renew duck = animal & {
            speak = @()->str {
                return "quack"
            }
        }
        print(duck.speak())       # quack

Suppose we're trying to describe animals.  All animals should be able to mate,
but only with the same species.  The sad way:

        abstract animal = {
            new name = str
            abstract mate = @(self)->bool   # 'self' means 'animal' here
        }

        new duck = animal & {
            mate = @(self other)->bool {
                return random(0,1) > 0.5
            }
        }
        # WARNING: duck is abstract

What happened?  When 'animal' used `self`, it meant 'animal'.  So anyone who
wants to implement 'animal' must provide a body for @mate(animal)->bool .  Then
when 'duck' used the word 'self', it meant 'duck'.  So it only implemented part
of the 'mate' contract, so it was still abstract.

The '$' operator becomes very useful here.  Used in an abstract, it takes on
the 'self' of any concrete implementor.

        abstract animal = {
            abstract mate = @($)->bool      # '$' means 'itself' for an implementor
        }

        new duck = animal & {
            mate = @($ spouse)->bool {      # '$' and 'self' mean the same thing here
                return random(0,1) > 0.5
            }
        }
        duck.mate(animal)     # Error: no function duck.mate(animal)
        duck.mate(horse)      # Error: no function duck.mate(horse)
        duck.mate(duck)       # You might have a duckling!
        new larry = duck
        new suzie = duck
        suzie.mate(larry)     # You might have a duckling!

This is useful for some basic primitives.  Consider:

        abstract orderable = {
            abstract op< = @($)->bool
            new by = @(object o)->object {
                return {
                    abstract op< = @(o)->bool
                }
            }
        }

        new date = orderable & {
            op< = @(self rhs)->bool {
                if year < rhs.year or
                        (year==rhs.year and month<rhs.month) or
                        (year==rhs.year and month==rhs.month and day<rhs.day) {
                    return true
                }
                return false
            }
        }

        renew date = orderable.by(int) & {
            op< = @(int rhs)->bool {
                # Some weird way to compare  (date < int)?
                return true
            }
        }

A sorting algorithm might take a list of orderables, for example, so it knows
statically that it can use operator< to compare objects of the same type.

Closure
-------
lush allows explicit closure over variables from a parent scope, by value but
not by reference.  This is really just a way to parameterize a function with
data present at the time of definition, as opposed to arguments that are passed
in when the function is called.

Here is our version of wikipedia's Closure (Programming) first Python3 example:

        new counter = @()->@() {
            new x = 0
            new increment = @(int) with(x) {     # explicitly closes over x
                x += y
                print(x)
            }
            return increment
        }
        counter1_increment = counter()
        counter2_increment = counter()
        counter1_increment(1)   # prints 1
        counter1_increment(7)   # prints 8
        counter2_increment(1)   # prints 1    # counter2 has its own copy of x
        counter1_increment(1)   # prints 9

Since interactively it's easy to forget (or not know) upfront what parent-scope
variables we might need, the `with()` clause can go at the end of the function:

        new increment = @(int) {
            x += y; print(x)
        } with(x)

In this form, the `with()` must be on the same line as the `}`.

If a function is a method (member of an object), it should not declare
`with(self)`.  The method can only be run in the context of an owning object,
so that `self` and its members are implied.  (I'm not sure if this is true /
actually works....)

Object attributes and permissions
---------------------------------
abstract, const

Exceptions
----------

Generators
----------
Probably python-esque (`yield` vs. `return`)

Locale
------

OS-specific functionality
--------------------------

Full grammar
------------

