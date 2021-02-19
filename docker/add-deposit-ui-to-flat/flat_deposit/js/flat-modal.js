(function ($) {

  Drupal.behaviors.FlatDepositModal = {
    attach: function (context, settings) {

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
          modal.modal('show');

          // and exit event
          return;

        }

        // inject loader
        content.html(settings.flat_modal_loader);

        // show modal
        modal.modal('show');

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
            return;
          }

          if (result && result.type === 'exists') {

            // label exists in db, show confirmation
            content.html(result.modal);

            return;
          }

          if (result && result.type === 'new') {

            // label is not found in db, hide modal and trigger save
            $('[data-role="flat-modal"]').modal('hide');
            $('button[name="' + data.save_name + '"]').trigger('saving_' + data.cmdi_id);

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
