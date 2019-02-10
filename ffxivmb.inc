<?php
/**
 * Sending Data to https://ffxivmb.com/public/api
 *
 * PHP version 5
 *
 * @category  Final_Fantasy_XIV
 * @package   Ffxivmb
 * @author    Aric Stewart <aricstewart@google.com>
 * @copyright 2019 Aric Stewart
 * @license   Apache License, Version 2.0
 * @link      <none>
 **/

/*.
    require_module 'standard';
    require_module 'json';
.*/

require __DIR__.'/vendor/autoload.php';
use GuzzleHttp\Client;

class Ffxivmb
{
    private $_server = "";
    private $_guid = "";
    private $_client;
    private $_cache;

    function __construct($server, $guid) 
    {
        $this->_server = $server;
        $this->_guid = $guid;
        $this->_client = new Client(
            ['base_uri' => "https://ffxivmb.com/"]
        );
        $this->_cache = [];
    }

    function _buildHistoryUrl($item) 
    {
        return "/public/api/History?Server=".
            $this->_server."&SourceGuid=".$this->_guid."&ItemKey=".$item;
    }

    function _buildPriceUrl($item) 
    {
        return "/public/api/Prices?Server=".
            $this->_server."&SourceGuid=".$this->_guid."&ItemKey=".$item;

    }

    function _sendBlock($url, $data) 
    {
        if (array_key_exists($url, $this->_cache)) {
            return;
        }
        $this->_cache[$url] = $this->_client->request(
            'POST', $url,
            ['json' => $data]
        );
    }

    function sendHistory($id, $data, $unused) 
    {
        $url = $this->_buildHistoryUrl($id);
        $this->_sendBlock($url, $data->History);
    }

    function sendPrices($unused, $data, $time) 
    {
        $url = $this->_buildPriceUrl($data->Item->ID);
        $output = array();
        foreach ($data->Prices as $item) {
            $item = [
                        "CraftSignature"=>$item->CraftSignature,
                        "IsCrafted"=>$item->IsCrafted,
                        "Materia"=>[],
                        'PricePerUnit'=>$item->PricePerUnit,
                        'PriceTotal'=>$item->PriceTotal,
                        'Quantity'=>$item->Quantity,
                        'IsHQ'=>$item->IsHQ,
                        "RetainerName"=>$item->RetainerName,
                        "Stain"=>$item->Stain,
                        "TownID"=> $item->Town->ID,
                        "DateObtained"=>$time
                    ];
            if (!empty($data->Material)) {
                foreach ($data->Materia as $materia) {
                    $item['Materia'][] = $materia;
                }
            }
            $output[] = $item;
        }
        if (empty($output)) {
            $output[] = [
                        "CraftSignature"=>"",
                        "IsCrafted"=>false,
                        "Materia"=>[],
                        'PricePerUnit'=>0,
                        'PriceTotal'=>0,
                        'Quantity'=>0,
                        'IsHQ'=>false,
                        "RetainerName"=>"",
                        "Stain"=>0,
                        "TownID"=>0,
                        "DateObtained"=>$time
                        ];
        }
        $this->_sendBlock($url, $output);
    }
};
