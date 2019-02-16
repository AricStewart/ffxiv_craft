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
    require_module 'json';
.*/

$_startSent = false;

function http_progress($type="", $data="", $info=null)
{
    global $_startSent;
    if ($type === "start") {
        if ($_startSent) {
            return;
        }
        $_startSent = true;
    }
    $data = array( "type" => $type,
                   "data" => $data);
    if ($info !== null) {
        $data = array_merge($data, $info);
    }
    echo "data: ".json_encode($data);
    echo "\n\n";
    ob_flush();
    flush();
}

function sortByProfit($a, $b)
{
    $p = max($a['Profit']['HQ%'], $a['Profit']['LQ%']);
    $p2 = max($b['Profit']['HQ%'], $b['Profit']['LQ%']);
    return $p2 - $p;
}

