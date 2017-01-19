# README

`dfc` is a tool to report file system space usage information. When the output
is a terminal, it uses color and graphs by default. It has a lot of features
such as HTML, JSON and CSV export, multiple filtering options, the ability to
show mount options and so on.

## BUILD

`cmake` is required to build `dfc`. For convenience, a simple `Makefile` (which
calls `cmake` under the hood) is provided.

I suggest you create a `build` directory in which you compile `dfc`. This is of
course not required but it will prevent files from being created everywhere in
the source folder.

### BUILD STEPS

Create the build directory:

	mkdir build

Navigate into it:

	cd build

Run the `cmake` command to generate a `Makefile` that works for your OS:

	cmake ..

Now you can use the make command to build `dfc`:

	make

The `dfc` binary will be placed into a `bin` sub-directory.

### BUILD OPTIONS

Several options can be tweaked before you compile `dfc`. To activate /
deactivate or change them, you need to do it at the `cmake` step. Note that you
may also use `ccmake` instead which gives you a `curses` interface to tweak the
options.

By default, translations are enabled. They require `gettext` to be installed on
the system. You can however easily disable them in which case no translations
nor language translated configuration files will be installed.

	cmake .. -DNLS_ENABLED=false

`dfc` has also the `LFS` option enabled by default. This option activates
compile flags in order to support listing of large file systems (over 4G) on
32-bit hosts. This will not harm 64-bit systems if activated but if you feel the
need to deactivate it, use the following:

    cmake .. -DLFS_ENABLED=false

Different types of build are available. Most people will only care about
`RELEASE` which is the build type that shall be used when distributing the
software as binary or installing it as it adds some optimization flags.
To enable `RELEASE` build use the following:

    cmake .. -DCMAKE_BUILD_TYPE=RELEASE

Developers might care about `DEBUG` build when debugging the program as it adds
debug flags such as `-g3`. Enable it like so:

    cmake .. -DCMAKE_BUILD_TYPE=DEBUG

Note that by default, `dfc` build with very strict compilers flags.

## RUN

Once built, you can run `dfc` by typing:

	./dfc

from within the directory where `dfc` is located.

See `./dfc -h` for quick options and usage overview or read the manual page.

## INSTALL

By default, `dfc` binary will be installed in `/usr/local/bin`. As `root`, type:

	make install

You can also choose some standards parameters like where the files need to be
installed. This needs to be done when using the `cmake` command.
Example (from the previously created build directory):

	cmake .. -DPREFIX=/usr -DSYSSCONFDIR=/etc -DCMAKE_BUILD_TYPE=RELEASE

Then run the `make install` and it will install `dfc` according to what you
chose in the previous step.

## CONFIGURATION FILE

The configuration file found in `conf/dfcrc` needs to be placed here if one
desires to use it:

	$XDG_CONFIG_HOME/dfc/dfcrc

Note that if, for instance, French is the language you use, you should then use
the configuration file that has been translated into French
(found in `conf/fr/dfcrc`) and so on for any language into which `dfc` has been
translated.

If your operating system does not support `XDG Base Directory Specification` it
can then be placed in this directory:

	$HOME/.config/dfc/dfcrc

Or, last choice, directly in `$HOME` (but the name has to be preceded by a dot):

	$HOME/.dfcrc

## INFORMATION FOR PACKAGERS

Here is the list of dependencies:

  * standard C library

Yep, that should be it. :)

Please, note that `gettext` is required in order to build translations.
If you do not want to package `dfc` with translation support, use the option to
disable translation as explained in the build section.

Of course, `cmake` is a build dependency.

<!-- vim: set filetype=markdown textwidth=80 -->
