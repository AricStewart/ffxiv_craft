<?php

/*.
    require_module 'pdo';
 .*/

require_once __DIR__."/apiData.inc";

class myPDO
{
    protected static /*.PDO.*/ $_instance = null;

    public static function instance()
    {
        if (self::$_instance === null) {
            // Setup a DB connecition via the parent PDO
            // (In this case, MySQL Specific)
            $options = [
            PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
            PDO::ATTR_EMULATE_PREPARES => 1,
            PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
            ];
            $username = $GLOBALS['DBUSER'];
            $password = $GLOBALS['DBPASS'];
            $charset = 'utf8';
            $dsn = 'mysql:';
            $dsn .= 'host='.$GLOBALS['DBHOST'];
            $dsn .= ';dbname='.$GLOBALS['DBNAME'];
            $dsn .= ';charset='.$charset;
            try {
                self::$_instance = new PDO($dsn, $username, $password, $options);
            } catch (Exception $e) {
throw $e;
            }
        }
        return self::$_instance;

    }

    public static function __callStatic($method, $args)
    {
        return call_user_func_array(array(self::instance(), $method), $args);

    }
};

class DB
{
    public static function run(/*.string.*/$sql, $params = [])
    {
        if (empty($params)) {
            return myPDO::instance()->query($sql);
        }
        $stmt = myPDO::instance()->prepare($sql);
        $stmt->execute($params);
        return $stmt;

    }
};


$db = new DB;
