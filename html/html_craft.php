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

require_once __DIR__."/../apiData.inc";
require_once __DIR__."/../ffxivData.inc";
require_once __DIR__."/../ffxivmb.inc";
require_once __DIR__."/../xivapi.inc";
require_once __DIR__."/../craft.inc";

if (!empty($_POST))
{
    $marketboard = new Ffxivmb($server, $ffxivmbGuid);
    $dataset = new FfxivDataSet('..');

    $arguments = [
        'server'        => FILTER_SANITIZE_SPECIAL_CHARS,
        'item'          => FILTER_SANITIZE_SPECIAL_CHARS,
    ];

    $data = filter_input_array(INPUT_POST, $arguments);
    if (!isset($data['server'])) {
        $server = "Coeurl";
    } else {
        $server = $data['server'];
    }
    if (!isset($data['item'])) {
        $itemID = '23815';
    } else {
        $itemID = htmlspecialchars_decode($data['item'], ENT_QUOTES);
    }

    $xiv = new Xivapi($server, $xivapiKey, $marketboard, "..");
    $xiv->silent = true;

    if (!is_numeric($itemID)) {
        $result = $dataset->getItemByName($itemID);
        if ($result === null) {
            print json_encode([]);
            exit();
        }
        $itemID = $result;
    }

    $output = doRecipie($itemID, $dataset, $xiv);
    print json_encode($output);
}
