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


function http_progress($type = "", $data = "", $info = null)
{
    global $_startSent;
    $data = array( "type" => $type,
                   "data" => $data);
    if ($type === "start") {
        if ($_startSent) {
            return;
        }
        $_startSent = true;
        $data['retry'] = 10000;
    }
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
    $p = max(floatval($a['Profit']['Market_HQ']), floatval($a['Profit']['Market_LQ']));
    $p2 = max(floatval($b['Profit']['Market_HQ']), floatval($b['Profit']['Market_LQ']));
    return floor((1000 * $p) - (1000 * $p2));

}
