var graphics = require("node_graphics");
var shm = require("node_sharedmemory");

var buf = new shm.SharedMemory(0x1000, 1000 * 1000 * 4, false);

var s = new graphics.Surface2D(1000, 1000);

var context = new graphics.GraphicalContext2D(s);
var paint = context.paint();

var saved = false;
var t0 = new Date().getTime();
setInterval(function() {

    // Update the bitmap image.
    var dt = (new Date().getTime() - t0) / 1000;
    context.clear(255, 255, 255, 0.5);
    paint.setMode(graphics.PAINTMODE_STROKE);
    paint.setStrokeWidth(10);
    context.drawCircle(500, 500, 400, paint);
    paint.setColor(0, 255, 0, 1);
    paint.setTypeface("Arial", graphics.FONTSTYLE_NORMAL);
    paint.setTextSize(120);
    paint.setTextAlign(graphics.TEXTALIGN_CENTER);
    paint.setMode(graphics.PAINTMODE_FILL);
    context.drawText("Hello World", 500, 500 + 100 * Math.sin(dt * 5), paint);
    paint.setTextSize(60);
    context.drawText("t = " + dt, 500, 600 + 100 * Math.sin(dt * 5), paint);
    buf.writeLock();
    s.pixels().copy(buf.buffer());
    buf.writeUnlock();
}, 10);
