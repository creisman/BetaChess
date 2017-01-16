var baseUrl = 'http://192.168.0.13:5094/'

chrome.browserAction.onClicked.addListener(function() {
	console.log('Action');
});

// On getting a move (or generally message from the mover script.	
chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) {
	var url;
	if (request.startsWith('start-game')) {
		url = baseUrl + 'test?start=' + request;
	} else {
		url = baseUrl + 'test?move=' + request;
	}

	console.log('ask for move with: ' + url);
	
	var responseCb = function(data, status) {
		if (status != 'success') {
			console.log('status: ', status);
		}
		
    respond(sender.tab.id, sendResponse, data);
  }

  var ret = $.get(url, null, responseCb, 'text');

  // TODO(creisman): Log failures so we know what's going wrong.
  //window.setTimeout(respond, 5000, sender.tab.id, makePoint(4, 4), makePoint(3, 3));
  return true;
});

function respond(tabId, sendResponse, data) {
  sendResponse({data: data});
}

function makePoint(x, y) {
  return {
    x: x,
    y: y
  };
}
