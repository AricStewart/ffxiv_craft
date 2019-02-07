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
require_once __DIR__."/ffxivmb.inc";
require_once __DIR__."/xivapi.inc";
require_once __DIR__."/apiData.inc";

$marketboard = new Ffxivmb($server, $ffxivmbGuid);
$xiv = new Xivapi($server, $xivapiKey, $marketboard);
$xiv->verbose = false;

print "Intializing...\n";
$data = new FfxivDataSet();
$data->loadGathering();
$data->loadItems();
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
            $result = $data->getItemByName($argv[$i]);
            if ($result === null) {
                print 'Could not find item \''.$argv[$i]."'\n";
                exit();
            }
            $stuff = [$data->gather[$result]];
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
    if ($data->item[$item['Item']] != null) {
        $idb = $data->item[$item['Item']];
        if ($idb['IsUntradable']) {
            continue;
        }
        if ($limit !== null) {
            $limit--;
            if ($limit < 0) {
                break;
            }
        }
        $added = false;
        print "($count/$limit): ".$idb['Name'].'('.$item['Item'].') ';

        $profitLQ = $xiv->itemProfit($item['Item'], false);
        $xiv->printProfit($profitLQ);
        if ($profitLQ['Profit'] > 0) {
            $profit[] = $profitLQ;
        }

        $profitHQ = $xiv->itemProfit($item['Item'], true);
        $xiv->printProfit($profitHQ);
        if ($profitHQ['Profit'] > 0) {
            $profit[] = $profitHQ;
        }
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
    print "! ".$data->item[$p['ID']]['Name'];
    $xiv->printProfit($p);
    print "\n";
}
