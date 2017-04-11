<?php

/**
 * @file
 * Code for the sending part of outsell_sharing.
 */

/**
 * Batch helper function to expand the node.
 */
function outsell_sharing_build_node($node) {

  // Define possible file fields that need contents loaded.
  $file_fields = array('file', 'image');

  // Get the metadata wrapper for the node.
  $wrap_node = entity_metadata_wrapper('node', $node);

  // Start a content collector.
  $content = array();

  // Go through all the fields and build out the data.
  foreach ($node as $key => $data) {
    $field_info = field_info_field($key);

    // If this is a field lets use the emw.
    if ($field_info && is_array($field_info) && $key !== 'comment_body') {

      // Get the field value and add it to our contents.
      $content[$key] = $wrap_node->$key->value();

      // Check if the field is a file field so we can grab the contents as well.
      if (in_array($field_info['type'], $file_fields)) {
        $file = $content[$key]['uri'];
        if ($file && !empty($file)) {
          $content[$key]['data'] = base64_encode(file_get_contents($file));
        }
      }
    }
    // Check for the $node->uid, load the user and send user info along too.
    elseif ($key == 'uid') {
      $user = user_load($data, TRUE);
      $content[$key] = $data;
      $content['user'] = $user;
      // Remove uid key as it is part of the user data.
      unset($content['uid']);
      // Remove the password.
      unset($content['pass']);
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
  $data = drupal_json_encode($content);

  // Specify the headers.
  $headers = array(
    'Content-Type: application/json',
    'Content-Length: ' . strlen($data),
    'Authorization: ' . $apikey
  );

  // Start the curl.
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
  curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
  curl_setopt($ch, CURLOPT_FOLLOWLOCATION, TRUE);
  curl_setopt($ch, CURLOPT_TIMEOUT, 10);


  // If we are POSTING.
  if (strtoupper($method) === 'POST') {
    curl_setopt($ch, CURLOPT_POST, TRUE);
  }
  // Else custom method.
  else {
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, strtoupper($method));
  }

  // If we are on kalabox lets whitelist self-signed certs.
  if ($_ENV['PLATFORM_ENVIRONMENT'] === 'kalabox' || $_ENV['KALABOX'] === 'ON') {
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
    curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, FALSE);
  }

  // If we are on production, temporarily disable SSL checking.
  // @todo: Figure out what's wrong here.
  if ($_ENV['PLATFORM_ENVIRONMENT'] === 'master') {
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
    curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, FALSE);
  }

  // Execute.
  $result = curl_exec($ch);
  $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);

  if (curl_errno($ch)) {
    $error = curl_error($ch);
    watchdog('outsell_sharing', 'Failed to send content NID %nid to %endpoint with message ' . print_r($error, TRUE), array('%nid' => $content['nid'], '%endpoint' => $endpoint), WATCHDOG_WARNING, NULL);
  }

  // Close connection.
  curl_close($ch);

  // Return result and code.
  return array(
    'code' => $code,
    'result' => $result,
    'sent' => $data
  );

}

/**
 * Batch helper function to load an ID, send it and collect the result.
 */
function outsell_sharing_send_node($nid, $sites, $debug, &$context) {

  // Do some operation.
  $node = node_load($nid, NULL, TRUE);

  // Build the content.
  $content = outsell_sharing_build_node($node);

  // Set up our initial results array if needed.
  foreach ($sites as $site => $key) {
    if (!isset($context['results'][$site])) {
      $context['results'][$site] = array(
        'succeeded' => array(),
        'failed' => array()
      );
    }
  }

  // Collect failed data.
  if ($debug && !isset($context['results']['bad_content'])) {
    $context['results']['bad_content'] = array();
  }

  // Ship the content and collect results.
  foreach ($sites as $site => $key) {

    // Send and get the result.
    $result = outsell_sharing_send_content('POST', $content, $site, $key);

    // Increment our situations as appropriate.
    if ($result['code'] === 200) {
      $context['results'][$site]['succeeded'][] = $content['uuid'];
    }
    else {

      // Add the bad node.
      $context['results'][$site]['failed'][] = $content['uuid'];

      // Add debug results if needed.
      if ($debug) {
        $context['results']['bad_content'][] = $result['sent'];
      }

    }

  }

}

/**
 * Batch helper function to report results.
 */
function outsell_sharing_batch_finished($success, $results, $operations) {

  // Build meaningful messages.
  if ($success) {

    // If we have "bad hombres" lets stash them for later.
    if (isset($results['bad_content'])) {
      $bad_content = $results['bad_content'];
      unset($results['bad_content']);
    }

    // Gather helpful info to report and log.
    foreach ($results as $site => $data) {

      // Print helpful success messages if applicable.
      if (count($data['succeeded']) > 0) {
        $success_message = '@site received @succeeded piece(s) of content.';
        $success_message_args = array('@site' => $site, '@succeeded' => count($data['succeeded']));
        drupal_set_message(t($success_message, $success_message_args), 'status');
        watchdog('outsell_sharing', $success_message, $success_message_args, WATCHDOG_INFO);
      }

      // Print helpful fail messages if applicable.
      if (count($data['failed']) > 0) {
        $success_message = '@site did not receive @failed piece(s) of content.';
        $success_message_args = array('@site' => $site, '@failed' => count($data['failed']));
        drupal_set_message(t($success_message, $success_message_args), 'warning');
        watchdog('outsell_sharing', $success_message, $success_message_args, WATCHDOG_WARNING);
      }

    }

    // Also print out the bad hombres if we have them.
    if (!empty($bad_content)) {
      drupal_set_message('<pre>' . print_r($bad_content, TRUE) . '</pre>', 'warning');
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
