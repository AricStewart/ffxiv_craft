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

$fullHistory = true;

function priceRecipe(&$input, $xiv) 
{
    global $fullHistory;

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
            $partCost = priceRecipe($bit['bits'], $xiv);
            $bit['craftCost'] = $partCost['Market'];
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
    return ['Market' => $marketCost, 'Optimal' => $optimalCost];
}

function doRecipie($itemID, $dataset, $xiv) 
{
    $recipe = [];

    $recipe['Recipe'] = $dataset->getFullRecipe($itemID);
    if ($recipe['Recipe'] == null) {
        $recipe['error'] = "Failed to find recipe for $itemID\n";
        return $recipe;
    }
    $recipe['Cost'] = priceRecipe($recipe['Recipe'], $xiv);
    $history = $xiv->getHistory($itemID);
    $recipe['Recent'] = $xiv->mostRecent($history);
    $market = $xiv->getMarket($itemID);
    $recipe['Cheap'] = $xiv->currentCheapest($market);
    $recipe['Name'] = $dataset->item[$itemID]['Name'];
    $recipe['ID'] = $itemID;

    return $recipe;
}

function line($line, $tab) 
{
    foreach ($line as $bit) {
        for ($i = 0; $i < $tab; $i++) {
            print "\t";
        }
        print $bit['name']."(".$bit['id'].") (x".$bit['count'].") -> ";
        if ($bit['marketCost'] > 0) {
            print number_format($bit['marketCost']);
            if ($bit['marketHQ']) {
                print "(*)";
            }
            print " gil ";
        } else {
            print "UNAVALIABLE ";
        }
        if ($bit['craftCost']) {
            print "/ crafted ".number_format($bit['craftCost'])." gil";
        }
        print "\n";
        line($bit['bits'], $tab+1);
    }
}


function printRecipe($recipe, $profitOnly=false) 
{
    if (array_key_exists('Error', $recipe)) {
        print $recipe['Error'];
        return;
    }

    print "\n===>   ".$recipe['Name']."(".$recipe['ID'].")\n";
    $recent = $recipe['Recent'];
    $Price = $recent->PricePerUnit;
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
        if ($recipe['Cheap'] !== null) {
            print "current: ";
            if ($recent->IsHQ) {
                print "(*) ";
            }
            print number_format($recipe['Cheap']->PricePerUnit)." gil \n";
        } else {
            print "current: NONE\n";
        }

        print "Market Cost: ";
        if ($recipe['Cost']['Market'] > 0) {
            print number_format($recipe['Cost']['Market'])." gil \n";
        } else {
            print "UNAVLIABLE\n";
        }
        print "Optimal Cost: ".number_format($recipe['Cost']['Optimal'])." gil \n";
    }

    if ($recipe['Cheap']) {
        $Price = min(Price, $recipe['Cheap']->PricePerUnit);
    }

    if ($recipe['Cost']['Optimal'] < $Price) {
        print "** Possible Profit: ".number_format($Price - $recipe['Cost']['Optimal'])." gil\n";
    }

    if (!$profitOnly) {
        print "====================================\n";
        line($recipe['Recipe'], 0);
        print "====================================\n";
    }
}

function test($dataset, $xiv) 
{
    $o = doRecipie(23815, $dataset, $xiv);
    printRecipe($o);
}

