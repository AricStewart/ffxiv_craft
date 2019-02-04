<?php
require_once __DIR__."/../apiData.inc";
?>
<html>
<head>
<title>Demonstrating Final Fantasy XIV Crafting Companion</title>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
<link rel="stylesheet" href="crafting.css">
<script src="crafting.js"></script>
</head>
<body>
<div style="text-align:center;">
<h2>Demonstrating Final Fantasy XIV Crafting Companion</h2>
<input id='server' value='<?php echo $server;?>' readonly>
<input id='item' value='rakshasa dogi of healing'>
<input type='button' onclick='getData()' value='Get Data'>
</div>
<hr>
<div id="output">
</div>
</body>
