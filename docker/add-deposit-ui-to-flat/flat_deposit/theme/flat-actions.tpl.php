<?php if (is_array($available_templates) && count($available_templates) > 0) : ?>
<div class="dropdown">
  <button class="btn btn-secondary dropdown-toggle" type="button" title="Load preset" id="load-menu-<?php echo $component_id; ?>" data-toggle="dropdown">
    <i class="fas fa-list-alt"></i>
  </button>
  <ul class="dropdown-menu pull-right" aria-labelledby="load-menu-<?php echo $component_id; ?>">
    <?php foreach ($available_templates as $cmdi_template_id => $available_template) : ?>
    <li data-role="available-template-<?php echo $cmdi_template_id; ?>">
      <a class="flat-cmdi-template-select" data-role="load-flat-cmdi-template" data-component-id="<?php echo $component_id; ?>" data-cmdi-template-id="<?php echo $cmdi_template_id; ?>" href="#"><?php echo $available_template; ?></a>
      <a class="flat-cmdi-template-delete" title="Delete preset" data-role="delete-flat-cmdi-template" data-cmdi-template-id="<?php echo $cmdi_template_id; ?>" href="#"><i class="fas fa-trash-alt"></i></a>
    </li>
    <?php endforeach; ?>
  </ul>
</div>
<?php endif; ?>
<div class="dropdown">
  <button class="btn btn-secondary dropdown-toggle" type="button" title="Save preset" id="save-menu-<?php echo $component_id; ?>" data-toggle="dropdown">
    <i class="fas fa-save"></i>
  </button>
  <div class="dropdown-menu pull-right" aria-labelledby="save-menu-<?php echo $component_id; ?>">
    <div class="flat-dropdown-save">
      <input type="text" class="form-control" placeholder="Label" data-role="cmdi-label-<?php echo $cmdi_id; ?>" data-cmdi-enter="true" />
      <button type="button" class="btn btn-success" data-role="open-flat-modal" data-cmdi-data='<?php echo $cmdi_data; ?>'>Save</button>
    </div>
  </div>
</div>
