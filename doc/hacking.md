# Hacking


## Building the code

By default, the code is built in release mode. To build a debug version, use

    $ mkdir builddebug
    $ cd builddebug
    $ cmake -DCMAKE_BUILD_TYPE=debug ..
    $ make

For a release version, use -DCMAKE_BUILD_TYPE=release

## Running the tests

    $ make
    $ make test

Note that "make test" alone is dangerous because it does not rebuild
any tests if either the library or the test files themselves need
rebuilding. It's not possible to fix this with cmake because cmake cannot
add build dependencies to built-in targets. To make sure that everything
is up-to-date, run "make" before running "make test"!

## Coverage

To build with the flags for coverage testing enabled and get coverage:

    $ mkdir buildcoverage
    $ cd buildcoverage
    $ cmake -DCMAKE_BUILD_TYPE=coverage
    $ make
    $ make test
    $ make coverage

Unfortunately, it is not possible to get 100% coverage for some files,
mainly due to gcc's generation of two destructors for dynamic and non-
dynamic instances. For abstract base classes and for classes that
prevent stack and static allocation, this causes one of the destructors
to be reported as uncovered.

There are also issues with some functions in header files that are
incorrectly reported as uncovered due to inlining, as well as
the impossibility of covering defensive assert(false) statements,
such as an assert in the default branch of a switch, where the
switch is meant to handle all possible cases explicitly.

If you run a binary and get lots of warnings about a "merge mismatch for summaries",
this is caused by having made changes to the source that add or remove code
that was previously run, so the new coverage output cannot sensibly be merged
into the old coverage output. You can get rid of this problem by running

    $ make clean-coverage

This deletes all the .gcda files, allowing the merge to (sometimes) succeed again.
If this doesn't work either, the only remedy is to do a clean build.

If lcov complains about unrecognized lines involving '=====',
you can patch geninfo and gcovr as explained here:

https://bugs.launchpad.net/gcovr/+bug/1086695/comments/2

## Code style

We use a format tool that fixes a whole lot of issues
regarding code style. The formatting changes made by
the tool are generally sensible (even though they may not be your
personal preference in all cases). If there is a case where the formatting
really messes things up, consider re-arranging the code to avoid the problem.
The convenience of running the entire code base through the pretty-printer
far outweighs any minor glitches with pretty printing, and it means that
we get consistent code style for free, rather than endlessly having to
watch out for formatting issues during code reviews.

As of clang-format-3.7, you can use

    // clang-format off
    void    unformatted_code  ;
    // clang-format on

to suppress formatting for a section of code.

To format specific files:

    ${CMAKE_BINARY_DIR}/tools/formatcode x.cpp x.h

If no arguments are provided, formatcode reads stdin and writes
stdout, so you can easily pipe code into the tool from within an
editor. For example, to reformat the entire file in vi (assuming
${CMAKE_BINARY_DIR}/tools is in your PATH):

    1G!Gformatcode

To re-format all source and header files in the tree:

    $ make formatcode

## Thread and address sanitizer

Set SANITIZER to "thread" or "address" to build with the
corresponding sanitizer enabled.

## Updating symbols file

To easily spot new/removed/changed symbols in the library, the debian
package maintains a .symbols file that lists all exported symbols
present in the library .so. If you add new public symbols to the library,
it's necessary to refresh the symbols file, otherwise the package will
fail to build. The easiest way to do that is using bzr-builddeb:

    $ bzr bd -- -us -uc -j8  # Don't sign source package or changes file, 8 compiles in parallel
    $ # this will exit with an error if symbols file isn't up-to-date
    $ cd ../build-area/location-service-[version]
    $ ./obj-[arch]/tools/symbol_diff

This creates a diff of the symbols in /tmp/symbols.diff.
(The demangled symbols from the debian build are in ./new_symbols.)

Review any changes in /tmp/symbols.diff. If they are OK:

    $ cd -
    $ patch -p0 < /tmp/symbols.diff

## ABI compliance test

To use this, install abi-compliance-checker package from the archives.

You can use abi-compliance-checker to test whether a particular build
is ABI compatible with another build. The tool does some source-level
analysis in addition to checking library symbols, so it catches things
that are potentially dangerous, but won't be picked up by just looking
at the symbol table.

Assume you have built devel in src/devel, and you have a later build
in src/mybranch and want to check that mybranch is still compatible.
To run the compliance test:

    $ cd src
    $ abi-compliance-checker -lib libunity-scopes.so -old devel/build/test/abi-compliance/abi.xml -new mybranch/build/test/abi-compliance/abi.xml

The script will take about two minutes to run. Now point your browser at 

    src/compat_reports/libunity-scopes.so/[version]_to_[version]/compat_report.html

The report provides a nicely layed-out page with all the details.
