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

function get_arguments($method, &$ffxiv_server, &$tier, &$event, &$crafter)
{
    $arguments = [
        'event'         => FILTER_SANITIZE_SPECIAL_CHARS,
        'server'        => FILTER_SANITIZE_SPECIAL_CHARS,
        'tier'          => FILTER_SANITIZE_SPECIAL_CHARS,
        'crafter'       => FILTER_SANITIZE_SPECIAL_CHARS,
    ];

    $data = filter_input_array($method, $arguments);

    $event = isset($data['event']);

    if (!isset($data['server'])) {
        header(404);
        exit();
    } else {
        $ffxiv_server = $data['server'];
    }

    $tier = $data['tier'];

    if (!isset($data['crafter'])) {
        $crafter = '';
    } else {
        $crafter = htmlspecialchars_decode($data['crafter'], ENT_QUOTES);
    }

}

if (!empty($_POST)) {
    get_arguments(INPUT_POST, $ffxiv_server, $tier, $event, $crafter);

    if ($ffxivmbGuid && !empty($ffxivmbGuid)) {
        $marketboard = new Ffxivmb($ffxiv_server, $ffxivmbGuid);
    } else {
        $marketboard = null;
    }
    $dataset = new FfxivDataSet('..');
    $xiv = new Xivapi($ffxiv_server, $xivapiKey, $marketboard, "..");
    $xiv->silent = true;

    $set = $dataset->getRecipeSet($crafter, (($tier-1)*5) + 1, $tier*5);
    foreach ($set as $i) {
        $output[] = doRecipie($i, $dataset, $xiv, null, $crafter);
    }
    usort($output, 'sortByProfit');
    print json_encode($output);
} else {
    header('Content-Type: text/event-stream');

    get_arguments(INPUT_GET, $ffxiv_server, $tier, $event, $crafter);
    if (!$event) {
        http_progress("done", json_encode([]));
        exit();
    }

    if ($ffxivmbGuid && !empty($ffxivmbGuid)) {
        $marketboard = new Ffxivmb($ffxiv_server, $ffxivmbGuid);
    } else {
        $marketboard = null;
    }
    $dataset = new FfxivDataSet('..');
    $xiv = new Xivapi($ffxiv_server, $xivapiKey, $marketboard, "..");
    $xiv->silent = true;

    $set = $dataset->getRecipeSet($crafter, (($tier-1)*5) + 1, $tier*5);
    http_progress("start", count($set), ["info" => $crafter, "tier" => $tier]);

    $size = count($set);
    foreach ($set as $index => $i) {
        http_progress("info", ($index+1)."/$size");
        $output[] = doRecipie($i, $dataset, $xiv, null, $crafter);
        http_progress("progress", "");
        usort($output, 'sortByProfit');
        http_progress("partial", json_encode($output));
    }
    usort($output, 'sortByProfit');
    http_progress("done", json_encode($output));
}
ob_end_flush();
sleep(1);
