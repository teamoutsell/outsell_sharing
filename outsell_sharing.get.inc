<?php

/**
 * @file
 * Code for the getting part of outsell_sharing.
 */

/**
 * Batch helper function to build a node from content.
 */
function outsell_sharing_build_node($content) {

  return $content;

}

/**
 * Helper function to route api requests.
 */
function outsell_sharing_api_router() {

  print_r(drupal_json_decode($GLOBALS['HTTP_RAW_POST_DATA']));
  $test = $test;

}
