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

/*  Archive using xivapi data */

$marketboard = new Ffxivmb($server, $ffxivmbGuid);
$xiv = new Xivapi($server, $xivapiKey, $marketboard);
$dataset = new FfxivDataSet();
$fullHistory = true;

function priceRecipe(&$input) 
{
    global $fullHistory, $xiv;
    $marketCost = 0;
    $optimalCost = 0;
    foreach ($input as &$bit) {
        $market = $xiv->getMarket($bit['id']);
        if ($fullHistory) {
            $xiv->getHistory($bit['id']);
        }
        $cheap = $xiv->currentCheapest($market);
        $bit['marketCost'] = $cheap->PricePerUnit * $bit['count'];
        $bit['marketHQ'] = $cheap->IsHQ;
        if ($bit['marketCost'] == 0) {
            $marketCost = -1;
            $bitOptimal = -1;
        } else {
            if ($marketCost != -1) {
                $marketCost += $bit['marketCost'];
            }
            $bitOptimal =  $bit['marketCost'];
        }
        if ($bit['bits']) {
            $partCost = priceRecipe($bit['bits']);
            $bit['craftCost'] = $partCost[0];
            if ($bitOptimal <= 0) {
                $bitOptimal = $bit['craftCost'];
            } else {
                $bitOptimal = min($bit['craftCost'], $bitOptimal);
            }
        }
        if ($optimalCost >= 0 && $bitOptimal > 0) {
            $optimalCost += $bitOptimal;
        } else {
            $optimalCost = -1;
        }
    }
    return [$marketCost, $optimalCost];
}

function doRecipie($itemID, $profitOnly=false) 
{
    global $xiv, $dataset;
    $output = $dataset->getFullRecipe($itemID);
    if ($output == null) {
        print ("Failed to find recipe for $itemID\n");
        exit();
    }
    print "Processing ".$dataset->item[$itemID]['Singular']." .";
    $cost = priceRecipe($output);
    $history = $xiv->getHistory($itemID);
    $recent = $xiv->mostRecent($history);
    $market = $xiv->getMarket($itemID);
    $cheap = $xiv->currentCheapest($market);
    print "\n";

    print "===>   ".$dataset->item[$itemID]['Singular']."(".$itemID.")\n";
    if (!$profitOnly) {
        if ($recent) {
            print "recent: ";
            if ($recent->IsHQ) {
                print "(*) ";
            }
            print number_format($recent->PricePerUnit)." gil \n";
        } else {
            print "recent: NONE\n";
        }
        if ($cheap) {
            print "current: ";
            if ($recent->IsHQ) {
                print "(*) ";
            }
            print number_format($cheap->PricePerUnit)." gil \n";
        } else {
            print "current: NONE\n";
        }

        print "Market Cost: ";
        if ($cost[0] > 0) {
            print number_format($cost[0])." gil \n";
        } else {
            print "UNAVLIABLE\n";
        }
        print "Optimal Cost: ".number_format($cost[1])." gil \n";
    }

    $price = $recent->PricePerUnit;
    if ($cheap) {
        $price = min($price, $cheap->PricePerUnit);
    }

    if ($cost[1] < $price) {
        print "** Possible Profit: ".number_format($price - $cost[1])." gil\n";
    }

    return $output;
}


if ($argv[1]) {
    if ($argv[1] == '-c') {
        array_map('unlink', glob("data/*.json"));
        $itemID = $argv[2];
    } elseif ($argv[1] == '-m') {
        $a = $dataset->getMastercraft($argv[2]);
        print count($a)."\n";
        if (count($a) > 0) {
            $xiv->verbose = false;
            foreach ($a as $i) {
                doRecipie($i, true);
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

$output = doRecipie($itemID);
$dataset->printRecipe($output);
