<?php

function flat_collection_add_form($form, &$form_state, $parent){

    drupal_set_title(t('Collection Form'));

    // Set selected profile as this is updated on every AJAX request
    if (isset($form_state['values']['select_profile_name'])) {
        $form_state['selected'] =  $form_state['values']['select_profile_name'];

    }

    // unset saved 'ajax_select' value if the ajax_select-button is unselected, the saved value of this button is empty and no button has been clicked
    if (!isset($form_state['values']['select_profile_name']) AND !empty($form_state['selected']) AND !isset($form_state['clicked_button'])) {

        $form_state['selected'] = '';
    }



    // get all available template xml
    form_load_include($form_state, 'inc','flat_deposit','Helpers/CMDI/Cmdi2FormParser');
    form_load_include($form_state, 'php','flat_deposit','Helpers/CMDI/CmdiHandler');
    $available_profiles = CmdiHandler::getAvailableTemplates('flat_collection');

    // set '$load_profile_specific_form' which will trigger loading of container with form elements
    #$load_profile_specific_form = array_search($form_state['selected'], $available_profiles);

    // Add option to import a external file
    $available_profiles ['Import'] = 'I want to import an existing CMDI file';

    global $user;
    $form['owner'] = array(
        '#title' => t('Owner of the collection'),
        '#type' => 'textfield',
        '#required' => TRUE,
        '#default_value' => $user->name,
    );

    if (!user_access('admin collection')){$form['owner']['#disabled'] = TRUE; }

    $form['trigger']['select_profile_name'] = array(
        '#title' => t('Which profile do you want to use?'),
        '#type' => 'select',
        '#empty_option' => '-- Select --',
        '#required' => TRUE,
        '#options' => $available_profiles,
        '#ajax' => array(
            'callback' => 'select_profile_name_ajax_callback',
            'wrapper' => 'template-form',
            'method' => 'replace',
            'effect' => 'fade',
        ),
    );
    if (isset($form_state['selected'])){$form['trigger']['select_profile_name']['#default_value'] = $form_state['selected'];};

    $form['cmdi_file'] = array(
        '#type' => 'file',
        '#title' => t('Choose a file'),
        '#states' => array(
            'visible' => array(
                ':input[name="select_profile_name"]' => array('value' => 'Import'),
            ),
            'required' => array(
                ':input[name="select_profile_name"]' => array('value' => 'Import'),
            ),
        ),
    );

    $form['select_policy'] = array(
        '#title' => t('Which accessibility policy do you want to apply'),
        '#type' => 'select',
        '#required' => TRUE,
        '#options' => drupal_map_assoc(array('public','private')),
        '#default_value' => 'public',
    );

    $form['hide_metadata'] = array(
        '#title' => t('Hide metadata of this collection'),
        '#type' => 'checkbox',
        '#required' => FALSE,
    );



    $form['template_container'] = array(
        '#type' => 'container',
        '#tree' => TRUE,
        '#attributes' => array(
            'id' => array('template-form'),
        ),
    );



    //**************************************************
    // Get profile specific form and attach to container
    //**************************************************

    // load form element container if profile with available template is chosen
    if (isset($form_state['selected']) and $form_state['selected'] == 'MPI_Collection'){

        $fields = CmdiHandler::generateDrupalForm($form_state['selected']);


         CmdiHandler::aggregateClickedButtons($form_state);

        // add manually generated fields to form depending on tracked button presses
        if (isset($form_state['count'])) {
            $fields = CmdiHandler::addMultivalElements($fields, $form_state['count'] );
        }



        // check if everything worked as expected
        if (!is_array($fields)){

            drupal_set_message($fields,'warning');
            return $form;

        }




        $form['template_container']['elements'] = $fields;
    }

    $form['Submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        '#validate' => array('flat_collection_add_form_final_validate'),
    );


    // attach hidden data
    $form['data'] = array(
        '#type' => 'value',
        '#value' => array(
            'parentFid' => $parent->id,
        ),
    );


    return $form;
}



