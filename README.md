## Style considerations
- Pretty much all visual style should be handled by `clang-format`.
- **Things should in general be written clearly enough to not need comments. Comments should be used to explain logical behaviors, not what your spaghetti code is supposed to mean.**
    - On that note, no capitalization in comments except for clarity or emphasis.
    - Comments should be casual, but not unnecessary or mean. `// spread this baby out` is fine if it's clear in context what that means.
- If you need a floating point type and don't have special memory considerations, just use a `double`.
- If you need an integer that will never be negative and don't have special memory considerations, use an `unsigned int`.
    - This includes loop iterators, which should go `i`, `j`, `k`, etc. unless there's a good reason not to.
- If a function will modify one of its parameters, make that parameter a pointer to what's being modified, even if not strictly necessary. 
    - Unless there's special memory considerations, things should be passed by value in general.
- Anything that allocates memory should be clearly commented as such, and if that memory needs to be freed by another function later, *explicitly say so*.
- Use explicit casts unless it becomes needlessly verbose.
- Avoid 'magic numbers'. Define things. Give them names. Main exception is situations like where you have an array of the points of a triangle; you don't need a `#DEFINE` to say there are three points in a triangle.