<?php

/**
 * @file outsell_sharing.drush.inc.
 *  Drush wrapper functions for outesll_sharing API.
 *
 * Primary role at this time is to be able to run the sharing batches on cli.
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
      'amount' => dt('Amount to send per content type.'),
      // 'types' => dt('The types to send (insight, news, reports).'),
      'debug' => dt('Turn on debub logging.'),
      'sites' => dt('The URLs of the sites you wish to share to.'),
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
  require_once(__DIR__ . '/../outsell_sharing.admin.inc');
  $amount = drush_get_option('amount', '0');
  $sites = drush_get_option('sites', '');
  $form_state['values']['outsell_sharing_batch_debug'] = drush_get_option('debug', FALSE);
  if ($sites == '') {
    $form_state['values']['outsell_sharing_sites'] = variable_get('outsell_sharing_sites');
  }
  $form_state['values']['outsell_sharing_send_insight'] = variable_get('outsell_sharing_send_insight');
  $form_state['values']['outsell_sharing_send_news'] = variable_get('outsell_sharing_send_news');
  $form_state['values']['outsell_sharing_send_reports'] = variable_get('outsell_sharing_send_reports');
  $form_state['values']['outsell_sharing_batch_order'] = variable_get('outsell_sharing_batch_order');
  $form_state['values']['outsell_sharing_batch_amount'] = variable_get('outsell_sharing_batch_amount');
  $form = outsell_sharing_settings_form();

  // Call the submit handler for /admin/config/outsell/sharing
  outsell_sharing_sender_batch_submit($form, $form_state);
  drush_print_r('made it');
}
