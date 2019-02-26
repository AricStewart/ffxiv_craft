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

function get_arguments($method, &$ffxiv_server, &$target, &$event)
{
    $arguments = [
        'event'         => FILTER_SANITIZE_SPECIAL_CHARS,
        'server'        => FILTER_SANITIZE_SPECIAL_CHARS,
        'target'        => FILTER_SANITIZE_SPECIAL_CHARS,
    ];

    $data = filter_input_array($method, $arguments);

    $event = isset($data['event']);

    if (!isset($data['server'])) {
        header(404);
        exit();
    } else {
        $ffxiv_server = $data['server'];
    }

    $target = $data['target'];
}

$output = array();
$ffxiv_server = "";
$target = 0;
$event = false;
if (!empty($_POST)) {
    get_arguments(INPUT_POST, $ffxiv_server, $target, $event);
    if ($ffxivmbGuid && !empty($ffxivmbGuid)) {
        $marketboard = new Ffxivmb($ffxiv_server, $ffxivmbGuid);
    } else {
        $marketboard = null;
    }
    $dataset = new FfxivDataSet('..');
    $xiv = new Xivapi($ffxiv_server, $xivapiKey, $marketboard, "..");
    $xiv->silent = true;
    $company = $dataset->loadCompanyCrafting();
    $recipe = getCompanyRecipe($target, $dataset, 'http_progress');
    $output = doCompanyRecipe($recipe, $dataset, $xiv, 'http_progress');
    print json_encode($output);
} else {
    header('Content-Type: text/event-stream');
    get_arguments(INPUT_GET, $ffxiv_server, $target, $event);
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
    $company = $dataset->loadCompanyCrafting();
    http_progress("info", $target);
    $recipe = getCompanyRecipe($target, $dataset, 'http_progress');
    http_progress("start", $recipe['Size']);
    $output = doCompanyRecipe($recipe, $dataset, $xiv, 'http_progress');
    http_progress("done", json_encode($output));
}
ob_end_flush();
sleep(1);
