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
    require_module 'json';
.*/

function countItems($recipe, &$i)
{
    $i += (count($recipe) * 2);
    foreach ($recipe as $bit) {
        countItems($bit['bits'], $i);
    }
}

function preDownloadData($input, $xiv, $progress)
{
    foreach ($input as $bit) {
        $xiv->getMarket($bit['id'], true, $progress);
        $xiv->getHistory($bit['id'], true, $progress);
        if ($bit['bits']) {
            preDownloadData($bit['bits'], $xiv, $progress);
        }
    }
}

function priceRecipe(&$input, $xiv, $dataset, $progress, $recipe)
{
    $marketCost = 0;
    $optimalCost = 0;
    foreach ($input as &$bit) {
        if ($progress !== null) {
            $progress("partial", json_encode($recipe));
        }
        $market = $xiv->getMarket($bit['id']);
        $xiv->getHistory($bit['id']);

        $cheap = $xiv->currentCheapest($market);
        if ($cheap != null) {
            $bit['marketCost'] = $cheap['Item']->PricePerUnit * $bit['count'];
            $bit['marketHQ'] = $cheap['Item']->IsHQ;
        }

        if (count($dataset->findItemInShop($bit['id'])) > 0) {
            $bit['shopCost'] *= $bit['count'];
        } else {
            $bit['shopCost'] = 0;
        }

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
            $partCost = priceRecipe($bit['bits'], $xiv, $dataset, $progress,
                                    $recipe);
            $bit['craftCost'] = $partCost['Optimal'];
            if ($bitOptimal <= 0) {
                $bitOptimal = $bit['craftCost'];
            } elseif ($bit['craftCost'] > 0) {
                $bitOptimal = min($bit['craftCost'], $bitOptimal);
            }
        }
        if ($bit['shopCost'] > 0) {
            if ($bitOptimal <= 0) {
                $bitOptimal = $bit['shopCost'];
            } else {
                $bitOptimal = min($bit['shopCost'], $bitOptimal);
            }
        }
        if ($optimalCost >= 0 && $bitOptimal > 0) {
            $optimalCost += $bitOptimal;
        } else {
            $optimalCost = -1;
        }
        if ($progress !== null) {
            $progress("partial", json_encode($recipe));
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
        $Price['LQ'] = min(
            $Price['LQ'],
            $recipe['Cheap']['LQ']['Item']->PricePerUnit
        );
    }
    if ($recipe['Cheap']['HQ']) {
        $Price['HQ'] = min(
            $Price['HQ'],
            $recipe['Cheap']['HQ']['Item']->PricePerUnit
        );
    }
    if ($recipe['Week']['LQ']['Average']) {
        $Price['LQ'] = min($Price['LQ'], $recipe['Week']['LQ']['Average']);
    }
    if ($recipe['Week']['HQ']['Average']) {
        $Price['HQ'] = min($Price['HQ'], $recipe['Week']['HQ']['Average']);
    }

    $Price['LQ'] *= $recipe['Info']->Result->Amount;
    $Price['HQ'] *= $recipe['Info']->Result->Amount;

    if ($recipe['Cost']['Optimal']) {
        $Price['LQ%'] = ($Price['LQ']/$recipe['Cost']['Optimal']) - 1.0;
        $Price['HQ%'] = ($Price['HQ']/$recipe['Cost']['Optimal']) - 1.0;
    }

    $Price['LQ'] -= $recipe['Cost']['Optimal'];
    $Price['HQ'] -= $recipe['Cost']['Optimal'];

    $recipe['Profit'] = $Price;

    return $Price;
}

function filloutRecipe(&$recipe, $dataset, $sub=false)
{
    if (!$sub) {
        $ingred = &$recipe['Recipe'];
    } else {
        $ingred = &$recipe;
    }
    foreach ($ingred as &$bit) {
        $item = $dataset->getItem($bit['id']);
        $bit['name'] = $item->Name;
        if (count($dataset->findItemInShop($bit['id'])) > 0) {
            $bit['shopCost'] = $item->PriceMid;
        }
        if (count($bit['bits'])>0) {
            filloutRecipe($bit['bits'], $dataset, true);
        }
    }
}

