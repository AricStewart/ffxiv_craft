#!/usr/bin/php
<?php

require_once __DIR__."/ffxivData.inc";
require_once __DIR__."/universalis.inc";
require __DIR__.'/vendor/autoload.php';

$dotenv = Dotenv\Dotenv::create(__DIR__);
$dotenv->load();

$xiv = new Universalis($_ENV['server']);
$dataset = new FfxivDataSet();

$itemID = null;
$inputFile = null;
$depth = 0;
$acnt = 2;
$output = [];
$max = 9999;

if (count($argv) > 1) {
    if ($argv[1] == '-f') {
        $inputFile = $argv[2];
        $acnt++;
        if (count($argv) > 2) {
            $max  = intval($argv[3]);
            $acnt++;
        }
    } else {
        $itemID = $argv[1];
        $max = 1;
    }
} else {
    $itemID = 23815;
    $max = 1;
}

if (count($argv) > $acnt) {
    $depth = intval($argv[$acnt]);
}

if (!is_null($itemID) && !is_numeric($itemID)) {
    $result = $dataset->getItem($itemID);
    if ($result === null) {
        print 'Could not find item \''.$itemID."'\n";
        exit();
    }
    $itemID = $result->Index;
}


function add_bits($result, $item, $d, $max_depth): array
{
    global $dataset, $xiv;
    if (($max_depth < 0 || $d < $max_depth) && !empty($item['bits'])) {
        foreach ($item['bits'] as $bit) {
            $result = add_bits($result, $bit, $d + 1, $max_depth);
        }
    } else {
        $data = $dataset->getItem($item['id']);
        if (!array_key_exists($data->Name, $result)) {
            $market = $xiv->getMarket($item['id'], false, null);
            $history = $xiv->getHistory($item['id']);
            $cheap = $xiv->currentCheapest($market[$item['id']]);
            $average = $xiv->weekAverage($history[$item['id']], false);
            $deal = $average['Average'] - $cheap['Item']->pricePerUnit;
            $result[$data->Name] = array('count' => $item['count'], 'cheap' => $cheap['Item']->pricePerUnit, 'average' => $average['Average'], 'deal' => $deal);
        } else {
            $result[$data->Name]['count'] += $item['count'];
        }
    }
    return $result;

}


function doit($itemID, $result, $max_depth = -1)
{
    global $dataset, $output;
    $recipe = $dataset->getFullRecipe($itemID, 1, $data, false, 'Armorcraft');
    foreach ($recipe['Recipe'] as $bit) {
        $result = add_bits($result, $bit, 0, $max_depth);
    }
    return $result;

}


$result = array();
$cnt = 0;

if (!is_null($itemID)) {
    $result = doit($itemID, $result, $depth);
} else {
    $file_to_read = fopen($inputFile, 'r');
    while (($data = fgetcsv($file_to_read, null, ',')) !== false) {
        if ($data[1] == 'ItemID') {
            continue;
        }
        $result = doit($data[1], $result, $depth);
        $cnt ++;
        if ($cnt >= $max) break;
    }
}

function cmp($a, $b)
{
    if ($a['deal'] == $b['deal']) {
        return 0;
    }
    return ($a['deal'] < $b['deal']) ? -1 : 1;
}

uasort($result, 'cmp');
print('item,'.implode(',',array_keys($result[array_key_first($result)])).PHP_EOL);
foreach ($result as $key => $out) {
    print($key.','.implode(',',$out).PHP_EOL);
}
