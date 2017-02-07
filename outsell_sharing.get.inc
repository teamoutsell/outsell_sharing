<?php

/**
 * @file
 * Code for the getting part of outsell_sharing.
 */

/**
 * Helper function to check for a bad method.
 */
function outsell_sharing_is_bad_method($supported_methods) {

  // Check for unsupported methods.
  if (!in_array($_SERVER['REQUEST_METHOD'], $supported_methods)) {

    // Make a message.
    $bad_method_message = 'API request using @method method not allowed.';
    $bad_method_message_options = array('@method' => $_SERVER['REQUEST_METHOD']);
    watchdog('outsell_sharing', $bad_method_message, $bad_method_message_options, WATCHDOG_WARNING);

    // Set unauthorized code.
    http_response_code(405);

  }

  // Return whether the method is bad.
  return (!in_array($_SERVER['REQUEST_METHOD'], $supported_methods));

}

/**
 * Helper function to set a bad request.
 */
function outsell_sharing_set_bad_request($message, $options) {

  // Inform the user of the failure.
  watchdog('outsell_sharing', $message, $options, WATCHDOG_WARNING);

  // Set an appropriate error code and return.
  http_response_code('400');

}

/**
 * Helper function to validate allowed content type.
 */
function outsell_sharing_has_valid_type($type) {

  // Check to see if this is an allowed type.
  $valid_type = variable_get('outsell_sharing_get_' . $type, FALSE);

  // If unallowed type let's also kick this out.
  if (!$valid_type) {
    $type_error_msg = 'Node type %type not allowed.';
    $type_error_msg_options = array('%type' => $type);
    outsell_sharing_set_bad_request($type_error_msg, $type_error_msg_options);
  }

  // Return valid type.
  return $valid_type;

}

/**
 * Helper function to validate content object props.
 */
function outsell_sharing_has_valid_props($content) {

  // Check to see if we have minimally needed properties.
  $has_props = (isset($content) && isset($content['uuid']) && isset($content['type']));

  // Bad request if applicable.
  if (!$has_props) {
    $uuid_missing = isset($content['uuid']) ? 'good' : 'missing';
    $type_missing = isset($content['type']) ? 'good' : 'missing';
    $props_error_msg = 'Missing required properties. uuid:%uuid, type:%type';
    $props_error_msg_options = array('%uuid' => $uuid_missing, '%type' => $type_missing);
    outsell_sharing_set_bad_request($props_error_msg, $props_error_msg_options);
  }

  // Return validity.
  return $has_props;

}

/**
 * Helper function to get content for POST/PUT requests.
 */
function outsell_sharing_get_body() {

  // Start up a data collector.
  $raw_data = '';

  // Let's first try to read the string.
  $incoming_data = fopen('php://input', 'r');
  $raw_data = stream_get_contents($incoming_data);
  fclose($incoming_data);

  // If the data is empty let's try to read it from $_GLOBALS.
  if (!isset($raw_data) || empty($raw_data)) {
    $raw_data = $GLOBALS['HTTP_RAW_POST_DATA'];
  }

  // Attempt to parse the data.
  $data = drupal_json_decode($raw_data);

  // Make sure the json is parsed correctly. If not watchdog, error and return.
  if (json_last_error()) {
    $parse_error_msg = 'Cannot json decode node. Reason given: %err';
    $parse_error_msg_options = array('%err' => json_last_error_msg());
    outsell_sharing_set_bad_request($parse_error_msg, $parse_error_msg_options);
  }

  // Return what we have.
  return $data;

}

/**
 * Helper function to route delete requests.
 */
function outsell_sharing_api_delete($uuid) {

  // Check whether this is an allowed method.
  if (outsell_sharing_is_bad_method(array('DELETE'))) {
    return;
  }

  // Build the content.
  $content = array('uuid' => $uuid);

  // @todo: need to get the type before we validate.
  print_r($content);

}

/**
 * Batch helper function to build a node from content.
 */
function outsell_sharing_build_node($content) {

  // Create a base entity.
  $e = entity_create('node', array('type' => $content['type']));

  // Add in any non-fieldy meta-data.
  foreach ($content as $key => $data) {
    if (field_info_field($key) === NULL) {
      $e->$key = $data;
    }
  }

  // Create the metadata wrapper.
  $new_node = entity_metadata_wrapper('node', $e);

  // Go through all the fields and build out the data.
  foreach ($content as $key => $data) {
    if (!empty($data) && field_info_field($key) !== NULL) {

      // Make sure we are doing the correct thing with each type of field.
      // @NOTE: drupal does not have a great unified way to attach data
      switch (field_info_field($key)['type']) {

        // Handle entityreference fields.
        case 'entityreference':
          break;

        // Handle field_collection fields.
        case 'field_collection':
          break;

        // Handle file fields.
        case 'file':
        case 'image':

          // Get the file contents and destination.
          $contents = base64_decode($data['data']);
          $dest = $data['uri'];
          $dirname = dirname($dest);

          // Prepare the file directory.
          // @todo: check for success
          $created = file_prepare_directory($dirname, FILE_CREATE_DIRECTORY);

          // Save the file.
          // @NOTE: This can handle updating as well
          $file = file_save_data($contents, $dest, FILE_EXISTS_REPLACE);

          // Add the file to the node.
          $new_node->$key->set(array_merge($data, (array) $file));

          break;

        // Handle node_reference fields.
        case 'node_reference':
          break;

        // Handle taxonomy_term_reference fields.
        case 'taxonomy_term_reference':

          // Grab matching terms.
          $matched_terms = taxonomy_get_term_by_name($data['name'], $data['vocabulary_machine_name']);

          // Add the term if it doesn't already exist.
          if (empty($matched_terms)) {

            // Start up a new term.
            $term = (object) $data;

            // Set the vocabulary id using the machine name.
            $term->vid = taxonomy_vocabulary_machine_name_load($term->vocabulary_machine_name)->vid;

            // Add the term.
            taxonomy_term_save($term);

          }

          // Use the first term result.
          else {
            $term = array_shift($matched_terms);
          }

          // Add the term to the node.
          $new_node->$key->set($term);

          break;

        // Handle user_reference fields.
        case 'user_reference':
          break;

        // Handle every other field.
        default:
          $new_node->$key->set($data);
      }

    }

  }

  // Save the node.
  $new_node->save();

}

/**
 * Helper function to route upsert requests.
 */
function outsell_sharing_api_upsert() {

  // Check whether this is an allowed method.
  if (outsell_sharing_is_bad_method(array('POST', 'PUT'))) {
    return;
  }

  // Let's try to get something to work with the payload.
  $content = outsell_sharing_get_body();

  // Validate our content.
  if (!outsell_sharing_has_valid_props($content) || !outsell_sharing_has_valid_type($content['type'])) {
    return;
  }

  // We've gotten this far so its time to do some magic.
  switch ($_SERVER['REQUEST_METHOD']) {

    // Create a new node.
    case 'POST':

      // Get the new or updated entity.
      $entity = outsell_sharing_build_node($content);

      break;

    // Update an existing node.
    case 'PUT':
      print "PUTTED";
      print_r($content);
      break;

    // Warn we are unsure of what to do with this request.
    // NOTE: it should be impossible to arrive here.
    default:
      outsell_sharing_set_bad_request('You have reached an unarrivable place', array());
  }

}
