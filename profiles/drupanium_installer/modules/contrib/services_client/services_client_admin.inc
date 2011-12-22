<?php

/**
 * @file
 * Administration pages for configuring services client module.
 */

/**
 * Settings page callback
 */
function services_client_settings() {
  $form = array();

  $roles = array('' => '<' . t('none') . '>') + user_roles();

  $form['services_client_forbidden_role'] = array(
    '#title' => t('Do not forward role'),
    '#type' => 'select',
    '#options' => $roles,
    '#default_value' => variable_get('services_client_forbidden_role', NULL),
    '#description' => t('If user that executes event has do not forward role no services_client actions will be executed.')
  );

  return system_settings_form($form);
}

/**
 * List of all connections page callback
 */
function services_client_conn_list() {
  $output = '';

  $list = services_client_get_client_list();

  $row = array();
  $header = array(
    t('Name'), t('Machine Name'), t('Domain'), t('Endpoint'), t('Version'),
    t('SSL'), t('Operations')
  );

  if (is_array($list)) {
    foreach ($list as $num => $i) {
      $operations = array(
        l(t('Edit'),'admin/structure/services_client/edit/' . $i->cid),
        l(t('Delete'),'admin/structure/services_client/delete/' . $i->cid),
        l(t('List Hooks'),'admin/structure/services_client/hook-list/' . $i->cid),
        l(t('Add Hooks'),'admin/structure/services_client/hook-add/' . $i->cid),
      );
      
      $row[] = array(
        $i->title, $i->name, $i->domain, $i->endpoint, $i->version, $i->secured,
        implode(' | ', $operations)
      );
    }
  }

  $output .= theme('table', array('header' => $header, 'rows' => $row));
  
  return $output;
}

/**
 * Add new connect page
 */
function services_client_add() {
  return drupal_get_form('services_client_connection_form');
}

/**
 * Edit existing connection page
 */
function services_client_edit($cid) {
  return drupal_get_form('services_client_connection_form', $cid);
}

function services_client_delete() {
  return '<p>I don\'t trust you to delete.</p>';
}

/**
 * Create and edit client connection form.
 */
function services_client_connection_form($form, &$form_state, $cid = NULL) {
  $form = array();

  if (is_numeric($cid)) {
    $conn = services_client_get_client_list(array('cid' => $cid));
  }
  else {
    $conn = array();
  }

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Connection Name'),
    '#description' => t('Specify a user-friendly name for this client connection.'),
    '#size' => 25,
    '#default_value' => !empty($conn['title']) ? $conn['title'] : '',
    '#required' => TRUE,
  );
  
  $form['name'] = array(
    '#type' => 'machine_name',
    '#title' => t('Name'),
    '#default_value' => !empty($conn['name']) ? $conn['name'] : '',
    '#maxlength' => 32,
    '#description' => t('A unique name for connection.'),
    '#machine_name' => array(
      'exists' => 'services_client_services_connection_exists',
      'source' => array('title'),
      'label' => t('Machine name'),
    ),
  );
  
  $form['domain'] = array(
    '#type' => 'textfield',
    '#title' => t('Domain Name'),
    '#description' => t('Enter the domain name or IP for this connection.'),
    '#size' => 25,
    '#default_value' => !empty($conn['domain']) ? $conn['domain'] : '',
    '#required' => TRUE,
  );

  $form['endpoint'] = array(
    '#type' => 'textfield',
    '#title' => t('Endpoint Path'),
    '#description' => t('Enter the enpoint for this connection. Example: "requests/rest"'),
    '#size' => 25,
    '#default_value' => !empty($conn['endpoint']) ? $conn['endpoint'] : '',
    '#required' => TRUE,
  );

  $form['version'] = array(
    '#type' => 'select',
    '#title' => t('Services Version'),
    '#description' => t('Choose the version of services the master is running'),
    '#options' => drupal_map_assoc(array(1, 2, 3)),
    '#default_value' => !empty($conn['version']) ? $conn['version'] : 1,
    '#required' => TRUE,
  );

  $form['username'] = array(
    '#type' => 'textfield',
    '#title' => t('User Name'),
    '#description' => t('Enter the domain name or IP for this connection.'),
    '#size' => 25,
    '#default_value' => !empty($conn['username']) ? $conn['username'] : '',
    '#required' => TRUE,
  );
  
  $form['password'] = array(
    '#type' => 'password',
    '#title' => t('Password'),
    '#description' => t('Enter the enpoint for this connection. Example: "requests/rest"'),
    '#size' => 20,
    '#default_value' => !empty($conn['password']) ? $conn['password'] : '',
    '#required' => TRUE,
  );
  
  $form['secured'] = array(
    '#type' => 'select',
    '#title' => t('Require SSL'),
    '#description' => t('Does this connection require SSL?'),
    '#options' => array(
      'no'  => t('No'),
      'yes' => t('Yes'),
    ),
    '#default_value' => !empty($conn['secured']) ? $conn['secured'] : 'no',
    '#required' => TRUE,
  );

  $form['debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Debug'),
    '#description' => t('If checked additional info like pushed data are added to watchdog.'),
    '#default_value' => !empty($conn['debug']) ? $conn['debug'] : 0,
  );
  
  if (is_numeric($cid)) {
    $form['cid'] = array(
      '#type' => 'value',
      '#value' => $cid,
    );
  }
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Connection'),
  );

  return $form;
}

