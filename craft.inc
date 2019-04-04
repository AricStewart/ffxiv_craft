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

require_once __DIR__."/progress.inc";


function countItems($recipe, &$i)
{
    $i += count($recipe);
    foreach ($recipe as $bit) {
        countItems($bit['bits'], $i);
    }

}


function preDownloadData($input, $xiv, $progress)
{
    foreach ($input as $bit) {
        $xiv->getMarket($bit['id'], true, $progress);
        report_progress($progress);
        $xiv->getHistory($bit['id'], true, $progress);
        report_progress($progress);
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
        report_progress($progress, "info", $bit['name']);
        report_progress($progress, "partial", json_encode($recipe));
        $market = $xiv->getMarket($bit['id'], false, $progress);
        report_progress($progress, "info", $bit['name'].' market');
        report_progress($progress, "partial", json_encode($recipe));
        $xiv->getHistory($bit['id'], false, $progress);
        report_progress($progress, "info", $bit['name'].' history');
        report_progress($progress, "partial", json_encode($recipe));

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
            $bitOptimal = $bit['marketCost'];
        }
        if ($bit['bits']) {
            $partCost = priceRecipe(
                $bit['bits'],
                $xiv,
                $dataset,
                $progress,
                $recipe
            );
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
        report_progress($progress, "partial", json_encode($recipe));
    }
    return ['Market' => $marketCost, 'Optimal' => $optimalCost];

}


function calculateProfit(&$recipe)
{
    $Price = ['HQ' => null, 'LQ' => null];
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
        $Price['LQ%'] = ($Price['LQ'] / $recipe['Cost']['Optimal']) - 1.0;
        $Price['HQ%'] = ($Price['HQ'] / $recipe['Cost']['Optimal']) - 1.0;
    }

    $Price['LQ'] -= $recipe['Cost']['Optimal'];
    $Price['HQ'] -= $recipe['Cost']['Optimal'];

    $recipe['Profit'] = $Price;

    return $Price;

}


function filloutRecipe(&$recipe, $dataset, $sub = false)
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
        if (count($bit['bits']) > 0) {
            filloutRecipe($bit['bits'], $dataset, true);
        }
    }

}


function getRecipe($itemID, $dataset, $preference = 'Armorcraft', $progress = null, $quantity = -1)
{
    $output = null;

    if ($quantity > 0) {
        $multiply = $quantity;
    } else {
        $multiply = 1;
    }

    $recipe = $dataset->getFullRecipe($itemID, $multiply, $output, false, $preference);
    if ($recipe == null) {
        $recipe['error'] = "Failed to find recipe for $itemID\n";
        return $recipe;
    }
    $item = $dataset->getItem($itemID);
    $recipe['Name'] = $item->Name;
    $recipe['ID'] = $itemID;
    filloutRecipe($recipe, $dataset);
    report_progress($progress, "partial", json_encode($recipe));
    $size = 0;
    countItems($recipe['Recipe'], $size);
    $recipe['Size'] = ($size * 4) + 4;

    return $recipe;

}


function doRecipieFromRecipe($recipe, $dataset, $xiv, $progress = null)
{
    if ($recipe == null) {
        $recipe['error'] = "No Recipe";
        return $recipe;
    }

    preDownloadData($recipe['Recipe'], $xiv, $progress);
    report_progress($progress, "partial", json_encode($recipe));

    $itemID = $recipe['ID'];
    /* Pre Download */
    $xiv->getHistory($itemID, true, $progress);
    $xiv->getMarket($itemID, true, $progress);
    $xiv->flushPool($progress);
    report_progress($progress, "partial", json_encode($recipe));

    $item = $dataset->getItem($recipe['ID']);
    $recipe['Cost'] = priceRecipe($recipe['Recipe'], $xiv, $dataset, $progress, $recipe);
    if (count($dataset->findItemInShop($itemID)) > 0) {
        $recipe['Cost']['Shop'] = $item->PriceMid * $recipe['Count'];
    } else {
        $recipe['Cost']['Shop'] = 0;
    }
    report_progress($progress, "partial", json_encode($recipe));
    $history = $xiv->getHistory($itemID, false, $progress);
    $recipe['Recent'] = ['LQ' => $xiv->mostRecent($history, false),
    'HQ' => $xiv->mostRecent($history, true)];
    $recipe['Week'] = ['LQ' => $xiv->weekAverage($history, false),
    'HQ' => $xiv->weekAverage($history, true)];
    report_progress($progress, "partial", json_encode($recipe));
    $market = $xiv->getMarket($itemID, false, $progress);
    $recipe['Cheap'] = ['LQ' => $xiv->currentCheapest($market, false),
    'HQ' => $xiv->currentCheapest($market, true)];

    $c = -1;
    if ($recipe['Cheap']['LQ'] !== null) {
        $c = $recipe['Cheap']['LQ']['Item']->PricePerUnit;
    }
    if ($recipe['Cheap']['HQ'] !== null) {
        if ($c > -1) {
            $c = min($c, $recipe['Cheap']['HQ']['Item']->PricePerUnit);
        } else {
            $c = $recipe['Cheap']['HQ']['Item']->PricePerUnit;
        }
    }
    if ($c != -1) {
        $c *= $recipe['Count'];
    }
    $recipe['Cost']['Marketboard'] = $c;

    report_progress($progress, "partial", json_encode($recipe));
    calculateProfit($recipe);

    return $recipe;

}


