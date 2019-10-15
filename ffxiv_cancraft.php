#!/usr/bin/php
<?php

require_once __DIR__."/ffxivData.inc";
require_once __DIR__."/parseInventory.inc";

function sortByLevel($a, $b) {
    return $a['level'] - $b['level'];
}

function canCraftItem($level, $inventory, $recipe, $dataset, $item_id, $cache=null, $quantity=1)
{
    $entry = $recipe[$item_id];
    if ($entry === null)
        return ['result'=>0, 'details' => null];

    if ($quantity == 0)
        return ['result'=>0, 'details' => null];

    $cancraft = 1;
    $pieces = [];
    foreach($entry['Ingred'] as $ingred_id => $ingred) {
        $need = intval($ingred['need']) * $quantity;
        $have = intval($inventory[$ingred_id]['count']);
        $make = canCraftItem(1, $inventory, $recipe, $dataset, $ingred_id, $cache, ($need-$have));
        $count = intval(floor(($have + $make['result']) / $need)) * $quantity;
        if ($count < 1)
        {
            $cancraft = 0;
        }
        $pieces[$ingred_id] = $count;
        if ($cache && array_key_exists($ingred_id, $cache))
            $name =  $cache[$ingred_id];
        else {
            $name = $dataset->getItem($ingred_id)->Name;
            if ($cache)
                $cache[$ingred_id] = $name;
        }
        $details[$ingred_id] = ['name'=>$name, 'qty'=>$quantity, 'need'=>$need, 'have'=>$have, 'make'=>$make];
    }
    $result = 0;
    if ($cancraft > 0)
        $cancraft = intval(min($pieces));
    if ($level != 0 && array_key_exists($item_id, $inventory)) {
        $result = $inventory[$item_id]['count'] + $cancraft;
    } else {
        $result = $cancraft;
    }
    return ['result'=>$result, 'details' => $details];
}

function load_inventory($dataset) {
    $inventory = [];
    $inv = false;
    $inv1 = false;
    $inv2 = false;
    if (file_exists('./Inventory/inv.json'))
        $inv = @filemtime('./Inventory/inv.json');
    if (file_exists('./Inventory/inv1.html'))
        $inv1 = @filemtime('./Inventory/inv1.html');
    if (file_exists('./Inventory/inv2.html'))
        $inv2 = @filemtime('./Inventory/inv2.html');

    if($inv !== false && (!$inv1 || $inv1 < $inv) && ((!$inv2 || $inv2 < $inv))) {
        $json = @file_get_contents('./Inventory/inv.json');
        $inventory = json_decode($json,true);
    } else {
        print("Parsing Inventory\n");
        $inventory = parse_file($dataset, './Inventory/inv1.html');
        $inventory = parse_file($dataset, './Inventory/inv2.html', $inventory);
        $fp = @fopen('./Inventory/inv.json', 'w');
        @fwrite($fp, json_encode($inventory, JSON_PRETTY_PRINT));
        @fclose($fp);
    }
    return $inventory;
}

function load_recipes() {
    $recipe = [];

    $sql = "SELECT * FROM `ingredients` ORDER BY `Recipe` ASC";
    $result = DB::run($sql);
    $value = $result->fetch();
    while ($value !== false) {
        if (!array_key_exists($value['Recipe'], $recipe)) {
            $recipe[$value['Recipe']] = [
                'Level' => $value['Level'],
                'Ingred' => []
            ];
        }
        if ($value['Item'] > 22) {
            $recipe[$value['Recipe']]['Ingred'][$value['Item']] = [
                'need' => $value['Amount'],
            ];
        }
        $value = $result->fetch();
    }
    return $recipe;
}

