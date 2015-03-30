
CONTENTS OF THIS FILE
---------------------

 * Author
 * Description
 * Installation
 * Usage

AUTHOR
------
Jim Berry ("solotandem", http://drupal.org/user/240748)

DESCRIPTION
-----------
This module provides a library interface to the Grammar Parser library code
available at http://drupal.org/project/grammar_parser. This interface enables
automatic loading of the classes defined in the library. The Drush Make file
included with this project will install this code library for use with the
Libraries API.

INSTALLATION
------------
To use this module, install it and the Libraries module in a modules directory.
See http://drupal.org/node/895232 for further information. Also, download the
Grammar Parser library and extract its files in a libraries directory (e.g.,
sites/all/libraries).

From the modules page on your site, enable this module and that should trigger
enabling its dependent module. Do not enable the Grammar Parser library (it
should not be an option if you place it in a proper libraries directory).

The included Drush Make file provides a convenient method of downloading and
installing the correct versions of the Libraries API (>=2) and the Grammar
Parser (>=1.2,<2.0) dependencies. From a command line, simply invoke:

  drush make --yes gplib.make

USAGE
-----
To use the Grammar Parser library in a module, add this module as a dependency
in the .info file:

  dependencies[] = gplib

and load the library files before any calls to library code:

  libraries_load('grammar_parser');

See http://drupal.org/node/1342238 for more information on the Libraries API.
For examples of using this module to interface with the Grammar Parser library,
see the Coder Upgrade module (part of the Coder project) and the Grammar Parser
UI module.
