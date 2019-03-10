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

    private $server = "";

    private $guid = "";

    private $client;

    private $cache;


    public function __construct($server, $guid)
    {
        $this->server = $server;
        $this->guid = $guid;
        $this->client = new Client(
            ['base_uri' => "https://ffxivmb.com/"]
        );
        $this->cache = [];

    }


    private function buildHistoryUrl($item)
    {
        return "/public/api/History?Server=".$this->server."&SourceGuid=".$this->guid."&ItemKey=".$item;

    }


    private function buildPriceUrl($item)
    {
        return "/public/api/Prices?Server=".$this->server."&SourceGuid=".$this->guid."&ItemKey=".$item;

    }


    private function sendBlock($url, $data)
    {
        if (array_key_exists($url, $this->cache)) {
            return;
        }
        $this->cache[$url] = $this->client->request(
            'POST',
            $url,
            ['json' => $data]
        );

    }


    public function sendHistory($id, $data, $unused)
    {
        $url = $this->buildHistoryUrl($id);
        $this->sendBlock($url, $data->History);

    }


    public function sendPrices($unused, $data, $time)
    {
        $url = $this->buildPriceUrl($data->Item->ID);
        $output = array();
        foreach ($data->Prices as $item) {
            $item = [
            "CraftSignature" => $item->CraftSignature,
            "IsCrafted" => $item->IsCrafted,
            "Materia" => [],
            'PricePerUnit' => $item->PricePerUnit,
            'PriceTotal' => $item->PriceTotal,
            'Quantity' => $item->Quantity,
            'IsHQ' => $item->IsHQ,
            "RetainerName" => $item->RetainerName,
            "Stain" => $item->Stain,
            "TownID" => $item->Town->ID,
            "DateObtained" => $time
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
            "CraftSignature" => "",
            "IsCrafted" => false,
            "Materia" => [],
            'PricePerUnit' => 0,
            'PriceTotal' => 0,
            'Quantity' => 0,
            'IsHQ' => false,
            "RetainerName" => "",
            "Stain" => 0,
            "TownID" => 0,
            "DateObtained" => $time
                        ];
        }
        $this->sendBlock($url, $output);

    }


    /* end */
}
