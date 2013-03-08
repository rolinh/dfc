# INFORMATION FOR TRANSLATORS

First, thanks for taking the time to translate dfc :-)

You will find instructions here about how you should proceed to perform the
translation.

If you want to translate dfc, there are three things to translate:

  * The program itself
  * The manpage
  * The configuration file

Once you translated all of this, send those three file to the author:

  * The po file
  * The translated manpage
  * The tanslated configuration file

Read the instructions below carefully in order to be able to produce a nice
translation.

## TRANSLATE THE PROGRAM

**IMPORTANT NOTES:**

  * Symbols like %s, \n, etc must remain untouched.
  * Strings from the file text.c that are completely capitalized, like
   "FILESYSTEM " for instance,  must be translated in a string of the
   same length. If not, the output of dfc will be messed up. So, just find a
   way and truncate the name if required. For instance, in French, I would
   translate it this way:
		"SYS. FICH. "
   Completely capitalized string from other files than text.c can be translated
   as usual.

### CREATE A NEW TRANSLATION

Here are the steps you need to follow in order to translate dfc into a new
language:

  * Build dfc as explained in the README file. This will generate an updated
    dfc.pot file in the po directory which you will use to generate a po file
    for your own language.

  * Navigate to the po directory.

  * Generate a new po file for the language you want to translate into. In the
    example below, I assume it will be translated into French:

		msginit -l fr -i dfc.pot -o fr.po

  **Note:** -l option is required only if you want to translate into another locale
  than the one used on your computer.

  * Now you can start translating all the strings in the *.po file with any text
    editor or *.po file editor (poedit for instance).
    You should then check your translation (see next step).

### CHECKING YOUR TRANSLATION

Simply use this command to check your translation for errors and completeness
(here, I assume you want to check the french translation):

	msgfmt -c --statistics fr.po -o /dev/null

If you get this error message when running the command:

	"invalid multibyte sequence"

make sure charset is set to "utf-8" (and not ASCII or something else) in the po
file:

	"Content-Type: text/plain; charset=utf-8\n"

### UPDATING A TRANSLATION

  * Build dfc as explained in the README file in order to generate the dfc.pot
    file.

  * Issue this command to merge the existing po file (here, I assume it is French):

		msgmerge --update fr.po dfc.pot

  * Open the *.po file and update translation. Once done, as usual, ckeck your
    translation.

## TRANSLATE THE MANPAGE

Simply translate the file 'dfc.1' located in 'man' folder using your favorite
text editor.

## TRANSLATE THE CONFIGURATION FILE

Simply translate the file 'dfcrc' located in 'conf' folder using your favorite
text editor.

Two things though:

  * Colors value must have exactly the same translation as the
    one you did for the po file.

  * Do NOT translate the values before the equal sign:

	  * color_header
	  * color_low
	  * color_medium
	  * ...

<!-- vim: set filetype=markdown textwidth=80 -->
