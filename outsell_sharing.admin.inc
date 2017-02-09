<?php

/**
 * @file
 * Code for the admin part of outsell_sharing.
 */

/**
 * Drupal form for our sharing settings.
 */
function outsell_sharing_settings_form() {

  // Get the available content types we can share.
  $content_types = outsell_sharing_node_info();

  // Basic sharing config.
  $form['outsell_sharing_config'] = array(
    '#type' => 'fieldset',
    '#title' => t('Sharing configuration'),
    '#weight' => 0,
  );
  $form['outsell_sharing_config']['outsell_sharing_sender'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow this site to send content to other sites.'),
    '#description' => t('This will allow this site to send new content to other sites.'),
    '#default_value' => variable_get('outsell_sharing_sender', FALSE),
  );
  $form['outsell_sharing_config']['outsell_sharing_getter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow this site to get content from other sites.'),
    '#description' => t('This will allow this site to receive new content from other sites.'),
    '#default_value' => variable_get('outsell_sharing_getter', FALSE),
  );

  // Options for sending.
  $form['outsell_sharing_sender_config'] = array(
    '#type' => 'fieldset',
    '#title' => t('Configure Sending'),
    '#states' => array(
      'visible' => array(
        ':input[name=\'outsell_sharing_sender\']' => array('checked' => TRUE),
      )
    )
  );
  $form['outsell_sharing_sender_config']['outsell_sharing_sites'] = array(
    '#type' => 'textarea',
    '#title' => t('Sites to send content and their api keys (one per line):'),
    '#cols' => 50,
    '#rows' => 5,
    '#default_value' => list_allowed_values_string(variable_get('outsell_sharing_sites', array())),
    '#description' => t('Enter sites here in the format url|apikey (one per line), e.g:
<br/>
<br/>https://editors.kbox.site|h39yfih9po3nop3u9fh3
<br/>https://outsellinc.com|09u3foin4fioh8hlkeshio
<br/>
<br/>NOTE: You must enable these sites to receive content. When you do you will get an API key to use above.
'),
  );

  // Loop through our sites and add options for each.
  foreach ($content_types as $type => $data) {
    $t_opts = array('!type' => '<strong><em>' . $type . '</em></strong>');
    $form['outsell_sharing_sender_config']['outsell_sharing_send_' . $type] = array(
      '#type' => 'checkbox',
      '#title' => t('Send !type.', $t_opts),
      '#description' => t('This will allow this site to receive !type content from other sites.', $t_opts),
      '#default_value' => variable_get('outsell_sharing_send_' . $type, FALSE),
    );
  }

  // Options for batching.
  $form['outsell_sharing_batch_config'] = array(
    '#type' => 'fieldset',
    '#title' => t('Batch send content'),
    '#states' => array(
      'visible' => array(
        ':input[name=\'outsell_sharing_sender\']' => array('checked' => TRUE),
      )
    )
  );
  $form['outsell_sharing_batch_config']['outsell_sharing_batch_description'] = array(
    '#type' => 'markup',
    '#markup' => t('Some options to restrict the batch (good for testing).<br><br><br>'),
  );
  $form['outsell_sharing_batch_config']['outsell_sharing_batch_amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Amount to send per content type'),
    '#description' => t('An integer value to limit the batch.'),
    '#size' => 60,
    '#maxlength' => 128
  );
  $form['outsell_sharing_batch_config']['outsell_sharing_batch_order'] = array(
    '#type' => 'radios',
    '#title' => t('Sorting criteria'),
    '#description' => t('Sort the content by most recent or oldest.'),
    '#default_value' => 'DESC',
    '#options' => array('DESC' => t('Most recent content.'), 'ASC' => t('Oldest content.'))
  );
  $form['outsell_sharing_batch_config']['outsell_sharing_batch_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Debug content that failed to send'),
    '#description' => t('This will print an array of failed JSON objects when batch is done.')
  );
  $form['outsell_sharing_batch_config']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send content'),
    '#submit' => array('outsell_sharing_sender_batch_submit'),
    '#validate' => array('outsell_sharing_sender_batch_validate')
  );

  // Options for getting.
  $form['outsell_sharing_getter_config'] = array(
    '#type' => 'fieldset',
    '#title' => t('Configure Getting'),
    '#states' => array(
      'visible' => array(
        ':input[name=\'outsell_sharing_getter\']' => array('checked' => TRUE),
      )
    )
  );
  $form['outsell_sharing_getter_config']['outsell_sharing_apikey'] = array(
    '#type' => 'textfield',
    '#title' => t('API Key'),
    '#description' => t('Use this to send content from another site to this one.'),
    '#default_value' => variable_get('outsell_sharing_apikey', md5(drupal_random_bytes(128))),
    '#prefix' => '<div id="outsell-sharing-apikey-ajax">',
    '#suffix' => '</div>',
    '#size' => 60,
    '#maxlength' => 128
  );
  $form['outsell_sharing_getter_config']['outsell_sharing_apikey_refresh'] = array(
    '#type' => 'button',
    '#value' => t('Regenerate token'),
    '#ajax' => array(
      'callback' => 'outsell_sharing_regenerate_apikey_ajax',
      'wrapper' => 'outsell-sharing-apikey-ajax',
      'method' => 'replace',
      'effect' => 'fade'
    )
  );

  // Loop through our content types and add options for each type.
  foreach ($content_types as $type => $data) {
    $t_opts = array('!type' => '<strong><em>' . $type . '</em></strong>');
    $form['outsell_sharing_getter_config']['outsell_sharing_get_' . $type] = array(
      '#type' => 'checkbox',
      '#title' => t('Get !type.', $t_opts),
      '#description' => t('This will allow this site to receive !type content from other sites.', $t_opts),
      '#default_value' => variable_get('outsell_sharing_get_' . $type, FALSE),
    );
  }

  // Add another submit handler to handle unsetting things correctly.
  $form['#submit'][] = 'outsell_sharing_settings_form_submit';

  // Wrap and return.
  return system_settings_form($form);

}

