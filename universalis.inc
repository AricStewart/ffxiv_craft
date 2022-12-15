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

    private string $server;

    private object $client;

    private array $promises;

    private int $poolSize = 5;

    public int $timeout = 3600;

    private array $localCache;


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


    public static function dropCacheTable()
    {
        $sql = "DROP TABLE IF EXISTS `universalisData`;";
        DB::run($sql);

    }


    public function __construct(string $server)
    {
        $this->server = $server;
        $this->timeout = 3600; /*  1 hour */

        $this->poolSize = 5;
        $this->promises = [];
        $this->client = new Client(['base_uri' => "https://universalis.app/api/v2/"]);

        $this->checkCacheTable();
        $this->localCache = [];

    }


    public function flushPool(?string $progress, int $cnt = -1)
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


    private function buildPath($type, $ID): string
    {
        return $this->server.".$type.$ID";

    }


    private function fromCache($path)
    {
        if (array_key_exists($path, $this->localCache)) {
            if ($this->localCache[$path] !== null) {
                return json_decode($this->localCache[$path]);
            }
            return null;
        }
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
        $this->localCache[$path] = json_encode($entry);
        $name = MyPDO::quote($path);
        $data = MyPDO::quote(base64_encode(gzcompress(json_encode($entry))));
        $sql = "INSERT INTO `universalisData` (Path,Data,Date) "." VALUES ($name, $data, NOW()) "." ON DUPLICATE KEY UPDATE "." Data = $data, Date = NOW();";
        DB::run($sql);

    }


    private function getUniversalisData(
        string $method,
        array $IDs,
        string $url,
        bool $async,
        ?string $progress
    ): ?array {
        if ($this->client === null) {
            return null;
        }

        $data = [];
        $ids = [];
        foreach ($IDs as $id) {
            $path = $this->buildPath($method, $id);
            $d = $this->fromCache($path);
            if ($d !== null) {
                $data[$id] = $d;
            } else {
                $ids[] = $id;
            }
        }
        if (empty($ids)) {
            return $data;
        }

        $basePath = $this->buildPath($method, implode(',', $ids));
        if (!array_key_exists($basePath, $this->promises)) {
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

                $url .= implode(',', $ids);
                $promise = $this->client->requestAsync('GET', $url);

                $promise->then(
                    function ($response) use ($ids) {
                        $contents = $response->getBody();
                        $data = json_decode($contents);
                        if (count($ids) < 2) {
                            $output = new stdClass();
                            $output->itemIDs = $ids;
                            $output->items = new stdClass();
                            $output->items->{$ids[0]} = $data;
                        } else {
                            $output = $data;
                        }
                        foreach ($output->itemIDs as $id) {
                            if (!property_exists($output->items, $id)) {
                                $path = $this->buildPath("market", $id);
                                $this->localCache[$path] = null;
                                $path = $this->buildPath("history", $id);
                                $this->localCache[$path] = null;
                                continue;
                            }
                            $item = $output->items->$id;
                            $path = $this->buildPath("market", $id);
                            $this->toCache($path, ['Prices' => $item->listings]);

                            /* Prune history */
                            $old = strtotime("-1 week");
                            foreach ($item->recentHistory as $key => $entry) {
                                if ($entry->timestamp < $old) {
                                    unset($item->recentHistory[$key]);
                                } else {
                                    unset($item->recentHistory[$key]->buyerName);
                                }
                            }
                            $path = $this->buildPath("history", $id);
                            $this->toCache($path, ['History' => $item->recentHistory]);
                        }
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

            $this->promises[$basePath] = $promise;
        }

        if ($async) {
            return null;
        }

        $this->flushPool($progress);

        foreach ($ids as $id) {
            $path = $this->buildPath($method, $id);
            $data[$id] = $this->fromCache($path);
        }
        return $data;

    }


    public function getHistory($ID, $async = false, $progress = null) : ?array
    {
        if (!is_array($ID)) {
            $ID = [$ID];
        }
        $data = $this->getUniversalisData(
            'history',
            $ID,
            "$this->server/",
            $async,
            $progress
        );
        if ($async) {
            return null;
        }
        report_progress($progress);
        return $data;

    }


    public function getMarket($ID, $async = false, $progress = null): ?array
    {
        if (!is_array($ID)) {
            $ID = [$ID];
        }
        $data = $this->getUniversalisData(
            'market',
            $ID,
            "$this->server/",
            $async,
            $progress
        );
        if ($async) {
            return null;
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
        if ($history->History !== null) {
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
        }
        return $winner;

    }


    private function timeCloseEnough($item1, $item2)
    {
        return (abs($item1->lastReviewTime - $item2->lastReviewTime)) < 3600;

    }


    public function currentCheapest($market, $HQ = null)
    {
        $count = 0;
        $quantity = 0;
        $winner = null;
        if ($market !== null && $market->Prices !== null) {
            foreach ($market->Prices as $entry) {
                if ($winner === null) {
                    $winner = $entry;
                    $quantity = $entry->quantity;
                } elseif ($entry->lastReviewTime > $winner->lastReviewTime) {
                    $winner = $entry;
                    $quantity = $entry->quantity;
                }
            }

            foreach ($market->Prices as $entry) {
                if ($HQ !== null && $HQ != $entry->hq) {
                    continue;
                }
                $count++;
                if ($winner === null) {
                    $winner = $entry;
                    $quantity = $entry->quantity;
                } elseif ($this->timeCloseEnough($entry, $winner)) {
                    if ($entry->pricePerUnit === $winner->pricePerUnit) {
                        $quantity += $entry->quantity;
                    } elseif ($entry->pricePerUnit < $winner->pricePerUnit) {
                        $winner = $entry;
                        $quantity = $entry->quantity;
                    }
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
        if ($history->History !== null) {
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
            $result['Cheapest'] = $this->currentCheapest($market[$itemID], $HQ);
            if ($result['Cheapest'] !== null) {
                $result['Week'] = $this->weekAverage($history[$itemID], $HQ);
                $result['Profit'] = $result['Week']['Average'] - $result['Cheapest']['Item']->pricePerUnit;
            }
        }
        return $result;

    }


    /*end*/
}