/**
 * Submit client connection form.
 */
function services_client_connection_form_submit($form, &$form_state) {
  $v = $form_state['values'];

  $v['domain'] = preg_replace('/http(s)?:\/\//i','',$v['domain']);

  if (empty($v['cid'])) {
    drupal_write_record('services_client_connection', $v);
    drupal_set_message(t('Connection %title successfully added.', array(
      '%title' => $v['title']
    )));
  }
  elseif (is_numeric($v['cid'])) {
    drupal_write_record('services_client_connection', $v, array('cid'));
    drupal_set_message(t('Connection %title successfully updated.', array(
      '%title' => $v['title']
    )));
  }
  
  $form_state['redirect'] = 'admin/structure/services_client/list';
}

/**
 * List of actions attached to hook
 */
function services_client_hooks_list($cid) {
  $output = '<p>' . l(t('Add Hook'), 'admin/structure/services_client/hook-add/' . $cid) . '</p>';

  $conn_name = services_client_get_conn_name($cid);
  $list = services_client_get_client_hooks_list(array('conn_name' => $conn_name, 'num' => 'all'));
  $row = array();

  $header = array('Title', 'Machine Name','Conn Name','Drupal Hook','Operations');
  if (is_array($list)) {
    foreach ($list as $num => $i) {
      $oprations = array(
        l(t('Edit Conditions'),'admin/structure/services_client/hook-condition/' . $cid . '/' . $i->hid),
        l(t('Edit Mapping'),'admin/structure/services_client/hook-mapping/' . $cid . '/' . $i->hid),
        l(t('Edit Hook'),'admin/structure/services_client/hook-edit/' . $cid . '/' . $i->hid),
        l(t('Delete Hook'),'admin/structure/services_client/hook-delete/' . $cid . '/' . $i->hid),
      );
      $row[] = array(
        $i->title, $i->name, $i->conn_name, $i->hook, implode(' | ', $oprations)
      );
    }
  }

  $output .= theme('table', array('header' => $header, 'rows' => $row));

  return $output;
}

function services_client_hooks_add($cid) {
  return drupal_get_form('services_client_hooks_form', $cid);
}

function services_client_hooks_edit($cid, $hid) {
  return drupal_get_form('services_client_hooks_form', $cid, $hid);
}

function services_client_hooks_delete($cid, $hid) {
  return '<p>I don\'t trust you to delete</p>';
}

function services_client_hooks_condition($cid, $hid) {
  return drupal_get_form('services_client_hooks_condition_form', $cid, $hid);
}

function services_client_hooks_mapping($cid, $hid) {
  return drupal_get_form('services_client_hooks_mapping_form', $cid, $hid);
}

/**
 * Create new hook event
 */
function services_client_hooks_form($form, &$form_state, $cid, $hid = NULL) {
  $form = array();

  // Retrieve connection name
  $conn_name = services_client_get_conn_name($cid);
  $form['conn_name'] = array(
    '#type' => 'value',
    '#value' => $conn_name,
  );

  if ($hid) {
    $hook = services_client_get_client_hooks_list(array('hid' => $hid));
  }

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Hook title'),
    '#description' => t('Specify a user-friendly name for this hook.'),
    '#size' => 25,
    '#default_value' => !empty($hook['title']) ? $hook['title'] : '',
    '#required' => TRUE,
  );
  
  $form['name'] = array(
    '#type' => 'machine_name',
    '#title' => t('Machine name'),
    '#default_value' => !empty($hook['name']) ? $hook['name'] : '',
    '#maxlength' => 32,
    '#description' => t('Specify a machine-name for this hook (lowercase and underscores only).'),
    '#machine_name' => array(
      'exists' => 'services_client_services_connection_hook_exists',
      'source' => array('title'),
      'label' => t('Machine name'),
      'conn_name' => $conn_name,
    ),
  );

  $avail_hooks = array(
    'node_save' => t('Node Save'),
    'user_save' => t('User Save'),
  );
  $form['hook'] = array(
    '#type' => 'select',
    '#title' => t('Drupal hook'),
    '#description' => t('Choose the Drupal hook for which to execute the services connection.'),
    '#options' => $avail_hooks,
    '#default_value' => !empty($hook['hook']) ? $hook['hook'] : '',
    '#required' => TRUE,
  );

  if (!empty($cid) && is_numeric($cid)) {
    $form['cid'] = array(
      '#type' => 'value',
      '#value' => $cid,
    );
  }
  
  if (!empty($hid) && is_numeric($hid)) {
    $form['hid'] = array(
      '#type' => 'value',
      '#value' => $hid,
    );
  }
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Hook'),
  );

  return $form;
}

