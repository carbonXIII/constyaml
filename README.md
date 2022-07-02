# constyaml
constexpr (strict)yaml parser

A small C++20 library for parsing a subset of YAML at compile time. Not quite ready yet, but should support enough features for normal configuration files. This allows you to parse, validate, and use configuration values at compile time.

It's still more a less a POC, due to these major caveats:

- Very few tests right now, so it's probably buggy

- No standard way to embed files into strings, so using it will probably involve some build system tricks

- Module support is broken by a weird compiler bug in GCC (probably this one: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=101449)

- Parser error handling is awkward, and usually results in an unreadable compilation error, or non-specific error message.

- The final goal is to support all of the strictyaml specification, but it currently doesn't support some things like multi-line strings

The plan is to keep working on it after the strange module ICE is fixed in GCC and I have more time. For now it's just a cool toy.
