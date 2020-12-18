<div class="modal-header">
  <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
  <h4 class="modal-title"><?php echo $title; ?></h4>
</div>
<div class="modal-body">
  <p><?php echo $description; ?></p>
</div>
<div class="modal-footer">
  <button class="btn btn-success btn btn-primary" data-role="confirm-flat-modal" data-cmdi-id="<?php echo $cmdi_id; ?>" data-dismiss="modal" value="Confirm">Confirm</button>
  <button class="btn btn-warning btn" data-dismiss="modal" value="Cancel">Cancel</button>
</div>