/**
 * Implements outsell_sharing_form_outsell_sharing_settings_form_alter().
 *
 * Checks to see if our sharing config is provided by the ENV and if it is lock it
 * down so other things cannot edit it.
 */
function outsell_sharing_form_outsell_sharing_settings_form_alter(&$form, $form_state) {

  // Check to see if we have ENVIRONMENTALLY INJECTED auth0 variables.
  if (isset($_ENV['PLATFORM_VARIABLES']) && !empty($_ENV['PLATFORM_VARIABLES'])) {

    // Decode our variables.
    $variables = json_decode(base64_decode($_ENV['PLATFORM_VARIABLES']), TRUE);

    // Define fields we expect to be managed.
    // NOTE: Variable name => location in form data (only works one level deep)
    $expected_settings = array(
      'outsell_sharing_getter' => 'outsell_sharing_config',
      'outsell_sharing_sender' => 'outsell_sharing_config',
      'outsell_sharing_apikey' => 'outsell_sharing_getter_config'
    );

    // Get the available content types we can share.
    $content_types = outsell_sharing_node_info();

    // Loop through our content types and add options for each type.
    foreach ($content_types as $type => $data) {
      $expected_settings['outsell_sharing_get_' . $type] = 'outsell_sharing_getter_config';
      $expected_settings['outsell_sharing_send_' . $type] = 'outsell_sharing_sender_config';
    }

    // Collect fields that actually are managed.
    $managed_settings = array();

    // Loop through our fields and disable ones that are set and non-empty.
    foreach ($expected_settings as $variable => $location) {

      // Construct variable key.
      $variable_key = implode(':', array('drupal', $variable));

      // Disable the form element if we have a value for it.
      if (isset($variables[$variable_key])) {
        $form[$location][$variable]['#disabled'] = TRUE;
        $managed_settings[] = $variable;

      }

    }

    // Provide some extra magic for the API key regen button.
    $outsell_sharing_apikey_form = $form['outsell_sharing_getter_config']['outsell_sharing_apikey'];
    if (isset($outsell_sharing_apikey_form['#disabled']) && $outsell_sharing_apikey_form['#disabled']) {
      $form['outsell_sharing_getter_config']['outsell_sharing_apikey_refresh']['#disabled'] = TRUE;
    }

    // Inform the user of this managed status.
    if (isset($managed_settings) && !empty($managed_settings)) {

      // The message.
      $msg = 'The following fields are managed by the PLATFORM_VARIABLES environmental variable:';
      $msg .= theme_item_list(array(
        'items' => $managed_settings,
        'title' => '',
        'attributes' => array(),
        'type' => 'ol'
      ));
      $msg .= 'They will be disabled while this is true. ';
      $msg .= 'For more information about how this works ';
      $msg .= l(
        t('read more.'),
        'https://docs.platform.sh/development/variables.html',
        array('attributes' => array('target' => '_blank'))
      );

      // Construct the markup field.
      $enviro_notice = array(
        '#prefix' => '<div class="messages status">',
        '#markup' => t($msg),
        '#suffix' => '</div>'
      );

      // Add to the begining of our form.
      array_unshift($form, $enviro_notice);

    }

  }

}

/**
 * Helper function to regenerate the api key.
 */
