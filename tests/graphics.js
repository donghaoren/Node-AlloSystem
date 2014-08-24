var graphics = require("node_graphics");

var render = function(context) {
    var paint = context.paint();

    context.clear(255, 255, 255, 1);
    context.drawLine(0, 0, 1000, 1000, paint);
    paint.setTypeface("Arial", graphics.FONTSTYLE_NORMAL);
    paint.setTextSize(120);
    paint.setTextAlign(graphics.TEXTALIGN_CENTER);
    paint.setMode(graphics.PAINTMODE_FILL);
    context.drawText("Hello World", 500, 500, paint);
    paint.setTextSize(60);
    context.drawText("This is Node-Graphics (Skia Backend)", 500, 700, paint);
};

var graphics = require("node_graphics");

var s = new graphics.Surface2D(1000, 1000);
var context = new graphics.GraphicalContext2D(s);
render(context);
s.save("test.png");

var s = new graphics.Surface2D(1000, 1000, graphics.SURFACETYPE_PDF);
var context = new graphics.GraphicalContext2D(s);
render(context);
s.save("test.pdf");
