<?php
require_once __DIR__."/../apiData.inc";
require_once __DIR__."/../ffxivData.inc";
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
<select id='server'>
<?php
    $dataset = new FfxivDataSet('..');
    $dataset->loadWorld();
    foreach($dataset->world as $entry) {
        if (strcasecmp($entry['Name'], $server) == 0) {
            echo "<option selected>";
        } else {
            echo "<option>";
        }
        echo $entry['Name']."</option>";
    }
?>
</select>
<input id='item' value='rakshasa dogi of healing'>
<input type='button' onclick='getData()' value='Get Data'>
<progress id='progress' style='display: none;'></progress>
</div>
<hr>
<div id="output">
</div>

<div style="text-align:center;">
<span>©︎ 2019 Aric Stewart
&nbsp;&nbsp;&nbsp;&nbsp;
<a href="https://github.com/AricStewart/ffxiv_craft"><img src="GitHub-Mark-32px.png"> Check the project out on GitHub</a></span>
</div>

</body>
