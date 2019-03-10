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

if ($ffxivmbGuid && !empty($ffxivmbGuid)) {
    $marketboard = new Ffxivmb($server, $ffxivmbGuid);
} else {
    $marketboard = null;
}
$xiv = new Xivapi($server, $xivapiKey, $marketboard);
$dataset = new FfxivDataSet();
$fullHistory = true;


function _sortByProfit($a, $b)
{
    $p = max($a['Profit']['HQ%'], $a['Profit']['LQ%']);
    $p2 = max($b['Profit']['HQ%'], $b['Profit']['LQ%']);
    return $p - $p2;

}


$i = 0;


function tick($stage, $data = null)
{
    global $i;
    if ($stage != 'progress') {
        return;
    }
    $i++;
    if ($i % 10 == 0) {
        print '|';
    } elseif ($i % 5 == 0) {
        print '-';
    } else {
        print '.';
    }

}


$i = 1;
if (count($argv) > 1) {
    if ($argv[1] == '-c') {
        array_map('unlink', glob("data/*.json"));
        $itemID = $argv[2];
        $i = 3;
    } elseif ($argv[1] == '-m') {
        $a = $dataset->getMastercraft($argv[2]);
        print count($a)."\n";
        if (count($a) > 0) {
            $output = [];
            foreach ($a as $key => $i) {
                print " $i (".($key + 1)."|".count($a).") ";
                $output[] = doRecipie($i, $dataset, $xiv, 'tick');
            }
            usort($output, '_sortByProfit');
            foreach ($output as $recipe) {
                printRecipe($recipe, true);
            }
        }
        exit();
    } elseif ($argv[1] == '-t') {
        $crafter = $argv[2];
        $tier = intval($argv[3]);
        $set = $dataset->getRecipeSet($crafter, ($tier - 1) * 5, $tier * 5);
        foreach ($set as $i => $r) {
            print " ".$r." (".($i + 1)."|".count($set).") ";
            $output[] = doRecipie($r, $dataset, $xiv, 'tick', $crafter);
        }
        usort($output, '_sortByProfit');
        foreach ($output as $recipe) {
            printRecipe($recipe, true);
        }
        exit();
    } elseif ($argv[1] == '-x') {
        $company = $dataset->loadCompanyCrafting();
        $recipe = getCompanyRecipe($argv[2], $dataset, 'tick');
        $output = doCompanyRecipe($recipe, $dataset, $xiv, 'tick');
        printCompanyRecipe($output);
        exit();
    } else {
        $itemID = $argv[1];
        $i = 2;
    }
} else {
    $itemID = 23815;
}

$crafter = 'Armorcraft';
if (count($argv) > $i) {
    $crafter = $argv[$i];
}

if (!is_numeric($itemID)) {
    $result = $dataset->getItem($itemID);
    if ($result === null) {
        print 'Could not find item \''.$itemID."'\n";
        exit();
    }
    $itemID = $result->Index;
}

$data = getRecipe($itemID, $dataset, $crafter, 'tick');
$output = doRecipieFromRecipe($data, $dataset, $xiv, 'tick', $crafter);
printRecipe($output);
