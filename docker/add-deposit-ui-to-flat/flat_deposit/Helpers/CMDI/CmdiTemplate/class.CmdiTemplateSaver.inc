<?php
module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/CmdiCreator/class.CmdiCreator');
autoload_cmdicreator();

/**
 * Saving CMDI fieldset block as a template for reuse
 *
 * @author  Ibrahim Abdullah <ibrahim.abdullah@mpi.nl>
 * @package FLAT CMDI Templates
 */
class CmdiTemplateSaver
{
    /**
     * @var array Reference to drupal form_state
     */
    private $form_state;

    /**
     * Storing form_state reference
     *
     * @param array $form_state
     */
    public function __construct(&$form_state)
    {
        $this->form_state = &$form_state;
    }

    /**
     * Checking whether save cmdi template ajax action was triggered
     *
     * @return bool
     */
    public function isSubmitted(): bool
    {
        return isset($this->form_state['triggering_element']['#ajax']['callback']) && $this->form_state['triggering_element']['#ajax']['callback'] === 'flat_cmdi_templates_ajax_save_callback';
    }

    /**
     * Get selected profile
     *
     * @return string|null
     */
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

    /**
     * Get uid
     *
     * @return string
     */
    public function getUid()
    {
        global $user;
        return $user->uid;
    }

    /**
     * Get tree of cmdi block to be saved
     *
     * @return array
     */
    public function getCmdiTree()
    {
        return $this->form_state['triggering_element']['#cmdi_tree'];
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

    /**
     * Generate json encoded string representation of template block to be saved to database
     *
     * @return string
     */
    public function generate()
    {
        // get component id
        $componentId = $this->getComponentId();

        // tree of the current block in the form
        $tree = $this->getCmdiTree();

        // get pressedButtons for tree
        $pressed = null;

        if (isset($this->form_state['pressedButtons']['template_container']['elements'])) {
            $pressed = drupal_array_get_nested_value($this->form_state['pressedButtons']['template_container']['elements'], $tree);
        }

        // getting form state for tree
        $state = drupal_array_get_nested_value($this->form_state['input']['template_container']['elements'], $tree);

        // removing all occurences of flat_cmdi_templates from form state for tree
        $state = drupal_array_unset_recursive($state, 'flat_cmdi_templates');

        // removing the last item from tree, which is the position in the current form
        // this is necessary so later this template can be loaded in any position
        array_pop($tree);

        return json_encode([

            'tree' => $tree,
            'state' => $state,
            'pressed' => $pressed,
        ]);
    }

    /**
     * Saving template block to db
     *
     * @return bool
     */
    public function save()
    {
        if (false === $this->isSubmitted()) {
            return false;
        }

        $label = $this->getLabel();

        if (empty($label)) {

            form_set_error('flat_cmdi_templates', 'Please provide a label for saving this block as a prest.');
            return false;
        }

        $profile = $this->getProfile();

        if (null === $profile) {
            return false;
        }

        $block = $this->generate();

        if (null !== $block) {

            CmdiTemplateDb::save($profile, $label, $this->getComponentId(), $block, $this->getUid());
            return true;
        }

        return false;
    }
}
function drupal_array_unset_recursive($haystack, $unsetKey) {

    if(is_array($haystack)) {

        unset($haystack[$unsetKey]);

        foreach ($haystack as $key => $val) {
            $haystack[$key] = drupal_array_unset_recursive($val, $unsetKey);
            unset($haystack[$unsetKey]);
        }
    }

    return $haystack;
}

function generate_pressed_buttons_state($input, &$form_state) {

    $pressed = [];
    foreach ($input as $key => $val) {
        if (is_array($val)) {
            $pressed[$key]['base'] = count($val);
        }
    }
}
