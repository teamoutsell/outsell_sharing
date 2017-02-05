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
  $file_fields = array('file', image);

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
function outsell_sharing_send_content($content, $endpoint) {

  // JSONofy our data.
  $data = json_encode($content);

  // Define headers.
  $headers = array(
    'Content-Type: application/json',
    'Content-Length: ' . strlen($data),
  );

  // cURL.
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $endpoint);
  curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'POST');
  curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
  curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
  curl_setopt($ch, CURLOPT_TIMEOUT, 5);
  curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 5);

  // Execute.
  $result = curl_exec($ch);

  // Close connection.
  curl_close($ch);
  $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);

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

  // Ship the content.
  $result = outsell_sharing_send_content($content, 'http://google.com');

  // Store some result for post-processing in the finished callback.
  $context['results'][] = check_plain($node->title);

}

/**
 * Batch helper function to report results.
 */
function outsell_sharing_batch_finished($success, $results, $operations) {
  if ($success) {
    // Here we could do something meaningful with the results.
    // We just display the number of nodes we processed...
    drupal_set_message(t('@count results processed in @requests HTTP requests.', array('@count' => count($results), '@requests' => 'wefwef')));
    drupal_set_message(t('The final result was "%final"', array('%final' => end($results))));
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    drupal_set_message(
    t('An error occurred while processing @operation with arguments : @args',
    array(
      '@operation' => $error_operation[0],
      '@args' => print_r($error_operation[0], TRUE),
    )
    ),
    'error'
    );
  }
}
