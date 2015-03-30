<?php

/**
 * @file
 * Provides routines for managing Output, Secure and Normal function calls.
 */

/**
 * Display output function calls.
 */
function security_testing_output() {
  $output = '';

  $header = array(
    array(
      'data' => t('Output function call'),
      'field' => 'ofc',
      'sort' => 'asc'),
    array(
      'data' => t('Operations'),
      'colspan' => 2),
  );

  $rows = array();

  // Getting all output function calls from database.
  $output_calls = db_select('security_testing_output', 'n')
    ->fields('n', array('ofc_id', 'name_of_ofc'))
    ->execute();
  foreach ($output_calls as $out) {
    $out_rows[] = array(
      check_plain($out->name_of_ofc),

      // Making link for deleting output function call with id $out->ofc_id.
      l(t('Delete'), 'admin/config/development/security_testing/delete_ofc/' . $out->ofc_id),
    );
  }

  $options = array(
    'header' => $header,
    'rows' => $out_rows,
  );

  $output .= theme('table', $options);

  // Adding a form to add ouput function call in the same page.
  $output .= @drupal_render(drupal_get_form('security_testing_add', "output"));
  return $output;
}

/**
 * Add form values to the database.
 */
function security_testing_add($form, $form_state, $func_call) {

  $form['ofc'] = array(
    '#type' => 'textarea',
    '#title' => t('Enter the @func_call function calls to add to the database', array('@func_call' => $func_call)),
    '#description' => t('If you want to add more than one function call then place "," in between two function calls.Example : one_call,two_call,three_call'),
    '#required' => TRUE,
  );

  $form['ofc_button'] = array(
    '#type' => 'submit',
    '#value' => t('Add @func_call function calls', array('@func_call' => $func_call)),
  );

  $form['func_call'] = array(
    '#type' => 'value',
    '#value' => $func_call,
  );

  return $form;
}


/**
 * Submit handler for security_testing_Add().
 */
function security_testing_add_submit($form, &$form_state) {

  $value = $form_state['values']['ofc'];
  $split = preg_split('[,]', $value);

  // Get the type of function call.
  $det = $form_state['values']['func_call'];

  if ($det == 'output') {
    $col_name = 'name_of_ofc';
    $table_name = 'security_testing_output';
    $status_msg = "are added to the output function call database";
  }
  elseif ($det == 'secure') {
    $col_name = 'name_of_sfc';
    $table_name = 'security_testing_secure';
    $status_msg = "are added to the secure function call database";
  }
  else {
    $col_name = 'name_of_nfc';
    $table_name = 'security_testing_normal';
    $status_msg = "are added to the normal function call database";
  }

  // Get the entries from database. Used for checking duplicate entry.
  $entries = db_select($table_name, 'n')
    ->fields('n', array($col_name))
    ->execute();
  $test = $print = array();
  foreach ($entries as $entry) {
    $test[] = $entry->$col_name;
  }

  // Checking if the newly added function call is already present in the.
  // database.If not add this function call to the database.
  foreach ($split as $value) {

    $value = trim(check_plain($value));

    // Issue message if the function call is already present in the database.
    if (in_array($value, $test)) {
      drupal_set_message(t('Function call @value already present in the @func_call function call database', array('@value' => $value, '@func_call' => $det)));
    }
    else {
      // Add the function call to the database.
      db_insert($table_name)
        ->fields(array(
          $col_name => $value,
        ))
        ->execute();
      $print[] = $value;
    }
  }
  $status_msg2 = implode($print, ',');

  if (count($print) != 0) {
    drupal_set_message(t('Function calls @print @status_msg', array('@print' => $status_msg2, '@status_msg' => $status_msg)));
  }
}

/**
 * Display confirmation message for deletion of output function call.
 */
function security_testing_delete_ofc_confirm($form) {
  $form = array();
  return confirm_form($form, t('Are you sure you want to delete this function call from the Database?'), 'admin/config/development/security_testing/ofc');
}

