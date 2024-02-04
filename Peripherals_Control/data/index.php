<!DOCTYPE html>
<html>

<head>
    <title>HieuP25 Test</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" type="text/css" href="style.css">
</head>

<body>
    <h1>T.i.k.T.o.k| @HieuP25 - Test</h1>
    <div class="container">
        <div class="circle temperature">
            <span id="temp">-- °C</span>
        </div>
        <div class="circle humidity">
            <span id="humi">-- %</span>
        </div>
    </div>

    <div class="color-picker">
        <label for="color">LED Color</label>
        <input type="color" id="color">
        <button id="apply-btn">Apply</button>
    </div>
    <script src="script.js"></script>
</body>

</html>