<?php


function flat_deposit_cmdi_form($form, &$form_state)
{

    global $user;
    $form['owner'] = array(
        '#title' => t('Owner of the collection'),
        '#type' => 'textfield',
        '#required' => TRUE,
        '#default_value' => $user->name,
    );
    $form['basic'] = array (
        '#type' => 'fieldset',
        '#tree' => false,
    );

    $form['basic']['Title'][0] = array (
        '#title' => 'Bundle title',
        '#type' => 'textfield',
        '#required' => false,
    );

    $form['basic']['Data Type'] = array(
        '#type' => 'container',
        '#prefix' => '<div id="data-type">',
        '#suffix' => '</div>',
        #'#attributes' => array('id' => array('data-type')),
    );
    $form['basic']['Data Type'][0] =  array (
        '#title' => 'Data Type',
        '#options' => array (
            'Audio recordings' => 'Audio recordings',
            'Behavioural data' => 'Behavioural data',
            'Breathing measurements' => 'Breathing measurements',
            'Cell biology data' => 'Cell biology data',
            'Computational modeling data' => 'Computational modeling data',
            'Demographic data' => 'Demographic data',
            'Dictionary' => 'Dictionary',
            'DNA sequences' => 'DNA sequences',
            'DTI data' => 'DTI data',
            'EEG data' => 'EEG data',
            'Eye tracking data' => 'Eye tracking data',
            'FCPP (forced choice pointing) data' => 'FCPP (forced choice pointing) data',
            'item notes' => 'item notes',
            'fMRI data' => 'fMRI data',
            'Genome data' => 'Genome data',
            'Geolocation data' => 'Geolocation data',
            'Grammatical description' => 'Grammatical description',
            'Grammaticality judgement data' => 'Grammaticality judgement data',
            'GSR data' => 'GSR data',
            'Histology data' => 'Histology data',
            'Images' => 'Images',
            'IQ test data' => 'IQ test data',
            'Kinematic data' => 'Kinematic data',
            'Kinship data' => 'Kinship data',
            'LENA recording data' => 'LENA recording data',
            'Lexicographic data' => 'Lexicographic data',
            'Linguistic annotations' => 'Linguistic annotations',
            'MEG data' => 'MEG data',
            'Microscopic images' => 'Microscopic images',
            'Molecular data' => 'Molecular data',
            'Neuropsychological test data' => 'Neuropsychological test data',
            'Phenotype data' => 'Phenotype data',
            'Phonetic analysis' => 'Phonetic analysis',
            'Photographs' => 'Photographs',
            'Phylogenetic data' => 'Phylogenetic data',
            'Proteomic data' => 'Proteomic data',
            'Questionnaire data' => 'Questionnaire data',
            'Reaction time data' => 'Reaction time data',
            'Resting state fMRI data' => 'Resting state fMRI data',
            'sMRI data' => 'sMRI data',
            'SNP genotype data' => 'SNP genotype data',
            'Statistical data' => 'Statistical data',
            'Stimuli' => 'Stimuli',
            'tACS data' => 'tACS data',
            'tDCS data' => 'tDCS data',
            'TMS data' => 'TMS data',
            'Transcriptions' => 'Transcriptions',
            'Transcriptome data' => 'Transcriptome data',
            'Video recordings' => 'Video recordings',
            'Word list' => 'Word list',
        ),
        '#type' => 'select',
        '#required' => false,
        '#description' => 'Kind of data that is acquired',
        '#attributes' => array('class' => array('data-type-0')),
    );

    $form['basic']['Data Type']['add'] = array (
        '#weight' => 999,
        '#name' => 'button-data-type-add',
        '#type' => 'button',
        '#value' => 'Add Data Type',
        '#ajax' => array (
            'callback' => 'data_type_buttons_ajax',
            'wrapper' => 'data-type',
            'method' => 'replace',
            'event' => 'click',
            'prevent' => 'submit click mousedown',
        ),
        '#limit_validation_errors' => array (),
    );
    if (isset($form_state['triggering_element']) AND $form_state['triggering_element']['#name'] == 'button-data-type-add'){
        $copy = $form['basic']['Data Type'][0];
        $form['basic']['Data Type'][1] = $copy;

    }
    $form['basic']['Data Type']['remove'] = array (
        '#weight' => 999,
        '#name' => 'button-data-type-remove',
        '#type' => 'button',
        '#value' => 'Remove Data Type',
        '#access' => true,
        '#ajax' =>
            array (
                'callback' => 'data_type_buttons_ajax',
                'wrapper' => 'data-type',
                'method' => 'replace',
                'effect' => 'fade',
            ),
        '#limit_validation_errors' => array (),
    );

    $form['Submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        '#validate' => array('flat_deposit_cmdi_form_final_validate'),
    );
/*
*/
    return $form;

}


