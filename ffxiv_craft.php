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
require_once __DIR__."/universalis.inc";
require_once __DIR__."/craft.inc";
require __DIR__.'/vendor/autoload.php';

$dotenv = Dotenv\Dotenv::create(__DIR__);
$dotenv->load();

$xiv = new Universalis($_ENV['server']);
$dataset = new FfxivDataSet();
$fullHistory = true;


function _sortByProfit(array $a, array $b): int
{
    $p = max($a['Profit']['Market_HQ'], $a['Profit']['Market_LQ']);
    if ($a['Profit']['Market_HQ'] > $a['Profit']['Market_LQ']) {
        $wc1 = $a['Week']['HQ']['Count'];
    } else {
        $wc1 = $a['Week']['LQ']['Count'];
    }
    $p2 = max($b['Profit']['Market_HQ'], $b['Profit']['Market_LQ']);
    $wc2 = $b['Week']['HQ']['Count'] + $b['Week']['LQ']['Count'];
    if ($b['Profit']['Market_HQ'] > $b['Profit']['Market_LQ']) {
        $wc2 = $b['Week']['HQ']['Count'];
    } else {
        $wc2 = $b['Week']['LQ']['Count'];
    }
    return floor((1000 * $p * $wc1) - (1000 * $p2 * $wc2));

}

function _sortByCost(array $a, array $b): int
{

    $lvl = $a['Info']->RecipeLevel->ClassJobLevel;
    $tkn = (($lvl - 80) * 9) + 114;
    $p = $a['Cost']['Market'] / $tkn;

    $lvl = $b['Info']->RecipeLevel->ClassJobLevel;
    $tkn = (($lvl - 80) * 9) + 114;
    $p2 = $b['Cost']['Market'] / $tkn;

    return floor((1000 * $p) - (1000 * $p2));

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
        fwrite(STDERR, '|');
    } elseif ($i % 5 == 0) {
        fwrite(STDERR, '-');
    } else {
        fwrite(STDERR, '.');
    }

}


$priceList = array();
if (file_exists(__DIR__."/item.cost.csv")) {
    $handle = fopen(__DIR__."/item.cost.csv", "r");
    while (($data = fgetcsv($handle, 2000, ",")) !== false) {
        $priceList[$data[0]] = $data[1];
    }
}

$i = 1;
if (count($argv) > 1) {
    if ($argv[1] == '-c') {
        array_map('unlink', glob("data/*.json"));
        $itemID = $argv[2];
        $i = 3;
        $xiv = new Universalis($_ENV['server']);
    } elseif ($argv[1] == '-m') {
        $a = $dataset->getMastercraft($argv[2]);
        /* 1 day timeout */
        $xiv = new Universalis($_ENV['server'], 43200);
        fwrite(STDERR, count($a).PHP_EOL);

        $ing = getIngredientList($a, $dataset);
        /* pre-cache all the items being retrieved */
        $xiv->getMarket(array_merge($a, $ing), true, null);
        $xiv->flushPool(null);

        if (count($a) > 0) {
            $output = [];
            foreach ($a as $key => $i) {
                fwrite(STDERR, " $i (".($key + 1)."|".count($a).") ");
                $output[] = doRecipie($i, $dataset, $xiv, 'tick', $priceList);
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
        if ($argc == 5) {
            $tiertop = intval($argv[4]);
        } else {
            $tiertop = intval($argv[3]);
        }
        if (strtolower($crafter) == 'all') {
            $crafter = null;
        }
        fwrite(STDERR, "Doing tier: ".$tier." - ".$tiertop.PHP_EOL);
        $set = $dataset->getRecipeSet($crafter, ($tier - 1) * 5, $tiertop * 5);
        if (empty($set)) {
            fwrite(STDERR, "NO RECIPIES".PHP_EOL);
            exit();
        }
        fwrite(STDERR, "recipies: ".count($set).PHP_EOL);
        /* 1 day timeout */
        $ing = getIngredientList($set, $dataset);
        fwrite(STDERR, "Ingredients: ".count($ing).PHP_EOL);

        $xiv = new Universalis($_ENV['server'], 43200);
        $cache = array_unique(array_merge($set, $ing));
        /* pre-cache all the items being retrieved */
        $xiv->getMarket($cache, true, null);
        $xiv->flushPool(null);

        foreach ($set as $i => $r) {
            fwrite(STDERR, " ".$r." (".($i + 1)."|".count($set).") ");
            $output[] = doRecipie($r, $dataset, $xiv, 'tick', $crafter, $priceList);
        }
        usort($output, '_sortByProfit');
        printRecipeSummaryTitle();
        foreach ($output as $recipe) {
            printRecipeSummary($recipe);
        }
        exit();
    } elseif ($argv[1] == '-x') {
        $xiv = new Universalis($_ENV['server']);
        $company = $dataset->loadCompanyCrafting();
        $recipe = getCompanyRecipe($argv[2], $dataset, 'tick');
        $output = doCompanyRecipe($recipe, $dataset, $xiv, 'tick');
        printCompanyRecipe($output);
        exit();
    } elseif ($argv[1] == '-r') {
        $crafter = $argv[2];
        $tier = intval($argv[3]);
        if ($argc == 5) {
            $tiertop = intval($argv[4]);
        } else {
            $tiertop = intval($argv[3]);
        }
        if (strtolower($crafter) == 'all') {
            $crafter = null;
        }
        fwrite(STDERR, "Doing tier: ".$tier." - ".$tiertop.PHP_EOL);
        $set = $dataset->getRecipeSet($crafter, ($tier - 1) * 5, $tiertop * 5, true);
        if (empty($set)) {
            fwrite(STDERR, "NO RECIPIES".PHP_EOL);
            exit();
        }
        fwrite(STDERR, "recipies: ".count($set).PHP_EOL);
        /* 1 day timeout */
        $ing = getIngredientList($set, $dataset);
        fwrite(STDERR, "Ingredients: ".count($ing).PHP_EOL);

        $xiv = new Universalis($_ENV['server'], 43200);
        $cache = array_unique(array_merge($set, $ing));
        /* pre-cache all the items being retrieved */
        $xiv->getMarket($cache, true, null);
        $xiv->flushPool(null);

        foreach ($set as $i => $r) {
            fwrite(STDERR, " ".$r." (".($i + 1)."|".count($set).") ");
            $output[] = doRecipie($r, $dataset, $xiv, 'tick', $crafter, $priceList);
        }
        usort($output, '_sortByCost');
        printRecipeCostSummaryTitle();
        foreach ($output as $recipe) {
            printRecipeCostSummary($dataset, $recipe);
        }
        exit();
    } else {
        $xiv = new Universalis($_ENV['server']);
        $itemID = $argv[1];
        $i = 2;
    }
} else {
    $xiv = new Universalis($_ENV['server']);
    $itemID = 23815;
}


$crafter = 'Armorcraft';
if (count($argv) > $i) {
    $crafter = $argv[$i];
}

if (!is_numeric($itemID)) {
    $result = $dataset->getItem($itemID);
    if ($result === null) {
        fwrite(STDERR, 'Could not find item \''.$itemID.PHP_EOL);
        exit();
    }
    $itemID = $result->Index;
}

$data = getRecipe($itemID, $dataset, $crafter, 'tick');
$output = doRecipieFromRecipe($data, $dataset, $xiv, 'tick', $priceList);
printRecipe($output);
