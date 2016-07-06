function fetchRate() {
  var req = new XMLHttpRequest();
  req.open('GET', 'https://api.coindesk.com/v1/bpi/currentprice.json');
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        console.log(req.responseText);
        var response = JSON.parse(req.responseText);
        var USD = response.bpi.USD.rate_float;
        var EURO = response.bpi.EUR.rate_float;
        console.log(EURO);
        console.log(USD);
        Pebble.sendAppMessage({
          'RATE_ICON_KEY': 0,
          'RATE_USD_KEY': USD + '$' ,
          'RATE_EURO_KEY' : EURO + '€',
        });
      } else {
        console.log('Error');
      }
    }
  };
  req.send(null);
}


Pebble.addEventListener('ready', function (e) {
  console.log('connect!' + e.ready);
  fetchRate();
  console.log(e.type);
});

Pebble.addEventListener('appmessage', function (e) {
  fetchRate();
  console.log(e.type);
  console.log(e.payload.USD);
  console.log(e.payload.EURO);
  console.log('message!');
});

Pebble.addEventListener('webviewclosed', function (e) {
  console.log('webview closed');
  console.log(e.type);
  console.log(e.response);
});
