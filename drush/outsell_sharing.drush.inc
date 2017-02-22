<?php

/**
 * @file
 * Drush wrapper functions for outsell_sharing API.
 *
 * Primary role at this time is to be able to run the sharing batches on cli.
 *
 * To run this on a local kalabox implementation of the editors site you need
 * to add the appropriate networking to the cli container.  In the file:
 * kalabox-cli.yml add the `extra_hosts` option to the drush section:
 *
 * #
 * # Drush
 * #
 * drush:
 *   extra_hosts:
 *     - "community.kbox.site:{IP_ADDRESS}"
 * #########################################
 *
 * where {IP_ADDRESS} is replaced with the ip address of the recieving
 * container.  You can get the ip address for the Community site with the
 * command: `docker inspect community_web_1 |grep IPAddreess`.
 */

/**
 * Implements hook_drush_command().
 */
function outsell_sharing_drush_command() {
  $items = array();
  $items['outsell-sharing-send'] = array(
    'description' => 'Run batch sharing of content from Editors site to Community and Companies sites.',
    'callback' => 'outsell_sharing_send',
    'options' => array(
      'amount' => dt('Amount to send per content type.  This defaults to 0; which will send all the nodes of type: `insight`, `news`, and `reports`.'),
      // 'types' => dt('The types to send (insight, news, reports).'), TODO: allow overriding of types in the future.
      'oss-debug' => dt('Turn on debug logging defaults to FALSE.  With `oss-debug` set to TRUE the json output of the failed nodes is output to stdio for further inspection.'),
      'sites' => dt('The URLs of the sites you wish to share to. By default this will pull from the sites that are configured via the configuration form but you can override that value(s) here if you wish.'),
    ),
    'required-arguments' => FALSE,
    'aliases' => array('oss'),
    'bootstrap' => DRUSH_BOOTSTRAP_MAX,
    'examples' => array(
      'Example' => 'drush outsell-sharing-send --amount=10',
    )
  );
  return $items;
}

/**
 * Command callback for outsell_sharing_send.
 */
function outsell_sharing_send() {
  require_once __DIR__ . '/../outsell_sharing.admin.inc';
  $amount = drush_get_option('amount', '0');
  $my_sites = drush_get_option('sites', '');
  $form_state = array();
  $form_state['values']['outsell_sharing_batch_debug'] = drush_get_option('oss-debug', FALSE);
  if ($my_sites == '') {
    $form_state['values']['outsell_sharing_sites'] = variable_get('outsell_sharing_sites');
  }
  else {
    $form_state['values']['outsell_sharing_sites'] = $my_sites;
  }
  $form_state['values']['outsell_sharing_send_insight'] = variable_get('outsell_sharing_send_insight');
  $form_state['values']['outsell_sharing_send_news'] = variable_get('outsell_sharing_send_news');
  $form_state['values']['outsell_sharing_send_report'] = variable_get('outsell_sharing_send_report');
  $form_state['values']['outsell_sharing_batch_order'] = variable_get('outsell_sharing_batch_order');
  $form_state['values']['outsell_sharing_batch_amount'] = $amount;
  $form = outsell_sharing_settings_form();

  $form_state['values']['outsell_sharing_sites'] = list_allowed_values_string($form_state['values']['outsell_sharing_sites']);

  drupal_form_submit('outsell_sharing_sender_batch_submit', $form_state);
}