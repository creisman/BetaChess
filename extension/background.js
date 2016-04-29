var tabs = {};

chrome.browserAction.onClicked.addListener(function() {
	console.log('Action');
});

chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) {
  console.log("From tab " + sender.tab.url);
  tabs[sender.tab.id] = sendResponse;
  window.setTimeout(respond, 5000, sender.tab.id, makePoint(4, 4), makePoint(3, 3));
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