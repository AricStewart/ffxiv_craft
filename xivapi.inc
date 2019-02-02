<?php
/**
 * Access to datafiles from https://xivapi.com/
 *
 * PHP version 5
 *
 * @category  Final_Fantasy_XIV
 * @package   Xivapi
 * @author    Aric Stewart <aricstewart@google.com>
 * @copyright 2019 Aric Stewart
 * @license   Apache License, Version 2.0
 * @link      <none>
 **/

/*.
    require_module 'standard';
    require_module 'curl';
    require_module 'json';
.*/

class Xivapi
{
    private $_server;
    private $_xivapiKey;
    private $_ffxivmb;

    public $verbose;
    public $timeout;

    function __construct($server, $xivapiKey, $ffxivmb=null) 
    {
        $this->_server = $server;
        $this->_xivapiKey = $xivapiKey;
        $this->_ffxivmb = $ffxivmb;
        $this->verbose = true;
        $this->timeout = 3600;
    }

    private function _getXIVdata($file, $url) 
    {
        $cached = true;
        if (!file_exists($file) || (time()-filemtime($file) > $this->timeout)) {
            if ($this->verbose) {
                print "From $url...\n";
            }
            $ch = curl_init();
            $url =  "https://xivapi.com/$url?key=$this->_xivapiKey";
            curl_setopt($ch, CURLOPT_URL,  $url);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            $content = curl_exec($ch);
            curl_close($ch);
            file_put_contents($file, $content);
            $cached = false;
        }
        $contents = file_get_contents($file);
        $data = json_decode($contents);
        if ($data) {
            $data->cached = $cached;
        }
        return $data;
    }

    function getHistory($ID) 
    {
        $history = $this->_getXIVdata(
            "data/history.$ID.json",
            "market/$this->_server/items/$ID/history"
        );
        if ($history == null) {
            print ("Failed to aquire history for $ID\n");
        } elseif (!$history->cached && $this->_ffxivmb) {
            $this->_ffxivmb->sendHistory($ID, $history);
        }
        print ".";
        return $history;
    }

    function getMarket($ID) 
    {
        $market = $this->_getXIVdata(
            "data/market.$ID.json",
            "market/$this->_server/items/$ID"
        );
        if ($market == null) {
            print ("Failed to aquire market for $ID\n");
        } elseif (!$market->cached && $this->market) {
            $time = filemtime("data/market.$ID.json");
            $this->_ffxivmb->sendPrices($market, $time);
        }
        print ".";
        return $market;
    }

    function getItem($ID) 
    {
        return $this->_getXIVdata("data/item.$ID.json", "item/$ID");
    }

    function getRecipie($ID) 
    {
        return $this->_getXIVdata("data/recipe.$ID.json", "recipe/$ID");
    }

    function mostRecent($history) 
    {
        $winner = null;
        foreach ($history->History as $entry) {
            if ($winner === null) {
                $winner = $entry;
            } elseif ($entry->PurchaseDate > $winner->PurchaseDate) {
                $winner = $entry;
            }
        }
        return $winner;
    }

    function currentCheapest($market, $HQ=null) 
    {
        $winner = null;
        foreach ($market->Prices as $entry) {
            if ($HQ !== null && $HQ != $entry->IsHQ) {
                continue;
            }
            if ($winner === null) {
                $winner = $entry;
            } elseif ($entry->PricePerUnit < $winner->PricePerUnit) {
                $winner = $entry;
            }
        }
        return $winner;
    }

    function weekAverage($history, $HQ=null) 
    {
        $old = strtotime("-1 week");
        $total = array();
        $count = 0;
        $min = 0;
        $max = 0;
        foreach ($history->History as $entry) {
            if ($HQ !== null && $HQ != $entry->IsHQ ) {
                continue;
            }
            if ($entry->PurchaseDate >= $old) {
                    $total[] = $entry->PricePerUnit * $entry->Quantity;
                    $count += $entry->Quantity;
                if ($min == 0) {
                    $min = $max = $entry->PricePerUnit;
                }
                    $min = min($min, $entry->PricePerUnit);
                    $max = max($max, $entry->PricePerUnit);
            }
        }
        if ($count > 0) {
            $total = array_filter($total);
            $average = intval(floor(array_sum($total)/$count));
        } else {
            $average = 0;
        }
        return array(
                'Minimum' => $min,
                'Average' => $average,
                'Maximum' => $max
                );
    }

    function itemProfit($itemID, $HQ, $marketRate=1.06)
    {
        $result = [
            'ID' => 0,
            'HQ' => false,
            'Cheapest' => 0,
            'Profit' => 0,
            'Week' => null
        ];
        $market = $this->getMarket($itemID);
        if ($market != null) {
            $history = $this->getHistory($itemID);
            $result['ID'] = $itemID;
            $result['HQ'] = $HQ;
            $result['Cheapest'] = $this->currentCheapest($market, $HQ);
            if ($result['Cheapest'] !== null) {
                $result['Week'] = $this->weekAverage($history, $HQ);
                $result['Profit'] = $result['Week']['Average'] -
                                    $result['Cheapest']->PricePerUnit;
            }
        }
        return $result;
    }

    function printProfit($profit)
    {
        if ($profit['Profit'] > 0) {
            $hq = '  ';
            if ($profit['HQ']) {
                $hq = '(*)';
            }
            print "\n\t$hq cheapest ".$profit['Cheapest']->PricePerUnit." gil\n";
            print "\t$hq Profit:".$profit['Profit']." gil\n";
            print "\t$hq week (".$profit['Week']['Minimum'].' <- '.
                $profit['Week']['Average'].' -> '.
                $profit['Week']['Maximum'].' gil)';

            $profit[] = $profit;
        }
    }
}
