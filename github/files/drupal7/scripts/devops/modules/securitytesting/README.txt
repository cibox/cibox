OVERVIEW
========
Security Testing is a tool for locating XSS,CSRF and SQL Injection
vulnerabilities in the Drupal contributed modules. This tool will 
parse and scans the source code of the contributed module for 
vulnerabilities and issue warnings. It also tries to exploit XSS
vulnerability using SimpleTest module by injecting XSS data
(<script>..</script>) into the database fields of the module and
checking if any injected data will come as an output on the screen
without santizion or not.It tries to exploit CSRF using SimpleTest
by visting each link of the contributed module and checkings if any
INSERT,DELETE or UPDATE query runs or not.


Dependencies
============
Grammer parsar
Simple Test


NOTE
====
This module has some heavy global variables and invasive hooks into
the database on every single page request. Hence, this module should
never be used on a production site.It can be used for testing
purposes on development sites.


Installation and Usage
======================
1). Enable the module (/admin/modules).

2). For scanning the source code of the module,visit the configuration page
    (admin/config/development/security_testing).

3) a).Enter the location of the directory containing the source code of the
      contributed module to be scanned in the text box and click on the 
      "Start scanning" button.
3) b).You can also scan the module using Drush command :
      drush sec-test name_of_the_directory
      name_of_the_directory = directory containing source code of a module.

4). The log files are stored at "security_testing/" in the default drupal file
    directory.
   "security_testing/testing.txt"           : contains the complete path
                                              followed and different function
                                              calls encountered by the
                                              scanner while backsearching.
   "security_testing/code.php"              : contains the complete source
                                              code of the contributed module.
   "security_testing/module_name.txt"       : contains the module name of the
                                              scanned contributed module.This
                                              file will be used during
                                              security_testing simpletest test
                                              run for getting the name of the
                                              module.
   "security_testing/simpletest_results.txt": contaings the result of the 
                                              simpletest test run of
                                              security_testing.
   
5). For exploiting and checking for XSS,CSRF, Go to
    "admin/config/development/Testing" ,Select the Security Testing from the
    list and then click on "Run tests" button. Simpletest security_testing
    will get the name of the module from the
    "security_testing/module_name.txt" and performs testing.So you first have
    to perform the scanning of code at
    "admin/config/development/security_testing" and then you can start testing
    by simpletest.
 
6). The results of the Simpletest test run will be store in the
    "security_testing/simpletest_results.txt" .
   
