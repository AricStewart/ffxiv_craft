<?php
require_once __DIR__."/ffxivData.inc";

function parse_file($dataset, $file, &$inventory=[])
{
    $needle = "item-list__list";
    $lastPos = 0;
    $content = file_get_contents($file);
    while (($lastPos = strpos($content, $needle, $lastPos))!== false) {
        $stack = strpos($content, 'data-stack', $lastPos);
        $sub = substr($content, $stack);
        $preg="/data-stack=\"(.*?)\"/si";
        preg_match($preg, $sub, $ui);
        $preg="/db-tooltip__item__name[^>]*>(.*?)</si";
        preg_match($preg, $sub, $name);
        $name = html_entity_decode($name[1]);
        $item = $dataset->getItem($name);
        if (array_key_exists($item->Index, $inventory)) {
            $inventory[$item->Index]['count'] += $ui[1];
        } else {
            $inventory[$item->Index] = [
                'name' => $item->Name,
                'count' => $ui[1]
            ];
        }
        $lastPos = $lastPos + strlen($needle);
    }
    return $inventory;
}

/*
$dataset = new FfxivDataSet();
$i = parse_file($dataset, './Inventory/inv1.html');
$i = parse_file($dataset, './Inventory/inv2.html', $i);
ksort($i);
print_r($i);
*/
