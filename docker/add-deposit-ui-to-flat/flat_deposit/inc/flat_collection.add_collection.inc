<?php

function flat_collection_add_form($form, &$form_state, $parent){

    drupal_set_title(t('Collection Form'));

    // Set selected profile as this is updated on every AJAX request
    $selected_profile = !empty($form_state['values']['select_profile_name']) ? $form_state['values']['select_profile_name'] : "";

    // get all available template xml
    module_load_include('php','flat_deposit','Helpers/CMDI/CmdiHandler');
    $available_profiles = CmdiHandler::getAvailableTemplates('flat_collection');

    // set '$load_profile_specific_form' which will trigger loading of container with form elements
    $load_profile_specific_form = array_search($selected_profile, $available_profiles);


    // Add option to import a external file
    $available_profiles ['Import'] = 'I want to import an existing CMDI file';


    global $user;
    $form['owner'] = array(
        '#title' => t('Owner of the collection'),
        '#type' => 'textfield',
        '#required' => TRUE,
        '#default_value' => $user->name,

    );

    if (!user_access('Admin collections')){$form['owner']['#disabled'] = TRUE; }

    $form['trigger']['select_profile_name'] = array(
        '#title' => t('Which profile do you want to use?'),
        '#type' => 'select',
        '#empty_option' => '-- Select --',
        '#required' => TRUE,
        '#options' => $available_profiles,
        '#default_value' => $selected_profile,
        '#ajax' => array(
            'callback' => 'select_profile_name_ajax_callback',
            'wrapper' => 'template-form',
            'method' => 'replace',
            'effect' => 'fade',
        ),
    );
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
    if ($load_profile_specific_form){

        $fields = CmdiHandler::generateDrupalForm($selected_profile);

        // check if everything worked as expected
        if (is_array($fields)){

            $form['template_container']['space'] = array(
                '#markup' => '<hr>',
            );

            $is_developer = user_role_load_by_name('developer');

            if ($is_developer){

            $form['template_container']['add_demodata'] = array(
                '#title' => t('Fill form with demo data'),
                '#type' => 'checkbox',
                '#ajax' => array(
                    'callback' => 'select_profile_name_ajax_callback',
                    'wrapper' => 'template-form',
                    'method' => 'replace',
                    'effect' => 'fade',
                )
            );

            }



            $form['template_container']['elements'] = $fields;

            ///////////
            // Fill form field with loaded data

            //first remove empty or preset values in form
            if (isset($form_state['input']['template_container']['elements'])) unset($form_state['input']['template_container']['elements']);
            $add_data = isset($form_state['values']['template_container']['add_demodata']) ? $form_state['values']['template_container']['add_demodata'] : 0;

            // set default values for form elements
            if ($add_data){

                $data =  json_decode(file_get_contents(drupal_realpath(drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/FormTemplates/Example_MPI_Collection.json')), TRUE);
                unset($data['data']);

                $result = array_merge_recursive($form['template_container']['elements'], $data);
                $result2 = exchange_numeric_key_with_default_value_property($result);

                $form['template_container']['elements'] = $result2;
                #$form['template_container']['elements']['field_04']['#default_value']= array('day'=>'10', 'month'=>'10', 'year'=>'2010');

            }



        } else {

            drupal_set_message($fields,'warning');

        }

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




function flat_collection_add_form_validate($form, &$form_state){

    $owner =  $form_state['values']['owner'];

}

function flat_collection_add_form_final_validate($form, &$form_state)
{
    module_load_include('php','flat_deposit','Helpers/CMDI/CmdiHandler');

    //Validate title
    // Special characters
    if ($form_state['values']['select_profile_name'] == 'MPI_Collection'){

        $title = $form_state['values']['template_container']['elements']['field_01'];
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

        // Validate that collection name is unique im this collection
        $collection_pid = $form_state['values']['data']['parentFid'];
        $labelNewCollection = $form_state['values']['template_container']['elements']['field_01'];
        $connection = islandora_get_tuque_connection();

        module_load_include('php','flat_deposit', 'inc/fedora_queries');

        $query = query_existing_labels_in_collection($collection_pid);
        $results = $connection->repository->ri->sparqlQuery($query);
        foreach ($results as $result) {
            if ($labelNewCollection === $result['label']['value']){

                form_set_error('title',t("Found existing collection with same collection title (%title)",array('%title' => $labelNewCollection)));
                return $form;
            };

        }


    }


    // Validate owner.
    $owner =  $form_state['values']['owner'];
    if (!user_load_by_name($owner)) {

        form_set_error('owner','Specified owner is unknown to drupal');
        return $form;
    };


    //validate existence output dir
    $export_dir = 'metadata://' . '/' . $owner . "/.collection/" ;
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
    $ingest_client = new IngestClient($sipType, $userName, $recordCmdi, $parentFid, $test);

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
    $new_node = new stdClass();
    $new_node->type = 'flat_collection';
    $new_node->language = 'und';
    $new_node->title = $form_state['values']['data']['label'];
    $new_node->uid = $owner->uid;
    $new_node->status = 1;
    $new_node->sticky = 0;
    $new_node->promote = 0;
    $new_node->format = 3;
    $new_node->revision = 0;
    $new_node->flat_fid['und'][0]['value'] = $form_state['values']['data']['fid'];
    node_save($new_node);




    drupal_set_message('New collection item has been created');
    $form_state['redirect']= 'islandora/object/' . $form_state['values']['data'][ 'parentFid'];

}
