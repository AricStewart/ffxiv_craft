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
require_once __DIR__."/../ffxivData.inc";
require_once __DIR__."/../universalis.inc";
require_once __DIR__."/../craft.inc";
require_once __DIR__."/common.inc";
require __DIR__.'/../vendor/autoload.php';

$dotenv = Dotenv\Dotenv::create(__DIR__);
$dotenv->load();


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
        $crafter = 'all';
    } else {
        $crafter = htmlspecialchars_decode($data['crafter'], ENT_QUOTES);
    }

}


$output = array();
if (!empty($_POST)) {
    get_arguments(INPUT_POST, $ffxiv_server, $tier, $event, $crafter);

    $dataset = new FfxivDataSet('..');
    $xiv = new Universalis($ffxiv_server);
    $xiv->silent = true;

    if (strtolower($crafter) == 'all') {
        $target_crafter = null;
    } else {
        $crafter = null;
    }

    $set = $dataset->getRecipeSet($target_crafter, (($tier - 1) * 5) + 1, $tier * 5);
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

    $dataset = new FfxivDataSet('..');
    $xiv = new Universalis($ffxiv_server);
    $xiv->silent = true;

    if (strtolower($crafter) == 'all') {
        $target_crafter = null;
    } else {
        $crafter = null;
    }

    $set = $dataset->getRecipeSet($target_crafter, (($tier - 1) * 5) + 1, $tier * 5);
    $size = count($set) * 2;
    http_progress("start", $size, ["info" => $crafter, "tier" => $tier]);
    foreach ($set as $key => $i) {
        $data = getRecipe($i, $dataset, $crafter);
        report_progress('http_progress', "partial", json_encode($data));
        report_progress('http_progress');
        $recp = doRecipieFromRecipe($data, $dataset, $xiv);
        report_progress('http_progress', "partial", json_encode($recp));
        report_progress('http_progress');
        array_unshift($output, $recp);
    }

    usort($output, 'sortByProfit');
    http_progress("done", json_encode($output));
}
ob_end_flush();
sleep(1);
