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

require_once __DIR__.'/db.inc';
require_once __DIR__.'/progress.inc';
require __DIR__.'/vendor/autoload.php';

use GuzzleHttp\Client;
use GuzzleHttp\Exception\ClientException;
use GuzzleHttp\Exception\ServerException;

class Xivapi
{
    private $_server;
    private $_xivapiKey;
    private $_ffxivmb;
    private $_base;
    private $_client;
    private $_promises;
    private $_poolSize;

    public $timeout;

    private function _checkCacheTable() 
    {
        $sql = "SHOW TABLES LIKE 'xivData';";
        $result = DB::run($sql);
        $value = $result->fetch();
        if ($value === false) {
            $sql = <<<SQL
                CREATE TABLE xivData (
                    Path VARCHAR(100) NOT NULL PRIMARY KEY,
                    Data TEXT NOT NULL,
                    Date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
                );
SQL;
            DB::run($sql);
        }
    }

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

        $this->_checkCacheTable();
    }

    function flushPool($progress, $cnt=-1)
    {
        $i = 0;
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
                $msg = "XIVAPI client error: ".
                      $e->getResponse()->getStatusCode();
                report_progress($progress, 'error', $msg);
            } catch (ServerException $e) {
                unset($this->_promises[$key]);
                $code = $e->getResponse()->getStatusCode();
                if ($code == 500) {
                    $this->_client = null;
                    $msg = "XIVAPI server error: ".$code.": Shutting down";
                    report_progress($progress, 'error', $msg);
                } else {
                    $msg = "XIVAPI server error: ".$code;
                    report_progress($progress, 'error', $msg);
                }
            }
        }
    }

    function buildPath($type, $ID) 
    {
        return $this->_server.".$type.$ID";
    }

    private function _fromCache($path) 
    {
        $qpath = myPDO::quote($path);
        $sql = "SELECT `Data` FROM `xivData` WHERE `Path` = $qpath AND ".
               "Date > (NOW() - INTERVAL ".$this->timeout." SECOND)";
        $result = DB::run($sql);
        $value = $result->fetch();
        if ($value !== false) {
            return json_decode($value['Data']);
        }
        return null;
    }

    private function _toCache($path, $entry) 
    {
        $name = myPDO::quote($path);
        $data = myPDO::quote(json_encode($entry));
        $sql = "INSERT INTO `xivData` (Path,Data,Date) ".
               " VALUES ($name, $data, NOW()) ".
               " ON DUPLICATE KEY UPDATE ".
               " Data = $data, Date = NOW();";
        DB::run($sql);
    }

    private function _getXIVdata($method, $ID, $url, $async, $progress,
        $report=null
    ) {
        if ($this->_client === null) {
            return null;
        }
        $path = $this->buildPath($method, $ID);
        $data = $this->_fromCache($path);
        if ($data !== null) {
            return $data;
        }

        if (!array_key_exists($path, $this->_promises)) {
            report_progress($progress, 'info', "From $url...");

            if ($this->_poolSize == 1
                || count($this->_promises) > $this->_poolSize
            ) {
                $this->flushPool($progress, $this->_poolSize/2);
            }

            try {
                if ($this->_client === null) {
                    return null;
                }
                $promise = $this->_client->requestAsync(
                    'GET',
                    "/$url?key=".$this->_xivapiKey
                );
                $promise->then(
                    function ($response) use ( $path, $ID, $report ) {
                        $contents = $response->getBody();
                        $data = json_decode($contents);
                        $this->_toCache($path, $data);
                        if ($report !== null && $this->_ffxivmb !== null) {
                            $time = microtime();
                            $this->_ffxivmb->$report($ID, $data, $time);
                        }
                    },
                    function (ClientException $e) use ( $progress ) {
                        $msg = "XIVAPI Promise Client Exception: ".
                            $e->getResponse()->getStatusCode();
                        report_progress($progress, 'error', $msg);
                        return null;
                    }
                );
            } catch (Exception $e) {
                $msg = "XIVAPI Promise Exception: ".
                    $e->getResponse()->getStatusCode();
                report_progress($progress, 'error', $msg);
                return null;
            }

            $this->_promises[$path] = $promise;
        }

        if ($async) {
            return null;
        }

        $this->flushPool($progress);

        $data = $this->_fromCache($path);
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
        report_progress($progress);
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
        report_progress($progress);
        return $market;
    }

    function mostRecent($history, $HQ=null) 
    {
        if ($history === null) {
            return null;
        }
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
        if ($history === null) {
            return array(
                    'Count' => 0,
                    'Minimum' => 0,
                    'Average' => 0,
                    'Maximum' => 0
                    );
        }
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
