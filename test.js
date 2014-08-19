var allosphere = require("./build/Release/ivnj_allosphere")
var canvas = require("./build/Release/ivnj_canvas")

allosphere.initialize();

allosphere.onFrame(function() {
    console.log("onframe");
});

allosphere.onDraw(function() {
    console.log("ondraw");
});

setInterval(function() {
    allosphere.tick();
}, 10)
