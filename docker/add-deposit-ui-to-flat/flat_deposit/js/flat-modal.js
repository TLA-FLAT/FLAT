(function ($) {

  // because of ajax, attach gets called a lot
  var FlatDepositModalAttached = false;

  Drupal.behaviors.FlatDepositModal = {

    attach: function (context, settings) {

      if (true === FlatDepositModalAttached) {
        return;
      }

      FlatDepositModalAttached = true;

      var isModalOpen = false;

      $('body').on('shown.bs.modal', '[data-role="flat-modal"]', function (e) {
        isModalOpen = true;
      });

      $('body').on('hidden.bs.modal', '[data-role="flat-modal"]', function (e) {
        isModalOpen = false;
      });

      $('body').on('keydown', '[data-cmdi-enter="true"]', function(event) {

        var keycode = (event.keyCode ? event.keyCode : event.which);
        var element = $(this);

        if (keycode == '13') {

          // enter detected, submit label
          element.siblings('[data-role="open-flat-modal"]:first').trigger('click');
        }
      });

      $('body').on('click', '[data-role="open-flat-modal"]', function(event) {

        event.preventDefault();

        var modal = $('[data-role="flat-modal"]');
        var content = $('[data-role="flat-modal-content"]');
        var button  = $(this);
        var data = button.data('cmdi-data');
        var label = $('[data-role="cmdi-label-' + data.cmdi_id + '"]').val();

        // updating label form field
        $('input[name="' + data.label_name + '"]').val(label);

        if (label.trim() === '') {

          // empty label, show error message
          content.html(settings.flat_modal_blank);

          // show modal
          if (false === isModalOpen) {
            modal.modal('show');
          }

          // and exit event
          return;

        }

        // inject loader
        content.html(settings.flat_modal_loader);

        // show modal
        if (false === isModalOpen) {
          modal.modal('show');
        }

        // prepare post data
        var postData = JSON.stringify({

          cmdi_data: {

            cmdi_id: data.cmdi_id,
            profile: data.profile,
            label: label,
            component_id: data.component_id,
          },
        });

        jQuery.post(data.url, postData, function(result) {

          if (result && result.type === 'error') {

            // error
            content.html(result.modal);

            window.setTimeout(function() {
              modal.modal('hide');
            }, 2000);

            return;
          }

          if (result && result.type === 'exists') {

            // label exists in db, show confirmation
            content.html(result.modal);

            return;
          }

          if (result && result.type === 'new') {

            // label is not found in db, trigger save
            $('button[name="' + data.save_name + '"]').trigger('saving_' + data.cmdi_id);

            // and show success modal, and fade it out
            content.html(result.modal);

            window.setTimeout(function() {
              modal.modal('hide');
            }, 2000);

            return;
          }
        });
      });

      $('body').on('click', '[data-role="confirm-flat-modal"]', function(event) {

          event.preventDefault();

          var modal = $('[data-role="flat-modal"]');
          var button = $(this);
          var cmdi_id = button.data('cmdi-id');

          modal.modal('hide');
          $('button[name="save_cmdi_template_' + cmdi_id + '"]').trigger('saving_' + cmdi_id);
      });

      $('body').on('click', '[data-role="delete-flat-cmdi-template"]', function(event) {

          event.preventDefault();

          var modal = $('[data-role="flat-modal"]');
          var content = $('[data-role="flat-modal-content"]');
          var button = $(this);
          var id = button.data('cmdi-template-id');

          content.html(settings.flat_modal_confirm_delete);
          $('[data-role="confirm-delete-flat-modal"]').data('cmdi-template-id', id);

          if (false === isModalOpen) {
            modal.modal('show');
          }
      });

      $('body').on('click', '[data-role="confirm-delete-flat-modal"]', function(event) {

          event.preventDefault();

          var modal = $('[data-role="flat-modal"]');
          var content = $('[data-role="flat-modal-content"]');
          var button = $(this);
          var id = button.data('cmdi-template-id');
          var url = button.data('cmdi-template-delete-url');

          content.html(settings.flat_modal_loader);

          if (false === isModalOpen) {
            modal.modal('show');
          }

          var postData = JSON.stringify({
            cmdi_template: id
          });

          jQuery.post(url, postData, function(result) {

            if (result && result.type === 'error') {

              // error
              content.html(result.modal);

              window.setTimeout(function() {
                modal.modal('hide');
              }, 2000);

              return;
            }

            if (result && result.type === 'deleted') {

              var element = $('[data-role="available-template-' + id + '"]');
              var total   = element.parent().children('li').length;
              var block   = element.parent().parent(); // dropdown div of load action

              // remove available template
              element.remove();

              if ((total - 1) <= 0) {

                // no more templates available, remove block
                block.remove();
              }

              // show success modal, and fade it out
              content.html(result.modal);

              window.setTimeout(function() {
                modal.modal('hide');
              }, 2000);

              return;
            }
          });
      });

      $('body').on('click', '[data-role="load-flat-cmdi-template"]', function(event) {

        event.preventDefault();

        var el = $(this);
        var component_id = el.data('component-id');
        var cmdi_template_id = el.data('cmdi-template-id');

        $('[data-role="flat-cmdi-template-loadable-' + component_id + '"]').val(cmdi_template_id).change();
      });
    }
  };

  })(jQuery);
