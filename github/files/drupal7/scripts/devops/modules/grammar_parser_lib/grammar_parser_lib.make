; Drush Make API version
api = 2

; Drupal core version
core = 7.x

; Dependencies

; select the development release or latest "recommended" release
projects[grammar_parser][type] = library
projects[grammar_parser][download][type] = file
;projects[grammar_parser][download][url] = http://ftp.drupal.org/files/projects/grammar_parser-7.x-2.x-dev.tar.gz
projects[grammar_parser][download][url] = http://ftp.drupal.org/files/projects/grammar_parser-7.x-2.1.tar.gz

; enable this for development
;projects[grammar_parser][type] = library
;projects[grammar_parser][download][type] = git
;projects[grammar_parser][download][url] = http://git.drupal.org/project/grammar_parser.git
;projects[grammar_parser][download][branch] = 7.x-2.x
;projects[grammar_parser][download][tag] = 7.x-2.1

projects[libraries][subdir] = contrib
projects[libraries][version] = 2.0