/**
 * Submit handler to save hook to DB.
 */
function services_client_hooks_form_submit($form, &$form_state) {
  $v = $form_state['values'];
  $v['hook_mappings'] = '';
  $v['hook_conditions'] = '';

  if (empty($v['hid'])) {
    drupal_write_record('services_client_connection_hook', $v);
    drupal_set_message(t('Hook %title successfully added.', array('%title' => $v['title'])));
  }
  elseif (is_numeric($v['hid'])) {
    drupal_write_record('services_client_connection_hook', $v, array('hid'));
    drupal_set_message(t('Hook %title successfully updated.', array('%title' => $v['title'])));
  }

  $form_state['redirect'] = 'admin/structure/services_client/hook-list/' . $v['cid'];
}

/**
 * Event execution condition form
 */
function services_client_hooks_condition_form($form, &$form_state, $cid, $hid) {
  $form = array();

  $form['cid'] = array(
    '#type' => 'hidden',
    '#value' => $cid
  );

  $hook = services_client_get_client_hooks_list(array('hid' => $hid));
  $form['hid'] = array(
    '#type' => 'hidden',
    '#value' => $hid
  );

  $cond = unserialize($hook['hook_conditions']);

  foreach(node_type_get_types() as $type => $info) {
    $types[$type] = $info->name;
  }

  if ($hook['hook'] == 'node_save') {
    $form['node_type'] = array(
      '#type' => 'select',
      '#title' => t('Content type'),
      '#description' => 'Select the content type on which this hook is conditional. Because each hook has its own mapping, only one content type per hook is allowed.',
      '#options' => $types,
      '#default_value' => isset($cond['node_type']) ? $cond['node_type'] : NULL,
      '#required' => TRUE,
    );
    $form['published'] = array(
      '#type' => 'select',
      '#title' => t('Published'),
      '#description' => 'Send only nodes that are published, unpublished, or both.',
      '#options' => array(
        'e' => 'Either',
        '1' => 'Published',
        '0' => 'Unpublished'
      ),
      '#default_value' => isset($cond['published']) ? $cond['published'] : NULL,
      '#required' => TRUE,
    );

  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Condition'),
  );

  return $form;
}

function services_client_hooks_condition_form_submit($form, &$form_state) {
  $v = $form_state['values'];

  $sql = "UPDATE {services_client_connection_hook} SET hook_conditions = ? WHERE hid = ?";
  $servals = serialize(array(
    'node_type' => $v['node_type'],
    'published' => $v['published'],
  ));
  db_query($sql, array($servals, $v['hid']));

  drupal_set_message(t('Condition successfully saved.'));

  $form_state['redirect'] = 'admin/structure/services_client/hook-list/' . $v['cid'];
}

/**
 * Mapping form
 */
function services_client_hooks_mapping_form($form, &$form_state, $cid, $hid) {
  $form = array();

  $form['cid'] = array(
    '#type' => 'hidden',
    '#value' => $cid
  );

  $hook = services_client_get_client_hooks_list(array('hid' => $hid));
  $mapping = unserialize($hook['hook_mappings']);
  $form['hid'] = array(
    '#type' => 'hidden',
    '#value' => $hid
  );

  $form['field_mapping'] = array(
    '#type' => 'textarea',
    '#title' => t('Field Mappings'),
    '#description' => t('Enter mappings for client/master fields. Master on left, client on right, pipe separated, one per line.'),
    '#rows' => 10,
    '#size' => 30,
    '#default_value' => !empty($mapping['field_mapping']) ? $mapping['field_mapping'] : '',
  );

  $form['node_type_mapping'] = array(
    '#type' => 'textarea',
    '#title' => t('Node Type Mappings'),
    '#description' => t('Enter mappings for client/master node types. Master on left, client on right, pipe separated, one per line.'),
    '#rows' => 5,
    '#size' => 30,
    '#default_value' => !empty($mapping['node_type_mapping']) ? $mapping['node_type_mapping'] : '',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Mapping'),
  );

  return $form;
}

function services_client_hooks_mapping_form_submit($form, &$form_state) {
  $v = $form_state['values'];
  $sql = "UPDATE {services_client_connection_hook} SET hook_mappings = ? WHERE hid = ?";
  $mapping = serialize(array(
    'field_mapping' => $v['field_mapping'],
    'node_type_mapping' => $v['node_type_mapping']
  ));
  db_query($sql, array($mapping, $v['hid']));
  drupal_set_message(t('Mapping successfully saved.'));
  $form_state['redirect'] = 'admin/structure/services_client/hook-list/' . $v['cid'];
}