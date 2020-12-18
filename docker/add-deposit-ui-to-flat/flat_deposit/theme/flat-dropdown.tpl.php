<div class="dropdown flat-dropdown">
  <button class="btn btn-secondary dropdown-toggle" type="button" id="dropdownMenuButton-<?php echo $component_id; ?>" data-toggle="dropdown">
    <i class="fas fa-search"></i>
  </button>
  <ul class="dropdown-menu">
    <?php foreach ($available_templates as $available_template) : ?>
    <li>
      <a class="flat-cmdi-template-select" href="#"><?php echo $available_template; ?></a>
      <a class="flat-cmdi-template-delete" href="#"><i class="fas fa-trash-alt"></i></a>
    </li>
    <?php endforeach; ?>
  </ul>
</div>
