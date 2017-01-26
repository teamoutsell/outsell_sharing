<?php

/**
 * @file
 * outsell_sharing.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function outsell_sharing_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => "1");
  }
}

/**
 * Implements hook_eck_bundle_info().
 */
function outsell_sharing_eck_bundle_info() {
  $items = array(
    'segment_segment' => array(
      'machine_name' => 'segment_segment',
      'entity_type' => 'segment',
      'name' => 'segment',
      'label' => 'Segment',
    ),
  );
  return $items;
}

/**
 * Implements hook_eck_entity_type_info().
 */
function outsell_sharing_eck_entity_type_info() {
  $items = array(
    'segment' => array(
      'name' => 'segment',
      'label' => 'Segment',
      'properties' => array(
        'title' => array(
          'label' => 'Title',
          'type' => 'text',
          'behavior' => 'title',
        ),
      ),
    ),
  );
  return $items;
}

/**
 * Implements hook_flag_default_flags().
 */
function outsell_sharing_flag_default_flags() {
  $flags = array();
  // Exported flag: "Headline".
  $flags['headline'] = array(
    'entity_type' => 'node',
    'title' => 'Headline',
    'global' => 0,
    'types' => array(
      0 => 'news',
    ),
    'flag_short' => 'Flag as headline',
    'flag_long' => '',
    'flag_message' => '',
    'unflag_short' => 'Unflag as headline',
    'unflag_long' => '',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'toggle',
    'weight' => 0,
    'show_in_links' => array(
      'full' => 0,
      'teaser' => 0,
      'rss' => 0,
      'diff_standard' => 0,
    ),
    'show_as_field' => 0,
    'show_on_form' => 1,
    'access_author' => '',
    'show_contextual_link' => 1,
    'i18n' => 0,
    'module' => 'outsell_sharing',
    'locked' => array(
      0 => 'name',
    ),
    'api_version' => 3,
  );
  return $flags;

}

/**
 * Implements hook_node_info().
 */
function outsell_sharing_node_info() {
  $items = array(
    'insight' => array(
      'name' => t('Insight'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
    'news' => array(
      'name' => t('News'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
    'report' => array(
      'name' => t('Report'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  drupal_alter('node_info', $items);
  return $items;
}
