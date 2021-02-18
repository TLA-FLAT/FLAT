<?php if (is_array($available_templates) && count($available_templates) > 0) : ?>
<div class="dropdown">
  <button class="btn btn-secondary dropdown-toggle" type="button" id="load-menu-<?php echo $component_id; ?>" data-toggle="dropdown">
    <i class="fas fa-search"></i>
  </button>
  <ul class="dropdown-menu pull-right" aria-labelledby="load-menu-<?php echo $component_id; ?>">
    <?php foreach ($available_templates as $available_template) : ?>
    <li>
      <a class="flat-cmdi-template-select" href="#"><?php echo $available_template; ?></a>
      <a class="flat-cmdi-template-delete" href="#"><i class="fas fa-trash-alt"></i></a>
    </li>
    <?php endforeach; ?>
  </ul>
</div>
<?php endif; ?>
<div class="dropdown">
  <button class="btn btn-secondary dropdown-toggle" type="button" id="save-menu-<?php echo $component_id; ?>" data-toggle="dropdown">
    <i class="fas fa-save"></i>
  </button>
  <div class="dropdown-menu pull-right" aria-labelledby="save-menu-<?php echo $component_id; ?>">
    <div class="flat-dropdown-save">
      <input type="text" class="form-control" placeholder="Label" />
      <button type="button" class="btn btn-success">Save</button>
    </div>
  </div>
</div>
