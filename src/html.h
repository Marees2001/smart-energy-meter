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

#endif // HTML_H