function figure_all(&$inventory, $recipe, $dataset, $cache)
{
    $pass = 0;
    do {
        print("Pass $pass ");
        $added = 0;
        foreach($recipe as $item_id => $entry) {
            if ($pass > 0 && array_key_exists($item_id, $inventory))
            {
                continue;
            }
            $cancraft = 1;
            $pieces = [];
            foreach($entry['Ingred'] as $ingred_id => $ingred) {
                if (!array_key_exists($ingred_id, $inventory) || ($inventory[$ingred_id]['count']) < $ingred['need']) {
                    $cancraft = 0;
                    break;
                } else {
                    $piece = $inventory[$ingred_id]['count'];
                    $need = $ingred['need'];
                    $count = intval(floor($piece / $need));
                    if ($count < 1)
                    {
                        $cancraft = 0;
                        break;
                    }
                    $pieces[] = $count;
                }
            }
            if ($cancraft > 0) $cancraft = min($pieces);
            if ($cancraft > 0) {
                if (array_key_exists($item_id, $inventory)) {
                    $inventory[$item_id]['count'] += $cancraft;
                } else {
                    if ($cache && array_key_exists($item_id, $cache)) {
                        $name = $cache[$item_id];
                    } else {
                        $name = $dataset->getItem($item_id)->Name;
                        if ($cache) {
                            $cache[$item_id] = $name;
                        }
                    }
                    $inventory[$item_id] = [
                    'name' => $name,
                        'level' => $entry['Level'],
                        'count' => $cancraft
                    ];
                    $added += 1;
                }
                $inventory[$item_id]['level'] = $entry['Level'];
            }
        }
        print ("- added $added\n");
        $pass++;
    } while ($added > 0);
}


function show_all($inventory, $recipe, $dataset)
{
    usort($inventory, sortByLevel);
    foreach($inventory as $part){
        print($part['name']."(".$part['level']."): ".$part['count']."\n");
    }
}

function print_line($part, $ind)
{
    print(str_repeat("  ", $ind));
    $bold = false;
    $makeable = 0;
    if ($part['make'] !== 0)
        $makeable = $part['make']['result'];
    else
        $makeable = 0;
    if ($part['need'] > ($part['have'] + $makeable)) {
        print("\033[0;31;1m");
        $bold = true;
    }
    if ($part['qty'] > 1) {
        $name = "{$part['qty']}x {$part['name']}";
    } else {
        $name = $part['name'];
    }
    printf("%-25s", $name);
    print(" need: {$part['need']} have: {$part['have']} makeable: ");
    if ($part['make'] !== 0)
        print("{$part['make']['result']}\n");
    else
        print("-\n");
    if ($bold) {
        print("\033[0m");
    }
}

function dig_part($plan, $ind)
{
    $ind++;
    if (is_array($plan) && array_key_exists('details', $plan) && is_array($plan['details'])) {
        foreach($plan['details'] as $part) {
            print_line($part, $ind);
            if ($part['have'] >= $part['need']) continue;
            if ($part['make'] !== 0 && array_key_exists('details', $part['make']))
                dig_part($part['make'], $ind);
        }
    }
}

function show_plan($item, $plan)
{
    $title = sprintf("|  Plan for %s  |", $item);
    print(str_repeat("=", strlen($title))."\n");
    print ($title."\n");
    print(str_repeat("=", strlen($title))."\n");
    print ("Can Craft Now: ".($plan['result'])."\n");
    $ind = 1;
    foreach($plan['details'] as $part) {
        print_line($part, $ind);
        if ($part['have'] >= $part['need']) continue;
        dig_part($part['make'], $ind);
    }
}


function getRecipeSet($dataset, $crafter, $cache, $levelLow = 0, $levelHigh = 0)
{
    if (!is_numeric($crafter)) {
        $crafter = array_search($crafter, $dataset->craftType);
        if ($crafter === false) {
            return null;
        }
    }
    $sql = "SELECT `Id`, `Item` FROM `recipes` WHERE `CraftType` <> $crafter";
    if ($levelLow > 0) {
        $sql .= " AND `RecipeLevel` >= $levelLow";
    }
    if ($levelHigh > 0) {
        $sql .= " AND `RecipeLevel` <= $levelHigh";
    }
    $result = DB::run($sql);
    $value = $result->fetch();
    $output = null;
    while ($value !== false) {
        $item = $dataset->getItem($value['Item']);
        if ($cache) {
            $cache[$value['Item']] = $item->Name;
        }
        if ($item->IsUntradable || $item->IsIndisposable) {
            $value = $result->fetch();
            continue;
        }
        if ($output === null) {
            $output = [];
        }
        $output[] = intval($value['Item']);
        $value = $result->fetch();
    }
    return $output;

}

