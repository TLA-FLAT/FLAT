(function($) {
Drupal.behaviors.flatBehavior = {
  attach: function (context, settings) {

    //code starts
    $("a.license").click(function() {
      return confirm('By downloading this resource you accept its license!');
    });
    //code ends

  }
};
})(jQuery);