var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function lookupTrains() {
  var from = "EUS";
  var to = "TRI";
  var url = "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:typ=\"http://thalesgroup.com/RTTI/2013-11-28/Token/types\" xmlns:ldb=\"http://thalesgroup.com/RTTI/2014-02-20/ldb/\">"+
   "<soap:Header>"+
   "<typ:AccessToken>"+
         "<typ:TokenValue>b96cdc35-fa1e-4791-9358-7c2ba1d3bedc</typ:TokenValue>"+
      "</typ:AccessToken>"+
   "</soap:Header>"+
   "<soap:Body>"+
      "<ldb:GetDepartureBoardRequest>"+
         "<ldb:numRows>5</ldb:numRows>"+
         "<ldb:crs>"+from+"</ldb:crs>"+
         "<ldb:filterCrs>"+to+"</ldb:filterCrs>"+
         "<ldb:filterType>to</ldb:filterType>"+
         "<ldb:timeOffset>0</ldb:timeOffset>"+
         "<ldb:timeWindow>120</ldb:timeWindow>"+
      "</ldb:GetDepartureBoardRequest>"+
   "</soap:Body>"+
"</soap:Envelope>";
  console.log("SOAP req" + url);
  xhrRequest(url, 'GET', 
    function(responseText) {
            console.log("SOAP resp" + responseText);
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log("parsed:"+json);
      });
}

             
function locationSuccess(pos) {
  // Construct URL
  var long = pos.coords.longitude*1000000;     
  var lat = pos.coords.latitude*1000000;    
  var acc = pos.coords.accuracy;
  var url = "http://api.openweathermap.org/data/2.5/weather?appid=0ad42e271b4629fd1bf38dd7d09e7d99&lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude;
  console.log("Phone location: lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude + "&acc=" + acc);

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      console.log("Temperature is " + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log("Conditions are " + conditions);
      
      // Conditions
      var location = json.name;      
      console.log("Location is " + location);      
      
      // Conditions
      console.log("Long is " + long);  
      
      // Conditions
  
      console.log("Lat is " + lat);  
      // Conditions
      var time = json.dt;      
      //console.log("Time is " + time);       
// Assemble dictionary using our keys
var dictionary = {
  "KEY_TEMP": temperature,
  "KEY_COND": conditions,
  "KEY_LOC": location,
  "KEY_LAT": lat,
  "KEY_LONG": long,
  "KEY_TIME": time
};

      // Send to Pebble
Pebble.sendAppMessage(dictionary,
  function(e) {
    console.log("Weather info sent to Pebble successfully!");
  },
  function(e) {
    console.log("Error sending weather info to Pebble!");
  }
);
      
var url2="http://holden.duckdns.org:9080/e/rec_loc.php?lat="+lat/1000000+"&long="+long/1000000+ "&acc=" + acc;
var xhr2 = new XMLHttpRequest();
xhr2.open('GET', url2);
xhr2.send();

    }      
  );
}

function locationError(err) {
  console.warn('Location error (' + err.code + '): ' + err.message);
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {enableHighAccuracy:true, timeout: 15000, maximumAge: 6000000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    getWeather();
  }
);


// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage now received!");
    //lookupTrains();
    getWeather();
  }                     
);
