#ifndef HTML_H
#define HTML_H

#include <Arduino.h>

//html page for wifi setup
const char wifi_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32S3 WiFi Setup</title>

<style>
body{
    font-family: Arial;
    background:#f2f2f2;
    text-align:center;
}

.box{
    width:320px;
    margin:auto;
    margin-top:100px;
    background:white;
    padding:20px;
    border-radius:10px;
    box-shadow:0 0 10px #ccc;
}

input{
    width:100%;
    padding:10px;
    margin-top:10px;
}

button{
    width:100%;
    padding:10px;
    margin-top:15px;
    background:#2196F3;
    color:white;
    border:none;
}

</style>
</head>

<body>

<div class="box">

<h2>ESP32S3 WiFi Setup</h2>

<form action="/wifi/save" method="POST">

<input type="text" name="ssid" placeholder="WiFi SSID">

<input type="password" name="pass" placeholder="Password">

<button type="submit">Connect</button>

</form>

</div>

</body>
</html>

)=====";


//html page for controller setup
const char config_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>Energy Meter Settings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">

    <style>
        body { font-family: Arial; background:#f4f4f4; }
        .container {
            max-width: 520px;
            margin: auto;
            background: #fff;
            padding: 15px;
            border-radius: 10px;
        }
        h2 { text-align: center; }
        h3 {
            background: #007BFF;
            color: white;
            padding: 8px;
            border-radius: 5px;
        }
        label { font-weight: bold; }
        input, select {
            width: 100%;
            padding: 8px;
            margin: 5px 0 15px;
        }
        .phase-group {
            border: 1px solid #ccc;
            padding: 10px;
            margin-bottom: 10px;
            border-radius: 5px;
        }
        button {
            width: 100%;
            padding: 10px;
            background: green;
            color: white;
            border: none;
        }
    </style>

    <script>
        function updateMode() {
            var mode = document.getElementById("mode").value;

            if (mode == "1") {
                document.getElementById("singlePhase").style.display = "block";
                document.getElementById("threePhase").style.display = "none";
            } else {
                document.getElementById("singlePhase").style.display = "none";
                document.getElementById("threePhase").style.display = "block";
            }
        }
    </script>
</head>

<body onload="updateMode()">

<div class="container">
    <h2>Energy Meter Settings</h2>

    <form action="/save" method="POST">

        <!-- 🔹 Device Config -->
        <h3>Device Configuration</h3>

        <label>Device Mode</label>
        <select name="mode" id="mode" onchange="updateMode()">
            <option value="1" %mode1%>1 Phase</option>
            <option value="3" %mode3%>3 Phase</option>
        </select>

        <!-- Single Phase -->
        <div id="singlePhase" class="phase-group">
            <h4>Single Phase</h4>

            <label>Voltage Min</label>
            <input type="number" name="vmin" value="%vmin%">

            <label>Voltage Max</label>
            <input type="number" name="vmax" value="%vmax%">

            <label>Current Max</label>
            <input type="number" name="imax" value="%imax%">
        </div>

        <!-- Three Phase -->
        <div id="threePhase">

            <div class="phase-group">
                <h4>R Phase</h4>
                <input type="number" name="vR_min" value="%vR_min%" placeholder="Voltage Min">
                <input type="number" name="vR_max" value="%vR_max%" placeholder="Voltage Max">
                <input type="number" name="iR_max" value="%iR_max%" placeholder="Current Max">
            </div>

            <div class="phase-group">
                <h4>Y Phase</h4>
                <input type="number" name="vY_min" value="%vY_min%" placeholder="Voltage Min">
                <input type="number" name="vY_max" value="%vY_max%" placeholder="Voltage Max">
                <input type="number" name="iY_max" value="%iY_max%" placeholder="Current Max">
            </div>

            <div class="phase-group">
                <h4>B Phase</h4>
                <input type="number" name="vB_min" value="%vB_min%" placeholder="Voltage Min">
                <input type="number" name="vB_max" value="%vB_max%" placeholder="Voltage Max">
                <input type="number" name="iB_max" value="%iB_max%" placeholder="Current Max">
            </div>

        </div>

        <!-- 🔹 Billing Config -->
        <h3>Billing Configuration</h3>

        <label>Name</label>
        <input type="text" name="name" value="%name%">

        <label>Consumer Number</label>
        <input type="text" name="consumer" value="%consumer%">

        <label>Tariff Rate</label>
        <input type="number" step="0.01" name="tariff" value="%tariff%">

        <label>Energy Charge</label>
        <input type="number" step="0.01" name="energy" value="%energy%">

        <label>Fixed Charge</label>
        <input type="number" step="0.01" name="fixed" value="%fixed%">

        <label>Tax (%)</label>
        <input type="number" step="0.01" name="tax" value="%tax%">

        <button type="submit">Save Settings</button>

    </form>
</div>

</body>
</html>
)=====";

#endif // HTML_H