function flat_collection_add_form_final_validate($form, &$form_state){

    // don't run validation if other validation has failed
    if (!empty(form_get_errors())) {

        form_set_error('debug','Other validations have failed');
        return $form;
    }

    module_load_include('php','flat_deposit','Helpers/CMDI/CmdiHandler');

    //Validate title
    // Special characters
    if ($form_state['values']['select_profile_name'] == 'MPI_Collection'){

        $title = $form_state['values']['template_container']['elements']['field_01'][0];
        if (!preg_match('/^[a-zA-Z0-9\_\s]{2,40}+$/', $title)) {
            form_set_error('title', t('Allowed are all letters, numbers and underscores (max length 40)'));
        }

        // todo Validate imported cmdi titles
    }



    // Existing collection with same label as title
// Validate chosen option

    // Validate chosen option
    if ($form_state['values']['select_profile_name'] === ''){
        form_set_error('select_profile_name','Please choose correct option');
        return $form;
    }

    if ($form_state['values']['select_profile_name'] !== 'Import'){

        // Validate that no other collection at same collection with very similar name exists
        module_load_include('inc','flat_deposit', 'inc/class.FlatTuque');
        $title = $form_state['values']['template_container']['elements']['field_01'][0];

        $collection_pid = $form_state['values']['data']['parentFid'];
        $values = FlatTuque::getChildrenLabels($collection_pid);

        if ($values === false){form_set_error('title','Unable to validate collection naming');
            return $form;
        }


        if (in_array(strtoupper($title),array_unique(array_map('strtoupper',$values)))){

            form_set_error('title','Another collection or bundle with same name exists at this location. Please use other name');
            return $form;
        }


    }

    // Validate owner.
    $owner =  $form_state['values']['owner'];
    if (!user_load_by_name($owner)) {

        form_set_error('owner','Specified owner is unknown to drupal');
        return $form;
    };


    //validate existence output dir
    $export_dir = 'metadata://' . str_replace('@', '_at_' , $owner) . "/.collection/" ;
    if (!file_exists($export_dir)){
        drupal_mkdir($export_dir, NULL, TRUE);
    }

    if (!file_exists($export_dir)){
        form_set_error('error','Cannot create directory to temporarily store cmdi files');
        return $form;
    }


    // set cmdi filename
    $profile = $form_state['values']['select_profile_name'];
    $recordCmdi = $export_dir . '/' . $profile . '_' . uniqid() . '.cmdi';

    // Get cmdi
    $import = $form_state['values']['select_profile_name'] === 'Import' ? TRUE : FALSE;
    $msg = get_cmdi($form_state['values'], $recordCmdi, $import);


    if ($msg !== TRUE){
        form_set_error('error', $msg);
        return $form;
    }

    /// do INGEST
    global $user;
    $userName = $user->name ;
    $sipType = 'Collection';
    $parentFid = $form_state['values']['data']['parentFid'];
    $test = FALSE;

    module_load_include('php','flat_deposit','Helpers/IngestService/IngestClient');
    $ingest_client = new IngestClient($sipType, $owner, $recordCmdi, $parentFid, $test);

    $options = [];
    $options ['policy'] = $form_state['values']['select_policy'];

    $fid = $ingest_client->requestSipIngest($options);

    $fObject = islandora_object_load($fid);

    if (!$fObject){
        form_set_error('error', t('Check of FID for new collection item did not reveal valid data. Error message:' . $fid));
        return $form;
    }


    $form_state['values']['data']['fid'] = (string)$fid;
    $form_state['values']['data']['label']=  $fObject->label;
    $form_state['values']['data']['owner']=  $fObject->owner;

    return $form;

}


/**
* Creates new flat_collection node and redirects to parent node.
 *
 */
function flat_collection_add_form_submit($form, &$form_state){

    $owner = user_load_by_name($form_state['values']['data']['owner']);
    $uid = $owner->uid;
    $label = $form_state['values']['data']['label'];
    $fid = $form_state['values']['data']['fid'];

    create_collection_node($label, $uid, $fid);

    drupal_set_message('New collection item has been created');
    $form_state['redirect']= 'islandora/object/' . $form_state['values']['data'][ 'parentFid'];

}


function create_collection_node($label, $uid, $fid){

    $new_node = new stdClass();
    $new_node->type = 'flat_collection';
    $new_node->language = 'und';
    $new_node->title = $label;
    $new_node->uid = $uid;
    $new_node->status = 1;
    $new_node->sticky = 0;
    $new_node->promote = 0;
    $new_node->format = 3;
    $new_node->revision = 0;
    $new_node->flat_fid['und'][0]['value'] = $fid;
    node_save($new_node);

}
