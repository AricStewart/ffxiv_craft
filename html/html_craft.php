<?php
/**
 * Crafting data engine  as HTML backend
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
    require_module 'filter';
.*/

header('Cache-Control: no-cache');
require_once __DIR__."/../ffxivData.inc";
require_once __DIR__."/../universalis.inc";
require_once __DIR__."/../craft.inc";
require_once __DIR__."/common.inc";
require __DIR__.'/../vendor/autoload.php';

$dotenv = Dotenv\Dotenv::create(__DIR__);
$dotenv->load();


function get_arguments(
    $method,
    &$ffxiv_server,
    &$itemID,
    &$event,
    &$crafter,
    &$match
) {
    $arguments = [
    'event'         => FILTER_SANITIZE_SPECIAL_CHARS,
    'server'        => FILTER_SANITIZE_SPECIAL_CHARS,
    'item'          => FILTER_SANITIZE_SPECIAL_CHARS,
    'crafter'       => FILTER_SANITIZE_SPECIAL_CHARS,
    'match'         => FILTER_SANITIZE_SPECIAL_CHARS,
    ];

    $data = filter_input_array($method, $arguments);

    $event = isset($data['event']);

    if (!isset($data['server'])) {
        header(404);
        exit();
    } else {
        $ffxiv_server = $data['server'];
    }
    if (!isset($data['item'])) {
        $itemID = '23815';
    } else {
        $itemID = htmlspecialchars_decode($data['item'], ENT_QUOTES);
    }

    if (!isset($data['crafter'])) {
        $crafter = '';
    } else {
        $crafter = htmlspecialchars_decode($data['crafter'], ENT_QUOTES);
    }

    if (!isset($data['match'])) {
        $match = true;
    } else {
        $match = ($data['match'] == 'exact');
    }

}


function getItemSet($dataset, $itemID, $match)
{
    $output = array();
    if ($match) {
        $set = explode(',', $itemID);
        foreach($set as $i) {
            $item = $dataset->getItem(trim($i));
            if ($item !== null) {
                $output[] = $item->Index;
            }
        }
        return $output;
    } else {
        $set = $dataset->getItems($itemID);
        if ($set === null) {
            return $set;
        }

        $size = count($set);
        if ($size > 1) {
            foreach ($set as $index => $i) {
                $r = $dataset->getRecipe($i, null);
                if ($r !== null) {
                    $output[] = $set[$index];
                    if (count($output) > 19) {
                        break;
                    }
                }
            }
            return $output;
        }

        return $set;
    }

}


$crafter = "";
$event = "";
$itemID = "";
$match = true;
$ffxiv_server = "";

if (!empty($_POST)) {
    get_arguments(INPUT_POST, $ffxiv_server, $itemID, $event, $crafter, $match);

    $dataset = new FfxivDataSet('..');
    $xiv = new Universalis($ffxiv_server);

    $set = getItemSet($dataset, $itemID, $match);
    if ($set === null) {
        echo "[]";
        exit();
    }

    $output = [];
    $size = count($set);
    foreach ($set as $index => $i) {
        if ($size == 1) {
            $recp = doRecipie($i, $dataset, $xiv, 'http_progress', $crafter);
            array_unshift($output, $recp);
        } else {
            http_progress("info", ($index + 1)."/$size");
            $recp = doRecipie($i, $dataset, $xiv, null, $crafter);
            if ($recp['Info'] !== null) {
                array_unshift($output, $recp);
            }
        }
    }
    usort($output, 'sortByProfit');
    print json_encode($output);
} else {
    header('Content-Type: text/event-stream');

    get_arguments(INPUT_GET, $ffxiv_server, $itemID, $event, $crafter, $match);
    if (!$event) {
        http_progress("done", json_encode([]));
    }

    $dataset = new FfxivDataSet('..');
    $xiv = new Universalis($ffxiv_server);

    $set = getItemSet($dataset, $itemID, $match);
    if ($set === null) {
        http_progress("done", json_encode([]));
        exit();
    }

    $output = array();
    $size = 0;
    foreach ($set as $index => $i) {
        $data = getRecipe($i, $dataset, $crafter, 'http_progress');
        $size += $data['Size'];
        $output[] = $data;
    }

    http_progress("start", $size);

    foreach ($output as $index => $i) {
        unset($output[$index]);
        $recp = doRecipieFromRecipe($i, $dataset, $xiv, 'http_progress');
        array_unshift($output, $recp);
    }
    usort($output, 'sortByProfit');
    http_progress("done", json_encode($output));
}
ob_end_flush();
sleep(1);
