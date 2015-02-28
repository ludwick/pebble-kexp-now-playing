/*
 * Copyright (C) 2015, Rachael Ludwick
 * Licensed under the terms of the MIT License.
 */
var kexp = (function() {
  
  // if entities come back in json which might not be the case with 
  // proper cache api
  var unescapeEntities = (function() {
     // This is not particularly unicode or internationalized.
    //
    var entityMap = {
      '&amp;': '&',
      '&lt;': '<',
      '&gt;' : '>',
      '&quot;': '"',
      '&apos;': "'"
    };
    
    var entityRegExpMap = {};
    for (var entity in entityMap) {
      if (entityMap.hasOwnProperty(entity)) {
        entityRegExpMap[entity] = new RegExp(entity, 'g');
      }
    }
  
    return function (s) {
      var final = s;
      for (var entity in entityMap) {
        if (entityMap.hasOwnProperty(entity)) {
          final = final.replace(entityRegExpMap[entity], entityMap[entity]);
        }
      }
      return final;
    };
  })();
  
  // This takes an object with these fields:
  // - __type
  // - Name
  // - other fields specific to type
  // But we only care about the name field.
  //
  function extractField(fieldName, obj) {
    if (obj.hasOwnProperty(fieldName)) {
      var field = obj[fieldName];
      if (field.hasOwnProperty('Name')) {
        return unescapeEntities(field.Name);
      }
    }
    return "--";
  }

  // API documented here: http://cache.kexp.org/cache/docs
  function extractPlayListData(jsonData) {
    var data = JSON.parse(jsonData);
    console.log("parsed data: " + JSON.stringify(data));
    if (data.hasOwnProperty('Plays') && data.Plays) {
      var plays = data.Plays;
      if (plays instanceof Array && plays.length > 0) {
      var firstPlay = plays[0];
      if (firstPlay.hasOwnProperty("Artist")) {
        return {
          isValid: true,
          artist: extractField('Artist', firstPlay),
          album: extractField('Release', firstPlay),
          track: extractField('Track', firstPlay)
        };
      }
    }
    return {isValid: false};
    }
  }

  var lastRequestDate;
  var updateCount = 0;
  
  function setLastUpdated(now) {
    lastRequestDate = now;
    updateCount++;
    console.log("KEXP update count=" + updateCount + ", last updated date=" + lastRequestDate.toISOString());
  }

  function getPlaylistUrl() {
    return "http://cache.kexp.org/cache/latestPlay?channel=1";
  }

  return {
    updateCurrentInfo: function (handler) {
      var now = new Date();
      var requestUrl = getPlaylistUrl();
      console.log("Calling KEXP via url: " + requestUrl);

      var req = new XMLHttpRequest();
      req.open('GET', requestUrl, true);
      req.onload = function() {
        if (req.readyState == 4 && req.status == 200) {
          var response = extractPlayListData(req.responseText);
          if (response.isValid) {
            var data = {
              'song_info': response.artist + "\n" + response.album + "\n" + response.track
            };
            
            handler(data);
            setLastUpdated(now);
          }
        } else {
          console.log('Failed to get results from KEXP.');
        }
      };
      req.send(null);
    }
  };
})();

function sendAppMsg(data) {
  console.log("sending back to pebble " + JSON.stringify(data));
  Pebble.sendAppMessage(data,
    function(e) { console.log("Track info sent to Pebble successfully!"); },
    function(e) { console.log("Error sending track info to Pebble!"); }
  );
}
  
// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");
    kexp.updateCurrentInfo(sendAppMsg);
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    kexp.updateCurrentInfo(sendAppMsg);
  }                     
);