function updateAlmost($item_id, $plan, $inventory, &$missing, $dataset = NULL)
{
    foreach($plan['details'] as  $partID=>$part) {
        if ($part['have'] >= $part['need']) continue;
        if ($part['make'] != NULL && $part['make']['details'] != NULL) {
            if ($part['have'] + $part['make']['result'] >= $part['need']) continue;
            updateAlmost($item_id, $part['make'], $inventory, $missing, $dataset);
            continue;
        }
        if ($dataset !== NULL && $dataset->gather[$partID] == NULL) continue;
        if ($missing[$partID]) {
            array_push($missing[$partID]['recipe'], $item_id);
            $missing[$partID]['number'] = min($missing[$partID]['number'], ($part['need'] - $part['have']));
        } else {
            $missing[$partID] = [
                'id' => $partID,
                'recipe' => [ $item_id ],
                'number' => ($part['need'] - $part['have'])
            ];
        }
    }
}

function almostItems($inventory, $recipe, $dataset, $level, $cache, $gatherOnly = false)
{
    $missing = [];
    $data = getRecipeSet($dataset, 'Cooking', $cache, $levelLow = $level, $levelHigh = 80);
    foreach($data as $item_id) {
        $entry = $recipe[$item_id];
        if (array_key_exists($item_id, $inventory))
        {
            print('.');
            continue;
        }
        $plan = canCraftItem(0, $inventory, $recipe, $dataset, $item_id, $cache);
        if ($plan['result'])
        {
            print('.');
            continue;
        }
        print('o');
        if ($gatherOnly)
            updateAlmost($item_id, $plan, $inventory, $missing, $dataset);
        else
            updateAlmost($item_id, $plan, $inventory, $missing);
    }
    return $missing;
}


function sortByCnt($a, $b)
{
    return count($a['recipe']) - count($b['recipe']);
}

$cache = [];
print("Begin\n");
$dataset = new FfxivDataSet();
$inventory = load_inventory($dataset);
print("Loaded Inventory\n");
$recipe = load_recipes();
print("Loaded Recipies\n");

if (count($argv) > 1 && $argv[1] == '-n') {
    $bottom = intval($argv[2]);
    $gatheronly = (count($argv) > 3 && $argv[3] == '-c');
    if ($gatheronly)
        $dataset->loadGathering();
    figure_all($inventory, $recipe, $dataset, $cache);
    $almost = almostItems($inventory, $recipe, $dataset, $bottom, $cache, $gatheronly);
    usort($almost,'sortByCnt');
    print("\n");
    for ($i = 0; $i < 20; $i++)  {
        $most = array_pop($almost);
        print($most['id'].": ");
        $item = $dataset->getItem($most['id']);
        print($item->Name." : ");
        print (count($most['recipe'])." :");
        print ($most['number']."\n");
    }
    exit();
}

$itemID = null;
if (count($argv) > 1) {
    $itemID = $argv[1];
    if (!is_numeric($itemID)) {
        $result = $dataset->getItem($itemID);
        if ($result === null) {
            print 'Could not find item \''.$itemID."'\n";
            exit();
        }
        $itemID = $result->Index;
    }
}

if ($itemID === null) {
    figure_all($inventory, $recipe, $dataset, $cache);
    show_all($inventory, $recipe, $dataset);
}
else {
    $plan = canCraftItem(0, $inventory, $recipe, $dataset, $itemID);
    show_plan($argv[1], $plan);
}
