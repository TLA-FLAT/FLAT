<?php

module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/CmdiCreator/class.CmdiCreator');
autoload_cmdicreator();

class CmdiTemplateSaver
{
    private $form_state;
    private $profile;

    public function __construct(&$form_state)
    {
        $this->form_state = &$form_state;
    }

    public function isSubmitted(): bool
    {
        return isset($this->form_state['triggering_element']['#ajax']['callback']) && $this->form_state['triggering_element']['#ajax']['callback'] === 'flat_cmdi_templates_ajax_save_callback';
    }

    public function getProfile()
    {
        if (isset($this->form_state['complete form']['select_profile_name']['#value'])) {

            // update_collection.inc does not have the trigger field
            // using this
            return $this->form_state['complete form']['select_profile_name']['#value'];
        }

        if (!isset($this->form_state['complete form']['trigger']['select_profile_name']['#value'])) {

            // trigger was not found
            return null;
        }

        return $this->form_state['complete form']['trigger']['select_profile_name']['#value'];
    }

    public function getOwner()
    {
        global $user;
        return $user->name;
    }

    public function getCmdiTree()
    {
        return $this->form_state['triggering_element']['#cmdi_tree'];
    }

    public function getElements()
    {
        return $this->form_state['complete form']['template_container']['elements'];
    }

    public function getElement()
    {
        return array_reduce($this->getCmdiTree(), function($result, $index) {

            if ($result && !is_null($index)) {
              return (array_key_exists($index, $result)) ? $result[$index] : null;
            }

        }, $this->getElements());
    }

    public function generate()
    {
        $tree = $this->getCmdiTree();

        // removing final node from tree, because element should be added into the parent
        array_pop($tree);

        // get specific form element
        $formElement = $this->getElement();

        if (null === $formElement) {

            // form element could not be found
            return null;
        }

        // build element
        $element = CmdiTemplateBuilder::buildTemplateBlock($formElement, $this->form_state);

        if (null === $element) {

            // element could not be built
            return null;
        }

        return json_encode([

            'tree'    => $tree,
            'element' => $element,
        ]);
    }

    public function getComponentId()
    {
        return $this->form_state['triggering_element']['#component_id'];
    }

    public function getLabel()
    {
        return $this->form_state['input'][
            $this->form_state['triggering_element']['#label_field_name']
        ];
    }

    public function save()
    {
        if (false === $this->isSubmitted()) {
            return false;
        }

        $label = $this->getLabel();

        if (empty($label)) {

            form_set_error('flat_cmdi_templates', 'Please provide a label for the cmdi template block');
            return false;
        }

        $profile = $this->getProfile();

        if (null === $profile) {
            return false;
        }

        $block = $this->generate();

        if (null !== $block) {

            global $user;

            CmdiTemplateDb::save($profile, $label, $this->getComponentId(), $block, $user->uid);
            return true;
        }

        return false;
    }
}
