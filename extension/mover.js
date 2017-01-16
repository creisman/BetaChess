window.betachess = {};

betachess.SQUARE_SIZE = 64;

betachess.tailRecursion = function(delay, board, innerEvent, after) {
	return function () {
		board.dispatchEvent(innerEvent);
		window.setTimeout(function() { after();	}, delay);
	};
}

betachess.move = function(src, dest, promo) {
	var board = $('.cg-board').get(0);

	var start = betachess.createMouseEvent('mousedown', betachess.findCoordinates(src.x, src.y));
	var middle = betachess.createMouseEvent('mousemove', betachess.findCoordinates(dest.x, dest.y));
	var end = betachess.createMouseEvent('mouseup', betachess.findCoordinates(dest.x, dest.y));

	var promoFunction = function () {};
	if (promo) {
		promoFunction = function () {
			var name = {"Q": "queen", "N": "knight", "R": "rook", "B": "bishop", "K": "king"}[promo];
			console.log("name:" + name);
			$("#promotion_choice piece." + name).click();
		};    
	}
	
	betachess.tailRecursion(256, board, start,
			betachess.tailRecursion(75, board, middle,	
					betachess.tailRecursion(228, board, end, promoFunction)))();
					
}


// This is indexed from (0,0) to (7,7)
betachess.findCoordinates = function(x, y) {
  var board = $('.cg-board-wrap');
  var isWhite = board.hasClass('orientation-white');

  y = 7 - y;
  if (!isWhite) {
    // Use 7 and 9 because the offsets below will push it over one square.
    x = 7 - x;
    y = 7 - y
  }
  
  var xCord = (x + 0.25 + 0.5 * Math.random()) * betachess.SQUARE_SIZE + board.offset().left - window.pageXOffset;
  var yCord = (y + 0.25 + 0.5 * Math.random()) * betachess.SQUARE_SIZE + board.offset().top  - window.pageYOffset;

  //console.log("(" + x + ", " + y + ") => (" + xCord + ", " + yCord + ")");
  
  // Places it into the bottom left corner of the piece.
  return {
    x: xCord,
    y: yCord
  };
}

betachess.startGame = function() {
	// TODO update with all supported variants or something
	var isGame = $('.lichess_game.variant_antichess')
	var isActive = $('.clock.running').length != 0;

	// TODO re add isActive
	if (isGame.length == 0 /*|| isActive */) {
		console.log("Doesn't appear to be an active game");
		return;
	}
	
	var signal = "start-game";
	console.log("signalling " + signal);
	chrome.runtime.sendMessage(signal, function(response) {
		console.log(signal + " acked: " + response.data);

		window.setInterval(betachess.monitorGame, 250);
	});
}
// When page is loaded check if it's a new game
$( window ).load(betachess.startGame);

var updating = false;
var requestedSuggest = false;
var currentUpdatedMove = 0;
betachess.monitorGame = function() {
	if (updating) {
		return;
	}

	var moves = $('.moves move:not(.empty)');
	if (moves.length > currentUpdatedMove) {
		updating = true;
		betachess.update(moves.eq(currentUpdatedMove).text());
		currentUpdatedMove += 1;
		requestedSuggest = false;
		return
	}
	
	if (moves.length != currentUpdatedMove) {
		console.log("doesn't make sense to request suggest with mismatched move numbers");
	}

	var isOurAccount = /peace-call|betachess/.test($('.username .text').text())
	var isOurFirstTurn = (moves.length < 2) && isOurAccount;
	var hasNoResult = $('.moves .result').length == 0;
  var isActive = ($('.clock.running').length != 0 || isOurFirstTurn) && hasNoResult;
		
  var isWhite = $('.cg-board-wrap').hasClass('orientation-white');
  var isWhiteTurn = $('.clock_white').hasClass('running');

	var isOurTurn = (isActive && isWhite == isWhiteTurn) || (isOurFirstTurn && isWhite);
	
	// TODO this seems to request a 0-th move for white when we are black.
	
	//console.log('isActive: ' + isActive + ', isOurTurn: ' + isOurTurn +
	//								 ', moves: ' + moves.length + ' > ' + currentUpdatedMove +
	//								 ', requestedSuggest: ' + requestedSuggest);
	if (isOurTurn && !requestedSuggest) {
		// consider looking at the square.last-move and calculating with division of translate.
  	console.log('requesting suggestion');
		requestedSuggest = true;
  	betachess.getMove();
	}
}
	
betachess.getMove = function() {
	chrome.runtime.sendMessage("suggest", function(response) {
		if (response.data) {
			var d = response.data
			if (/^[a-h][1-8] - [a-h][1-8]/.test(response.data)) {
				var charCodeA = "a".charCodeAt(0);
				var charCode1 = "1".charCodeAt(0);
				response.src = {x: d.charCodeAt(0) - charCodeA, y: d.charCodeAt(1) - charCode1};
				response.dest = {x: d.charCodeAt(5) - charCodeA, y: d.charCodeAt(6) - charCode1};
				if (/\([NBRQK]\)$/.test(d)) {			
					response.promo = d.charCodeAt(9);
				}
			} else {
		  console.log("response was not parsed: " + response.data);
			}
		}
		if (response.src && response.dest) {
			console.log("Got move from server:", response.data, response.src, response.dest);
			betachess.move(response.src, response.dest, response.promo);
		}
	});
}

betachess.update = function(move) {
	chrome.runtime.sendMessage(move, function(response) {
		console.log("response to update: " + response.data);
		// TODO make sure that state matches or something
		updating = false;
	});
}

betachess.createMouseEvent = function(type, coords) {
  return new MouseEvent(type, {
      bubbles: true,
      cancelable: true,
      clientX: coords.x,
      clientY: coords.y,
      button: 0,
      buttons: 1,
      view: window,
  });
}
