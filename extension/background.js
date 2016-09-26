var tabs = {};

chrome.browserAction.onClicked.addListener(function() {
	console.log('Action');
});

chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) {
  console.log("From tab " + sender.tab.url);
  tabs[sender.tab.id] = sendResponse;
  var ret = $.get('http://localhost:31415/getmove?request=' + JSON.stringify(request));
  console.log(ret);
  ret.done(function(data) {
    console.log(data);
    respond(sender.tab.id, data.from, data.to);
  });
  // TODO(creisman): Log failures so we know what's going wrong.
  //window.setTimeout(respond, 5000, sender.tab.id, makePoint(4, 4), makePoint(3, 3));
  return true;
});

function respond(tabId, src, dest) {
  tabs[tabId]({src: src, dest: dest});
  delete tabs[tabId];
}

function makePoint(x, y) {
  return {
    x: x,
    y: y
  };
}
