class BetaChess {
  constructor() {
    this.SQUARE_SIZE = 64;
    this.PROMO_MAP = {"Q": "queen", "N": "knight", "R": "rook", "B": "bishop", "K": "king"};
    
    this.eventQueue = [];
    this.updating = false;
    this.requestedSuggest = false;
    this.currentUpdatedMove = 0;
  }
  
  monitorGame() {
    if (updating) {
      return;
    }

    let moves = $('.moves move:not(.empty)');
    if (moves.length > currentUpdatedMove) {
      updating = true;
      update(moves.eq(currentUpdatedMove).text());
      currentUpdatedMove += 1;
      requestedSuggest = false;
      return
    }
  
    if (moves.length != currentUpdatedMove) {
      console.log("doesn't make sense to request suggest with mismatched move numbers");
    }

    let isOurAccount = /peace-call|betachess/.test($('.username .text').text())
    let isOurFirstTurn = (moves.length < 2) && isOurAccount;
    let hasNoResult = $('.moves .result').length == 0;
    let isActive = ($('.clock.running').length != 0 || isOurFirstTurn) && hasNoResult;
    
    let isWhite = $('.cg-board-wrap').hasClass('orientation-white');
    let isWhiteTurn = $('.clock_white').hasClass('running');

    let isOurTurn = (isActive && isWhite == isWhiteTurn) || (isOurFirstTurn && isWhite);
  
    // TODO this seems to request a 0-th move for white when we are black.
  
    //console.log('isActive: ' + isActive + ', isOurTurn: ' + isOurTurn +
    //                 ', moves: ' + moves.length + ' > ' + currentUpdatedMove +
    //                 ', requestedSuggest: ' + requestedSuggest);
    if (isOurTurn && !requestedSuggest) {
      // consider looking at the square.last-move and calculating with division of translate.
      console.log('requesting suggestion');
      requestedSuggest = true;
      getMove();
    }
  }
  
  move(src, dest, promo) {
    eventQueue.push(fireEvent.bind(this, createMouseEvent('mousedown', findCoordinates(src.x, src.y))));
    eventQueue.push(fireEvent.bind(this, createMouseEvent('mousemove', findCoordinates(dest.x, dest.y))));
    eventQueue.push(fireEvent.bind(this, createMouseEvent('mouseup', findCoordinates(dest.x, dest.y))));
    
    if (promo) {
      eventQueue.push(function() {
        $("#promotion_choice piece." + PROMO_MAP[promo]).click();
      })
    }
  }
  
  // This is indexed from (0,0) to (7,7)
  findCoordinates(x, y) {
    let boardWrapper = $('.cg-board-wrap');
    let isWhite = boardWrapper.hasClass('orientation-white');
    
    y = 7 - y;
    if (!isWhite) {
      x = 7 - x;
      y = 7 - y
    }
    
    let xCord = (x + 0.25 + 0.5 * Math.random()) * SQUARE_SIZE + boardWrapper.offset().left - window.pageXOffset;
    let yCord = (y + 0.25 + 0.5 * Math.random()) * SQUARE_SIZE + boardWrapper.offset().top  - window.pageYOffset;
    
    return {
      x: xCord,
      y: yCord
    };
  }
  
  getMove() {
    chrome.runtime.sendMessage("suggest", function(response) {
      if (response.data) {
        let d = response.data
        if (/^[a-h][1-8] - [a-h][1-8]/.test(response.data)) {
          let charCodeA = "a".charCodeAt(0);
          let charCode1 = "1".charCodeAt(0);
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
        move(response.src, response.dest, response.promo);
      }
    });
  }
  
  checkForStart() {
    // TODO update with all supported variants or something
    let isGame = $('.lichess_game.variant_antichess')
    let isActive = $('.clock.running').length != 0;

    // TODO re add isActive
    if (isGame.length == 0 /*|| isActive */) {
      console.log("Doesn't appear to be an active game. Check again later.");
      return false;
    }
  
    console.log("signalling start-game");
    chrome.runtime.sendMessage("start-game", function(response) {
      console.log(signal + " acked: " + response.data);

      setInterval(monitorGame.bind(this), 250);
    });
    return true;
  }
  
  update(move) {
    chrome.runtime.sendMessage(move, function(response) {
      console.log("response to update: " + response.data);
      // TODO make sure that state matches or something
      updating = false;
    });
  }
  
  processEvent() {
    if (eventQueue.length > 0) {
      eventQueue.shift()();
      setTimeout(processEvent.bind(this), Math.random() * 500);
    }
  }
  
  fireEvent(event) {
    $('.cg-board').get(0).dispatchEvent(event);
  }
  
  createMouseEvent(type, coords) {
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
}

var game = new BetaChess();
// When page is loaded, periodically check if it's a new game
$(window).load(function() {
  let checkStartFunction = function() {
    let started = game.checkForStart();
    if (!started) {
      console.log('Board not found');
      setTimeout(checkStartFunction, 1000);
    }
  }
  checkStartFunction();
});
