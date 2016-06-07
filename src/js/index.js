var TzClock = function(options) {
  this._apiKey = '';
  this._sessionID = '';
  this._timeZone = '';

  options = options || {};

  this._xhrWrapper = function(url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
      callback(xhr);
    };
    xhr.open(type, url);
    xhr.send();
  };

  this.sendToPebble = function(json) {
    console.log(this._sessionID + ' : ' + JSON.stringify(json));
    Pebble.sendAppMessage({
      'SESSION_ID': parseInt(this._sessionID),
      'OFFSET': parseInt(json.gmtOffset)
    });
  };

  this.fetchTimezone = function() {

    var url = 'http://api.timezonedb.com/?zone=' + this._timeZone +
                '&format=json&key=' + this._apiKey;

    console.log(this._sessionID + ' : ' + url);

    this._xhrWrapper(url, 'GET', function(req) {
      if(req.status == 200) {
        this.sendToPebble(JSON.parse(req.response));
      }
    }.bind(this));
  };

  this.appMessageHandler = function(dict) {
    if(dict.payload['API_KEY'] && dict.payload['TIMEZONE']) {
      this._sessionID = dict.payload['SESSION_ID'];
      this._apiKey = dict.payload['API_KEY'];
      this._timeZone = dict.payload['TIMEZONE'];
      this.fetchTimezone();
    }
  };
};

module.exports = TzClock;
