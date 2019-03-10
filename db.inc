<?php

/*.
    require_module 'pdo';
 .*/

require_once __DIR__."/apiData.inc";
require_once __DIR__."/pdo.inc";

class DB
{


    public static function run(/*.string.*/$sql, $params = [])
    {
        if (empty($params)) {
            return MyPDO::instance()->query($sql);
        }
        $stmt = MyPDO::instance()->prepare($sql);
        $stmt->execute($params);
        return $stmt;

    }


    /* End */
}


$db = new DB;
