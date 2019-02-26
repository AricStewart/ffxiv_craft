<?php
/**
 * Progress reporting common function
 *
 * PHP version 5
 *
 * @category  Final_Fantasy_XIV
 * @package   FfxivDataSet
 * @author    Aric Stewart <aricstewart@google.com>
 * @copyright 2019 Aric Stewart
 * @license   Apache License, Version 2.0
 * @link      <none>
 **/

/*.
    require_module 'core';
.*/

function report_progress($progress, $type="progress", $message="") 
{
    if ($progress !== null) {
        $progress($type, $message);
    }
}
