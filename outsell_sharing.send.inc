<?php

/**
 * @file
 * Code for the sending part of outsell_sharing.
 */

/**
 * Batch helper function to load an ID, send it and collect the result.
 */
function outsell_sharing_send_node($nid, $site, $total, &$context) {

  // Do some operation.
  $node = node_load($nid, NULL, TRUE);

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
