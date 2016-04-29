console.log('loaded');
window.chessex = {};

chrome.runtime.sendMessage({greeting: 'hello'}, function(response) {
  chessex.move(response.src, response.dest);
});

chessex.move = function(src, dest) {
    var board = $('.cg-board');

    var start = chessex.createMouseEvent('mousedown', chessex.findSquare(src.x, src.y));
    var middle = chessex.createMouseEvent('mousemove', chessex.findSquare(dest.x, dest.y));
    var end = chessex.createMouseEvent('mouseup', chessex.findSquare(dest.x, dest.y));

    board.get(0).dispatchEvent(start);
    window.setTimeout(function() {
        board.get(0).dispatchEvent(middle);
        window.setTimeout(function() {
            board.get(0).dispatchEvent(end);
        }, 100);
    }, 100);
}

chessex.findSquare = function(x, y) {
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

chessex.createMouseEvent = function(type, square) {
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