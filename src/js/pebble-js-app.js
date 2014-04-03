Pebble.addEventListener("appmessage", function(e) {
    console.log("got a message: " + e.payload);
}

function getImage() {
    var req = new XMLHttpRequest();
    req.open('GET', 'https://files.nomcopter.com/doppler.pbi', true);
    req.onload = function(e) {
        if (req.readyState == 4 && req.status == 200) {
            if(req.status == 200) {
                var response = req.responseText;
            } else { console.log("Error"); }
        }
    }
    req.send(null);
}
