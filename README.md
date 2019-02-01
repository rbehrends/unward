# Documentation

The `unward` tool is designed to modify GAP source files for use in HPC-GAP.
We safeguard access to objects belonging to regions in HPC-GAP by inserting
checks in `ADDR_OBJ()`, `PTR_BAG()`, `CONST_ADDR_OBJ()`, and
`CONST_PTR_BAG()`. However, parts of the GAP kernel need to bypass those
checks to implement concurrency primitives or for performance because
we have statically checked that those checks are unnecessary.

Such regions are marked with the preprocessor directive:

    #ifndef WARD_ENABLED

    ...

    #endif

Code inside that calls any of the functions above (`ADDR_OBJ()` etc.)
either directly or indirectly will be rewritten to use unsafe versions
of the same functions. The rewriting process also generates the
necessary unsafe functions.

Use `./configure && make` to build the tool. The `unward` executable
will be put in the `bin` directory.

Run `unward --help` for information on how it is used.

This tool depends on the AdLib library and uses the same license (see
the README.AdLib.MD and LICENSE files for more information).
