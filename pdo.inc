<?php

/*.
    require_module 'pdo';
 .*/

require __DIR__.'/vendor/autoload.php';

$dotenv = Dotenv\Dotenv::create(__DIR__);
$dotenv->load();


class MyPDO
{

    protected static /*.PDO.*/ $my_instance = null;


    public static function instance()
    {
        if (self::$my_instance === null) {
            // Setup a DB connecition via the parent PDO
            // (In this case, MySQL Specific)
            $options = [
            PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
            PDO::ATTR_EMULATE_PREPARES => 1,
            PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
            ];
            $username = $_ENV['DBUSER'];
            $password = $_ENV['DBPASS'];
            $charset = 'utf8';
            $dsn = 'mysql:';
            $dsn .= 'host='.$_ENV['DBHOST'];
            $dsn .= ';dbname='.$_ENV['DBNAME'];
            $dsn .= ';charset='.$charset;
            try {
                self::$my_instance = new PDO($dsn, $username, $password, $options);
            } catch (Exception $e) {
                throw $e;
            }
        }
        return self::$my_instance;

    }


    public static function __callStatic($method, $args)
    {
        return call_user_func_array(array(self::instance(), $method), $args);

    }


    /* end */
}
