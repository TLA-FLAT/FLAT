(function ($) {

  Drupal.behaviors.FlatDepositModal = {
    attach: function (context, settings) {

      $('body').on('click', '[data-role="open-flat-modal"]', function(event) {

        console.log('testing');
        event.preventDefault();

        var modal = $('[data-role="flat-modal"]');
        var content = $('[data-role="flat-modal-content"]');
        var button  = $(this);
        var url = button.data('check-url');
        var data = button.data('cmdi-template-data');
        var label = $('input[name="' + data.label_field_name + '"]').val();

        if (label.trim() === '') {

          console.log(label);
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
        var postData = {

          cmdi_template: {

            cmdi_id: data.cmdi_id,
            profile: data.profile,
            component_id: data.component_id,
            label: label
          },
        };

        jQuery.post(url, JSON.stringify(postData), function(result) {

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
            button.trigger(result.cmdi_id);

            return;
          }
        });

        // var button  = $(this);
        // var modalId = button.data('modal-id');
        // var modal   = $('[data-role="flat-modal"][data-model-id="' + modalId + '"]');
        // var submit  = $('[data-role="confirm-flat-modal"][data-model-id="' + modalId + '"]');

        // button.prop('disabled', true);
        // modal.modal('show');

        // modal.on('hidden.bs.modal', function() {
        //   button.prop('disabled', false);
        // });

        // submit.on('click', function() {
        //   button.prop('disabled', false);
        //   button.trigger('your_custom_click');
        // });
      });

      $('body').on('click', '[data-role="confirm-flat-modal"]', function(event) {

          event.preventDefault();

          var modal = $('[data-role="flat-modal"]');
          var button = $(this);
          var cmdi_id = button.data('cmdi-id');

          modal.modal('hide');
          $('[data-role="open-flat-modal"][data-cmdi-id="' + cmdi_id + '"]').trigger(cmdi_id);
      });
    }
  };

  })(jQuery);