/**
 * Delete confirmation callback for output function call.
 */
function security_testing_delete_ofc_confirm_submit($form, &$form_state) {

  $ofc_id = $form_state['build_info']['args'][0];

  if ($form_state['values']['confirm']) {

    // Delete from DB.
    db_delete('security_testing_output')
      ->condition('ofc_id', $ofc_id)
      ->execute();

    $form_state['redirect'] = 'admin/config/development/security_testing/ofc';
  }
}

/**
 * Display secure function calls.
 */
function security_testing_secure() {
  $output = '';

  $header = array(
    array(
      'data' => t('Secure function call'),
      'field' => 'sfc',
      'sort' => 'asc'),
    array(
      'data' => t('Operation'),
      'colspan' => 2),
  );

  $rows = array();
  $secure_calls = db_select('security_testing_secure', 'n')
    ->fields('n', array('sfc_id', 'name_of_sfc'))
    ->execute();
  foreach ($secure_calls as $sec) {
    $sec_rows[] = array(
      check_plain($sec->name_of_sfc),
      l(t('Delete'), 'admin/config/development/security_testing/delete_sfc/' . $sec->sfc_id),
    );
  }

  $options = array(
    'header' => $header,
    'rows' => $sec_rows,
  );

  $output .= theme('table', $options);
  $output .= @drupal_render(drupal_get_form('security_testing_add', "secure"));
  return $output;
}

/**
 * Display confirmation message for deletion of secure function call.
 */
function security_testing_delete_sfc_confirm($form) {
  $form = array();
  return confirm_form($form, t('Are you sure you want to delete this functionCall from the Database?'), 'admin/config/development/security_testing/sfc');
}

/**
 * Delete confirmation callback for secure function call.
 */
function security_testing_delete_sfc_confirm_submit($form, &$form_state) {
  $sfc_id = $form_state['build_info']['args'][0];

  if ($form_state['values']['confirm']) {

    // Delete from DB.
    db_delete('security_testing_secure')
      ->condition('sfc_id', $sfc_id)
      ->execute();

    $form_state['redirect'] = 'admin/config/development/security_testing/sfc';
  }
}

/**
 * Display secure function calls.
 */
function security_testing_normal() {
  $output = '';

  $header = array(
    array(
      'data' => t('Normal function call'),
      'field' => 'nfc',
      'sort' => 'asc'),
    array('data' => t('Operation'), 'colspan' => 2),
  );

  $rows = array();
  $normal_calls = db_select('security_testing_normal', 'n')
    ->fields('n', array('nfc_id', 'name_of_nfc'))
    ->execute();
  foreach ($normal_calls as $nor) {
    $nor_rows[] = array(
      check_plain($nor->name_of_nfc),
      l(t('Delete'), 'admin/config/development/security_testing/delete_nfc/' . $nor->nfc_id),
    );
  }

  $options = array(
    'header' => $header,
    'rows' => $nor_rows,
  );

  $output .= theme('table', $options);
  $output .= @drupal_render(drupal_get_form('security_testing_add', "normal"));
  return $output;
}

/**
 * Display confirmation message for deletion of normal function call.
 */
function security_testing_delete_nfc_confirm($form) {
  $form = array();
  return confirm_form($form, t('Are you sure you want to delete this functionCall from the Database?'), 'admin/config/development/security_testing/nfc');
}

/**
 * Delete confirmation callback for normal function call.
 */
function security_testing_delete_nfc_confirm_submit($form, &$form_state) {
  $nfc_id = $form_state['build_info']['args'][0];

  if ($form_state['values']['confirm']) {

    // Delete from DB.
    db_delete('security_testing_normal')
      ->condition('nfc_id', $nfc_id)
      ->execute();

    $form_state['redirect'] = 'admin/config/development/security_testing/nfc';
  }
}
