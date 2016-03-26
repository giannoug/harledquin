(function() {
  var app = angular.module('HarledquinApp', ['ngWebSocket']);

  app.controller('AppCtrl', ['$scope', '$websocket', 'Mapper', function($scope, $websocket, Mapper) {

    var socket = $websocket('ws://192.168.1.1:81/');

    $scope.leds = [
      '#f47d43',
      '#e66665',
      '#ffd602',
      '#cdb48c',
      '#a9b2b1',
      '#9eceb4',
      '#9295ca',
      '#accb38'
    ];

    $scope.payload = [];

    $scope.updateColors = function() {
      var jsonPayload = JSON.stringify(Mapper.model2payload($scope.leds));

      console.log('Sending payload: ' + jsonPayload);
      socket.send(jsonPayload);
    }

    socket.onMessage(function(message) {
      console.log('Got payload: ' + message.data);

      try {
        $scope.leds = Mapper.payload2model(JSON.parse(message.data));
      } catch (err) {
        console.error('Could not parse incoming JSON: ' + message.data);
      }

    });

  }]);
})();
