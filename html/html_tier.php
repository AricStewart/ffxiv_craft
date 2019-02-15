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

function http_progress($type="", $data="")
{
    $data = array( "type" => $type,
                   "data" => $data);
    echo "data: ".json_encode($data);
    echo "\n\n";
    ob_flush();
    flush();
}


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

function _sortByProfit($a, $b)
{
    $p = max($a['Profit']['HQ%'], $a['Profit']['LQ%']);
    $p2 = max($b['Profit']['HQ%'], $b['Profit']['LQ%']);
    return $p2 - $p;
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
    usort($output, '_sortByProfit');
    print json_encode($output);
} else {
    header('Content-Type: text/event-stream');

    get_arguments(INPUT_GET, $ffxiv_server, $tier, $event, $crafter);
    if (!$event) {
        $data = array( "type" => "done",
                       "data" => json_encode([]));
        echo "data: ".json_encode($data);
        echo "\n\n";
        ob_flush();
        flush();
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
    $data = array(
            "type" => "start",
            "info" => $crafter,
            "tier" => $tier,
            "data" => count($set));
    $result = json_encode($data);
    echo "data:".$result;
    echo "\n\n";
    ob_flush();
    flush();

    $size = count($set);
    foreach ($set as $index => $i) {
        $data = array(
                "type" => "info",
                "data" => ($index+1)."/$size");
        $result = json_encode($data);
        echo "data:".$result;
        echo "\n\n";
        ob_flush();
        flush();

        $output[] = doRecipie($i, $dataset, $xiv, null, $crafter);

        $data = array(
                "type" => "progress",
                "data" => "");
        $result = json_encode($data);
        echo "data:".$result;
        echo "\n\n";
        ob_flush();
        flush();
    }
    usort($output, '_sortByProfit');

    $data = array(
        "type" => "done",
        "data" => json_encode($output));
    $result = json_encode($data);
    echo "data:".$result;
    echo "\n\n";
    ob_flush();
    flush();
}
ob_end_flush();
sleep(1);