function doRecipie($itemID, $dataset, $xiv, $progress=null,
                   $preference='Armorcraft')
{
    $output = array();
    $recipe = $dataset->getFullRecipe($itemID, 1, $output , false, $preference);
    if ($recipe == null) {
        $recipe['error'] = "Failed to find recipe for $itemID\n";
        return $recipe;
    }
    $item = $dataset->getItem($itemID);
    $recipe['Name'] = $item->Name;
    $recipe['ID'] = $itemID;
    if ($progress !== null) {
        $progress("partial", json_encode($recipe));
    }
    filloutRecipe($recipe, $dataset);
    if ($progress !== null) {
        $progress("partial", json_encode($recipe));
    }

    $count = 0;
    countItems($recipe['Recipe'], $count);
    if ($progress !== null) {
        $progress("start", $count + 4);
    }
    preDownloadData($recipe['Recipe'], $xiv, $progress);
    if ($progress !== null) {
        $progress("partial", json_encode($recipe));
    }

    /* Pre Download */
    $xiv->getHistory($itemID, true, $progress);
    $xiv->getMarket($itemID, true, $progress);
    $xiv->flushPool();
    if ($progress !== null) {
        $progress("partial", json_encode($recipe));
    }

    $recipe['Cost'] = priceRecipe($recipe['Recipe'], $xiv, $dataset, $progress, $recipe);
    if (count($dataset->findItemInShop($itemID)) > 0) {
        $recipe['Cost']['Shop'] = $item->PriceMid;
    } else {
        $recipe['Cost']['Shop'] = 0;
    }
    if ($progress !== null) {
        $progress("partial", json_encode($recipe));
    }
    if ($progress !== null) {
        $progress('progress');
    }
    $history = $xiv->getHistory($itemID);
    $recipe['Recent'] = ['LQ' => $xiv->mostRecent($history, false),
                         'HQ' => $xiv->mostRecent($history, true)];
    $recipe['Week'] = ['LQ' => $xiv->weekAverage($history, false),
                       'HQ' => $xiv->weekAverage($history, true)];
    if ($progress !== null) {
        $progress("partial", json_encode($recipe));
    }
    $market = $xiv->getMarket($itemID);
    $recipe['Cheap'] = ['LQ' => $xiv->currentCheapest($market, false),
                        'HQ' => $xiv->currentCheapest($market, true)];
    if ($progress !== null) {
        $progress("partial", json_encode($recipe));
    }
    if ($progress !== null) {
        $progress("partial", json_encode($recipe));
    }
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
                print "/ crafted (".$bit['craftedBy'].") ".
                      number_format($bit['craftCost'])." gil";
            } else {
                print "/ crafted UNAVALIABLE";
            }
        }
        if ($bit['shopCost'] > 0) {
            print "/ vendor ".number_format($bit['shopCost'])." gil";
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

    print "\n===>   ".$recipe['Name']."(".$recipe['ID'].")";
    if ($recipe['Info']->Result->Amount > 1) {
        print " x".$recipe['Info']->Result->Amount;
    }
    print "\n";
    if ($recipe['Info'] !== null) {
        print "   ".$recipe['Info']->CraftTypeName." - lvl ".
              $recipe['Info']->RecipeLevel->ClassJobLevel;
        if ($recipe['Info']->Book !== null) {
            print " from '".$recipe['Info']->Book->Name."'";
        }
        print "\n";
    }
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

        print "Weekly Average: ";
        if ($recipe['Week']['LQ']) {
            print number_format($recipe['Week']['LQ']['Average'])." gil";
            $sales = $recipe['Week']['LQ']['Count'];
            $plu = 's';
            if ($sales === 1) {
                $plu = '';
            }
            print " ( ".number_format($sales)." sale".$plu." )";
        } else {
            print " NONE";
        }
        if ($recipe['Week']['HQ']) {
            print " / (*) ";
            print number_format($recipe['Week']['HQ']['Average'])." gil";
            $sales = $recipe['Week']['HQ']['Count'];
            $plu = 's';
            if ($sales === 1) {
                $plu = '';
            }
            print " ( ".number_format($sales)." sale".$plu." )";
        }
        print "\n";

        print "current: ";
        if ($recipe['Cheap']['LQ']) {
            print number_format($recipe['Cheap']['LQ']['Item']->PricePerUnit)." gil";
            print " (".$recipe['Cheap']['LQ']['Count']." listings)";
        } else {
            print " NONE";
        }
        if ($recipe['Cheap']['HQ']) {
            print " / (*) ";
            print number_format($recipe['Cheap']['HQ']['Item']->PricePerUnit)." gil";
            print " (".$recipe['Cheap']['HQ']['Count']." listings)";
        }
        print "\n";

        print "------------------------------------\n";

        print "Shop Cost: ";
        if ($recipe['Cost']['Shop'] > 0) {
            print number_format($recipe['Cost']['Shop'])." gil \n";
        } else {
            print "None\n";
        }

        print "Craft at Market Cost: ";
        if ($recipe['Cost']['Market'] > 0) {
            print number_format($recipe['Cost']['Market'])." gil \n";
        } else {
            print "UNAVLIABLE\n";
        }
        print "Craft at Optimal Cost: ";
        if ($recipe['Cost']['Optimal'] > 0) {
            print number_format($recipe['Cost']['Optimal'])." gil \n";
        } else {
            print "UNAVLIABLE\n";
        }
    }
    if ($recipe['Profit']['LQ'] > 0 || $recipe['Profit']['HQ'] > 0) {
        print "------------------------------------\n";
    }
    if ($recipe['Profit']['LQ'] > 0) {
        print "** Possible Profit: ".number_format($recipe['Profit']['LQ'])." gil";
        print " (".round(($recipe['Profit']['LQ%']*100))."%)";
        print "\n";
    }

    if ($recipe['Profit']['HQ'] > 0) {
        print "** Possible Profit: ".number_format($recipe['Profit']['HQ']).
              " (*) gil";
        print " (".round(($recipe['Profit']['HQ%']*100))."%)";
        print "\n";
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

