#!/usr/bin/php
<?php
/**
 * Gathering data harvester
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
require __DIR__.'/vendor/autoload.php';

$dotenv = Dotenv\Dotenv::create(__DIR__);
$dotenv->load();


function printProfit($profit)
{
    if ($profit['Profit'] > 0) {
        $hq = '  ';
        if ($profit['HQ']) {
            $hq = '(*)';
        }
        print "\n\t$hq cheapest ".$profit['Cheapest']->PricePerUnit." gil\n";
        print "\t$hq Profit:".$profit['Profit']." gil\n";
        print "\t$hq week (".$profit['Week']['Minimum'].' <- '.$profit['Week']['Average'].' -> '.$profit['Week']['Maximum'].' gil)';

        $profit[] = $profit;
    }

}


print "Intializing...\n";
$data = new FfxivDataSet();
$data->loadGathering();
print "Processing ...\n";
$profit = [];
$stuff = array_reverse($data->gather);
$limit = count($stuff);

if ($argv[1]) {
    $i = 1;
    if ($argv[$i] == '-l') {
        $i++;
        $limit = intval($argv[$i]);
    }
    if ($argv[$i]) {
        if (!is_numeric($argv[$i])) {
            $result = $data->getItem($argv[$i]);
            if ($result === null) {
                print 'Could not find item \''.$argv[$i]."'\n";
                exit();
            }
            $stuff = [$data->gather[$result->Index]];
            $limit = 1;
        } else {
            $stuff = [$data->gather[$argv[$i]]];
            $limit = 1;
        }
    }
}

$count = 0;
foreach ($stuff as $item) {
    $count++;
    $idb = $data->getItem($item['Item']);
    if ($idb != null) {
        if ($idb->IsUntradable) {
            continue;
        }
        if ($limit !== null) {
            $limit--;
            if ($limit < 0) {
                break;
            }
        }
        $added = false;
        print "($count/$limit): ".$idb->Name.'('.$item['Item'].') ';
        print "\n";
    }
}

print "<===================================================================>\n";


function _sortByOrder($a, $b)
{
    return $a['Profit'] - $b['Profit'];

}


usort($profit, '_sortByOrder');
foreach ($profit as $p) {
    print "! ".$data->item[$p['ID']]->Name;
    printProfit($p);
    print "\n";
}
