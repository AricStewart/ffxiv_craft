<?php
/**
 * PHP version 5
 *
 * @category  Final_Fantasy_XIV
 * @package   FfxivDataSet
 * @author    Aric Stewart <aricstewart@google.com>
 * @copyright 2019 Aric Stewart
 * @license   Apache License, Version 2.0
 * @link      <none>
 **/

namespace App\ffxivCore;

use Amp\Parallel\Worker\Task;
use Amp\Sync\Channel;
use Amp\Cancellation;

require_once __DIR__."/../../db.inc";

class recipieTask implements Task
{
    public $idx;
    public $data;
    public $count;
    public $env;
    public $RecipeLevelTable;

    public function __construct($data, $ffxivData, $index, $count) {
        $this->data = $data;
        $this->idx = $index;
        $this->count = $count;
        $this->RecipeLevelTable = $ffxivData->RecipeLevelTable;

        $this->env = $_ENV;
    }

    private static function isTrue($val)
    {
        $boolval = ( is_string($val) ? filter_var(
            $val,
            FILTER_VALIDATE_BOOLEAN,
            FILTER_NULL_ON_FAILURE
        ) : (bool) $val );
        return ( $boolval === null ? false : $boolval );

    }

    private static function filterIngredient($item)
    {
        return ($item['Item'] > 0 && $item['Amount'] > 0);

    }

    public function run(Channel $channel, Cancellation $cancellation): string
    {
        $_ENV = $this->env;

        $entry = ['Index' => intval($this->data[0]),
        'Number' => intval($this->data[1]),
        'CraftType' => intval($this->data[2]),
        'RecipeLevelTable' => intval($this->data[3]),
        'Result' => [
        'Item' => intval($this->data[4]),
        'Amount' => intval($this->data[5])],
        'Ingredients' => [
        ['Item' => intval($this->data[6]),
        'Amount' => intval($this->data[7])],
        ['Item' => intval($this->data[8]),
        'Amount' => intval($this->data[9])],
        ['Item' => intval($this->data[10]),
        'Amount' => intval($this->data[11])],
        ['Item' => intval($this->data[12]),
        'Amount' => intval($this->data[13])],
        ['Item' => intval($this->data[14]),
        'Amount' => intval($this->data[15])],
        ['Item' => intval($this->data[16]),
        'Amount' => intval($this->data[17])],
        ['Item' => intval($this->data[18]),
        'Amount' => intval($this->data[19])],
        ['Item' => intval($this->data[20]),
        'Amount' => intval($this->data[21]),]],
        'RecipeNotebookList' => $this->data[22],
        'DisplayPriority' => $this->data[23],
        'IsSecondary' => $this->isTrue($this->data[24]),
        'MaterialQualityFactor' => $this->data[25],
        'DifficultyFactor' => $this->data[26],
        'QualityFactor' => $this->data[27],
        'DurabilityFactor' => $this->data[28],
        'RequiredQuality' => $this->data[29],
        'RequiredCraftsmanship' => $this->data[30],
        'RequiredControl' => $this->data[31],
        'QuickSynthCraftsmanship' => $this->data[32],
        'QuickSynthControl' => $this->data[33],
        'SecretRecipeBook' => $this->data[34],
        'Quest' => $this->data[35],
        'CanQuickSynth' => $this->isTrue($this->data[36]),
        'CanHq' => $this->isTrue($this->data[37]),
        'ExpRewarded' => $this->isTrue($this->data[38]),
        'Status{Required}' => $this->data[39],
        'Item{Required}' => $this->data[40],
        'IsSpecializationRequired' => $this->isTrue($this->data[41]),
        'IsExpert' => $this->isTrue($this->data[42]),
        'PatchNumber' => $this->data[45]
        ];

        $entry['Ingredients'] = array_filter(
            $entry['Ingredients'],
            'App\ffxivCore\recipieTask::filterIngredient'
        );

        $id = $entry['Index'];
        $item = $entry['Result']['Item'];
        $type = $entry['CraftType'];
        if ($entry['SecretRecipeBook'] > 0) {
            $book = $entry['SecretRecipeBook'];
        } else {
            $book = 'NULL';
        }
        $dbdata = \MyPDO::quote(json_encode($entry));
        $level = $this->RecipeLevelTable[$entry['RecipeLevelTable']]->ClassJobLevel;

        $sql = "INSERT INTO `recipes` (Id, Item, CraftType,"."RecipeLevel, Book, Data) "."VALUES ($id, $item, $type, $level, $book, $dbdata); ";

        foreach ($entry['Ingredients'] as $ingred) {
            if ($ingred['Item'] > 0) {
                $i = strval($ingred['Item']);
                $a = strval($ingred['Amount']);
                $sql .= "INSERT INTO `ingredients` (Id, Recipe, Item, Amount, Level) VALUES ($id, $item, $i, $a, $level); ";
            }
        }

        \DB::run($sql);
        return "";
    }
}
