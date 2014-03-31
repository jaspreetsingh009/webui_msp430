var fs      =  require('fs');
var server  =  require('./server');

function sendInterface(response) {
  console.log("Request handler 'webui' was called.");
  response.writeHead(200, {"Content-Type": "text/html"});
  var html = fs.readFileSync(__dirname + "/webpage/main.html")
  response.end(html);
}

exports.sendInterface = sendInterface;
