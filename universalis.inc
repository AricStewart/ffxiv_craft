<?php
/**
 * Access to datafiles from https://universalis.app/
 *
 * PHP version 5
 *
 * @category  Final_Fantasy_XIV
 * @package   Xivapi
 * @author    Aric Stewart <aricstewart@google.com>
 * @copyright 2021 Aric Stewart
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

class Universalis
{

    private $server;

    private $client;

    private $promises;

    private $poolSize;

    public $timeout;


    private function checkCacheTable()
    {
        $sql = "SHOW TABLES LIKE 'universalisData';";
        $result = DB::run($sql);
        $value = $result->fetch();
        if ($value === false) {
            $sql = <<<SQL
                CREATE TABLE universalisData (
                    Path VARCHAR(100) NOT NULL PRIMARY KEY,
                    Data TEXT NOT NULL,
                    Date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
                );
SQL;
            DB::run($sql);
        }

    }


    public function __construct($server)
    {
        $this->server = $server;
        $this->timeout = 3600; /*  1 hour */

        $this->poolSize = 5;
        $this->promises = [];
        $this->client = new Client(['base_uri' => "https://universalis.app/api/"]);

        $this->checkCacheTable();

    }


    public function flushPool($progress, $cnt = -1)
    {
        $i = 0;
        foreach ($this->promises as $key => $promise) {
            try {
                $promise->wait();
                $i++;
                unset($this->promises[$key]);
                if ($cnt > 0 && $i > $cnt) {
                    break;
                }
            } catch (ClientException $e) {
                unset($this->promises[$key]);
                $msg = "universalis client error: ".$e->getResponse()->getStatusCode();
                report_progress($progress, 'error', $msg);
            } catch (ServerException $e) {
                unset($this->promises[$key]);
                $code = $e->getResponse()->getStatusCode();
                if ($code == 500) {
                    $this->client = null;
                    $msg = "universalis server error: ".$code.": Shutting down";
                    report_progress($progress, 'error', $msg);
                } else {
                    $msg = "universalis server error: ".$code;
                    report_progress($progress, 'error', $msg);
                }
            }
        }

    }


    private function buildPath($type, $ID)
    {
        return $this->server.".$type.$ID";

    }


    private function fromCache($path)
    {
        $qpath = MyPDO::quote($path);
        $sql = "SELECT `Data` FROM `universalisData` WHERE `Path` = $qpath AND "."Date > (NOW() - INTERVAL ".$this->timeout." SECOND)";
        $result = DB::run($sql);
        $value = $result->fetch();
        if ($value !== false) {
            return json_decode(gzuncompress(base64_decode($value['Data'])));
        }
        return null;

    }


    private function toCache($path, $entry)
    {
        $name = MyPDO::quote($path);
        $data = MyPDO::quote(base64_encode(gzcompress(json_encode($entry))));
        $sql = "INSERT INTO `universalisData` (Path,Data,Date) "." VALUES ($name, $data, NOW()) "." ON DUPLICATE KEY UPDATE "." Data = $data, Date = NOW();";
        DB::run($sql);

    }


    private function getUniversalisData(
        $method,
        $ID,
        $url,
        $async,
        $progress
    ) {
        if ($this->client === null) {
            return null;
        }
        $path = $this->buildPath($method, $ID);
        $data = $this->fromCache($path);
        if ($data !== null) {
            return $data;
        }

        if (!array_key_exists($path, $this->promises)) {
            report_progress($progress, 'info', "From $url...");

            if ($this->poolSize == 1
                || count($this->promises) > $this->poolSize
            ) {
                $this->flushPool($progress, $this->poolSize / 2);
            }

            try {
                if ($this->client === null) {
                    return null;
                }

                $promise = $this->client->requestAsync('GET', $url);

                $promise->then(
                    function ($response) use ($path, $ID) {
                        $contents = $response->getBody();
                        $data = json_decode($contents);
                        $path = $this->buildPath("market", $ID);
                        $this->toCache($path, ['Prices' => $data->listings]);

                        /* Prune history */
                        $old = strtotime("-1 week");
                        foreach ($data->recentHistory as $key => $entry) {
                            if ($entry->timestamp < $old) {
                                unset($data->recentHistory[$key]);
                            } else {
                                unset($data->recentHistory[$key]->buyerName);
                            }
                        }
                        $path = $this->buildPath("history", $ID);
                        $this->toCache($path, ['History' => $data->recentHistory]);
                    },
                    function (Exception $e) use ($progress) {
                        $msg = "universalis Promise Client Exception: ".$e->getResponse()->getStatusCode();
                        report_progress($progress, 'error', $msg);
                        return null;
                    }
                );
            } catch (Exception $e) {
                $msg = "universalis Promise Exception: ".$e->getResponse()->getStatusCode();
                report_progress($progress, 'error', $msg);
                return null;
            }

            $this->promises[$path] = $promise;
        }

        if ($async) {
            return null;
        }

        $this->flushPool($progress);

        $data = $this->fromCache($path);
        return $data;

    }


    public function getHistory($ID, $async = false, $progress = null)
    {
        $data = $this->getUniversalisData(
            'history',
            $ID,
            "$this->server/$ID",
            $async,
            $progress
        );
        if ($async) {
            return;
        }
        report_progress($progress);
        return $data;

    }


    public function getMarket($ID, $async = false, $progress = null)
    {
        $data = $this->getUniversalisData(
            'market',
            $ID,
            "$this->server/$ID",
            $async,
            $progress
        );
        if ($async) {
            return;
        }
        report_progress($progress);
        return $data;

    }


    public function mostRecent($history, $HQ = null)
    {
        if ($history === null) {
            return null;
        }
        $winner = null;
        foreach ($history->History as $entry) {
            if ($HQ !== null && $HQ != $entry->hq) {
                continue;
            }
            if ($winner === null) {
                $winner = $entry;
            } elseif ($entry->timestamp > $winner->timestamp) {
                $winner = $entry;
            }
        }
        return $winner;

    }


    public function currentCheapest($market, $HQ = null)
    {
        $count = 0;
        $quantity = 0;
        $winner = null;
        if ($market !== null && $market->Prices !== null) {
            foreach ($market->Prices as $entry) {
                if ($HQ !== null && $HQ != $entry->hq) {
                    continue;
                }
                $count++;
                if ($winner === null) {
                    $winner = $entry;
                    $quantity = $entry->quantity;
                } elseif ($entry->pricePerUnit === $winner->pricePerUnit) {
                    $quantity += $entry->quantity;
                } elseif ($entry->pricePerUnit < $winner->pricePerUnit) {
                    $winner = $entry;
                    $quantity = $entry->quantity;
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


    public function weekAverage($history, $HQ = null)
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
            if ($HQ !== null && $HQ != $entry->hq) {
                continue;
            }
            if ($entry->timestamp >= $old) {
                $total[] = $entry->pricePerUnit * $entry->quantity;
                $count += $entry->quantity;
                if ($min == 0) {
                    $min = $max = $entry->pricePerUnit;
                }
                $min = min($min, $entry->pricePerUnit);
                $max = max($max, $entry->pricePerUnit);
            }
        }
        if ($count > 0) {
            $total = array_filter($total);
            $average = intval(floor(array_sum($total) / $count));
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


    public function itemProfit($itemID, $HQ, $marketRate = 1.06)
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
                $result['Profit'] = $result['Week']['Average'] - $result['Cheapest']['Item']->pricePerUnit;
            }
        }
        return $result;

    }


    /*end*/
}
