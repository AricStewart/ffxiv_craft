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
    require_module 'curl';
.*/

class Ffxivmb
{
    private $_server = "";
    private $_guid = "";

    function __construct($server, $guid) 
    {
        $this->_server = $server;
        $this->_guid = $guid;
    }

    function _buildHistoryUrl($item) 
    {
        return "https://ffxivmb.com/public/api/History?Server=".
            $this->_server."&SourceGuid=".$this->_guid."&ItemKey=".$item;
    }

    function _buildPriceUrl($item) 
    {
        return "https://ffxivmb.com/public/api/Prices?Server=".
            $this->_server."&SourceGuid=".$this->_guid."&ItemKey=".$item;

    }

    function _sendBlock($url, $data) 
    {
        $json = json_encode($data);
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL,  $url);
        curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "POST");
        curl_setopt($ch, CURLOPT_POSTFIELDS, $json);
        curl_setopt(
            $ch, CURLOPT_HTTPHEADER, array(
            'Content-Type: application/json',
            'Content-Length: ' . strlen($json))
        );

        /*
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        $result = curl_exec($ch);

        var_dump($result);
        var_dump(curl_getinfo($ch));
        */

        curl_setopt($ch, CURLOPT_TIMEOUT, 0.1);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, false);
        curl_setopt($ch, CURLOPT_FORBID_REUSE, true);
        curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 0.1);
        curl_setopt($ch, CURLOPT_DNS_CACHE_TIMEOUT, 10);
        curl_setopt($ch, CURLOPT_FRESH_CONNECT, true);
        curl_exec($ch);
        curl_close($ch);
    }

    function sendHistory($id, $data) 
    {
        $url = $this->_buildHistoryUrl($id);
        $this->_sendBlock($url, $data->History);
    }

    function sendPrices($data, $time) 
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
