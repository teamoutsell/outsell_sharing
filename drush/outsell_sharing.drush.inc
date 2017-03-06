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
      'insight' => dt('Enable the insight content type as part of this migration.'),
      'news' => dt('Enable the news content type as part of this migration.'),
      'report' => dt('Enable the report content type as part of this migration.'),
      'oss-debug' => dt('Turn on debug logging defaults to FALSE.  With `oss-debug` set to TRUE the json output of the failed nodes is output to stdio for further inspection.'),
      'sites' => dt('The URLs of the sites you wish to share to. By default this will pull from the sites that are configured via the configuration form but you can override that value(s) here if you wish.'),
      'order' => dt('The order to select the nodes: asc (oldest), desc (newest)'),
      'min' => dt("The minimum nid; i.e. select nodes that are bigger than or equal to this number."),
      'max' => dt("The maximum nid; i.e. select nodes that are less than or equal to this number."),
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
  $order = drush_get_option('order', variable_get('outsell_sharing_batch_order', 'desc'));
  $min = drush_get_option('min');
  $max = drush_get_option('max');
  $insight = drush_get_option('insight', variable_get('outsell_sharing_send_insight'));
  $news = drush_get_option('news', variable_get('outsell_sharing_send_news'));
  $report = drush_get_option('report', variable_get('outsell_sharing_send_report'));
  $my_sites = drush_get_option('sites', '');
  $form_state = array();
  $form_state['values']['outsell_sharing_batch_debug'] = drush_get_option('oss-debug', FALSE);
  if ($my_sites == '') {
    $form_state['values']['outsell_sharing_sites'] = variable_get('outsell_sharing_sites');
  }
  else {
    $form_state['values']['outsell_sharing_sites'] = $my_sites;
  }
  $form_state['values']['outsell_sharing_send_insight'] = $insight;
  $form_state['values']['outsell_sharing_send_news'] = $news;
  $form_state['values']['outsell_sharing_send_report'] = $report;
  $form_state['values']['outsell_sharing_batch_order'] = variable_get('outsell_sharing_batch_order');
  $form_state['values']['outsell_sharing_batch_amount'] = $amount;
  $form_state['values']['outsell_sharing_batch_order'] = $order;
  $form_state['values']['min'] = $min;
  $form_state['values']['max'] = $max;
  $form = outsell_sharing_settings_form();

  $form_state['values']['outsell_sharing_sites'] = list_allowed_values_string($form_state['values']['outsell_sharing_sites']);

  drupal_form_submit('outsell_sharing_sender_batch_submit', $form_state);
}
