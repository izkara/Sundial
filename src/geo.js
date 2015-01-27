function locationSuccess(pos) {
	var latitude = pos.coords.latitude;
	var longitude = pos.coords.longitude;

	console.log("LAT: " + latitude);
	console.log("LON: " + longitude);

	var dictionary = {
		"KEY_LATITUDE": latitude,
		"KEY_LONGITUDE": longitude
	};

	// Send to Pebble
	Pebble.sendAppMessage(dictionary,
		function(e) {
			console.log("LatLong sent");
		},
		function(e) {
			console.log("Error sending LatLong");
		}
	);
}

function locationError(err) {
	console.log("Error requesting location");
}

function getLatLong() {
	navigator.geolocation.getCurrentPosition(
		locationSuccess,
		locationError,
		{timeout: 15000, maximumAge: 60000}
	);
}

Pebble.addEventListener('ready',
	function(e) {
		console.log("PebbleKit JS is ready");
		getLatLong();
	}
);

Pebble.addEventListener('appmessage',
	function(e) {
		console.log("AppMessage received");
		getLatLong();
	}
);
