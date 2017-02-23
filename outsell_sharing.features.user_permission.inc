<?php

/**
 * @file
 * outsell_sharing.features.user_permission.inc
 */

/**
 * Implements hook_user_default_permissions().
 */
function outsell_sharing_user_default_permissions() {
  $permissions = array();

  // Exported permission: 'administer pathauto'.
  $permissions['administer pathauto'] = array(
    'name' => 'administer pathauto',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'pathauto',
  );

  // Exported permission: 'notify of path changes'.
  $permissions['notify of path changes'] = array(
    'name' => 'notify of path changes',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'pathauto',
  );

  return $permissions;
}
