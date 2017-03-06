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
  return variable_get('outsell_sharing_get_' . $type, FALSE);

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

  // Try to find an existing node.
  $node = array_shift(entity_uuid_load('node', array($content['uuid'])));

  // Add the type.
  $content['type'] = $node->type;

  // Validate our content.
  if (!outsell_sharing_has_valid_props($content) || !outsell_sharing_has_valid_type($content['type'])) {
    return;
  }

  // Delete the node.
  node_delete($node->nid);

}

/**
 * Batch helper function to build a node from content.
 */
function outsell_sharing_build_node($content) {

  // Get node query.
  $query = 'SELECT nid FROM {node} WHERE uuid = :uuid ORDER BY created LIMIT 1';

  // Get the node id.
  $id = array_shift(db_query($query, array(':uuid' => $content['uuid']))->fetchCol());

  // Try to find an existing node if we have an id.
  if ($id) {
    $node = node_load($id, NULL, TRUE);
  }

  // If node does not exist already lets spin up a new one.
  if (empty($node)) {

    // Create a base entity.
    $node = entity_create('node', array('type' => $content['type']));

    // Add in any non-fieldy meta-data (properties).
    foreach ($content as $key => $data) {
      if (field_info_field($key) === NULL) {
        $node->$key = $data;
      }
    }

  }

  // Handle user mapping.
  if ($content['user']['mail']) {
    $user = user_load_by_mail($content['user']['mail']);
    if ($user) {
      $node->uid = $user->uid;
    }
    else {
      $mail = $content['user']['mail'];
      watchdog('Outsell Sharing', "The user does not exist on recieving site: $mail.", array(), WATCHDOG_NOTICE, 'link');
      // We may eventuatlly create new users when they do not exist.
      // _create_new_user($data);
      // For now if the user from editors DNE on receiving site give uid 1.
      $node->uid = 1;
    }
  }

  // Create the metadata wrapper.
  $new_node = entity_metadata_wrapper('node', $node);

  // Go through all the fields and build out the data.
  foreach ($content as $key => $data) {
    if ($data && !empty($data) && field_info_field($key) !== NULL) {

      // Make sure we are doing the correct thing with each type of field.
      // @NOTE: drupal does not have a great unified way to attach data
      switch (field_info_field($key)['type']) {

        // Handle entityreference fields.
        case 'entityreference':

          $ids = array();
          foreach ($data as $datum) {
            if (!empty($datum['type'])) {
              $eck = 'eck_' . $datum['type'];
              $sql = db_query(
                "select id from $eck where title = :datum",
                array(
                  ':datum' => $datum['title'],
                )
              );
              foreach ($sql as $s) {
                $ids[] = $s->id;
              }  
            }
          }
          $new_node->$key->set($ids);

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

          // Is this a multivalued field or not.
          $is_multivalue = (field_info_field($key)['cardinality'] != 1);

          // If not mutlivalue array lets spoof it for now.
          if (!$is_multivalue) {
            $data = array($data);
          }

          // Collect terms.
          $terms = array();

          // Lets go through our terms.
          foreach ($data as $datum) {

            // Grab matching terms.
            $matched_terms = taxonomy_get_term_by_name($datum['name'], $datum['vocabulary_machine_name']);

            // Add the term if it doesn't already exist.
            if (empty($matched_terms)) {

              // Start up a new term.
              $term = (object) $datum;

              // Set the vocabulary id using the machine name.
              $term->vid = taxonomy_vocabulary_machine_name_load($term->vocabulary_machine_name)->vid;

              // Remove the term id.
              unset($term->tid);

              // Add the term.
              taxonomy_term_save($term);

            }

            // Use the first term result.
            else {
              $term = array_shift($matched_terms);
            }

            // Collect the term.
            $terms[] = $term;

          }

          // Add the term(s).
          $term_data = ($is_multivalue) ? $terms : $terms[0];
          $new_node->$key->set($term_data);

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

  // The tethys legacy ID string.
  $tlid = _map_tethys_nodes($content['title']);

  // watchdog('my tlid ----->', 'message:: ' . $tlid . ' ::', array(), WATCHDOG_NOTICE, 'link'); @debug.
  if (!empty($tlid)) {
    $new_node->field_tlid->set($tlid);
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

    // Create or update a node.
    case 'POST':
    case 'PUT':

      // Get the new or updated entity.
      $entity = outsell_sharing_build_node($content);

      break;

    // Warn we are unsure of what to do with this request.
    // NOTE: it should be impossible to arrive here.
    default:
      outsell_sharing_set_bad_request('You have reached an unarrivable place', array());
  }

}

/**
 * Helper function to create a user.
 *
 * @param $user
 *  The drupal_json_decod'ed user data from the editors site.
 *
 * We are currently not creating or updating users.  I left in the function
 * stub in case it is decided to implement this.
 *
 * function _create_new_user($user) {
 *   // Create a base entity.
 * }
 */

/**
 * Helper function to map old tethys ID strings to tlid field on Drupal nodes.
 *
 * @param string $title
 *   The title from the $content json object from the outsell_sharing api.
 *
 * @return string
 *   The tethys lecacy ID string; for example: 'entity:d7entity/61044:und'.
 */
function _map_tethys_nodes($title) {
  $sql = "select nid from node where type='tethys' and title=:title limit 1";
  $result = db_query($sql, array(':title' => $title));
  foreach ($result as $r) {
    $node = node_load($r->nid, NULL, TRUE);
  }
  if ($node) {
    $tlid = ($node->field_entity_id) ? $node->field_entity_id['und'][0]['value'] : '';
  }

  return $tlid;
}
