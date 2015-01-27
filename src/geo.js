Pebble.addEventListener('ready',
	function(e) {
		console.log("PebbleKit JS is ready");
	}
);

Pebble.addEventListener('appmessage',
	function(e) {
		console.log("AppMessage received");
	}
);

