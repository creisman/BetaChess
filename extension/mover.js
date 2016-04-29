window.betachess = {};

chrome.runtime.sendMessage({greeting: 'hello'}, function(response) {
  betachess.move(response.src, response.dest);
});

betachess.move = function(src, dest) {
    var board = $('.cg-board');

    var start = betachess.createMouseEvent('mousedown', betachess.findSquare(src.x, src.y));
    var middle = betachess.createMouseEvent('mousemove', betachess.findSquare(dest.x, dest.y));
    var end = betachess.createMouseEvent('mouseup', betachess.findSquare(dest.x, dest.y));

    board.get(0).dispatchEvent(start);
    window.setTimeout(function() {
        board.get(0).dispatchEvent(middle);
        window.setTimeout(function() {
            board.get(0).dispatchEvent(end);
        }, 100);
    }, 100);
}

betachess.findSquare = function(x, y) {
    var board = $('.cg-board');
    var isWhite = board.hasClass('orientation-white');
    var index = (8 - x) + y * 8;
    if (isWhite) {
        index = 64 - index + 1;
    }
    var square = board.find(' > square:nth-child(' + index + ')');
    console.log(square.get(0));
    return square;
}

betachess.createMouseEvent = function(type, square) {
    return new MouseEvent(type, {
        bubbles: true,
        cancelable: true,
        clientX: $(square).offset().left + 2,
        clientY: $(square).offset().top + 2,
        button: 0,
        buttons: 1,
        view: window,
    });
}