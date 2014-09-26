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

  $items['bubbleqa-blow'] = array(
    'description' => 'Builds a testing environment.',
    'arguments' => array(
      'path' => 'Local path to place source files.',
    ),
    'required-arguments' => TRUE,
    'examples' => array(
      'drush bubbleqa-blow /var/www/2582' => 'Clones the current Drupal project into /var/www/2582.',
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
 * Command callback for bubbleqa-blow.
 */
function drush_bubbleqa_blow($path) {
  $return = drush_invoke_process('@self', 'core-rsync', array('@self', $path));
  if ($return['error_status']) {
    return drush_set_error('BUBBLEQA_RSYNC_FAILED', 'Could not copy files from master site to bubble site.');
  }
  drush_log('Bubble site successfully blowed.', 'success');
}

/**
 * Implements drush_hook_COMMAND().
 *
 * Command callback for bubbleqa-pop.
 */
function drush_bubbleqa_pop($path) {
  $return = drush_shell_exec('chmod +w -R %s', $path);
  if (!$return) {
    return drush_set_error('BUBBLEQA_CHMOD_FAILED', 'Could not change permissions of the bubble site prior to delete them.');
  }
  $return = drush_shell_exec('rm -rf %s', $path);
  if (!$return) {
    return drush_set_error('BUBBLEQA_RM_FAILED', 'Could not delete bubble site files.');
  }
  drush_log('Bubble site successfully popped.', 'success');
}
