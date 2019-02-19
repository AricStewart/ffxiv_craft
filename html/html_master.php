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
.*/

header('Cache-Control: no-cache');
require_once __DIR__."/../apiData.inc";
require_once __DIR__."/../ffxivData.inc";
require_once __DIR__."/../ffxivmb.inc";
require_once __DIR__."/../xivapi.inc";
require_once __DIR__."/../craft.inc";
require_once __DIR__."/common.inc";

function get_arguments($method, &$ffxiv_server, &$bookID, &$event)
{
    $arguments = [
        'event'         => FILTER_SANITIZE_SPECIAL_CHARS,
        'server'        => FILTER_SANITIZE_SPECIAL_CHARS,
        'book'          => FILTER_SANITIZE_SPECIAL_CHARS,
    ];

    $data = filter_input_array($method, $arguments);

    $event = isset($data['event']);

    if (!isset($data['server'])) {
        header(404);
        exit();
    } else {
        $ffxiv_server = $data['server'];
    }
    $bookID = htmlspecialchars_decode($data['book'], ENT_QUOTES);

}

$output = array();
if (!empty($_POST)) {
    get_arguments(INPUT_POST, $ffxiv_server, $bookID, $event);

    if ($ffxivmbGuid && !empty($ffxivmbGuid)) {
        $marketboard = new Ffxivmb($ffxiv_server, $ffxivmbGuid);
    } else {
        $marketboard = null;
    }
    $dataset = new FfxivDataSet('..');
    $xiv = new Xivapi($ffxiv_server, $xivapiKey, $marketboard, "..");
    $xiv->silent = true;

    $a = $dataset->getMasterCraft($bookID);
    $crafter = $dataset->getMasterBookJob($bookID);
    foreach ($a as $key => $i) {
        $output[] = doRecipie($i, $dataset, $xiv, null, $crafter);
    }
    usort($output, 'sortByProfit');
    print json_encode($output);
} else {
    header('Content-Type: text/event-stream');

    get_arguments(INPUT_GET, $ffxiv_server, $bookID, $event);

    if (!$event) {
        http_progress("done", json_encode([]));
    }

    if ($ffxivmbGuid && !empty($ffxivmbGuid)) {
        $marketboard = new Ffxivmb($ffxiv_server, $ffxivmbGuid);
    } else {
        $marketboard = null;
    }
    $dataset = new FfxivDataSet('..');
    $xiv = new Xivapi($ffxiv_server, $xivapiKey, $marketboard, "..");
    $xiv->silent = true;

    $a = $dataset->getMasterCraft($bookID);
    $crafter = $dataset->getMasterBookJob($bookID);
    $size = 0;
    foreach ($a as $key => $i) {
        $data = getRecipe($i, $dataset, $crafter, 'http_progress');
        $size += $data['Size'];
        $output[] = $data;
    }

    http_progress("start", $size, ["info" => $crafter]);

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