function data_type_buttons_ajax($form, &$form_state)
{

    return $form['basic']['Data Type'];
}


function flat_deposit_cmdi_form_final_validate($form, &$form_state)
{
    // remove when done
    /*
    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node', $node);


    $title = $node->title;
    $collection = $wrapper->flat_parent_title->value();
    $owner = $form_state['values']['owner'];

    $export_dir = 'metadata://' . '/' . $owner . "/$collection/$title";


    $profile = $form_state['values']['select_profile_name'];
    $recordCmdi = $export_dir . '/' . $profile . '_' . uniqid() . '.cmdi';
    $form_state['values']['recordCmdi'] = $recordCmdi;

    // Get cmdi
    $import =  FALSE;
*/

    $templateName = $form_state['selected'];
    $form_data = $form_state['values']['template_container']['elements'];
    $owner = $form_state['values']['owner'];

    // get new simplexml object
    module_load_include('inc','flat_deposit','Helpers/CMDI/CmdiCreator/class.CmdiCreator');
    $creator = new CmdiCreator();
    try{

        $creator->setCmdi($templateName, $form_data, $owner);
        $cmdi = $creator->getCmdi();

        $str = $cmdi->asXML();

    } catch (CmdiCreatorException $e){


        form_set_error('debug',$e->getMessage());
        return $form;

    }

    form_set_error('debug','debug');
    return $form;


    // remove when done untill here

    // Validate owner.
    $owner = $form_state['values']['owner'];
    if (!user_load_by_name($owner)) {

        form_set_error('owner', 'Specified owner is unknown to drupal');
        return $form;
    };

    // Validate chosen option
    if ($form_state['values']['select_profile_name'] === '-- Select --') {
        form_set_error('select_profile_name', 'Please choose correct option');
        return $form;
    }

    //validate existence output dir
    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node', $node);

    $title = $node->title;
    $collection = $wrapper->flat_parent_title->value();


    $export_dir = 'metadata://' . '/' . $owner . "/$collection/$title";
    if (!file_exists($export_dir)) {
        drupal_mkdir($export_dir, NULL, TRUE);
    }

    if (!file_exists($export_dir)) {
        form_set_error('error', 'Cannot create directory to temporarily store cmdi files');
        return $form;
    }


    // stop validation if errors have occurred
    if(form_get_errors()){
        return $form;
    }

    // set cmdi filename
    $profile = $form_state['values']['select_profile_name'];
    $recordCmdi = $export_dir . '/' . $profile . '_' . uniqid() . '.cmdi';
    $form_state['values']['recordCmdi'] = $recordCmdi;

    // Get cmdi
    $import =  FALSE;
    module_load_include('inc','flat_deposit','Helpers/CMDI/class.CmdiHandler');
    $msg = get_cmdi($form_state['values'], $recordCmdi, $import);


    if ($msg !== TRUE){
        form_set_error('error', $msg);
        return $form;
    }




}



function flat_deposit_cmdi_form_submit($form, &$form_state){
    /*

        $node = menu_get_object();
        $wrapper = entity_metadata_wrapper('node', $node);

        $new_file = file_save((object)array(
            'filename' => 'record.cmdi',
            'uri' => $form_state['values']['recordCmdi'],
            'status' => FILE_STATUS_PERMANENT,
            'filemime' => file_get_mimetype($form_state['values']['recordCmdi']),
            'display' => '1',
            'description' =>'',
        ));



        // for some unknown reason flat_location and flat_original_path are messed up by attaching the newly created cmdi file, so we need to restore it
        $flat_location_original = $wrapper->flat_location->value();
        $flat_original_path_original = $wrapper->flat_original_path->value();

        $wrapper->flat_cmdi_file->file->set($new_file);
        $wrapper->save();

        $node = menu_get_object();
        $wrapper = entity_metadata_wrapper('node', $node);
        $wrapper->flat_location->set($flat_location_original);
        $wrapper->flat_original_path->set($flat_original_path_original);
        $wrapper->save();
        $form_state['redirect'] = 'node/' .$node->nid;
        drupal_set_message(t('Metadata for bundle %title has been saved', array('%title' => $node->title)));
    */

    drupal_set_message(t('Metadata for bundle has been saved'));


}