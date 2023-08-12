const express = require('express');
var crypto = require('crypto'); // use for create md5
var fs = require('fs'); // use read file OTA
const app = express();
const version = "2";

app.get('/getOTA', (req, res) => {
    console.log(req.headers);
    let h = req.headers;
    if (parseInt(h['x-esp8266-version']) == parseInt(version)) {
        res.sendStatus(304);
    } else {
        console.log(`Send firmware ${version} to client`);
        fs.readFile('./code_esp8266.ino.generic.bin', function(error, content) {
            res.writeHead(200, {
                'Content-Type': 'application/octet-stream',
                'Content-Length': Buffer.byteLength(content),
                'x-MD5': crypto.createHash('md5').update(content).digest("hex"),
                'Content-Disposition': 'attachment; filename=code_esp8266.ino.generic.bin'
            });
            res.end(content);
        });
    }
});

var server = app.listen(3000, function () {
    var host = server.address().address;
    var port = server.address().port;
    console.log("Example app listening at http://%s:%s", host, port);
});