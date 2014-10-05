# CHANGELOG

## version 3.0.5

BUGS:
  * fixed initialization of LFS (see v3.0.2 release notes). LFS option as
    existing in v3.0.2-v3.0.4 was just wrong: setting LFS option to false
    would actually enable LFS option...
  * do not install translated configuration files and man pages when NLS option
    is disabled
  * fixed typos in the man

## version 3.0.4

BUGS:

  * fixed compilation error on *BSD systems introduced in dfc version 3.0.3

## version 3.0.3

BUGS:

  * fixed warning message wrongly displayed when calling 'dfc -u h'
  * fixed various typo fixes
  * fixed some memory management issues
  * fixed if tests on potentially NULL values
  * fixed potential security holes (potentially exploitable buffer overflows
    caused by misuse of printf function)
  * fixed dfc hang on no more accessible remote file systems
  * display a warning in any case when file system cannot be stated instead of
    exiting on error

MISCELLANEOUS:

  * updated license header and copyright year

## version 3.0.2

BUGS:

  * fixed dfc failing when getting volume size information from volume >= 4 To
    on 32-bit hosts (added LFS option, which can be triggered on or off but is
    on by default)
  * fixed missing separator on CSV export when -T and -s options where triggered

## version 3.0.1

BUGS:

  * fixed CMake minimal required version
  * fixed hyphens and spelling mistakes in manpages

FEATURES:

  * added this changelog file

## version 3.0.0

BUGS:

  * fixed wrong usage values for FreeBSD
  * fixed wrong inodes count on FreeBSD

FEATURES:

  * DragonFly BSD support
  * NetBSD support
  * OpenBSD support
  * implemented -o option for Mac OSX
  * internationalization support
  * french translation
  * added an optional configuration file
  * improved auto-adjust feature
  * new option that shows used size using -d
  * new option that allows export to other formats (CSV, HTML, TeX)
  * new option that allows to show information only for locally mounted file
    systems using -l
  * new option to filter based on file system name using -p
  * new option that allows to sort the output based on file system name using -q

Code has also been reorganised and the build system has switched from a simple
makefile to using CMake

## version 2.5.0

BUGS:

  * paths that have more than 3 / are not weirdly truncated anymore
  * no more overlap when using the -o option
  * various minor bug fixes and optimizations

FEATURES:

  * Makefile is now compatible with BSD-make
  * Mac OSX support (still misses the -o option)
  * -o option support for FreeBSD
  * new option that prevents the names from being truncated by using -W
  * sign used to draw the graph has changed from a * to an = sign

## version 2.4.0

FEATURES:

  * FreeBSD support
  * reorganised code to make it less monolithic

## version 2.3.0

BUGS:

  * fixed width of display when aflag not triggered

FEATURES:

  * strings that are too long to be displayed correctly are now truncated
    and a + sign indicates it. Example:
    "/dev/mapper/foo-lv_lxc23764-home" ==> "+pper/foo-lv_lxc23764-home"
  * auto-adjust output based on terminal width
  * new option to force display (disable auto-adjust mode)
  * implemented negative matching on file system filtering
  * various small optimizations

## version 2.2.0

BUGS:

  * various small bug fixes

FEATURES:

  * color option is now improved: you can choose between 3 modes (auto, always,
    never); by default, auto is activated
  * multiple selection is now possible when filtering with -t option

## version 2.1.0

BUGS:

  * -s option now sum the total of inodes when -i option is triggered
  * when using human readable, sizes in bytes were displayed like so 3.0B which
    does not make sense at all cause there is no half-byte

FEATURES:

  * color is now automatically in "color-auto" mode
  * unit option has been completely reorganized; they have all been replaced by
    the new -u option
  * support Tio, Pio, Eio, Zio and Yio units (and as such, To, Po, Eo, Zo, Yo)
  * when using -i option, a k is now appended to the number of inodes and this
    number is divided by 1000
  * new option (-b) allows you to not display the graph bar
  * new option (-T) shows the filesystem type (filesystem type is not displayed
    by default anymore)
  * new option (-t) allows you to perform filtering on filesystem type

## version 2.0.2

BUGS:

  * fixed a bug messing up display of information when have gvfs-fuse-daemon

## version 2.0.1

BUGS:

  * fixed display error when using humanize
  * fixed a bug causing some special devices not to be skipped
  * updated manpage

## version 2.0.0

FEATURES:

  * color support (enabled by default but can be disabled using -c option)
  * new option (-o) shows information about mount options

## version 1.2.0

FEATURES:

  * filesystem name is now being less truncated
  * default behavior is now to print size in human-readable format
  * new option (-b) to display size in bytes (used to be the default display)
  * new option (-i) that prints information about the number of inodes and the
    amount of available ones

## version 1.1.4

BUGS:

  * fixed a graphical bug that occured when displaying fsname and type

## version 1.1.3

BUGS:

  * fixed a bug that was causing header to be messed up when using -K, -M or -G
    option when producing the grand total

## version 1.1.2

BUGS:

  * fixed percentage calculation for the total

## version 1.1.1

BUGS:

  * fixed a bug in the function that truncates str
  * fixed bogus display when using -n option

## version 1.1.0

BUGS:

  * large filesystem names should now be truncated to avoid messing up what is
    displayed
  * Makefile now supports the DESTDIR option

FEATURES:

  * proc, sys and devpts are not displayed anymore
  * new option to hide the filesystem type by using -t
  * first decimal is now displayed when using Mio and Mo
  * new option to display the size in Ko, Mo and Go instead of Kio, Mio and Gio
  * new option to display a wider bar by using -w
  * new option that display the total usage by using -s
  * new option that allows to skip displaying the header

## version 1.0.1

BUGS:

  * fixed bugs in Makefile

## version 1.0.0

  * initial release

<!-- vim: set filetype=markdown textwidth=80 -->
