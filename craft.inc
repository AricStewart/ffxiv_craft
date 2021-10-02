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

$SHARD_DEFAULT = 60;

function countItems($recipe, &$i)
{
    $i += count($recipe);
    foreach ($recipe as $bit) {
        countItems($bit['bits'], $i);
    }

}


function priceRecipe(&$input, $dataset, $progress, $recipe, $priceList)
{
    global $SHARD_DEFAULT;

    $listCost = 0;
    $optimalCost = 0;
    foreach ($input as &$bit) {
        report_progress($progress, "info", $bit['name']);
        report_progress($progress, "partial", json_encode($recipe));
        report_progress($progress, "info", $bit['name'].' market');
        report_progress($progress, "partial", json_encode($recipe));
        report_progress($progress, "info", $bit['name'].' history');
        report_progress($progress, "partial", json_encode($recipe));

        if ($priceList && array_key_exists($bit['id'], $priceList)) {
            $cheap = $priceList[$bit['id']];
        } else {
            $cheap = null;
            if (strstr($bit['name'], " Shard")) {
                $cheap = $SHARD_DEFAULT;
            }
        }
        if ($cheap != null) {
            $bit['listCost'] = $cheap * $bit['count'];
        }

        if (count($dataset->findItemInShop($bit['id'])) > 0) {
            $bit['shopCost'] *= $bit['count'];
        } else {
            $bit['shopCost'] = 0;
        }

        if ($bit['listCost'] == 0) {
            $listCost = -1;
            $bitOptimal = -1;
        } else {
            if ($listCost != -1) {
                $listCost += $bit['listCost'];
            }
            $bitOptimal = $bit['listCost'];
        }
        if ($bit['bits']) {
            $partCost = priceRecipe(
                $bit['bits'],
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
    return ['List' => $listCost, 'Optimal' => $optimalCost];

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


function doRecipieFromRecipe($recipe, $dataset, $progress = null, $priceList = null)
{
    if ($recipe == null) {
        $recipe['error'] = "No Recipe";
        return $recipe;
    }

    report_progress($progress, "partial", json_encode($recipe));

    $itemID = $recipe['ID'];
    /* Pre Download */
    report_progress($progress, "partial", json_encode($recipe));

    $item = $dataset->getItem($recipe['ID']);
    $recipe['Cost'] = priceRecipe($recipe['Recipe'], $dataset, $progress, $recipe, $priceList);
    if (count($dataset->findItemInShop($itemID)) > 0) {
        $recipe['Cost']['Shop'] = $item->PriceMid * $recipe['Count'];
    } else {
        $recipe['Cost']['Shop'] = 0;
    }
    report_progress($progress, "partial", json_encode($recipe));
    report_progress($progress, "partial", json_encode($recipe));

    $recipe['Cost']['Marketboard'] = null;

    report_progress($progress, "partial", json_encode($recipe));

    return $recipe;

}


function doRecipie(
    $itemID,
    $dataset,
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
    return doRecipieFromRecipe($recipe, $dataset, $progress, $priceList);

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


function doCompanyRecipe($target, $dataset, $progress = null, $priceList = null)
{
    if ($target == null) {
        $target['error'] = "No Recipe";
        return $target;
    }

    $target['listCost'] = -1;
    $target['Cost'] = array('Optimal' => 0, 'List' => 0, 'Marketboard' => 0);
    foreach ($target['Parts'] as &$part) {
        $part['Cost'] = array('Optimal' => 0, 'List' => 0, 'Marketboard' => 0);
        foreach ($part['Process'] as &$process) {
            $process['Cost'] = array('Optimal' => 0, 'List' => 0, 'Marketboard' => 0);
            foreach ($process['Set'] as &$item) {
                $recipe = doRecipieFromRecipe($item['Recipe'], $dataset, $progress, $priceList);
                $process['Cost']['Optimal'] += $recipe['Cost']['Optimal'];
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
        if ($bit['listCost'] > 0) {
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

    print "====================================\n";
    line($recipe['Recipe'], 0);
    print "====================================\n";

}


function test($dataset)
{
    $o = doRecipie(23815, $dataset);
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
            print "Cost Marketboard: ".number_format($process['Cost']['Marketboard'])." gil \n";
            print "<><><><><><><><><><><><><><><><><><><><><><><><><><><><><>\n";
        }
        print "[][][][][][][][][][][][][] Part ".($pindex + 1)." [][][][][][][][][][][][][]\n";
        print "Part Cost Craft Optimal: ".number_format($part['Cost']['Optimal'])." gil \n";
        print "Part Cost Craft List: ".number_format($part['Cost']['List'])." gil \n";
        print "Part Cost Marketboard: ".number_format($part['Cost']['Marketboard'])." gil \n";
        print "[][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]\n";
    }
    print "===>   ".$target['Name']."(".$target['ID'].")\n";
    print "Total Cost Craft Optimal: ".number_format($target['Cost']['Optimal'])." gil \n";
    print "Total Cost Craft List: ".number_format($target['Cost']['List'])." gil \n";
    print "Total Cost MarketBoard: ".number_format($target['Cost']['Marketboard'])." gil \n";
    print "Current List: ".number_format($target['listCost'])." gil";
    print " (".$target['MarketNumber']." sellers)\n";
    print "Recent: ".number_format($target['Recent']->PricePerUnit)." gil on ";
    print date('l jS \of F Y h:i:s A', $target['Recent']->PurchaseDate)." \n";
    print "Week: ".number_format($target['Week']->PricePerUnit)." gil\n";

}
