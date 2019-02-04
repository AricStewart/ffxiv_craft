#!/usr/bin/php
<?php
/**
 * Crafting data engine
 *
 * PHP version 5
 *
 * @category  Final_Fantasy_XIV
 * @package   FfxivDataSet
 * @author    Aric Stewart <aricstewart@google.com>
 * @copyright 2019 Aric Stewart
 * @license   Apache License, Version 2.0
 * @link      <none>
 **/

/*.
    require_module 'standard';
.*/

require_once __DIR__."/ffxivData.inc";
require_once __DIR__."/ffxivmb.inc";
require_once __DIR__."/xivapi.inc";
require_once __DIR__."/apiData.inc";
require_once __DIR__."/craft.inc";

/*  Archive using xivapi data */

$marketboard = new Ffxivmb($server, $ffxivmbGuid);
$xiv = new Xivapi($server, $xivapiKey, $marketboard);
$dataset = new FfxivDataSet();
$fullHistory = true;

if (count($argv) > 1) {
    if ($argv[1] == '-c') {
        array_map('unlink', glob("data/*.json"));
        $itemID = $argv[2];
    } elseif ($argv[1] == '-m') {
        $a = $dataset->getMastercraft($argv[2]);
        print count($a)."\n";
        if (count($a) > 0) {
            $xiv->verbose = false;
            foreach ($a as $i) {
                $output = doRecipie($i, $dataset, $xiv);
                printRecipe($output, true);
            }
        }
        exit();
    } else {
        $itemID = $argv[1];
    }
} else {
    $itemID = 23815;
}

if (!is_numeric($itemID)) {
    $result = $dataset->getItemByName($itemID);
    if ($result === null) {
        print 'Could not find item \''.$itemID."'\n";
        exit();
    }
    $itemID = $result;
}

$output = doRecipie($itemID, $dataset, $xiv);
printRecipe($output);
