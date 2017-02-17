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
    'category_regions' => array(
      'name' => 'Category Regions',
      'machine_name' => 'category_regions',
      'description' => '',
      'hierarchy' => 0,
      'module' => 'taxonomy',
      'weight' => 0,
    ),
    'headline_topics' => array(
      'name' => 'Headline topics',
      'machine_name' => 'headline_topics',
      'description' => '',
      'hierarchy' => 0,
      'module' => 'taxonomy',
      'weight' => 0,
    ),
    'industry_segment' => array(
      'name' => 'Industy Segment',
      'machine_name' => 'industry_segment',
      'description' => '',
      'hierarchy' => 0,
      'module' => 'taxonomy',
      'weight' => 0,
    ),
    'keywords' => array(
      'name' => 'Keywords',
      'machine_name' => 'keywords',
      'description' => '',
      'hierarchy' => 0,
      'module' => 'taxonomy',
      'weight' => 0,
    ),
    'report_type' => array(
      'name' => 'Report type',
      'machine_name' => 'report_type',
      'description' => '',
      'hierarchy' => 0,
      'module' => 'taxonomy',
      'weight' => 0,
    ),
    'segments' => array(
      'name' => 'Segments',
      'machine_name' => 'segments',
      'description' => '',
      'hierarchy' => 0,
      'module' => 'taxonomy',
      'weight' => 0,
    ),
    'tags' => array(
      'name' => 'Tags',
      'machine_name' => 'tags',
      'description' => 'Use tags to group articles on similar topics into categories.',
      'hierarchy' => 0,
      'module' => 'taxonomy',
      'weight' => 0,
    ),
    'type_news' => array(
      'name' => 'Type news',
      'machine_name' => 'type_news',
      'description' => '',
      'hierarchy' => 0,
      'module' => 'taxonomy',
      'weight' => 0,
    ),
  );
}
