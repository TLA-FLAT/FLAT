<?php
/**
 * @file
 * This is the template file for the metadata display for an object.
 *
 * Available variables:
 * - $islandora_object: The Islandora object rendered in this template file
 * - $metadata: XSLT output
 *
 * @see template_preprocess_cmdi_display()
 * @see theme_cmdi_display()
 */
?>

<div class="islandora-cmdi-object islandora">
  <div class="islandora-cmdi-content-wrapper clearfix">
    <?php if (isset($islandora_content)): ?>
      <div class="islandora-cmdi-content">
        <?php print $islandora_content; ?>
      </div>
    <?php endif; ?>
  </div>
  <div class="islandora-cmdi-metadata">
    <?php print $metadata; ?>
  </div>
</div>