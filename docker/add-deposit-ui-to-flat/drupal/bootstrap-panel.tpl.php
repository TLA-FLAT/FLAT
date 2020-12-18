<?php

/**
 * @file
 * This is a copy of default Bootstrap theme template called 'bootstrap-panel'
 * and adds support for adding panel actions, used in flat_deposit module.
 *
 * @todo Fill out list of available variables.
 *
 * @ingroup templates
 */
?>
<fieldset <?php print $attributes; ?>>
  <?php if ($title): ?>
    <?php if ($collapsible): ?>
    <legend class="panel-heading<?php print isset($panel_actions) && null !== $panel_actions ? ' panel-heading--actions' : ''; ?>">
      <a href="<?php print $target; ?>" class="panel-title fieldset-legend<?php print ($collapsed ? ' collapsed' : ''); ?>" data-toggle="collapse"><?php print $title; ?></a>
      <?php if (isset($panel_actions) && null !== $panel_actions) : ?>
        <div class="panel-actions">
          <?php print $panel_actions; ?>
        </div>
      <?php endif; ?>
    </legend>
    <?php else: ?>
    <legend class="panel-heading<?php print isset($panel_actions) && null !== $panel_actions ? ' panel-heading--actions' : ''; ?>">
      <span class="panel-title fieldset-legend"><?php print $title; ?></span>
      <?php if (isset($panel_actions) && null !== $panel_actions) : ?>
        <div class="panel-actions">
          <?php print $panel_actions; ?>
        </div>
      <?php endif; ?>
    </legend>
    <?php endif; ?>
  <?php endif; ?>
  <div<?php print $body_attributes; ?>>
    <?php if ($description): ?><div class="help-block"><?php print $description; ?></div><?php
    endif; ?>
    <?php print $content; ?>
  </div>
</fieldset>
