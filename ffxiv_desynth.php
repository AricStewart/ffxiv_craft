#!/usr/bin/php
<?php

require_once __DIR__."/ffxivData.inc";
require_once __DIR__."/universalis.inc";
require __DIR__.'/vendor/autoload.php';

$dotenv = Dotenv\Dotenv::create(__DIR__);
$dotenv->load();

$dataset = new FfxivDataSet();
$xiv = new Universalis($_ENV['server']);

$typeID = 8;
$target = 100;
$limit = 1000;

$JOBS = [
  "Carpenter",
  "Blacksmith",
  "Armorer",
  "Goldsmith",
  "Leatherworker",
  "Weaver",
  "Alchemist",
  "Culinarian"
];

if (count($argv) > 1) {
    $typeID = $argv[1];
}

if (count($argv) > 2) {
    $target = intval($argv[2]);
}

if (count($argv) > 3) {
    $limit = intval($argv[3]);
}

if (!is_numeric($typeID)) {
    $result = array_search($typeID, $JOBS);
    if ($result === false) {
        print 'Could not find crafting type \''.$typeID."'\n";
        exit();
    }
    $typeID = 8 + $result;
}

function doit($target, $type)
{
    global $dataset, $limit;
    $result = array();
    $ds = $dataset->getItems();
    foreach ($ds as $i) {
        if (!$i->IsUntradable && $i->{'Level{Item}'} >= $target && $i->{'Level{Item}'} <= $target + $limit &&
            $i->{'ClassJob{Repair}'} == $type) {
            $result[] = $i;
        }
    }
    return $result;

}

function sortMe($a, $b)
{
    return $a->Cost - $b->Cost;

}


function noop($stage, $data = null)
{
}

$result = doit($target, $typeID);
foreach ($result as $idx => &$i) {
    $data = $xiv->getMarket($i->Index, false, 'noop');
    if (!$data) {
        unset($result[$idx]);
        continue;
    }
    $cheap = $xiv->currentCheapest($data);
    if (!$cheap) {
        unset($result[$idx]);
        continue;
    }
    $i->Cost = $cheap['Item']->pricePerUnit;
}
usort($result, "sortMe");
foreach ($result as $i) {
    print ($i->{'Level{Item}'}).' : '.$i->Name." ".$i->Cost." \n";
}