function doRecipie(
    $itemID,
    $dataset,
    $xiv,
    $progress = null,
    $preference = 'Armorcraft'
) {
    $recipe = getRecipe($itemID, $dataset, $preference, $progress);
    if ($recipe == null) {
        $recipe['error'] = "Failed to find recipe for $itemID\n";
        return $recipe;
    }
    report_progress($progress, "start", $recipe['Size']);
    report_progress($progress, "partial", json_encode($recipe));
    return doRecipieFromRecipe($recipe, $dataset, $xiv, $progress);

}


function getCompanyRecipe($itemID, $dataset, $progress = null)
{
    $set = $dataset->loadCompanyCrafting($itemID);
    $data = $set[$itemID];
    $recipe = array();
    if ($data == null) {
        $recipe['error'] = "Failed to find data for $itemID\n";
        return $recipe;
    }
    $recipe['Info'] = $data;
    $item = $dataset->getItem($itemID);
    $recipe['Name'] = $item->Name;
    $recipe['ID'] = $itemID;
    $recipe['Size'] = 0;
    $recipe['Parts'] = array();

    foreach ($data->CompanyCraftPart as $part) {
        $partData = array();
        $partData['Info'] = $part;
        $partData['Process'] = array();
        foreach ($part->CompanyCraftProcess as $process) {
            $processData = array();
            $processData['Info'] = $process;
            $processData['Set'] = array();
            foreach ($process->Set as $item) {
                $itemData = array();
                $itemData['Info'] = $item;
                $itemData['Recipe'] = getRecipe($item->Item->Item, $dataset, 'Armorcraft', $progress, ($item->Quantity * $item->Required));
                if ($itemData['Recipe'] !== null) {
                    $recipe['Size'] += $itemData['Recipe']['Size'];
                }
                $processData['Set'][] = $itemData;
            }
            $partData['Process'][] = $processData;
        }
        $recipe['Parts'][] = $partData;
    }

    return $recipe;

}


