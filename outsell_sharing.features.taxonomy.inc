<?php

/**
 * @file
 * outsell_sharing.features.taxonomy.inc
 */

/**
 * Implements hook_taxonomy_default_vocabularies().
 */
function outsell_sharing_taxonomy_default_vocabularies() {
  return array(
    'report_type' => array(
      'name' => 'Report type',
      'machine_name' => 'report_type',
      'description' => '',
      'hierarchy' => 0,
      'module' => 'taxonomy',
      'weight' => 0,
    ),
  );
}
