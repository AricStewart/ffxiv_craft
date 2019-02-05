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
            } elseif ($bit['craftCost'] > 0) {
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

function calculateProfit(&$recipe) 
{
    $Price = ['HQ'=> null, 'LQ' => null];
    if ($recipe['Recent']['LQ']) {
        $Price['LQ'] = $recipe['Recent']['LQ']->PricePerUnit;
    }
    if ($recipe['Recent']['HQ']) {
        $Price['HQ'] = $recipe['Recent']['HQ']->PricePerUnit;
    }
    if ($recipe['Cheap']['LQ']) {
        $Price['LQ'] = min($Price['LQ'], $recipe['Cheap']['LQ']->PricePerUnit);
    }
    if ($recipe['Cheap']['HQ']) {
        $Price['HQ'] = min($Price['HQ'], $recipe['Cheap']['HQ']->PricePerUnit);
    }

    $Price['LQ'] -= $recipe['Cost']['Optimal'];
    $Price['HQ'] -= $recipe['Cost']['Optimal'];

    $recipe['Profit'] = $Price;

    return $Price;
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
    $recipe['Recent'] = ['LQ' => $xiv->mostRecent($history, false),
                         'HQ' => $xiv->mostRecent($history, true)];
    $market = $xiv->getMarket($itemID);
    $recipe['Cheap'] = ['LQ' => $xiv->currentCheapest($market, false),
                        'HQ' => $xiv->currentCheapest($market, true)];
    $recipe['Name'] = $dataset->item[$itemID]['Name'];
    $recipe['ID'] = $itemID;

    calculateProfit($recipe);

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
            if ($bit['craftCost'] > 0) {
                print "/ crafted ".number_format($bit['craftCost'])." gil";
            } else {
                print "/ crafted UNAVALIABLE";
            }
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
    if (!$profitOnly) {
        print "recent: ";
        if ($recipe['Recent']['LQ']) {
            print number_format($recipe['Recent']['LQ']->PricePerUnit)." gil";
        } else {
            print " NONE";
        }
        if ($recipe['Recent']['HQ']) {
            print " / (*) ";
            print number_format($recipe['Recent']['HQ']->PricePerUnit)." gil";
        }
        print "\n";

        print "current: ";
        if ($recipe['Cheap']['LQ']) {
            print number_format($recipe['Cheap']['LQ']->PricePerUnit)." gil";
        } else {
            print " NONE";
        }
        if ($recipe['Cheap']['HQ']) {
            print " / (*) ";
            print number_format($recipe['Cheap']['HQ']->PricePerUnit)." gil";
        }
        print "\n";

        print "Market Cost: ";
        if ($recipe['Cost']['Market'] > 0) {
            print number_format($recipe['Cost']['Market'])." gil \n";
        } else {
            print "UNAVLIABLE\n";
        }
        print "Optimal Cost: ".number_format($recipe['Cost']['Optimal'])." gil \n";
    }
    if ($recipe['Profit']['LQ'] > 0) {
        print "** Possible Profit: ".number_format($recipe['Profit']['LQ'])." gil\n";
    }

    if ($recipe['Profit']['HQ'] > 0) {
        print "** Possible Profit: ".number_format($recipe['Profit']['HQ'])." (*) gil\n";
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

