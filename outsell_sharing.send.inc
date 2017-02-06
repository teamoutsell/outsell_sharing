<?php

/**
 * @file
 * Code for the sending part of outsell_sharing.
 */

/**
 * Batch helper function to expand the node.
 */
function outsell_sharing_build_content($node) {

  // Define possible file fields that need contents loaded.
  $file_fields = array('file', 'image');

  // Get the metadata wrapper for the node.
  $wrap_node = entity_metadata_wrapper('node', $node);

  // Start a content collector.
  $content = array();

  // Go through all the fields and build out the data.
  foreach ($node as $key => $data) {

    // If this is a field lets use the emw.
    if (field_info_field($key) !== NULL) {

      // Get the field value and add it to our contents.
      $content[$key] = $wrap_node->$key->value();

      // Check if the field is a file field so we can grab the contents as well.
      if (in_array(field_info_field($key)['type'], $file_fields)) {
        $file = $content[$key]['uri'];
        if ($file && !empty($file)) {
          $content[$key]['data'] = base64_encode(file_get_contents($file));
        }
      }

    }

    // Else return what we already have.
    else {
      $content[$key] = $data;
    }

  }

  // Return the content object.
  return $content;

}

/**
 * Helper function to send content to an endpoint.
 */
function outsell_sharing_send_content($method, $content, $endpoint, $apikey) {

  // Build the URL.
  $url = $endpoint . '/outsell/sharing/api/share';

  // JSONify our data.
  $data = array(
    'data' => json_encode($content),
    'auth' => $apikey,
    'XDEBUG_SESSION_START' => 'sublime.xdebug'
  );

  // Build our options.
  $options = array(
    'method' => strtoupper($method),
    'data' => drupal_http_build_query($data),
    'timeout' => 15,
    'headers' => array('Content-Type' => 'application/x-www-form-urlencoded'),
  );

  // Execute.
  $result = drupal_http_request($url, $options);

  // Return result.
  return $result;

}

/**
 * Batch helper function to load an ID, send it and collect the result.
 */
function outsell_sharing_send_node($nid, $sites, &$context) {

  // Do some operation.
  $node = node_load($nid, NULL, TRUE);

  // Build the content.
  $content = outsell_sharing_build_content($node);

  // Set up our initial results array if needed.
  foreach ($sites as $site => $key) {
    if (!isset($context['results'][$site])) {
      $context['results'][$site] = array(
        'succeeded' => array(),
        'failed' => array()
      );
    }
  }

  // Ship the content and collect results.
  foreach ($sites as $site => $key) {

    // Get the result.
    $result = outsell_sharing_send_content('POST', $content, $site, $key);

    // Increment our situation as appropriate.
    if ($result->code === '200') {
      $context['results'][$site]['succeeded'][] = $content['nid'];
    }
    else {
      $context['results'][$site]['failed'][] = $content['nid'];
    }

  }

}

/**
 * Batch helper function to report results.
 */
function outsell_sharing_batch_finished($success, $results, $operations) {

  // Build meaningful messages.
  if ($success) {

    // Gather helpful info to report and log.
    foreach ($results as $site => $data) {

      // Print helpful success messages if applicable.
      if (count($data['succeeded']) > 0) {
        $success_message = '@site received @succeeded pieces of content.';
        $success_message_args = array('@site' => $site, '@succeeded' => count($data['succeeded']));
        drupal_set_message(t($success_message, $success_message_args), 'status');
        watchdog('outsell_sharing', $success_message, $success_message_args, WATCHDOG_INFO);
      }

      // Print helpful fail messages if applicable.
      if (count($data['failed']) > 0) {
        $success_message = '@site did not receive @failed pieces of content.';
        $success_message_args = array('@site' => $site, '@failed' => count($data['failed']));
        drupal_set_message(t($success_message, $success_message_args), 'warning');
        watchdog('outsell_sharing', $success_message, $success_message_args, WATCHDOG_WARNING);
      }

    }

  }

  // Log some sort of error.
  else {

    // Get the leftover ops that failed.
    $error_operation = reset($operations);

    // Construct an error message.
    $err_message = t('An error occurred while processing @operation with arguments : @args');
    $err_message_args = array(
      '@operation' => $error_operation[0],
      '@args' => print_r($error_operation[0], TRUE)
    );

    // Print and log the error.
    drupal_set_message($err_message, $err_message_args, 'error');
    watchdog('outsell_sharing', $err_message, $err_message_args, WATCHDOG_ERROR);

  }

}
