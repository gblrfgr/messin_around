## Style considerations
- Pretty much all visual style should be handled by `clang-format`.
- If you need a floating point type and don't have special memory considerations, just use a `double`.
- If you need an integer that will never be negative and don't have special memory considerations, use an `unsigned int`.
    - This includes loop iterators, which should go `i`, `j`, `k`, etc. unless there's a good reason not to.
- If a function will modify one of its parameters, make that parameter a pointer to what's being modified, even if not strictly necessary. 
    - Unless there's special memory considerations, things should be passed by value in general.
- Anything that allocates memory should be clearly commented as such, and if that memory needs to be freed by another function later, *explicitly say so*.
    - Use explicit casts unless it becomes needlessly verbose.