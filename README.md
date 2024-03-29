# suckulent

# Requirements
- libreadline
- libarchive

# TODO
- Database
    - Versioning
    - Notes
    - Filtering
        - length / time to make (0hr 50min)
        - difficulty (easy/medium/hard)
        - country / region (ex. Mexican Food)
        - meal (bfast/brunch/dinner/snack/dessert)
        - special ingredients / store (ex. Asian Market)
    - Branching/copying
    - File serializing/deserializing
- Unit conversion system
- Command line interface
    - libreadline
    - adjustable serving size
    - filtering
    - shell integration
        - piping with linux commands
        - one shot mode

# Style guide
Braces on same line, except for function definitions or multi-line conditionals
```c
int foo()
{
    if (bar()) {
    }
}
```
lower-case snake-case naming for variables and functions
`#define` constants, enum values upper-case snake-case
globals prefixed with g_, avoid as much as possible
structs and typedefs are seperate, typedef types suffixed with _t
```c
typedef struct foo foo_t;
struct foo {
};
```
use static functions, sanity check arguments on non-static functions
