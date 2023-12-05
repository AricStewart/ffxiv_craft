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

$SHARD_DEFAULT = 20;
$CRYSTAL_DEFAULT = 20;
$CLUSTER_DEFAULT = 20;


function countItems($recipe, &$i)
{
    $i += count($recipe);
    foreach ($recipe as $bit) {
        countItems($bit['bits'], $i);
    }

}


function buildIdArray($bits, &$ids)
{
    foreach ($bits as $bit) {
        if (!in_array($bit['id'], $ids)) {
            $ids[] = $bit['id'];
        }
        if ($bit['bits']) {
            buildIdArray($bit['bits'], $ids);
        }
    }
    return $ids;

}


function preDownloadData($itemId, $input, $xiv, $progress)
{
    $ids = [$itemId];
    buildIdArray($input, $ids);
    $xiv->getMarket($ids, true, $progress);
    report_progress($progress);
    $xiv->flushPool($progress);

}


function priceRecipe(&$input, $xiv, $dataset, $progress, $recipe, $priceList)
{
    global $SHARD_DEFAULT, $CRYSTAL_DEFAULT, $CLUSTER_DEFAULT;

    $marketCost = 0;
    $listCost = 0;
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

        $cheap = null;
        $listcheap = null;
        $found = false;
        if ($priceList && array_key_exists($bit['id'], $priceList)) {
            $listcheap = $priceList[$bit['id']];
            $found = true;
        } else {
            $cheap = $xiv->currentCheapest($market[$bit['id']]);
        }
        if ($cheap != null) {
            $bit['marketCost'] = $cheap['Item']->pricePerUnit * $bit['count'];
            $bit['marketHQ'] = $cheap['Item']->hq;
            $bit['lastUpdate'] = time() - $cheap['Item']->lastReviewTime;
            $found = true;
        }
        if ($listcheap !== null) {
            $bit['listCost'] = $listcheap * $bit['count'];
            $found = true;
        }

        if (count($dataset->findItemInShop($bit['id'])) > 0) {
            $bit['shopCost'] *= $bit['count'];
            $found = true;
        } else {
            $bit['shopCost'] = 0;
        }

        if (!$found) {
            $bit['marketCost'] = 999999999;
        }

        $bitOptimal = -1;
        if ($bit['marketCost'] == 0) {
            $marketCost = -1;
        } else {
            $marketCost += $bit['marketCost'];
            $bitOptimal = $bit['marketCost'];
        }
        if ($bit['listCost'] == 0) {
            $listCost = -1;
        } else {
            $listCost += $bit['listCost'];
            if ($bitOptimal == -1 || $listCost < $bitOptimal) {
                $bitOptimal = $bit['listCost'];
            }
        }
        if ($bit['bits']) {
            $partCost = priceRecipe(
                $bit['bits'],
                $xiv,
                $dataset,
                $progress,
                $recipe,
                $priceList
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
    return ['Market' => $marketCost, 'List' => $listCost, 'Optimal' => $optimalCost];

}


function calculateProfit(&$recipe)
{
    $Price = ['HQ' => null, 'LQ' => null];
    if ($recipe['Recent']['LQ']) {
        $Price['LQ'] = $recipe['Recent']['LQ']->pricePerUnit;
    }
    if ($recipe['Recent']['HQ']) {
        $Price['HQ'] = $recipe['Recent']['HQ']->pricePerUnit;
    }
    if ($recipe['Cheap']['LQ']) {
        $Price['LQ'] = min(
            $Price['LQ'],
            $recipe['Cheap']['LQ']['Item']->pricePerUnit
        );
    }
    if ($recipe['Cheap']['HQ']) {
        $Price['HQ'] = min(
            $Price['HQ'],
            $recipe['Cheap']['HQ']['Item']->pricePerUnit
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

    if ($recipe['Cost']['Market']) {
        $Price['Market_LQ'] = $Price['LQ'] - $recipe['Cost']['Market'];
        $Price['Market_HQ'] = $Price['HQ'] - $recipe['Cost']['Market'];
        $Price['Market_LQ%'] = ($Price['LQ'] / $recipe['Cost']['Market']) - 1.0;
        $Price['Market_HQ%'] = ($Price['HQ'] / $recipe['Cost']['Market']) - 1.0;
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
            $bit['shopCost'] = $item->PriceLow;
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


function doRecipieFromRecipe($recipe, $dataset, $xiv, $progress = null, $priceList = null)
{
    if ($recipe == null) {
        $recipe['error'] = "No Recipe";
        return $recipe;
    }

    $itemID = $recipe['ID'];
    /* Pre Download */
    preDownloadData($itemID, $recipe['Recipe'], $xiv, $progress);
    report_progress($progress, "partial", json_encode($recipe));

    $item = $dataset->getItem($recipe['ID']);
    $recipe['Cost'] = priceRecipe($recipe['Recipe'], $xiv, $dataset, $progress, $recipe, $priceList);
    if (count($dataset->findItemInShop($itemID)) > 0) {
        $recipe['Cost']['Shop'] = $item->PriceMid * $recipe['Count'];
    } else {
        $recipe['Cost']['Shop'] = 0;
    }
    report_progress($progress, "partial", json_encode($recipe));
    $history = $xiv->getHistory($itemID, false, $progress);
    $recipe['Recent'] = ['LQ' => $xiv->mostRecent($history[$itemID], false),
    'HQ' => $xiv->mostRecent($history[$itemID], true)];
    $recipe['Week'] = ['LQ' => $xiv->weekAverage($history[$itemID], false),
    'HQ' => $xiv->weekAverage($history[$itemID], true)];
    report_progress($progress, "partial", json_encode($recipe));
    $market = $xiv->getMarket($itemID, false, $progress);
    $recipe['Cheap'] = ['LQ' => $xiv->currentCheapest($market[$itemID], false),
    'HQ' => $xiv->currentCheapest($market[$itemID], true)];

    $c = -1;
    if ($recipe['Cheap']['LQ'] !== null) {
        $c = $recipe['Cheap']['LQ']['Item']->pricePerUnit;
    }
    if ($recipe['Cheap']['HQ'] !== null) {
        if ($c > -1) {
            $c = min($c, $recipe['Cheap']['HQ']['Item']->pricePerUnit);
        } else {
            $c = $recipe['Cheap']['HQ']['Item']->pricePerUnit;
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
    $preference = 'Armorcraft',
    $priceList = null
) {
    $recipe = getRecipe($itemID, $dataset, $preference, $progress);
    if ($recipe == null) {
        $recipe['error'] = "Failed to find recipe for $itemID\n";
        return $recipe;
    }
    report_progress($progress, "start", $recipe['Size']);
    report_progress($progress, "partial", json_encode($recipe));
    return doRecipieFromRecipe($recipe, $dataset, $xiv, $progress, $priceList);

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


function doCompanyRecipe($target, $dataset, $xiv, $progress = null, $priceList = null)
{
    if ($target == null) {
        $target['error'] = "No Recipe";
        return $target;
    }

    $target['listCost'] = -1;
    $target['marketCost'] = -1;
    $market = $xiv->getMarket($target['ID'], false, $progress);
    $target['MarketNumber'] = count($market);
    $cheap = $xiv->currentCheapest($market[$target['ID']]);
    if ($cheap != null) {
        $target['marketCost'] = $cheap['Item']->pricePerUnit;
    } else {
        $target['marketCost'] = -1;
    }
    $history = $xiv->getHistory($target['ID'], false, $progress);
    $target['Recent'] = $xiv->mostRecent($history[$target['ID']], false);
    $target['Week'] = $xiv->weekAverage($history[$target['ID']], false);

    $target['Cost'] = array('Optimal' => 0, 'List' => 0, 'Market' => 0, 'Marketboard' => 0);
    foreach ($target['Parts'] as &$part) {
        $part['Cost'] = array('Optimal' => 0, 'List' => 0, 'Market' => 0, 'Marketboard' => 0);
        foreach ($part['Process'] as &$process) {
            $process['Cost'] = array('Optimal' => 0, 'List' => 0, 'Marketboard' => 0);
            foreach ($process['Set'] as &$item) {
                $recipe = doRecipieFromRecipe($item['Recipe'], $dataset, $xiv, $progress, $priceList);
                $process['Cost']['Optimal'] += $recipe['Cost']['Optimal'];
                $process['Cost']['Market'] += $recipe['Cost']['Market'];
                $process['Cost']['List'] += $recipe['Cost']['List'];
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
            $part['Cost']['List'] += $process['Cost']['List'];
            if ($part['Cost']['Marketboard'] >= 0) {
                if ($process['Cost']['Marketboard'] > 0) {
                    $part['Cost']['Marketboard'] += $process['Cost']['Marketboard'];
                } else {
                    $part['Cost']['Marketboard'] = -1;
                }
            }
        }
        $target['Cost']['Optimal'] += $part['Cost']['Optimal'];
        $target['Cost']['List'] += $part['Cost']['List'];
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


function add_ingred(&$result, $dataset, $bit, $d, $max_depth, $ids): array
{
    if (array_key_exists($bit['id'], $result)) {
        return $result;
    }
    if (($max_depth < 0 || $d < $max_depth) && !empty($bit['bits'])) {
        foreach ($bit['bits'] as $b) {
            $result = add_ingred($result, $dataset, $b, $d + 1, $max_depth, $ids);
        }
    } else {
        if ($ids) {
            $result[] = $bit['id'];
        } else {
            $data = $dataset->getItem($bit['id']);
            $result[$bit['id']] = $data->Name;
        }
    }
    return $result;

}


function getIngredientList($items, $dataset, $max_depth = -1, $ids = true)
{
    $result = [];
    $cache = [];
    foreach ($items as $itemID) {
        $cache = $dataset->getNeeded($itemID);
        if ($cache !== null) {
            $result = array_merge($result, $cache);
            continue;
        }
        $data = null;
        $itemNeeds = [];
        $recipe = getRecipe($itemID, $dataset, 'Armorcraft', null);
        foreach ($recipe['Recipe'] as $bit) {
            $itemNeeds = add_ingred($itemNeeds, $dataset, $bit, 0, $max_depth, $ids);
        }
        $itemNeeds = array_unique($itemNeeds);
        $dataset->setNeeded($itemID, $itemNeeds);
        $result = array_merge($result, $itemNeeds);
    }
    $result = array_unique($result);
    return $result;

}


function secondsToTime($seconds)
{
    $d = $seconds;
    $q = floor($d / (24 * 3600));
    $h = floor($d / 3600);
    $m = floor($d % 3600 / 60);
    $s = floor($d % 3600 % 60);

    $dDisplay = $q > 0 ? $q.($q == 1 ? " day, " : " days, ") : "";
    $hDisplay = $h > 0 ? $h.($h == 1 ? " hour, " : " hours, ") : "";
    $mDisplay = $m > 0 ? $m.($m == 1 ? " minute, " : " minutes, ") : "";
    $sDisplay = $s > 0 ? $s.($s == 1 ? " second" : " seconds") : "";
    return $dDisplay.$hDisplay.$mDisplay.$sDisplay;

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
        } elseif ($bit['listCost'] > 0) {
            print number_format($bit['listCost']);
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
        print(' ('.secondsToTime($bit['lastUpdate']).')');
        print "\n";
        line($bit['bits'], $tab + 1);
    }

}


function printRecipe($recipe)
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

    print "------------------------------------\n";

    print "Shop Cost: ";
    if ($recipe['Cost']['Shop'] > 0) {
        print number_format($recipe['Cost']['Shop'])." gil \n";
    } else {
        print "None\n";
    }

    print "Craft at List Cost: ";
    if ($recipe['Cost']['List'] > 0) {
        print number_format($recipe['Cost']['List'])." gil \n";
    } else {
        print "UNAVLIABLE\n";
    }
    print "Craft at Optimal Cost: ";
    if ($recipe['Cost']['Optimal'] > 0) {
        print number_format($recipe['Cost']['Optimal'])." gil \n";
    } else {
        print "UNAVLIABLE\n";
    }

    print "Craft at Market Board: ";
    if ($recipe['Cost']['Market'] > 0) {
        print number_format($recipe['Cost']['Market'])." gil \n";
    } else {
        print "UNAVLIABLE\n";
    }

    print "Possible Profit:\n";
    print "\tHQ ".($recipe['Profit']['HQ'])." gil (".floor($recipe['Profit']['HQ%'] * 100.0)."%)";
    print "\tLQ ".($recipe['Profit']['LQ'])." gil (".floor($recipe['Profit']['LQ%'] * 100.0)."%)\n";
    print "Marketboard Profit:\n";
    print "\tHQ ".($recipe['Profit']['Market_HQ'])." gil (".floor($recipe['Profit']['Market_HQ%'] * 100.0)."%)";
    print "\tLQ ".($recipe['Profit']['Market_LQ'])." gil (".floor($recipe['Profit']['Market_LQ%'] * 100.0)."%)\n";

    print "Sales:\n";
    if ($recipe['Recent']['HQ']) {
        print "\tRecent:\t\tHQ ".number_format($recipe['Recent']['HQ']->pricePerUnit).' gil';
    } else {
        print "\tRecent:\t\tHQ NONE    ";
    }
    if ($recipe['Recent']['LQ']) {
        print "\tLQ ".number_format($recipe['Recent']['LQ']->pricePerUnit)." gil\n";
    } else {
        print "\tLQ NONE    \n";
    }
    if ($recipe['Week']['HQ']) {
        print "\tWeekly Average:\tHQ ".number_format($recipe['Week']['HQ']['Average']).' gil';
    } else {
        print "\tWeekly Average:\tHQ NONE    ";
    }
    if ($recipe['Week']['LQ']['Average']) {
        print "\tLQ ".number_format($recipe['Week']['LQ']['Average'])." gil\n";
    } else {
        print "\tLQ NONE    \n";
    }

    print "====================================\n";
    line($recipe['Recipe'], 0);
    print "====================================\n";

}


function printRecipeSummaryTitle()
{
    print "Name,ItemID,Cost Optimal,Cost Market,Recent Sale HQ,Recent Sale LQ,Weekly Average LQ,Weekly Average HQ,Sales this Week,Cheapest LQ,Cheapest HQ,Profit LQ,Profit LQ%,Profit HQ,Profit HQ%,Max Profit,Max Profit%,Weighted Profit\n";

}

function printRecipeCostSummaryTitle()
{
    print "Name,Cost Optimal,Cost Market,Level,Tokens,Per Token\n";

}


function printRecipeSummary($recipe)
{
    if (array_key_exists('Error', $recipe)) {
        print $recipe['Error'];
        return;
    }

    print $recipe['Name'].",".$recipe['ID'].",";
    if ($recipe['Cost']['Optimal'] > 0) {
        print $recipe['Cost']['Optimal'];
    } else {
        print "";
    }
    print ",";
    if ($recipe['Cost']['Market'] > 0) {
        print $recipe['Cost']['Market'];
    } else {
        print "";
    }
    print ",";
    if ($recipe['Recent']['HQ']) {
        print $recipe['Recent']['HQ']->pricePerUnit;
    } else {
        print "";
    }
    print ",";
    if ($recipe['Recent']['LQ']) {
        print $recipe['Recent']['LQ']->pricePerUnit;
    } else {
        print "";
    }
    print ",";
    if ($recipe['Week']['HQ']) {
        print $recipe['Week']['HQ']['Average'];
    } else {
        print "";
    }
    print ",";
    if ($recipe['Week']['LQ']['Average']) {
        print $recipe['Week']['LQ']['Average'];
    } else {
        print "";
    }

    print ",";
    $weekCount = $recipe['Week']['HQ']['Count'] + $recipe['Week']['LQ']['Count'];
    print $weekCount;

    print ",";
    if ($recipe['Cheap']['LQ'] !== null) {
        print $recipe['Cheap']['LQ']['Item']->pricePerUnit;
    } else {
        print '';
    }
    print ",";
    if ($recipe['Cheap']['HQ'] !== null) {
        print $recipe['Cheap']['HQ']['Item']->pricePerUnit;
    } else {
        print '';
    }

    print ",";
    print $recipe['Profit']['Market_LQ'];
    print ",";
    print $recipe['Profit']['Market_LQ%'];

    print ",";
    print $recipe['Profit']['Market_HQ'];
    print ",";
    print $recipe['Profit']['Market_HQ%'];

    print ",";
    print max($recipe['Profit']['Market_HQ'], $recipe['Profit']['Market_LQ']);
    print ",";
    print max($recipe['Profit']['Market_HQ%'], $recipe['Profit']['Market_LQ%']);

    print ",";
    print max($recipe['Profit']['Market_HQ'], $recipe['Profit']['Market_LQ']) * $weekCount;
    print "\n";

}

function printRecipeCostSummary($dataset, $recipe)
{
    if (array_key_exists('Error', $recipe)) {
        print $recipe['Error'];
        return;
    }

    print $recipe['Name'].",";
    if ($recipe['Cost']['Optimal'] > 0) {
        print $recipe['Cost']['Optimal'];
    } else {
        print "--";
    }
    print ",";
    if ($recipe['Cost']['Market'] > 0) {
        print $recipe['Cost']['Market'];
    } else {
        print "--";
    }
    print ",";
    $lvl = $recipe['Info']->RecipeLevel->ClassJobLevel;
    print $lvl.",";
    $tkn = (($lvl - 80) * 9) + 114;
    print $tkn.",";
    print $recipe['Cost']['Market'] / $tkn;
    print "\n";

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
            print "Cost Craft List: ".number_format($process['Cost']['List'])." gil \n";
            print "Cost Craft Market: ".number_format($process['Cost']['Market'])." gil \n";
            print "Cost Marketboard: ".number_format($process['Cost']['Marketboard'])." gil \n";
            print "<><><><><><><><><><><><><><><><><><><><><><><><><><><><><>\n";
        }
        print "[][][][][][][][][][][][][] Part ".($pindex + 1)." [][][][][][][][][][][][][]\n";
        print "Part Cost Craft Optimal: ".number_format($part['Cost']['Optimal'])." gil \n";
        print "Part Cost Craft List: ".number_format($part['Cost']['List'])." gil \n";
        print "Part Cost Craft Market: ".number_format($part['Cost']['Market'])." gil \n";
        print "Part Cost Marketboard: ".number_format($part['Cost']['Marketboard'])." gil \n";
        print "[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]\n";
    }
    print "===>   ".$target['Name']."(".$target['ID'].")\n";
    print "Total Cost Craft Optimal: ".number_format($target['Cost']['Optimal'])." gil \n";
    print "Total Cost Craft List: ".number_format($target['Cost']['List'])." gil \n";
    print "Total Cost Craft Market: ".number_format($target['Cost']['Market'])." gil \n";
    print "Total Cost MarketBoard: ".number_format($target['Cost']['Marketboard'])." gil \n";
    print "Current List: ".number_format($target['listCost'])." gil";
    print "Current Market: ".number_format($target['marketCost'])." gil";
    print " (".$target['MarketNumber']." sellers)\n";
    print "Recent: ".number_format($target['Recent']->pricePerUnit)." gil on ";
    print date('l jS \of F Y h:i:s A', $target['Recent']->PurchaseDate)." \n";
    print "Week: ".number_format($target['Week']->pricePerUnit)." gil\n";

}