function doCompanyRecipe($target, $dataset, $xiv, $progress = null)
{
    if ($target == null) {
        $target['error'] = "No Recipe";
        return $target;
    }

    $market = $xiv->getMarket($target['ID'], false, $progress);
    $target['MarketNumber'] = count($market);
    $cheap = $xiv->currentCheapest($market);
    if ($cheap != null) {
        $target['marketCost'] = $cheap['Item']->PricePerUnit;
    } else {
        $target['marketCost'] = -1;
    }
    $history = $xiv->getHistory($target['ID'], false, $progress);
    $target['Recent'] = $xiv->mostRecent($history, false);
    $target['Week'] = $xiv->weekAverage($history, false);

    $target['Cost'] = array('Optimal' => 0, 'Market' => 0, 'Marketboard' => 0);
    foreach ($target['Parts'] as &$part) {
        $part['Cost'] = array('Optimal' => 0, 'Market' => 0, 'Marketboard' => 0);
        foreach ($part['Process'] as &$process) {
            $process['Cost'] = array('Optimal' => 0, 'Market' => 0, 'Marketboard' => 0);
            foreach ($process['Set'] as &$item) {
                $recipe = doRecipieFromRecipe($item['Recipe'], $dataset, $xiv, $progress);
                $process['Cost']['Optimal'] += $recipe['Cost']['Optimal'];
                $process['Cost']['Market'] += $recipe['Cost']['Market'];
                if ($process['Cost']['Marketboard'] >= 0) {
                    if ($recipe['Cost']['Marketboard'] > 0) {
                        $process['Cost']['Marketboard'] += $recipe['Cost']['Marketboard'];
                    } else {
                        $process['Cost']['Marketboard'] = -1;
                    }
                }
                $item['Recipe'] = $recipe;
            }
            $part['Cost']['Optimal'] += $process['Cost']['Optimal'];
            $part['Cost']['Market'] += $process['Cost']['Market'];
            if ($part['Cost']['Marketboard'] >= 0) {
                if ($process['Cost']['Marketboard'] > 0) {
                    $part['Cost']['Marketboard'] += $process['Cost']['Marketboard'];
                } else {
                    $part['Cost']['Marketboard'] = -1;
                }
            }
        }
        $target['Cost']['Optimal'] += $part['Cost']['Optimal'];
        $target['Cost']['Market'] += $part['Cost']['Market'];
        if ($target['Cost']['Marketboard'] >= 0) {
            if ($part['Cost']['Marketboard'] > 0) {
                $target['Cost']['Marketboard'] += $part['Cost']['Marketboard'];
            } else {
                $target['Cost']['Marketboard'] = -1;
            }
        }
    }

    return $target;

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
                print "/ crafted (".$bit['craftedBy'].") ".number_format($bit['craftCost'])." gil";
            } else {
                print "/ crafted UNAVALIABLE";
            }
        }
        if ($bit['shopCost'] > 0) {
            print "/ vendor ".number_format($bit['shopCost'])." gil";
        }
        print "\n";
        line($bit['bits'], $tab + 1);
    }

}


function printRecipe($recipe, $profitOnly = false)
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
        print "   ".$recipe['Info']->CraftTypeName." - lvl ".$recipe['Info']->RecipeLevel->ClassJobLevel;
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
        print " (".round(($recipe['Profit']['LQ%'] * 100))."%)";
        print "\n";
    }

    if ($recipe['Profit']['HQ'] > 0) {
        print "** Possible Profit: ".number_format($recipe['Profit']['HQ'])." (*) gil";
        print " (".round(($recipe['Profit']['HQ%'] * 100))."%)";
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


function printCompanyRecipe($target)
{
    foreach ($target['Parts'] as $pindex => $part) {
        foreach ($part['Process'] as $index => $process) {
            foreach ($process['Set'] as $item) {
                printRecipe($item['Recipe'], true);
            }
            print "<><><><><><><><><><><><> Stage ".($index + 1)." <><><><><><><><><><><><>\n";
            print "Cost Craft Optimal: ".number_format($process['Cost']['Optimal'])." gil \n";
            print "Cost Craft Market: ".number_format($process['Cost']['Market'])." gil \n";
            print "Cost Marketboard: ".number_format($process['Cost']['Marketboard'])." gil \n";
            print "<><><><><><><><><><><><><><><><><><><><><><><><><><><><><>\n";
        }
        print "[][][][][][][][][][][][][] Part ".($pindex + 1)." [][][][][][][][][][][][][]\n";
        print "Part Cost Craft Optimal: ".number_format($part['Cost']['Optimal'])." gil \n";
        print "Part Cost Craft Market: ".number_format($part['Cost']['Market'])." gil \n";
        print "Part Cost Marketboard: ".number_format($part['Cost']['Marketboard'])." gil \n";
        print "[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]\n";
    }
    print "===>   ".$target['Name']."(".$target['ID'].")\n";
    print "Total Cost Craft Optimal: ".number_format($target['Cost']['Optimal'])." gil \n";
    print "Total Cost Craft Market: ".number_format($target['Cost']['Market'])." gil \n";
    print "Total Cost MarketBoard: ".number_format($target['Cost']['Marketboard'])." gil \n";
    print "Current Market: ".number_format($target['marketCost'])." gil";
    print " (".$target['MarketNumber']." sellers)\n";
    print "Recent: ".number_format($target['Recent']->PricePerUnit)." gil on ";
    print date('l jS \of F Y h:i:s A', $target['Recent']->PurchaseDate)." \n";
    print "Week: ".number_format($target['Week']->PricePerUnit)." gil\n";

}
