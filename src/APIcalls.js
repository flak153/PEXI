var googleKey = "AIzaSyANz6rDMkj0vbUxFQQYuz3HuMluKtIa-Kk";
var uberKey = "mPwREC7G-2Va31dK1MxF5d2jOrfmcJ2EiaVlIl8t";

function getGPSCoordinates(callback) {  //Callback gets lat and long
  var locationOptions = {
    enableHighAccuracy: true,
    maximumAge: 10000, 
    timeout: 10000
  };

  function locationSuccess(pos) {https://cloudpebble.net/ide/settings
    console.log('lat= ' + pos.coords.latitude + ' lon= ' + pos.coords.longitude);
    callback(pos.coords.latitude, pos.coords.longitude);
  }

  function locationError(err) {
    console.log('location error (' + err.code + '): ' + err.message);
  }

  // Request current position
  navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
}


// function requestUber(s_lat, s_long, e_lat, e_long) {
// 	function getProductId(callback) {
// 		var req = new XMLHttpRequest();
// 		var url = 'https://api.uber.com/v1/products?latitude=' 
// 			+ lat + '&longitude=' + long;
//  		console.log(url);
  
// 		req.open('GET', url);
// 		req.setRequestHeader("Authorization", "token " + uberKey);

// 		req.onLoad = function() {
// 			if (req.readyState === 4) {
// 				if (req.status === 200) {
// 					var res = JSON.parse(req.response);
// 					var products = res.products.filter(function(item){
// 						return item.displayName === "uberX";
// 					});
// 					var id = products[0].product_id;

// 					callback(s_lat, s_long, e_lat, e_long, id);
// 				}
// 			}
// 		};
// 	}

// 	//Calls Uber after getting product id for uberX
// 	getProductId(function(s_lat, s_long, e_lat, e_long, id) {
// 		var req = new XMLHttpRequest();
// 		var url = 'https://api.uber.com/v1/requests?product_id=' + id + '&start_latitude'
// 			+ start_latitude + '&start_longitude=' + start_longitude + '&end_latitude='
// 			+ end_latitude + '&end_longitude=' + end_longitude;
// 		console.log(url);

// 		req.open('POST', url);
// 		req.setRequestHeader("Authorization", "request " + uberKey);

// 		req.onLoad = function() {
// 			if (req.readyState === 4) {
// 				if (req.status === 200) {
// 					var res = JSON.parse(req.response);
// 					Pebble.sendAppMessage(res.status);
// 				}
// 			}
// 		};
// 	});
// }


function fetchPriceAndDistance(start_latitude, start_longitude, end_latitude, end_longitude) {
  var req = new XMLHttpRequest();

  var url = 'https://api.uber.com/v1/estimates/price?start_latitude=' 
    + start_latitude + '&start_longitude=' + start_longitude + '&end_latitude='
    + end_latitude + '&end_longitude=' + end_longitude;
  console.log(url);
  
  req.open('GET', url);
  req.setRequestHeader("Authorization", "token " + uberKey);

  req.onload = function() {
    if (req.readyState === 4) {
      if (req.status === 200) {
        console.log("fetchPriceAndDistance received data");
  
        var output = {};
        var res = JSON.parse(req.response);

        output.UBER = res.prices[0].estimate;
        output.CABBIE = (Math.floor(100 * (3.5 * (res.prices[0].distance) + 2.5)))/100 + "-" +  (Math.floor(100 * (3.7 * (res.prices[0].distance) + 3.5)))/100;

        console.log("Uber output: " + JSON.stringify(output));
        Pebble.sendAppMessage(output);
        return output;
      }
    }
  };
  req.send(null);
}


function manageMsg(msg) {
  var dest = {};
  var uberPrice;
  
  function fetchCoordinateFromAddress(address) {
    var req = new XMLHttpRequest();
    var url = 'https://maps.googleapis.com/maps/api/geocode/json?address=' + 
      address + '&key=' + googleKey;

    console.log(url);

    req.open('GET', url );
    req.onload = function() {
      if (req.readyState === 4) {
        if (req.status === 200) {
          console.log("fetchCoord received data");

          var res = JSON.parse(req.response);

          dest.address = address;
          dest.coord = res.results[0].geometry.location;

          console.log(JSON.stringify(res.results[0].geometry.location));
          uberPrice = fetchPriceAndDistance(40.730794, -73.997330, dest.coord.lat, dest.coord.lng );
        }
      }
    };
    req.send(null);
  }  
  
  function getProbableLocation(lat, long, search) {
    var req = new XMLHttpRequest();
    var url = 'https://maps.googleapis.com/maps/api/place/autocomplete/json?input=' + search 
      + '&location=' + lat + ',' + long + '&radius=1000&key=' + googleKey;
    console.log(url);

    req.open('GET', url);
    req.onload = function() {
      if (req.readyState === 4) {
        if (req.status === 200) {
          console.log("getProbableLocation received data");

          var output = {};
          var res = JSON.parse(req.response);

          console.log(JSON.stringify(res));

          output.address = res.predictions[0].description;
          fetchCoordinateFromAddress(output.address);
        }
      }
    };
    req.send(null);
  }
  
  console.log("Start with getProbableLocation");

  getProbableLocation(40.730794, -73.997330, msg);
}


Pebble.addEventListener('ready', function(e) {
  console.log('JavaScript app ready and running!' + e.ready);
  console.log(e.type);
});


Pebble.addEventListener('appmessage', function(e) {
    var msg = JSON.stringify(e.payload.TARGET);
    console.log('Received message: ' + msg);

    var output = manageMsg(msg);
  
});