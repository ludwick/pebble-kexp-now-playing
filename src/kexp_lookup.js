

// This util converts and formats a date:
//    2015-01-31T10%3A00%3A00.000
// in the same timezone as the KEXP server (PST).
//
var dateUtil = (function() {
  function zeroPad(num) {
    return num < 10 ? ("0" + num) : num;
  }
  
  function convertDateToPST(date) {
    var serverUtcOffset = -480;
		var ticks = date.getTime();
		var localUtcOffset = date.getTimezoneOffset();
		var difference = serverUtcOffset + localUtcOffset;
		return new Date(ticks + (difference * 60 * 1000));
	}

  return {
    formatDate: function(date) {
      var sd = convertDateToPST(date);
      var year = sd.getFullYear();
      var month = zeroPad(sd.getMonth() + 1);
      var day = zeroPad(sd.getDate());
      var hour = zeroPad(sd.getHours());
      var min = zeroPad(sd.getMinutes());
      var sec = zeroPad(sd.getSeconds());
      return encodeURIComponent(
                year + "-" +
                month + "-" +
                day + "T" +
                hour + ":" +
                min + ":" +
                sec + ".000"
      );
    }
  };
}());

var kexp = (function() {
  
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
  }());

  // This is an egregious hack. The HTML returned will contain
  // a data attribute on the main div that has the JSON version
  // of what is in the rest of the returned HTML. On subsequent
  // calls (controlled by the 'since' query param), the content
  // will be empty if the track hasn't changed so it's possible
  // the data won't be found in the html.
  // 
  // The HTML and the data attribute seem to handle unicode
  // differently such that JSON.parse of the data ends up
  // with malformed UTF-8 data.
  //
  function extractPlayListData(htmlText) {
    var dataAttrRegExp = new RegExp("data-playlistitem='(.+)'>");
    var results = dataAttrRegExp.exec(htmlText);
    if (results && typeof results !== "undefined" && results.hasOwnProperty("1")) {
      var jsonData = results[1];
      if (jsonData !== "undefined") {
        var parsedData = JSON.parse(jsonData);
        console.log("got data: " + JSON.stringify(parsedData));
        return {
          isValid: parsedData.hasOwnProperty("PlayUri"),
          getField: function(fieldName) {
            if (typeof parsedData[fieldName] !== "undefined") {
              return unescapeEntities(parsedData[fieldName]);
            }
            return "--";
          }
        };
      }
    }
    return {isValid: false};
  }
  
 
  var lastRequestDate;
  var updateCount = 0;
  
  function haveUpdated() {
    return updateCount > 0;
  }
  
  function getLastUpdated(now) {
    return haveUpdated() ? lastRequestDate : now;
  }
  
  function setLastUpdated(now) {
    lastRequestDate = now;
    updateCount++;
    console.log("KEXP update count=" + updateCount + ", last updated date=" + lastRequestDate.toISOString());
  }

  function getPlaylistUrl(now) {
    var url = "http://kexp.org/playlist/playlistupdates?";
    url += "channel=1";
    url += "&";
    url += "start=";
    url += dateUtil.formatDate(now);
    if (haveUpdated()) {
      url += "&";
      url += "since=";
      url += dateUtil.formatDate(getLastUpdated(now));
    }
  
    return url;
  }

  return {
    updateCurrentInfo: function (handler) {
      var now = new Date();
      var requestUrl = getPlaylistUrl(now);
      console.log("Calling KEXP via url: " + requestUrl);

      var req = new XMLHttpRequest();
      req.open('GET', requestUrl, true);
      req.onload = function() {
        if (req.readyState == 4 && req.status == 200) {
          var response = extractPlayListData(req.responseText);
          if (response.isValid) {
            var track = response.getField("TrackName");
            var artist = response.getField("ArtistName");
            var album = response.getField("ReleaseName");
            var data = { 'song': track, 'artist': artist, 'album': album};
            
            console.log("handing off msg=" + JSON.stringify(data));
            handler(data);
            setLastUpdated(now);
          }
        } else {
          console.log('Failed to get results from KEXP');
        }
      };
      req.send(null);
    }
  };
}());

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
