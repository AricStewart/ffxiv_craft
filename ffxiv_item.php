#!/usr/bin/php
<?php

require_once __DIR__."/ffxivData.inc";

$dataset = new FfxivDataSet();

$itemID = "Silver Ore";
$depth = -1;

if (count($argv) > 1) {
    $itemID = $argv[1];
}

if (count($argv) > 2) {
    $depth = intval($argv[2]);
}

if (!is_numeric($itemID)) {
    $result = $dataset->getItem($itemID);
    if ($result === null) {
        print 'Could not find item \''.$itemID."'\n";
        exit();
    }
    $itemID = $result->Index;
}

function doit($itemID, $d=0, $max_depth=-1)
{
    global $dataset;
    $result = array();
    $ds = $dataset->listRecipiesUsingItem($itemID);
    foreach ($ds as $i) {
        print('.');
        $itm = $dataset->getItem($i['Recipe']);
        $result[] = ['item' => $itm->Name,
                     'level'=> $i['Level'],
                     'depth' => $d
                    ];
        if ($itm->Index)
        {
            if ($max_depth < 0 || $d+1 <= $max_depth)
            {
                $next = doit($itm->Index, $d+1, $max_depth);
                $result = array_merge($result, $next);
            }
        }
    }
    return $result;
}

function sortByLevel($a, $b) {
    return $a['level'] - $b['level'];
}

$result = doit($itemID, 0, $depth);
print("\n");
usort($result, sortBylevel);
foreach ($result as $i) {
    for ($j = 0; $j < $i['depth']; $j++)
        print '.';
    print $i['item'].":".$i['level']."\n";
}
