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

class itemTask implements Task
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

    private static function filterBaseparam($item)
    {
        return ($item['Param'] > 0 && $item['Value'] > 0);

    }

    public function run(Channel $channel, Cancellation $cancellation): string
    {
        $_ENV = $this->env;

        $entry = [
        'Index' => intval($this->data[0]),
        'Singular' => $this->data[1],
        'Adjective' => intval($this->data[2]),
        'Plural' => $this->data[3],
        'PossessivePronoun' => intval($this->data[4]),
        'StartsWithVowel' => intval($this->data[5]),
        /* data[6] */
        'Pronoun' => intval($this->data[7]),
        'Article' => intval($this->data[8]),
        'Description' => $this->data[9],
        'Name' => $this->data[10],
        'Icon' => intval($this->data[11]),
        'Level{Item}' => intval($this->data[12]),
        'Rarity' => intval($this->data[13]),
        'FilterGroup' => intval($this->data[14]),
        'AdditionalData' => intval($this->data[15]),
        'ItemUICategory' => intval($this->data[16]),
        'ItemSearchCategory' => intval($this->data[17]),
        'EquipSlotCategory' => intval($this->data[18]),
        'ItemSortCategory' => intval($this->data[19]),
        /* $this->data[20] */
        'StackSize' => intval($this->data[21]),
        'IsUnique' => $this->isTrue($this->data[22]),
        'IsUntradable' => $this->isTrue($this->data[23]),
        'IsIndisposable' => $this->isTrue($this->data[24]),
        'Lot' => $this->isTrue($this->data[25]),
        'PriceMid' => intval($this->data[26]),
        'PriceLow' => intval($this->data[27]),
        'CanBeHq' => $this->isTrue($this->data[28]),
        'IsDyeable' => $this->isTrue($this->data[29]),
        'IsCrestWorthy' => $this->isTrue($this->data[30]),
        'ItemAction' => intval($this->data[31]),
        'CastTime<s>' => $this->data[32],
        'Cooldown<s>' => intval($this->data[33]),
        'ClassJob{Repair}' => intval($this->data[34]),
        'Item{Repair}' => intval($this->data[35]),
        'Item{Glamour}' => intval($this->data[36]),
        'Salvage' => intval($this->data[37]),
        'IsCollectable' => $this->isTrue($this->data[38]),
        'AlwaysCollectable' => $this->isTrue($this->data[39]),
        'AetherialReduce' => intval($this->data[40]),
        'Level{Equip}' => intval($this->data[41]),
        /* $this->data[42] */
        'EquipRestriction' => intval($this->data[43]),
        'ClassJobCategory' => intval($this->data[44]),
        'GrandCompany' => intval($this->data[45]),
        'ItemSeries' => intval($this->data[46]),
        'BaseParamModifier' => intval($this->data[47]),
        'Model{Main}' => $this->data[48],
        'Model{Sub}' => $this->data[49],
        'ClassJob{Use}' => intval($this->data[50]),
        /* data[51] */
        'Damage{Phys}' => intval($this->data[52]),
        'Damage{Mag}' => intval($this->data[53]),
        'Delay<ms>' => intval($this->data[54]),
        /* data[55] */
        'BlockRate' => intval($this->data[56]),
        'Block' => intval($this->data[57]),
        'Defense{Phys}' => intval($this->data[58]),
        'Defense{Mag}' => intval($this->data[59]),
        'BaseParam' => [
        ['Param' => intval($this->data[60]),
        'Value' => intval($this->data[61])],
        ['Param' => intval($this->data[62]),
        'Value' => intval($this->data[63])],
        ['Param' => intval($this->data[64]),
        'Value' => intval($this->data[65])],
        ['Param' => intval($this->data[66]),
        'Value' => intval($this->data[67])],
        ['Param' => intval($this->data[68]),
        'Value' => intval($this->data[69])],
        ['Param' => intval($this->data[70]),
        'Value' => intval($this->data[71])]],
        'ItemSpecialBonus' => intval($this->data[72]),
        'ItemSpecialBonus{Param}' => intval($this->data[73]),
        'BaseParam{Special}' => [
        ['Param' => intval($this->data[74]),
        'Value' => intval($this->data[75])],
        ['Param' => intval($this->data[76]),
        'Value' => intval($this->data[77])],
        ['Param' => intval($this->data[78]),
        'Value' => intval($this->data[79])],
        ['Param' => intval($this->data[80]),
        'Value' => intval($this->data[81])],
        ['Param' => intval($this->data[82]),
        'Value' => intval($this->data[83])],
        ['Param' => intval($this->data[84]),
        'Value' => intval($this->data[85])]],
        'MaterializeType' => intval($this->data[86]),
        'MateriaSlotCount' => intval($this->data[87]),
        'IsAdvancedMeldingPermitted' => $this->isTrue($this->data[88]),
        'IsPvP' => $this->isTrue($this->data[89]),
        'SubStatCategory' => intval($this->data[90]),
        'IsGlamourous' => $this->isTrue($this->data[91])
        ];

        $entry['BaseParam'] = array_filter(
            $entry['BaseParam'],
            'App\ffxivCore\itemTask::filterBaseparam'
        );
        $entry['BaseParam{Special}'] = array_filter(
            $entry['BaseParam{Special}'],
            'App\ffxivCore\itemTask::filterBaseparam'
        );

        $id = $entry['Index'];
        $name = strip_tags($entry['Name']);
        $name = preg_replace('/[^A-Za-z0-9 \'\-]/', '', $name);
        $name = \MyPDO::quote($name);
        $dbdata = \MyPDO::quote(json_encode($entry));

        $sql = "INSERT INTO `items` (Id, Name, Data) "."VALUES ($id, $name, $dbdata); ";
        \DB::run($sql);

        return "";
    }

}
