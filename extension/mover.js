window.betachess = {};

betachess.SQUARE_SIZE = 64;

betachess.move = function(src, dest) {
    var board = $('.cg-board');

    var start = betachess.createMouseEvent('mousedown', betachess.findCoordinates(src.x, src.y));
    var middle = betachess.createMouseEvent('mousemove', betachess.findCoordinates(dest.x, dest.y));
    var end = betachess.createMouseEvent('mouseup', betachess.findCoordinates(dest.x, dest.y));

    board.get(0).dispatchEvent(start);
    window.setTimeout(function() {
        board.get(0).dispatchEvent(middle);
        window.setTimeout(function() {
            board.get(0).dispatchEvent(end);
        }, 100);
    }, 100);
}

betachess.findCoordinates = function(x, y) {
    var board = $('.cg-board');
    var isWhite = board.hasClass('orientation-white');
    y = 8 - y;
    if (!isWhite) {
        // Use 7 and 9 because the offsets below will push it over one square.
        x = 7 - x;
        y = 9 - y
    }
    // Places it into the bottom left corner of the piece.
    return {
      x: x * betachess.SQUARE_SIZE + board.offset().left + 2,
      y: y * betachess.SQUARE_SIZE + board.offset().top - 2
    };
}

betachess.createMouseEvent = function(type, coordinates) {
    return new MouseEvent(type, {
        bubbles: true,
        cancelable: true,
        clientX: coordinates.x,
        clientY: coordinates.y,
        button: 0,
        buttons: 1,
        view: window,
    });
}

chrome.runtime.sendMessage({greeting: 'hello'}, function(response) {
  betachess.move(response.src, response.dest);
});

