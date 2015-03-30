<?php

/**
 * @file
 * Provides Drush commands.
 */

/**
 * Implements hook_drush_command().
 */
function security_testing_drush_command() {
  $items['security-testing-run'] = array(
    'callback' => 'drush_security_testing_run',
    'description' => dt('security testing run'),
    'aliases' => array('sec-test'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  return $items;
}

/**
 * Callback for drush command sec-test.
 */
function drush_security_testing_run($directory = '') {
  if (!is_dir($directory)) {
    drush_print(dt("This is not a directory"));
    return;
  }
  security_testing_testing($directory);
  drush_print(dt("Log saved"));
  return;
}