function outsell_sharing_regenerate_apikey_ajax($form, $form_state) {
  $apikey = $form['outsell_sharing_getter_config']['outsell_sharing_apikey'];
  $apikey['#value'] = md5(drupal_random_bytes(128));
  return $apikey;
}

/**
 * Submit handler to unset any suboptions if needed.
 */
function outsell_sharing_settings_form_submit($form, &$form_state) {

  // Get the available content types we can share.
  $content_types = outsell_sharing_node_info();

  // Make sure we arrayify our list.
  $sites = $form_state['values']['outsell_sharing_sites'];
  $form_state['values']['outsell_sharing_sites'] = list_extract_allowed_values($sites, 'list_text', FALSE);

  // Unset getters if needed.
  if ($form_state['values']['outsell_sharing_getter'] === 0) {
    foreach ($content_types as $type => $data) {
      $form_state['values']['outsell_sharing_get_' . $type] = 0;
    }
  }

  // Unset senders if needed.
  if ($form_state['values']['outsell_sharing_sender'] === 0) {

    // Remove content senders.
    foreach ($content_types as $type => $data) {
      $form_state['values']['outsell_sharing_send_' . $type] = 0;
    }

    // Remove sites.
    $form_state['values']['outsell_sharing_sites'] = NULL;

  }

}

/**
 * Validate handler to check the batch.
 */
function outsell_sharing_sender_batch_validate($form, &$form_state) {

  // Get the available content types we can share.
  $allowed_content_types = outsell_sharing_node_info();

  // Collect errors.
  $content_type_responses = 0;

  // Collect responses.
  foreach ($allowed_content_types as $allowed_content_type => $data) {
    if ((bool) $form_state['values']['outsell_sharing_send_' . $allowed_content_type] === TRUE) {
      $content_type_responses++;
    }
  }

  // Throw errors if needed.
  if ($content_type_responses === 0) {
    form_set_error('Content Type', t('Need to select at least one type of content!'));
  }
  if (empty($form_state['values']['outsell_sharing_sites'])) {
    form_set_error('Site', t('Need to specify at least one site!'));
  }
  if ($form_state['values']['outsell_sharing_batch_amount'] && !is_numeric($form_state['values']['outsell_sharing_batch_amount'])) {
    form_set_error('Batch', t('Batch amount needs to be an integer'));
  }

}

/**
 * Submit handler to execute the batch.
 */
function outsell_sharing_sender_batch_submit($form, &$form_state) {

  // Get the available content types we can share.
  $allowed_content_types = outsell_sharing_node_info();

  // Get the debug mode.
  $debug = (bool) $form_state['values']['outsell_sharing_batch_debug'];

  // Get the sites we want to send to.
  $sites = list_extract_allowed_values($form_state['values']['outsell_sharing_sites'], 'list_text', FALSE);

  // Collectors for sites and content and node ids.
  $content_types = array();
  $nids = array();

  // What we sending?
  foreach ($allowed_content_types as $allowed_content_type => $data) {
    if ((bool) $form_state['values']['outsell_sharing_send_' . $allowed_content_type] === TRUE) {
      $content_types[] = $allowed_content_type;
    }
  }

  // Grab all our node ids.
  $sites_string = implode(', ', array_keys($sites));
  foreach ($content_types as $content_type) {

    // Construct the query string.
    $query = 'SELECT nid, created FROM {node} WHERE type = :type ORDER BY created ';

    // Add the sort order.
    $query .= $form_state['values']['outsell_sharing_batch_order'];

    // If we have a batch amount let us add that in as well.
    if ($form_state['values']['outsell_sharing_batch_amount']) {
      $query .= ' LIMIT ' . $form_state['values']['outsell_sharing_batch_amount'];
    }

    // Get the nodes.
    $type_nids = db_query($query, array(':type' => $content_type))->fetchCol();
    $type_nids_count = count($type_nids);

    // Tell the user WTFIGO.
    $message = t(
      'Sending @num of %type to %sites',
      array(
        '@num' => $type_nids_count,
        '%type' => $content_type,
        '%sites' => $sites_string
      )
    );
    drupal_set_message($message);

    // Add to our nids.
    $nids = array_merge($nids, $type_nids);

  }

  // Collect some operations.
  $operations = array();

  // Iterate through all the ids add an op to each.
  foreach ($nids as $nid) {

    // Build the op args.
    $args = array($nid, $sites, $debug);

    // Add an operation.
    $operations[] = array('outsell_sharing_send_node', $args);

  }

  // Construct the batch.
  $batch = array(
    'operations' => $operations,
    'finished' => 'outsell_sharing_batch_finished',
    'file' => drupal_get_path('module', 'outsell_sharing') . '/outsell_sharing.send.inc'
  );

  // Set the batch.
  batch_set($batch);

}
