<?php

/**
 * @file
 * Bubble QA command definitions.
 */

/**
 * Implements hook_drush_command().
 */
function bubbleqa_drush_command() {
  $items = array();

  $items['bubbleqa-build'] = array(
    'description' => 'Builds a testing environment.',
    'arguments' => array(
      'path' => 'Local path to place source files.',
    ),
    'required-arguments' => TRUE,
    'examples' => array(
      'drush bubbleqa-build /var/www/2582' => 'Clones the current Drupal project into /var/www/2582.',
    ),
    'aliases' => array('bqb'),
  );

  $items['bubbleqa-pop'] = array(
    'description' => 'Tears down a testing environment.',
    'arguments' => array(
      'path' => 'Local path to the route path of the project to tear down.',
    ),
    'required-arguments' => TRUE,
    'examples' => array(
      'drush bubbleqa-pop /var/www/2582' => 'Tears down the Drupal project located at /var/www/2582.',
    ),
    'aliases' => array('bqp'),
    'bootstrap' => 'DRUSH_BOOTSTRAP_DRUPAL_ROOT',
  );

  return $items;
}

/**
 * Implements drush_hook_COMMAND().
 *
 * Command callback for bubbleqa-build.
 */
function drush_bubbleqa_build($path) {
  // 1. Copy project files to the given directory.
  $return = drush_invoke_process('@self', 'core-rsync', array('@self', $path));
  if ($return['error_status']) {
    return drush_set_error('BUBBLEQA_RSYNC_FAILED', 'Could not copy files from master site to bubble site.');
  }
  drush_log('Directory cloned.', 'status');

  // 2. Clone the database.
  $db_name = 'bubble_' . time();
  $db_file = drush_tempnam('bubble');
  $return = drush_invoke_process('@self', 'sql-query', array('CREATE DATABASE ' . $db_name));
  if ($return['error_status']) {
    return drush_set_error('BUBBLEQA_DB_CREATE', 'Could not create a database.');
  }
  $return = drush_invoke_process('@self', 'sql-dump', array(), array('result-file' => $db_file));
  if ($return['error_status']) {
    return drush_set_error('BUBBLEQA_DB_INSTALL', 'Could not dump the master database.');
  }
  $db_dump = $db_file;
  $sql = drush_sql_get_class();
  $sql->db_spec['database'] = $db_name;
  $sql_connect = $sql->connect();
  $return = drush_shell_exec($sql_connect . ' < ' . $db_dump);
  if (!$return) {
    return drush_set_error('BUBBLEQA_INSTALL_DB', 'Could not install the database dump.');
  }
  drush_log('Database cloned.', 'status');

  // 3. Adjust database credentials at the bubble site.
  $db_spec = array('default' => array('default' => $sql->db_spec));
  file_put_contents($path . '/sites/default/settings.php', "\n" . '$databases = ' . var_export($db_spec, TRUE) . ';', FILE_APPEND);
  drush_log('Bubble site successfully built.', 'success');
}

/**
 * Implements drush_hook_COMMAND().
 *
 * Command callback for bubbleqa-pop.
 */
function drush_bubbleqa_pop($path) {
  // 1. Drop database.
  $return = drush_invoke_process('@self', 'core-status', array('Database name'), array(
    'pipe',
    'format' => 'list',
    'root' => $path,
  ));
  $db_name = $return['output'];
  $return = drush_invoke_process('@self', 'sql-query', array('DROP DATABASE ' . $db_name));
  if ($return['error_status']) {
    return drush_set_error('BUBBLEQA_DB_DROP', 'Could not drop the bubble database.');
  }
  else {
    drush_log('Database dropped.', 'success');
  }

  // 2. Drop the project files.
  $return = drush_shell_exec('chmod +w -R %s', $path);
  if (!$return) {
    return drush_set_error('BUBBLEQA_CHMOD_FAILED', 'Could not change permissions of the bubble site prior to delete them.');
  }
  $return = drush_shell_exec('rm -rf %s', $path);
  if (!$return) {
    drush_set_error('BUBBLEQA_RM_FAILED', 'Could not delete bubble site files.');
  }
  else {
    drush_log('File structure deleted.', 'success');
  }

  if (!drush_get_error()) {
    drush_log('Bubble site successfully popped.', 'success');
  }
}
