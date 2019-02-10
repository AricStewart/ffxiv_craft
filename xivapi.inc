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
    require_module 'json';
    require_module 'file';
.*/


require __DIR__.'/vendor/autoload.php';
use GuzzleHttp\Client;
use GuzzleHttp\Exception\ClientException;

class Xivapi
{
    private $_server;
    private $_xivapiKey;
    private $_ffxivmb;
    private $_base;
    private $_client;
    private $_promises;
    private $_poolSize;
    private $_cache;

    public $timeout;

    function __construct($server, $xivapiKey, $ffxivmb=null, $base='.') 
    {
        $this->_server = $server;
        $this->_xivapiKey = $xivapiKey;
        $this->_ffxivmb = $ffxivmb;
        $this->timeout = 3600; /*  1 hour */
        $this->_base = $base;

        $this->_poolSize = 5;
        $this->_client = new Client(['base_uri' => "https://xivapi.com/"]);
        $this->_promises = [];

        $this->_cache = [];
    }

    function flushPool($cnt=-1)
    {
        foreach ($this->_promises as $key => $promise) {
            try {
                $promise->wait();
                $i++;
                unset($this->_promises[$key]);
                if ($cnt > 0 && $i > $cnt) {
                    break;
                }
            } catch (ClientException $e) {
                unset($this->_promises[$key]);
            } catch (Exception $e) {
                unset($this->_promises[$key]);
            }
        }
    }

    function buildPath($type, $ID) 
    {
        return $this->_base."/data/".$this->_server.".$type.$ID.json";
    }

    private function _getXIVdata($method, $ID, $url, $async, $progress,
        $report=null
    ) { 
        $file = $this->buildPath($method, $ID);
        if (array_key_exists($file, $this->_cache)) {
            if ($progress != null) {
                $progress('progress');
            }
            return $this->_cache[$file];
        }
        if (!file_exists($file) || (time()-filemtime($file) > $this->timeout)) {
            if ($progress !== null) {
                $progress('info', "From $url...");
            }

            if (count($this->_promises) > $this->_poolSize) {
                $this->flushPool($this->_poolSize/2);
            }

            try {
                $promise = $this->_client->requestAsync(
                    'GET',
                    "/$url?key=".$this->_xivapiKey
                );
                $promise->then(
                    function ($response) use ( $file, $progress, $ID, $report ) {
                        $contents = $response->getBody();
                        @file_put_contents($file, $contents);
                        @chmod($file, 0666);
                        if ($progress != null) {
                            $progress('progress');
                        }
                        $data = json_decode($contents);
                        $this->_cache[$file] = $data;
                        if ($report !== null && $this->_ffxivmb !== null) {
                            $time = filemtime($file);
                            $this->_ffxivmb->$report($ID, $data, $time);
                        }
                    },
                    function (ClientException $e) {
                        return;
                    }
                );
            } catch (Exception $e) {
                return;
            }

            $this->_promises[] = $promise;

            if ($async) {
                return null;
            }
            $this->flushPool();
        } else {
            if ($progress != null) {
                $progress('progress');
            }
        }

        if ($async) {
            return null;
        }

        if (!file_exists($file)) {
            sleep(1);
            return $this->_getXIVdata(
                $method, $ID, $url, false, $progress,
                $report
            );
        }
        $contents = file_get_contents($file);
        $data = json_decode($contents);
        $this->_cache[$file] = $data;
        return $data;
    }

    function getHistory($ID, $async=false, $progress=null) 
    {
        $history = $this->_getXIVdata(
            'history', $ID,
            "market/$this->_server/items/$ID/history",
            $async, $progress, 'sendHistory'
        );
        if ($async) {
            return;
        }
        if ($progress != null) {
            $progress('progress');
        }
        return $history;
    }

    function getMarket($ID, $async=false, $progress=null) 
    {
        $market = $this->_getXIVdata(
            'market', $ID,
            "market/$this->_server/items/$ID",
            $async, $progress, 'sendPrices'
        );
        if ($async) {
            return;
        }
        if ($progress != null) {
            $progress('progress');
        }
        return $market;
    }

    function mostRecent($history, $HQ=null) 
    {
        $winner = null;
        foreach ($history->History as $entry) {
            if ($HQ !== null && $HQ != $entry->IsHQ) {
                continue;
            }
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
        $count = 0;
        $quantity = 0;
        $winner = null;
        if ($market->Prices !== null) {
            foreach ($market->Prices as $entry) {
                if ($HQ !== null && $HQ != $entry->IsHQ) {
                    continue;
                }
                $count++;
                if ($winner === null) {
                    $winner = $entry;
                    $quantity = $entry->Quantity;
                } elseif ($entry->PricePerUnit === $winner->PricePerUnit) {
                    $quantity += $entry->Quantity;
                } elseif ($entry->PricePerUnit < $winner->PricePerUnit) {
                    $winner = $entry;
                    $quantity = $entry->Quantity;
                }
            }
        }
        if ($winner !== null) {
            return array(
                    'Item' => $winner,
                    'Quantity' => $quantity,
                    'Count' => $count
                   );
        } else {
            return null;
        }
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
                'Count' => $count,
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
                                    $result['Cheapest']['Item']->PricePerUnit;
            }
        }
        return $result;
    }
}
