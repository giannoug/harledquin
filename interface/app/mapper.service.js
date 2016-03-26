(function() {

  var service = function() {
    return {
      model2payload: function(model) {
        var payload = [];

        angular.forEach(model, function(item) {
          var value = parseInt(item.replace('#', ''), 16);
          payload.push(value);
        });

        return payload;
      },
      payload2model: function(payload) {
        var model = [];

        angular.forEach(payload, function(item) {
          var hex = item.toString(16);
          var padded = ("000000" + hex).slice(-6);

          model.push('#' + padded);
        });

        return model;
      }
    }
  }

  angular.module('HarledquinApp')
    .factory('Mapper', service);

})();
