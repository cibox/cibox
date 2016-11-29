<?php

/**
 * @file
 * Test if XHProf is available and working.
 */

$success = TRUE;

xhprof_enable();
$data = xhprof_disable();

if (isset($data['main()==>xhprof_disable'])) {
  print "XHProf profiling working.\r\n";
}
else {
  print "XHProf profiling not working.\r\n";
  $success = FALSE;
}

$output_dir = ini_get("xhprof.output_dir");
if (!empty($output_dir) && is_writable($output_dir)) {
  print "XHProf output directory writable.\r\n";
}
else {
  print "XHProf output directory not writable.\r\n";
  $success = FALSE;
}

if (!$success) {
  exit(1);
}
